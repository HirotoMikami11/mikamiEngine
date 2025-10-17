#include "Object3DCommon.h"

Object3DCommon* Object3DCommon::GetInstance()
{
	static Object3DCommon instance;
	return &instance;
}

void Object3DCommon::Initialize(DirectXCommon* dxCommon)
{
	directXCommon_ = dxCommon;
}

void Object3DCommon::setCommonSpriteRenderSettings(ID3D12GraphicsCommandList* commandList)
{
	// スプライト専用のPSOを設定
	commandList->SetGraphicsRootSignature(directXCommon_->GetRootSignature());
	commandList->SetPipelineState(directXCommon_->GetPipelineState());
	// プリミティブトポロジを設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


