#pragma once
#include "ICollider.h"

/// <summary>
/// OBBコライダー（未実装）
/// </summary>
class OBBCollider : public ICollider {
public:
	OBBCollider() { colliderType_ = ColliderType::OBB; }

	void OnCollision(ICollider* other) override {}
	Vector3 GetWorldPosition() override { return {}; }
};
