#include "Parts/TailParts.h"
#include "Boss.h"

void TailParts::Initialize(DirectXCommon* dxCommon, const Vector3& position) {
	// 基底クラスの初期化
	BaseParts::Initialize(dxCommon, position);

	// 緑色に設定（RGBA: 0x00FF00FF）
	SetColor(0x00FF00FF);
	SetDefaultColor({ 0.0f, 1.0f, 0.0f, 1.0f });

	// デフォルトはPhase1（Objects属性）
	SetPhase1Attribute();
}

void TailParts::OnCollision(Collider* other) {
	// アクティブでない場合は何もしない
	if (!isActive_) {
		return;
	}

	// プレイヤーとの衝突判定
	uint32_t otherAttribute = other->GetCollisionAttribute();

	if (otherAttribute & kCollisionAttributePlayer) {
		// プレイヤーからのダメージ（仮で0ダメージ）
		TakeDamage(1.0f);

		// Bossにもダメージを与える
		if (boss_) {
			boss_->TakeDamageFromPart(1.0f);
		}
	}
}

void TailParts::SetPhase1Attribute() {
	if (!isActive_) {
		return;
	}

	// Phase1: Objects属性
	SetCollisionAttribute(kCollisionAttributeObjects);
	SetCollisionMask(0x00000000);  // どことも衝突しない
}

void TailParts::SetPhase2Attribute() {
	if (!isActive_) {
		return;
	}

	// Phase2: Enemy属性
	SetCollisionAttribute(kCollisionAttributeEnemy);
	SetCollisionMask(kCollisionAttributePlayer);  // Playerと衝突
}