#include "Object3DRenderer.h"
#include "CameraController.h"
#include "LightManager.h"
#include "Mesh.h"
#include <algorithm>
#include <cassert>

Object3DRenderer* Object3DRenderer::GetInstance() {
	static Object3DRenderer instance;
	return &instance;
}

void Object3DRenderer::Initialize(DirectXCommon* dxCommon) {
	assert(dxCommon != nullptr && "Object3DRenderer::Initialize: dxCommon is null");
	dxCommon_ = dxCommon;

	// 自前の PSO と RootSignature を PSOFactory で生成する
	InitializePSO();

	// TransformationMatrix 用リングバッファを初期化（起動時に一括確保）
	transformRingBuffer_.Initialize(dxCommon_->GetDevice(), kMaxTransforms);

	// MaterialData 用リングバッファを初期化（起動時に一括確保）
	materialRingBuffer_.Initialize(dxCommon_->GetDevice(), kMaxMaterials);

	// 描画リクエストリストのメモリを事前確保（フレーム中の再アロケーション削減）
	submissions_.reserve(kMaxTransforms);
}

void Object3DRenderer::InitializePSO() {
	PSOFactory* psoFactory = dxCommon_->GetPSOFactory();
	assert(psoFactory != nullptr && "Object3DRenderer::InitializePSO: PSOFactory is null");

	// --- RootSignature 構築 ---
	// Object3d シェーダーのルートパラメータ対応（Object3d.VS.hlsl / Object3d.PS.hlsl）:
	//  [0] b0 PIXEL_SHADER  → MaterialData         (Material CBV)
	//  [1] b0 VERTEX_SHADER → TransformationMatrix  (Transform CBV)
	//  [2] t0 PIXEL_SHADER  → Texture2D             (SRV DescriptorTable)
	//  [3] b1 PIXEL_SHADER  → LightingData          (Lighting CBV)
	//  [4] b2 PIXEL_SHADER  → CameraForGPU          (Camera CBV)
	//  s0 PIXEL_SHADER      → SamplerState          (StaticSampler)
	RootSignatureBuilder rsBuilder;
	rsBuilder
		.AddCBV(0, D3D12_SHADER_VISIBILITY_PIXEL)       // [0] Material
		.AddCBV(0, D3D12_SHADER_VISIBILITY_VERTEX)      // [1] Transform
		.AddSRV(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)    // [2] Texture
		.AddCBV(1, D3D12_SHADER_VISIBILITY_PIXEL)       // [3] Lighting
		.AddCBV(2, D3D12_SHADER_VISIBILITY_PIXEL)       // [4] Camera
		.AddStaticSampler(0);                           // s0 Sampler

	// --- PSO 設定 ---
	// PSODescriptor::Create3D() で 3D 用デフォルト設定（シェーダーパス・頂点レイアウト・深度設定等）を取得
	PSODescriptor psoDesc = PSODescriptor::Create3D();

	// PSOFactory に RootSignature と PSO を一括生成させる
	pso_ = psoFactory->CreatePSO(psoDesc, rsBuilder);
	assert(pso_.IsValid() && "Object3DRenderer::InitializePSO: PSO creation failed");
}

void Object3DRenderer::Finalize() {
	// GPU リソースを dxCommon_->Finalize() より前に明示的に解放する。
	// シングルトンのデストラクタ（プログラム終了時）まで解放を遅らせると、
	// デバイス破棄後にリソースが残りリソースリークとして検出されるため。
	transformRingBuffer_.Finalize();
	materialRingBuffer_.Finalize();
	submissions_.clear();
	submissions_.shrink_to_fit();
	pso_.rootSignature.Reset();
	pso_.pipelineState.Reset();
	dxCommon_ = nullptr;
}

void Object3DRenderer::BeginFrame() {
	// リングバッファをリセット（O(1): インデックスを0に戻すだけ）
	transformRingBuffer_.BeginFrame();
	materialRingBuffer_.BeginFrame();
	// 前フレームの描画リクエストをクリア（メモリは再利用）
	submissions_.clear();
}

void Object3DRenderer::Submit(const ModelSubmission& submission) {
	submissions_.push_back(submission);
}

void Object3DRenderer::Draw3D() {
	// 描画リクエストがない場合は PSO の切り替えも発生させない
	if (submissions_.empty()) {
		return;
	}

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// --- 共通ステート設定 ---
	// 自前の RootSignature と PSO を設定（DirectXCommon の PSO は使わない）
	commandList->SetGraphicsRootSignature(pso_.rootSignature.Get());
	commandList->SetPipelineState(pso_.pipelineState.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Lighting CBV をバインド（ルートパラメータ [3]: b1 PS）
	// LightManager が管理する統合ライティングデータを全オブジェクト共通で使用する
	LightManager* lightManager = LightManager::GetInstance();
	commandList->SetGraphicsRootConstantBufferView(
		3, lightManager->GetLightingResource()->GetGPUVirtualAddress());

	// Camera CBV をバインド（ルートパラメータ [4]: b2 PS）
	// 鏡面反射計算に使うカメラのワールド座標を渡す
	ID3D12Resource* cameraResource = CameraController::GetInstance()->GetCameraForGPUResource();
	if (cameraResource) {
		commandList->SetGraphicsRootConstantBufferView(
			4, cameraResource->GetGPUVirtualAddress());
	}

	// --- 描画リクエストをソート ---
	// Opaque:           sortDepth 昇順（前から後ろ → 早期深度カリングで不要ピクセルを削減）
	// AlphaBlend / Add: sortDepth 降順（奥から手前 → 正しいアルファブレンド合成順）
	std::stable_sort(submissions_.begin(), submissions_.end(),
		[](const ModelSubmission& a, const ModelSubmission& b) {
			// まず RenderGroup 順にソート（Opaque < AlphaBlend < Add）
			if (a.group != b.group) {
				return static_cast<int>(a.group) < static_cast<int>(b.group);
			}
			// 同一グループ内は深度値でソート
			if (a.group == RenderGroup::Opaque) {
				return a.sortDepth < b.sortDepth;  // Opaque: 前から後ろ
			}
			return a.sortDepth > b.sortDepth;      // AlphaBlend / Add: 奥から手前
		});

	// --- 各描画リクエストを処理 ---
	for (const ModelSubmission& sub : submissions_) {
		// Material CBV バインド（ルートパラメータ [0]: b0 PS）
		// transformGpuAddr / materialGpuAddr は Object3D::Draw() で AllocateTransform/Material から取得済み
		commandList->SetGraphicsRootConstantBufferView(0, sub.materialGpuAddr);

		// Transform CBV バインド（ルートパラメータ [1]: b0 VS）
		commandList->SetGraphicsRootConstantBufferView(1, sub.transformGpuAddr);

		// Texture SRV バインド（ルートパラメータ [2]: t0 PS）
		commandList->SetGraphicsRootDescriptorTable(2, sub.textureHandle);

		// VB/IB をコマンドリストにバインドしてから描画命令を発行
		sub.mesh->Bind(commandList);
		sub.mesh->Draw(commandList);
	}
}
