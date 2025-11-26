#include "Object3DCommon.h"
#include "CameraController.h"
#include "LightManager.h"

Object3DCommon* Object3DCommon::GetInstance()
{
	static Object3DCommon instance;
	return &instance;
}

void Object3DCommon::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
}

void Object3DCommon::setCommonRenderSettings()
{
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	// 3Dオブジェクト専用のPSOを設定
	commandList->SetGraphicsRootSignature(dxCommon_->GetRootSignature());
	commandList->SetPipelineState(dxCommon_->GetPipelineState());
	// プリミティブトポロジを設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ライトを設定（LightManagerから取得）
	LightManager* lightManager = LightManager::GetInstance();
	commandList->SetGraphicsRootConstantBufferView(3, lightManager->GetLightingResource()->GetGPUVirtualAddress());
	// カメラを設定
	ID3D12Resource* cameraResource = CameraController::GetInstance()->GetCameraForGPUResource();
	if (cameraResource) {
		commandList->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
	}

}


