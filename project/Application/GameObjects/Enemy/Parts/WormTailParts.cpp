#include "WormTailParts.h"

void WormTailParts::Initialize(DirectXCommon* dxCommon, const Vector3& position, const std::string& modelName, const std::string& textureName)
{
	// 基底クラスの初期化
	BaseEnemyParts::Initialize(dxCommon, position, modelName, textureName);
	
	// 緑色に設定
	SetColor(0x00FF00FF);
	SetDefaultColor(0x00FF00FF);

	// Objects属性で、弾と衝突判定する
	SetCollisionAttribute(kCollisionAttributeObjects);
	SetCollisionMask(kCollisionAttributePlayerBullet);
}

void WormTailParts::OnCollision(Collider* other) {
	// 基底クラスの処理を呼ぶ
	BaseEnemyParts::OnCollision(other);
}
