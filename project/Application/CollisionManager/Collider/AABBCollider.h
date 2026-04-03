#pragma once
#include "ICollider.h"
#include "Engine/MyMath/MyFunction.h"

/// <summary>
/// AABBコライダー
/// </summary>
class AABBCollider : public ICollider {
public:
	AABBCollider() { colliderType_ = ColliderType::AABB; }

	void DebugLineAdd() override;

	AABB GetAABB() const { return aabb_; }

	void SetAABB(const AABB& aabb) {
		aabb_ = aabb;
		FixAABBMinMax(aabb_);
	}

	/// <summary>サイズ指定でAABBをセット（中心からの半分サイズ）</summary>
	void SetAABBSize(const Vector3& size) {
		Vector3 half = { size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };
		aabb_.min = { -half.x, -half.y, -half.z };
		aabb_.max = { half.x,  half.y,  half.z };
		FixAABBMinMax(aabb_);
	}

	/// <summary>衝突判定用のAABBをワールド座標で返す</summary>
	AABB GetWorldAABB() {
		Vector3 pos = GetWorldPosition();
		AABB world;
		world.min = aabb_.min + pos;
		world.max = aabb_.max + pos;
		return world;
	}

protected:
	AABB aabb_ = { {-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f} };
};
