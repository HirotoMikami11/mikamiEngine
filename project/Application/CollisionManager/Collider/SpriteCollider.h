#pragma once
#include "ICollider.h"

/// <summary>
/// 2Dスプライトコライダー（未実装）
/// </summary>
class SpriteCollider : public ICollider {
public:
	SpriteCollider() { colliderType_ = ColliderType::SPRITE; }

	void OnCollision(ICollider* other) override {}
	Vector3 GetWorldPosition() override { return {}; }
};
