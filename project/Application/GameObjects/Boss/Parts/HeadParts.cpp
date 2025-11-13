#include "Parts/HeadParts.h"

void HeadParts::Initialize(DirectXCommon* dxCommon, const Vector3& position) {
	// 基底クラスの初期化
	BaseParts::Initialize(dxCommon, position);

	// 黄色に設定（RGBA: 0xFFFF00FF）
	SetColor(0xFFFF00FF);
	SetDefaultColor({ 1.0f, 1.0f, 0.0f, 1.0f });

	// 常にObjects属性（ダメージを受けない）
	SetCollisionAttribute(kCollisionAttributeObjects);
	SetCollisionMask(0x00000000);  // どことも衝突判定しない

	// 頭はHP管理しない（無敵）
	isActive_ = true;
}

void HeadParts::OnCollision(Collider* other) {
	// 頭は衝突しても何もしない
	(void)other;
}

void HeadParts::TakeDamage(float damage) {
	// 頭はダメージを受けない
	(void)damage;
}