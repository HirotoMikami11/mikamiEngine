#include "Parts/TailParts.h"
#include "Boss.h"

void TailParts::Initialize(DirectXCommon* dxCommon, const Vector3& position, const std::string& modelName, const std::string& textureName)
{
	// 基底クラスの初期化
	BaseParts::Initialize(dxCommon, position,modelName,textureName);

	// 緑色に設定
	SetColor(0x00FF00FF);
	SetDefaultColor(0x00FF00FF);
	// デフォルトはPhase1（Objects属性）
	SetPhase1Attribute();
}

void TailParts::OnCollision(Collider* other) {

	// 可視性フラグがfalseなら描画しない
	if (!isVisible_) {
		return;
	}

	if (!isActive_) {
		//damege
		AudioManager::GetInstance()->Play("EnemyHitMuteki", false, 0.25f);
		return;
	}

	uint32_t otherAttribute = other->GetCollisionAttribute();

	// プレイヤーの弾との衝突
	if (otherAttribute & kCollisionAttributePlayerBullet) {
		float attackPower = other->GetAttackPower();

		// 倍率を掛ける
		float actualDamage = TakeDamage(attackPower * 1.5f);

		// Bossに実際に減ったHP分のダメージを与える
		if (boss_ && actualDamage > 0.0f) {
			boss_->TakeDamageFromPart(actualDamage);
			//damege
			AudioManager::GetInstance()->Play("EnemyHit", false, 0.25f);
		}
	}

	// プレイヤーとの衝突（Phase2でのみ発生）
	if (otherAttribute & kCollisionAttributePlayer) {
		// 必要ならここにも倍率を掛けて処理
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