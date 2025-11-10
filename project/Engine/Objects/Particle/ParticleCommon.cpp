#include "ParticleCommon.h"

ParticleCommon* ParticleCommon::GetInstance()
{
	static ParticleCommon instance;
	return &instance;
}

void ParticleCommon::Initialize(DirectXCommon* dxCommon)
{
	directXCommon_ = dxCommon;
}

void ParticleCommon::setCommonRenderSettings(ID3D12GraphicsCommandList* commandList)
{
	// パーティクル用のPSOを設定
	commandList->SetGraphicsRootSignature(directXCommon_->GetParticleRootSignature());
	commandList->SetPipelineState(directXCommon_->GetParticlePipelineState());
	// プリミティブトポロジを設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


