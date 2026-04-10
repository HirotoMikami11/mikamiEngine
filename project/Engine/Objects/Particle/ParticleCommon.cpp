#include "ParticleCommon.h"
#include <cassert>

ParticleCommon* ParticleCommon::GetInstance()
{
	static ParticleCommon instance;
	return &instance;
}

void ParticleCommon::Initialize(DirectXCommon* dxCommon)
{
	assert(dxCommon != nullptr && "ParticleCommon::Initialize: dxCommon is null");
	dxCommon_ = dxCommon;

	// PSO を自前生成（DirectXCommon の PSO には依存しない）
	InitializePSO();
}

void ParticleCommon::InitializePSO()
{
	PSOFactory* psoFactory = dxCommon_->GetPSOFactory();
	assert(psoFactory != nullptr && "ParticleCommon::InitializePSO: PSOFactory is null");

	// --- RootSignature 構築 ---
	// Particle シェーダーのルートパラメータ対応（Particle.VS.hlsl / Particle.PS.hlsl）:
	//  [0] b0 PIXEL_SHADER  → MaterialData             (Material CBV)
	//  [1] t0 VERTEX_SHADER → StructuredBuffer<ParticleForGPU> (SRV DescriptorTable)
	//  [2] t0 PIXEL_SHADER  → Texture2D                (SRV DescriptorTable)
	//  s0 PIXEL_SHADER      → SamplerState             (StaticSampler)
	RootSignatureBuilder rsBuilder;
	rsBuilder
		.AddCBV(0, D3D12_SHADER_VISIBILITY_PIXEL)       // [0] Material
		.AddSRV(0, 1, D3D12_SHADER_VISIBILITY_VERTEX)   // [1] ParticleForGPU StructuredBuffer
		.AddSRV(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)    // [2] Texture
		.AddStaticSampler(0);                           // s0 Sampler

	// --- PSO 設定 ---
	PSODescriptor psoDesc = PSODescriptor::CreateParticle();

	// PSOFactory に RootSignature と PSO を一括生成させる
	pso_ = psoFactory->CreatePSO(psoDesc, rsBuilder);
	assert(pso_.IsValid() && "ParticleCommon::InitializePSO: PSO creation failed");
}

void ParticleCommon::Finalize()
{
	pso_.rootSignature.Reset();
	pso_.pipelineState.Reset();
	dxCommon_ = nullptr;
}

void ParticleCommon::setCommonRenderSettings()
{
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// 自前の RootSignature と PSO を設定（DirectXCommon の PSO は使わない）
	commandList->SetGraphicsRootSignature(pso_.rootSignature.Get());
	commandList->SetPipelineState(pso_.pipelineState.Get());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
