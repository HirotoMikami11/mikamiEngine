#include "Object3DRenderer.h"
#include "CameraController.h"
#include "LightManager.h"
#include "Mesh.h"
#include <algorithm>
#include <cassert>
#ifdef USEIMGUI
#include "ImGui/ImGuiManager.h"
#endif

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
	transformRingBuffer_.Initialize(dxCommon_->GetDevice(), GraphicsConfig::kMaxObject3DTransforms);

	// MaterialData 用リングバッファを初期化（起動時に一括確保）
	materialRingBuffer_.Initialize(dxCommon_->GetDevice(), GraphicsConfig::kMaxObject3DMaterials);

	// 描画リクエストリストのメモリを事前確保（フレーム中の再アロケーション削減）
	submissions_.reserve(GraphicsConfig::kMaxObject3DTransforms);
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

	// --- Opaque PSO（RootSignature を所有）---
	// BlendMode::None + 深度書き込み有効（デフォルト設定）
	PSODescriptor psoOpaque = PSODescriptor::Create3D();
	psoOpaque_ = psoFactory->CreatePSO(psoOpaque, rsBuilder);
	assert(psoOpaque_.IsValid() && "Object3DRenderer::InitializePSO: Opaque PSO creation failed");

	// --- AlphaBlend PSO（RootSignature は psoOpaque_ から共有）---
	// アルファブレンド有効 + 深度書き込み無効（奥から手前ソートと組み合わせて正しい合成順を保証）
	PSODescriptor psoAlphaBlend = PSODescriptor::Create3D()
		.SetBlendMode(BlendMode::AlphaBlend)
		.EnableDepthWrite(false);
	psoAlphaBlend_ = psoFactory->CreatePSO(psoAlphaBlend, psoOpaque_.rootSignature.Get());
	assert(psoAlphaBlend_ != nullptr && "Object3DRenderer::InitializePSO: AlphaBlend PSO creation failed");

	// --- Add PSO（RootSignature は psoOpaque_ から共有）---
	// 加算ブレンド有効 + 深度書き込み無効
	PSODescriptor psoAdd = PSODescriptor::Create3D()
		.SetBlendMode(BlendMode::Add)
		.EnableDepthWrite(false);
	psoAdd_ = psoFactory->CreatePSO(psoAdd, psoOpaque_.rootSignature.Get());
	assert(psoAdd_ != nullptr && "Object3DRenderer::InitializePSO: Add PSO creation failed");

	// --- Wireframe PSO（RootSignature は psoOpaque_ から共有）---
	// ワイヤーフレーム描画（デバッグ・確認用）
	PSODescriptor psoWireframe = PSODescriptor::Create3D()
		.SetFillMode(D3D12_FILL_MODE_WIREFRAME);
	psoWireframe_ = psoFactory->CreatePSO(psoWireframe, psoOpaque_.rootSignature.Get());
	assert(psoWireframe_ != nullptr && "Object3DRenderer::InitializePSO: Wireframe PSO creation failed");
}

void Object3DRenderer::Finalize() {
	// GPU リソースを dxCommon_->Finalize() より前に明示的に解放する。
	// シングルトンのデストラクタ（プログラム終了時）まで解放を遅らせると、
	// デバイス破棄後にリソースが残りリソースリークとして検出されるため。
	transformRingBuffer_.Finalize();
	materialRingBuffer_.Finalize();
	submissions_.clear();
	submissions_.shrink_to_fit();
	psoOpaque_.rootSignature.Reset();
	psoOpaque_.pipelineState.Reset();
	psoAlphaBlend_.Reset();
	psoAdd_.Reset();
	psoWireframe_.Reset();
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

void Object3DRenderer::FlushOffscreen() {
	Flush(false); // RenderGroup != UI
}

void Object3DRenderer::FlushUI() {
	Flush(true); // RenderGroup::UI のみ
}

#ifdef USEIMGUI
void Object3DRenderer::ImGui() {
	if (ImGui::CollapsingHeader("Object3DRenderer")) {
		uint32_t usedT = transformRingBuffer_.GetUsedCount();
		uint32_t capT = transformRingBuffer_.GetCapacity();
		ImGui::Text("Transform : %u / %u", usedT, capT);
		ImGui::ProgressBar(static_cast<float>(usedT) / static_cast<float>(capT),
			ImVec2(-1.0f, 0.0f));

		uint32_t usedM = materialRingBuffer_.GetUsedCount();
		uint32_t capM = materialRingBuffer_.GetCapacity();
		ImGui::Text("Material  : %u / %u", usedM, capM);
		ImGui::ProgressBar(static_cast<float>(usedM) / static_cast<float>(capM),
			ImVec2(-1.0f, 0.0f));

		size_t opaqueCount = 0, alphaCount = 0, addCount = 0, wireCount = 0;
		for (const auto& s : submissions_) {
			if (s.psoVariant == PSOVariant::Wireframe) { wireCount++; continue; }
			switch (s.group) {
			case RenderGroup::Opaque:     opaqueCount++; break;
			case RenderGroup::AlphaBlend: alphaCount++;  break;
			case RenderGroup::Add:        addCount++;    break;
			default: break;
			}
		}
		ImGui::Text("Opaque    : %zu", opaqueCount);
		ImGui::Text("AlphaBlend: %zu", alphaCount);
		ImGui::Text("Add       : %zu", addCount);
		ImGui::Text("Wireframe : %zu", wireCount);
	}
}
#endif

void Object3DRenderer::Flush(bool uiOnly) {
	// 対象グループに描画すべきものがあるか確認
	bool hasAny = false;
	for (const auto& sub : submissions_) {
		if ((sub.group == RenderGroup::UI) == uiOnly) {
			hasAny = true;
			break;
		}
	}
	if (!hasAny) return;

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

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// 自前の RootSignature を設定（DirectXCommon の PSO は使わない）
	commandList->SetGraphicsRootSignature(psoOpaque_.rootSignature.Get());
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

	// --- 各描画リクエストを処理 ---
	// PSO が変わるときのみ SetPipelineState を発行する（不要な GPU 状態切り替えを抑制）
	ID3D12PipelineState* currentPSO = nullptr;

	for (const ModelSubmission& sub : submissions_) {
		if ((sub.group == RenderGroup::UI) != uiOnly) continue;

		// PSOVariant と RenderGroup から使用する PSO を決定する
		ID3D12PipelineState* targetPSO;
		if (sub.psoVariant == PSOVariant::Wireframe) {
			targetPSO = psoWireframe_.Get();
		} else {
			switch (sub.group) {
			case RenderGroup::AlphaBlend: targetPSO = psoAlphaBlend_.Get(); break;
			case RenderGroup::Add:        targetPSO = psoAdd_.Get();        break;
			default:                      targetPSO = psoOpaque_.pipelineState.Get(); break;
			}
		}

		// PSO が前回と異なる場合のみ切り替える
		if (targetPSO != currentPSO) {
			commandList->SetPipelineState(targetPSO);
			currentPSO = targetPSO;
		}

		// Material CBV バインド（ルートパラメータ [0]: b0 PS）
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
