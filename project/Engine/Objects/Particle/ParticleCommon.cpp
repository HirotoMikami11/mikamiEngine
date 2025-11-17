#include "ParticleCommon.h"

ParticleCommon* ParticleCommon::GetInstance()
{
	static ParticleCommon instance;
	return &instance;
}

void ParticleCommon::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
}

void ParticleCommon::setCommonRenderSettings(ID3D12GraphicsCommandList* commandList)
{
	// パーティクル用のPSOを設定
	commandList->SetGraphicsRootSignature(dxCommon_->GetParticleRootSignature());
	commandList->SetPipelineState(dxCommon_->GetParticlePipelineState());
	// プリミティブトポロジを設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


