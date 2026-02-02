#include "WormHeadParts.h"
#include "Audio/AudioManager.h"

void WormHeadParts::Initialize(DirectXCommon* dxCommon, const Vector3& position, const std::string& modelName, const std::string& textureName)
{
	// 基底クラスの初期化
	BaseEnemyParts::Initialize(dxCommon, position, modelName, textureName);
	
	// 黄色に設定
	SetColor(0xFFFF00FF);
	SetDefaultColor(0xFFFF00FF);

	// Objects属性で、弾と衝突判定する
	SetCollisionAttribute(kCollisionAttributeObjects);
	SetCollisionMask(kCollisionAttributePlayerBullet);
}

void WormHeadParts::OnCollision(Collider* other) {
	// 基底クラスの処理を呼ぶ
	BaseEnemyParts::OnCollision(other);
}
