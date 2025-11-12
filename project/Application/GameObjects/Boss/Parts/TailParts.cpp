#include "Parts/TailParts.h"

void TailParts::Initialize(DirectXCommon* dxCommon, const Vector3& position) {
	// 基底クラスの初期化
	BaseParts::Initialize(dxCommon, position);

	// 緑色に設定（RGBA: 0x00FF00FF）
	SetColor(0x00FF00FF);
}