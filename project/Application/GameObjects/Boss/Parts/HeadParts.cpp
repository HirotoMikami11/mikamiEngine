#include "Parts/HeadParts.h"

void HeadParts::Initialize(DirectXCommon* dxCommon, const Vector3& position) {
	// 基底クラスの初期化
	BaseParts::Initialize(dxCommon, position);

	// 黄色に設定（RGBA: 0xFFFF00FF）
	SetColor(0xFFFF00FF);
}