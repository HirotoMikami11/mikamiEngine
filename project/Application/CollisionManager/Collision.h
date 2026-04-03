#pragma once
#include "Engine/MyMath/MyFunction.h"

/// <summary>
/// 衝突判定の数学ロジックをまとめた静的ユーティリティクラス
/// </summary>
class Collision {
public:
	/// <summary>球と球</summary>
	static bool IsCollision(const SphereMath& a, const SphereMath& b);

	/// <summary>球と平面</summary>
	static bool IsCollision(const SphereMath& sphere, const PlaneMath& plane);

	/// <summary>球とAABB</summary>
	static bool IsCollision(const SphereMath& sphere, const AABB& aabb);

	/// <summary>AABBとAABB</summary>
	static bool IsCollision(const AABB& a, const AABB& b);

	/// <summary>AABBと線分</summary>
	static bool IsCollision(const AABB& aabb, const Segment& seg);

	/// <summary>AABBと点</summary>
	static bool IsCollision(const AABB& aabb, const Vector3& point);

	/// <summary>線分と平面</summary>
	static bool IsCollision(const Segment& seg, const PlaneMath& plane);

	/// <summary>三角形と線分</summary>
	static bool IsCollision(const TriangleMath& tri, const Segment& seg);
};
