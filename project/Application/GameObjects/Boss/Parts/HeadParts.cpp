#include "Parts/HeadParts.h"

void HeadParts::Initialize(DirectXCommon* dxCommon, const Vector3& position, const std::string& modelName, const std::string& textureName)
{
	// 基底クラスの初期化
	BaseParts::Initialize(dxCommon, position, modelName, textureName);
	// 黄色に設定
	SetColor(0xFFFF00FF);
	SetDefaultColor(0xFFFF00FF);

	// Objects属性で、弾と衝突判定する
	SetCollisionAttribute(kCollisionAttributeObjects);
	SetCollisionMask(kCollisionAttributePlayerBullet);	// 弾と衝突判定する

	// 頭はHP管理しない（無敵）
	isActive_ = true;
}

void HeadParts::OnCollision(Collider* other) {


	// 可視性フラグがfalseなら描画しない
	if (!isVisible_) {
		return;
	}
	// 頭は衝突判定はあるが、ダメージは受けない
	// 衝突相手の属性を取得
	uint32_t otherAttribute = other->GetCollisionAttribute();

	// プレイヤーの弾との衝突
	if (otherAttribute & kCollisionAttributePlayerBullet) {
		//damege
		AudioManager::GetInstance()->Play("EnemyHitMuteki", false, 0.25f);
	}

	// プレイヤーとの衝突
	if (otherAttribute & kCollisionAttributePlayer) {

	}

}