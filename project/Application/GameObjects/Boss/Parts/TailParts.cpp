#include "Parts/TailParts.h"
#include "Boss.h"

void TailParts::Initialize(DirectXCommon* dxCommon, const Vector3& position) {
	// 基底クラスの初期化
	BaseParts::Initialize(dxCommon, position);

	// 緑色に設定
	SetColor(0x00FF00FF);
	SetDefaultColor(0x00FF00FF);
	// デフォルトはPhase1（Objects属性）
	SetPhase1Attribute();
}

void TailParts::OnCollision(Collider* other) {
	// 非アクティブの場合はダメージを受けない
	if (!isActive_) {
		return;
	}

	// 衝突相手の属性を取得
	uint32_t otherAttribute = other->GetCollisionAttribute();

	// プレイヤーの弾との衝突
	if (otherAttribute & kCollisionAttributePlayerBullet) {
		// 弾の攻撃力を取得
		float attackPower = other->GetAttackPower();

		// ダメージを受けて、実際に減ったHP分を取得
		float actualDamage = TakeDamage(attackPower);

		// Bossに実際に減ったHP分のダメージを与える
		if (boss_ && actualDamage > 0.0f) {
			boss_->TakeDamageFromPart(actualDamage);
		}
	}

	// プレイヤーとの衝突（Phase2でのみ発生）
	if (otherAttribute & kCollisionAttributePlayer) {
		// プレイヤーとの衝突処理（今回は特に実装なし）
	}
}

void TailParts::SetPhase1Attribute() {
	// 非アクティブの場合は属性を変更しない
	if (!isActive_) {
		return;
	}

	// Phase1: Objects属性
	SetCollisionAttribute(kCollisionAttributeObjects);
	SetCollisionMask(kCollisionAttributePlayerBullet);  // 弾にのみ衝突
}

void TailParts::SetPhase2Attribute() {
	// 非アクティブの場合は属性を変更しない
	if (!isActive_) {
		return;
	}

	// Phase2: Enemy属性
	SetCollisionAttribute(kCollisionAttributeEnemy);
	SetCollisionMask(kCollisionAttributePlayer | kCollisionAttributePlayerBullet);  // プレイヤーと弾に衝突
}