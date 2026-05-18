#include "SkyboxRenderer.h"
#include "CameraController.h"
#include "PSODescriptor.h"
#include "MyFunction.h"
#include "MyMath.h"
#include "Logger.h"
#include <cassert>
#include <cstring>

SkyboxRenderer* SkyboxRenderer::GetInstance() {
	static SkyboxRenderer instance;
	return &instance;
}

void SkyboxRenderer::Initialize(DirectXCommon* dxCommon) {
	assert(dxCommon != nullptr);
	dxCommon_ = dxCommon;

	InitializePSO();
	CreateBoxMesh();
	CreateConstantBuffers();
}

void SkyboxRenderer::InitializePSO() {
	PSOFactory* psoFactory = dxCommon_->GetPSOFactory();
	assert(psoFactory != nullptr);

	// RootSignature:
	//	[0] b0 VS → SkyboxTransformData (WVP)
	//	[1] b0 PS → SkyboxMaterialData (color)
	//	[2] t0 PS → TextureCube SRV
	//		s0 PS → StaticSampler
	RootSignatureBuilder rsBuilder;
	rsBuilder
		.AddCBV(0, D3D12_SHADER_VISIBILITY_VERTEX)		// [0] WVP
		.AddCBV(0, D3D12_SHADER_VISIBILITY_PIXEL)		// [1] Material
		.AddSRV(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)	// [2] TextureCube
		.AddStaticSampler(0);							// s0 Sampler

	// PSO設定
	PSODescriptor desc;
	desc.SetVertexShader(L"resources/Shader/Skybox/Skybox.VS.hlsl")
		.SetPixelShader(L"resources/Shader/Skybox/Skybox.PS.hlsl")
		.SetBlendMode(BlendMode::None)
		.SetCullMode(CullMode::Back)										// 外向きCCW定義のため背面カリング
		.EnableDepth(true, D3D12_COMPARISON_FUNC_LESS_EQUAL)				// 深度テスト有効
		.EnableDepthWrite(false)											// z=1に描くため深度書き込み不要
		.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
		.AddInputElement({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT });// 位置のみ

	pso_ = psoFactory->CreatePSO(desc, rsBuilder);
	assert(pso_.IsValid() && "SkyboxRenderer: PSO creation failed");
}

void SkyboxRenderer::CreateBoxMesh() {
	// ±1の8頂点（Skyboxは常に原点中心）
	// w = 1.0f 必須（アフィン変換の斉次座標）
	SkyboxVertex vertices[8] = {
		{ { -1.0f, -1.0f, -1.0f, 1.0f } }, // 0
		{ {  1.0f, -1.0f, -1.0f, 1.0f } }, // 1
		{ { -1.0f,  1.0f, -1.0f, 1.0f } }, // 2
		{ {  1.0f,  1.0f, -1.0f, 1.0f } }, // 3
		{ { -1.0f, -1.0f,  1.0f, 1.0f } }, // 4
		{ {  1.0f, -1.0f,  1.0f, 1.0f } }, // 5
		{ { -1.0f,  1.0f,  1.0f, 1.0f } }, // 6
		{ {  1.0f,  1.0f,  1.0f, 1.0f } }, // 7
	};

	uint32_t indices[36] = {
		// +Z面（外向きCCW）
		5, 4, 6,  5, 6, 7,
		// -Z面（外向きCCW）
		0, 1, 3,  0, 3, 2,
		// +X面（外向きCCW）
		1, 5, 7,  1, 7, 3,
		// -X面（外向きCCW）
		4, 0, 2,  4, 2, 6,
		// +Y面（外向きCCW）
		2, 3, 7,  2, 7, 6,
		// -Y面（外向きCCW）
		4, 5, 1,  4, 1, 0,
	};

	// 頂点バッファ作成
	vertexBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), sizeof(vertices));
	SkyboxVertex* vertexData = nullptr;
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices, sizeof(vertices));
	vertexBuffer_->Unmap(0, nullptr);

	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(vertices);
	vertexBufferView_.StrideInBytes = sizeof(SkyboxVertex);

	// インデックスバッファ作成
	indexBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), sizeof(indices));
	uint32_t* indexData = nullptr;
	indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	std::memcpy(indexData, indices, sizeof(indices));
	indexBuffer_->Unmap(0, nullptr);

	indexBufferView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(indices);
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
}

void SkyboxRenderer::CreateConstantBuffers() {
	auto device = dxCommon_->GetDeviceComPtr();

	transformBuffer_ = CreateBufferResource(device, sizeof(SkyboxTransformData));
	transformBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&transformData_));
	transformData_->WVP = MakeIdentity4x4();

	materialBuffer_ = CreateBufferResource(device, sizeof(SkyboxMaterialData));
	materialBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
}

void SkyboxRenderer::Draw(D3D12_GPU_DESCRIPTOR_HANDLE cubemapHandle, const Vector4& color) {
	if (cubemapHandle.ptr == 0) {
		return;
	}

	CameraController* cameraController = CameraController::GetInstance();
	if (!cameraController) {
		return;
	}
	transformData_->WVP = cameraController->GetSkyboxViewProjectionMatrix();
	materialData_->color = color;

	ID3D12GraphicsCommandList* cmdList = dxCommon_->GetCommandList();

	cmdList->SetGraphicsRootSignature(pso_.rootSignature.Get());
	cmdList->SetPipelineState(pso_.pipelineState.Get());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->SetGraphicsRootConstantBufferView(0, transformBuffer_->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(1, materialBuffer_->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootDescriptorTable(2, cubemapHandle);
	cmdList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	cmdList->IASetIndexBuffer(&indexBufferView_);
	cmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

void SkyboxRenderer::Finalize() {
	if (transformBuffer_ && transformData_) {
		transformBuffer_->Unmap(0, nullptr);
		transformData_ = nullptr;
	}
	if (materialBuffer_ && materialData_) {
		materialBuffer_->Unmap(0, nullptr);
		materialData_ = nullptr;
	}

	transformBuffer_.Reset();
	materialBuffer_.Reset();
	vertexBuffer_.Reset();
	indexBuffer_.Reset();
	pso_.rootSignature.Reset();
	pso_.pipelineState.Reset();
	dxCommon_ = nullptr;
}
