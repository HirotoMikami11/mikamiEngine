#include "LineRenderer.h"
#include "ImGui/ImGuiManager.h"
#include "Logger.h"
#include <algorithm>
#include <cassert>

void LineRenderer::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// 最大線分数分の頂点バッファを作成（線分1本につき2頂点）
	const size_t totalVertexCount = kMaxLineCount * kVertexCountPerLine;
	const size_t vertexBufferSize = sizeof(LineVertex) * totalVertexCount;

	// 頂点バッファ作成（永続 map）
	vertexBuffer_ = CreateBufferResource(dxCommon_->GetDevice(), vertexBufferSize);
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	// 頂点バッファビューを設定
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(vertexBufferSize);
	vertexBufferView_.StrideInBytes = sizeof(LineVertex);

	// トランスフォームバッファ作成（永続 map）
	transformBuffer_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformationMatrix));
	transformBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&transformData_));

	// 線分データの初期化
	lineData_.reserve(kMaxLineCount);

	// PSO を自前生成
	InitializePSO();

	isInitialized_ = true;
	Logger::Log(Logger::GetStream(), "LineRenderer: Initialized with max lines !!\n");
}

void LineRenderer::InitializePSO() {
	PSOFactory* psoFactory = dxCommon_->GetPSOFactory();
	assert(psoFactory != nullptr && "LineRenderer::InitializePSO: PSOFactory is null");

	// --- RootSignature 構築 ---
	// Line シェーダーのルートパラメータ対応（Line.VS.hlsl / Line.PS.hlsl）:
	//  [0] b0 VERTEX_SHADER → TransformationMatrix (Transform CBV)
	RootSignatureBuilder rsBuilder;
	rsBuilder
		.AddCBV(0, D3D12_SHADER_VISIBILITY_VERTEX)  // [0] Transform
		.AddStaticSampler(0);                       // s0 Sampler（テクスチャなしだが必要）

	// --- PSO 設定 ---
	PSODescriptor psoDesc = PSODescriptor::CreateLine();

	// PSOFactory に RootSignature と PSO を一括生成させる
	pso_ = psoFactory->CreatePSO(psoDesc, rsBuilder);
	assert(pso_.IsValid() && "LineRenderer::InitializePSO: PSO creation failed");
}

void LineRenderer::AddLine(const Vector3& start, const Vector3& end, const Vector4& color) {
	if (!isInitialized_) {
		Logger::Log(Logger::GetStream(), "LineRenderer: Not initialized!\n");
		return;
	}

	if (IsFull()) {
		Logger::Log(Logger::GetStream(), "LineRenderer: Maximum line reached!\n");
		return;
	}

	LineData lineData;
	lineData.start = start;
	lineData.end = end;
	lineData.color = color;
	lineData_.push_back(lineData);

	needsVertexUpdate_ = true;
}

void LineRenderer::Reset() {
	lineData_.clear();
	needsVertexUpdate_ = true;
}

void LineRenderer::Draw(const Matrix4x4& viewProjectionMatrix) {
	if (!isInitialized_ || !dxCommon_ || !isVisible_ || IsEmpty()) {
		return;
	}

	// 頂点バッファ更新
	if (needsVertexUpdate_) {
		UpdateVertexBuffer();
		needsVertexUpdate_ = false;
	}

	// トランスフォーム更新
	if (transformData_) {
		transformData_->WVP = viewProjectionMatrix;
		transformData_->World = MakeIdentity4x4();
	}

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// 自前の RootSignature と PSO を設定（DirectXCommon の PSO は使わない）
	commandList->SetGraphicsRootSignature(pso_.rootSignature.Get());
	commandList->SetPipelineState(pso_.pipelineState.Get());

	// プリミティブトポロジを線分に設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	// 頂点バッファをバインド
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

	// トランスフォーム設定（RootParameter[0]: b0 VS）
	commandList->SetGraphicsRootConstantBufferView(0, transformBuffer_->GetGPUVirtualAddress());

	// 一括描画（線分数 * 2頂点）
	const uint32_t vertexCount = GetLineCount() * kVertexCountPerLine;
	commandList->DrawInstanced(vertexCount, 1, 0, 0);
}

void LineRenderer::UpdateVertexBuffer() {
	if (!vertexData_ || lineData_.empty()) {
		return;
	}

	for (size_t i = 0; i < lineData_.size(); ++i) {
		const LineData& line = lineData_[i];

		size_t vertexIndex = i * kVertexCountPerLine;

		// 開始点の頂点
		vertexData_[vertexIndex].position = { line.start.x, line.start.y, line.start.z, 1.0f };
		vertexData_[vertexIndex].color = line.color;
		// 未使用
		vertexData_[vertexIndex].texcoord = { 0.0f, 0.0f };
		vertexData_[vertexIndex].normal = { 0.0f, 1.0f, 0.0f };

		// 終了点の頂点
		vertexData_[vertexIndex + 1].position = { line.end.x, line.end.y, line.end.z, 1.0f };
		vertexData_[vertexIndex + 1].color = line.color;
		// 未使用
		vertexData_[vertexIndex + 1].texcoord = { 1.0f, 1.0f };
		vertexData_[vertexIndex + 1].normal = { 0.0f, 1.0f, 0.0f };
	}
}

void LineRenderer::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNodeEx("Line Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("Line Count: %d / %d", GetLineCount(), kMaxLineCount);

		float usage = static_cast<float>(GetLineCount()) / static_cast<float>(kMaxLineCount);
		ImGui::ProgressBar(usage, ImVec2(0.0f, 0.0f),
			std::format("{:.1f}%", usage * 100.0f).c_str());

		if (IsEmpty()) {
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Status: Empty");
		} else if (IsFull()) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Status: Full");
		} else {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Normal");
		}

		if (ImGui::Button("Clear All Lines")) {
			Reset();
		}

		ImGui::TreePop();
	}
#endif
}