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

void Object3DCommon::setCommonRenderSettings()
{
}


