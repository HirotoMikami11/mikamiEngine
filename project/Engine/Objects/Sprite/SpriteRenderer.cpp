#include "SpriteRenderer.h"
#include <algorithm>
#include <cassert>
#include "ImGui/ImGuiManager.h"

SpriteRenderer* SpriteRenderer::GetInstance() {
	static SpriteRenderer instance;
	return &instance;
}

void SpriteRenderer::Initialize(DirectXCommon* dxCommon) {
	assert(dxCommon != nullptr && "SpriteRenderer::Initialize: dxCommon is null");
	dxCommon_ = dxCommon;

	// 自前の PSO と RootSignature を PSOFactory で生成する
	InitializePSO();

	// TransformationMatrix 用リングバッファを初期化（起動時に一括確保）
	transformRingBuffer_.Initialize(dxCommon_->GetDevice(), kMaxSprites);

	// SpriteMaterialData 用リングバッファを初期化（起動時に一括確保）
	materialRingBuffer_.Initialize(dxCommon_->GetDevice(), kMaxSprites);

	// 描画リクエストリストのメモリを事前確保（フレーム中の再アロケーション削減）
	submissions_.reserve(kMaxSprites);
}

void SpriteRenderer::InitializePSO() {
	PSOFactory* psoFactory = dxCommon_->GetPSOFactory();
	assert(psoFactory != nullptr && "SpriteRenderer::InitializePSO: PSOFactory is null");

	// --- RootSignature 構築 ---
	// Sprite シェーダーのルートパラメータ対応（Sprite.VS.hlsl / Sprite.PS.hlsl）:
	//  [0] b0 PIXEL_SHADER		→ SpriteMaterialData	(Material CBV)
	//  [1] b0 VERTEX_SHADER	→ TransformationMatrix	(Transform CBV)
	//  [2] t0 PIXEL_SHADER		→ Texture2D				(SRV DescriptorTable)
	//  s0 PIXEL_SHADER			→ SamplerState			(StaticSampler)
	RootSignatureBuilder rsBuilder;
	rsBuilder
		.AddCBV(0, D3D12_SHADER_VISIBILITY_PIXEL)		// [0] Material
		.AddCBV(0, D3D12_SHADER_VISIBILITY_VERTEX)		// [1] Transform
		.AddSRV(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)	// [2] Texture
		.AddStaticSampler(0);							// s0 Sampler

	// --- PSO 設定 ---
	PSODescriptor psoDesc = PSODescriptor::CreateSprite();

	// PSOFactory に RootSignature と PSO を一括生成させる
	pso_ = psoFactory->CreatePSO(psoDesc, rsBuilder);
	assert(pso_.IsValid() && "SpriteRenderer::InitializePSO: PSO creation failed");
}

void SpriteRenderer::Finalize() {
	transformRingBuffer_.Finalize();
	materialRingBuffer_.Finalize();
	submissions_.clear();
	submissions_.shrink_to_fit();
	pso_.rootSignature.Reset();
	pso_.pipelineState.Reset();
	dxCommon_ = nullptr;
}

void SpriteRenderer::BeginFrame() {
	// リングバッファをリセット（O(1): インデックスを0に戻すだけ）
	transformRingBuffer_.BeginFrame();
	materialRingBuffer_.BeginFrame();
	// 前フレームの描画リクエストをクリア（メモリは再利用）
	submissions_.clear();
}

void SpriteRenderer::Submit(const SpriteSubmission& submission) {
	submissions_.push_back(submission);
}

void SpriteRenderer::FlushOffscreen() {
	Flush(false);// RenderGroup != UI
}

void SpriteRenderer::FlushUI() {
	Flush(true);// RenderGroup::UI のみ
}

#ifdef USEIMGUI
void SpriteRenderer::ImGui() {
	if (ImGui::CollapsingHeader("SpriteRenderer")) {
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

		size_t uiCount = std::count_if(submissions_.begin(), submissions_.end(),
			[](const SpriteSubmission& s) { return s.group == RenderGroup::UI; });
		ImGui::Text("UI Sprites    : %zu", uiCount);
		ImGui::Text("World Sprites : %zu", submissions_.size() - uiCount);
	}
}
#endif

void SpriteRenderer::Flush(bool uiOnly) {
	// 対象グループに描画すべきものがあるか確認
	bool hasAny = false;
	for (const auto& sub : submissions_) {
		if ((sub.group == RenderGroup::UI) == uiOnly) {
			hasAny = true;
			break;
		}
	}
	if (!hasAny) return;

	// (group, layerOrder) でソート（FlushOffscreen → FlushUI の順に呼ばれるため、
	// 初回ソート後は次の Flush でも正しい順序が維持される）
	std::stable_sort(submissions_.begin(), submissions_.end(),
		[](const SpriteSubmission& a, const SpriteSubmission& b) {
			if (a.group != b.group) {
				return static_cast<int>(a.group) < static_cast<int>(b.group);
			}
			return a.layerOrder < b.layerOrder;
		});

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// 自前の RootSignature と PSO を設定（DirectXCommon の PSO は使わない）
	commandList->SetGraphicsRootSignature(pso_.rootSignature.Get());
	commandList->SetPipelineState(pso_.pipelineState.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 対象グループの描画リクエストを処理
	for (const auto& sub : submissions_) {
		if ((sub.group == RenderGroup::UI) != uiOnly) continue;

		// Material CBV バインド（ルートパラメータ [0]: b0 PS）
		commandList->SetGraphicsRootConstantBufferView(0, sub.materialGpuAddr);

		// Transform CBV バインド（ルートパラメータ [1]: b0 VS）
		commandList->SetGraphicsRootConstantBufferView(1, sub.transformGpuAddr);

		// Texture SRV バインド（ルートパラメータ [2]: t0 PS）
		commandList->SetGraphicsRootDescriptorTable(2, sub.textureHandle);

		// VB/IB バインド → 描画
		commandList->IASetVertexBuffers(0, 1, sub.vbv);
		commandList->IASetIndexBuffer(sub.ibv);
		commandList->DrawIndexedInstanced(sub.indexCount, 1, 0, 0, 0);
	}
}
