#include "Parts/BodyParts.h"
#include "Boss.h"

void BodyParts::Initialize(DirectXCommon* dxCommon, const Vector3& position) {
	// 基底クラスの初期化
	BaseParts::Initialize(dxCommon, position);

	// 白色に設定（RGBA: 0xFFFFFFFF）
	SetColor(0xFFFFFFFF);
	SetDefaultColor({ 1.0f, 1.0f, 1.0f, 1.0f });

	// デフォルトはPhase1（Enemy属性）
	SetPhase1Attribute();
}

void BodyParts::OnCollision(Collider* other) {
	// アクティブでない場合は何もしない
	if (!isActive_) {
		return;
	}

	// プレイヤーとの衝突判定
	uint32_t otherAttribute = other->GetCollisionAttribute();

	if (otherAttribute & kCollisionAttributePlayer) {
		// プレイヤーからのダメージ
		TakeDamage(1.0f);

		// Bossにもダメージを与える
		if (boss_) {
			boss_->TakeDamageFromPart(1.0f);
		}
	}
}

void BodyParts::SetPhase1Attribute() {
	if (!isActive_) {
		return;
	}

	// Phase1: Enemy属性
	SetCollisionAttribute(kCollisionAttributeEnemy);
	SetCollisionMask(kCollisionAttributePlayer);  // Playerと衝突
}

void BodyParts::SetPhase2Attribute() {
	if (!isActive_) {
		return;
	}

	// Phase2: Objects属性
	SetCollisionAttribute(kCollisionAttributeObjects);
	SetCollisionMask(0x00000000);  // どことも衝突しない
}