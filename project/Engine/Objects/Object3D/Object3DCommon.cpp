#include "Object3DCommon.h"

Object3DCommon* Object3DCommon::GetInstance()
{
	static Object3DCommon instance;
	return &instance;
}

void Object3DCommon::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
}

void Object3DCommon::setCommonRenderSettings(ID3D12GraphicsCommandList* commandList)
{
	// 3Dオブジェクト専用のPSOを設定
	commandList->SetGraphicsRootSignature(dxCommon_->GetRootSignature());
	commandList->SetPipelineState(dxCommon_->GetPipelineState());
	// プリミティブトポロジを設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


