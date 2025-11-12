#include "Parts/BodyParts.h"

void BodyParts::Initialize(DirectXCommon* dxCommon, const Vector3& position) {
	// 基底クラスの初期化
	BaseParts::Initialize(dxCommon, position);

	// 白色に設定（RGBA: 0xFFFFFFFF）
	SetColor(0xFFFFFFFF);
}