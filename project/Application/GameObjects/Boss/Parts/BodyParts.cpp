#include "Parts/BodyParts.h"
#include "Boss.h"

void BodyParts::Initialize(DirectXCommon* dxCommon, const Vector3& position, const std::string& modelName, const std::string& textureName)
{
	// 基底クラスの初期化
	BaseParts::Initialize(dxCommon, position, modelName, textureName);

	// 白色に設定
	SetColor(0xFFFFFFFF);
	SetDefaultColor(0xFFFFFFFF);

	// デフォルトはPhase1（Enemy属性）
	SetPhase1Attribute();
}

void BodyParts::OnCollision(Collider* other) {

	// 可視性フラグがfalseなら描画しない
	if (!isVisible_) {
		return;
	}
	// 非アクティブの場合はダメージを受けない
	if (!isActive_) {
		//damege
		AudioManager::GetInstance()->Play("EnemyHitMuteki", false, 0.25f);
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
			//damege
			AudioManager::GetInstance()->Play("EnemyHit", false, 0.25f);
		}
	}
}

void BodyParts::SetPhase1Attribute() {
	// 非アクティブの場合は属性を変更しない
	if (!isActive_) {
		return;
	}

	// Phase1: Enemy属性
	SetCollisionAttribute(kCollisionAttributeEnemy);
	SetCollisionMask(kCollisionAttributePlayer | kCollisionAttributePlayerBullet);  // プレイヤーと弾に衝突
}

void BodyParts::SetPhase2Attribute() {
	// 非アクティブの場合は属性を変更しない
	if (!isActive_) {
		return;
	}

	// Phase2: Objects属性
	SetCollisionAttribute(kCollisionAttributeObjects);
	SetCollisionMask(kCollisionAttributePlayerBullet);  // 弾にのみ衝突
}