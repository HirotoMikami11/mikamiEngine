#include "WormBodyParts.h"

void WormBodyParts::Initialize(DirectXCommon* dxCommon, const Vector3& position, const std::string& modelName, const std::string& textureName)
{
	// 基底クラスの初期化
	BaseEnemyParts::Initialize(dxCommon, position, modelName, textureName);
	
	// 白色に設定
	SetColor(0xFFFFFFFF);
	SetDefaultColor(0xFFFFFFFF);

	// Objects属性で、弾と衝突判定する
	SetCollisionAttribute(kCollisionAttributeObjects);
	SetCollisionMask(kCollisionAttributePlayerBullet);
}

void WormBodyParts::OnCollision(Collider* other) {
	// 基底クラスの処理を呼ぶ
	BaseEnemyParts::OnCollision(other);
}
