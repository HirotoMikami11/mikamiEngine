#pragma once
#include "ICollider.h"
#include "Engine/MyMath/MyFunction.h"

/// <summary>
/// 球体コライダー
/// </summary>
class SphereCollider : public ICollider {
public:
	SphereCollider() { colliderType_ = ColliderType::SPHERE; }

	void DebugLineAdd() override;

	float GetRadius() const { return radius_; }
	void SetRadius(float radius) { radius_ = radius; }

	/// <summary>衝突判定用の SphereMath をワールド座標で返す</summary>
	SphereMath GetWorldSphere() {
		SphereMath s;
		s.center = GetWorldPosition();
		s.radius = radius_;
		return s;
	}

protected:
	float radius_ = 1.0f;
};
