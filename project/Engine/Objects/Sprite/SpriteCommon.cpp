#include "SpriteCommon.h"

SpriteCommon* SpriteCommon::GetInstance()
{
	static SpriteCommon instance;
	return &instance;
}

void SpriteCommon::Initialize(DirectXCommon* dxCommon)
{
	directXCommon_ = dxCommon;
}

void SpriteCommon::setCommonSpriteRenderSettings(ID3D12GraphicsCommandList* commandList)
{
	// スプライト専用のPSOを設定
	commandList->SetGraphicsRootSignature(directXCommon_->GetSpriteRootSignature());
	commandList->SetPipelineState(directXCommon_->GetSpritePipelineState());
	// プリミティブトポロジを設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


