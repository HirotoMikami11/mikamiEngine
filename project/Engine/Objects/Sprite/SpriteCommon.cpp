#include "SpriteCommon.h"

SpriteCommon* SpriteCommon::GetInstance()
{
	static SpriteCommon instance;
	return &instance;
}

void SpriteCommon::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
}

void SpriteCommon::setCommonRenderSettings()
{
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	// スプライト専用のPSOを設定
	commandList->SetGraphicsRootSignature(dxCommon_->GetSpriteRootSignature());
	commandList->SetPipelineState(dxCommon_->GetSpritePipelineState());
	// プリミティブトポロジを設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


