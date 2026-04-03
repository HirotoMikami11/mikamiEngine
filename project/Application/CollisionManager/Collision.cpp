#define NOMINMAX
#include "Collision.h"
#include <cmath>
#include <algorithm>
#include <cassert>

using namespace MyMath;

/*-----------------------------------------------------------------------*/
// 球と球
/*-----------------------------------------------------------------------*/
bool Collision::IsCollision(const SphereMath& a, const SphereMath& b) {
	float distance = Length(Subtract(a.center, b.center));
	return distance <= a.radius + b.radius;
}

/*-----------------------------------------------------------------------*/
// 球と平面
/*-----------------------------------------------------------------------*/
bool Collision::IsCollision(const SphereMath& sphere, const PlaneMath& plane) {
	float distance = std::abs(Dot(plane.normal, sphere.center) - plane.distance);
	return distance <= sphere.radius;
}

/*-----------------------------------------------------------------------*/
// 球とAABB
/*-----------------------------------------------------------------------*/
bool Collision::IsCollision(const SphereMath& sphere, const AABB& aabb) {
	Vector3 closestPoint{
		std::clamp(sphere.center.x, aabb.min.x, aabb.max.x),
		std::clamp(sphere.center.y, aabb.min.y, aabb.max.y),
		std::clamp(sphere.center.z, aabb.min.z, aabb.max.z),
	};
	float distance = Length(Subtract(closestPoint, sphere.center));
	return distance <= sphere.radius;
}

/*-----------------------------------------------------------------------*/
// AABBとAABB
/*-----------------------------------------------------------------------*/
bool Collision::IsCollision(const AABB& a, const AABB& b) {
	return (a.min.x <= b.max.x && a.max.x >= b.min.x &&
		a.min.y <= b.max.y && a.max.y >= b.min.y &&
		a.min.z <= b.max.z && a.max.z >= b.min.z);
}

/*-----------------------------------------------------------------------*/
// AABBと線分
/*-----------------------------------------------------------------------*/
bool Collision::IsCollision(const AABB& aabb, const Segment& segment) {
	// 点の場合
	if (segment.diff.x == 0.0f && segment.diff.y == 0.0f && segment.diff.z == 0.0f) {
		return (segment.origin.x >= aabb.min.x && segment.origin.x <= aabb.max.x &&
			segment.origin.y >= aabb.min.y && segment.origin.y <= aabb.max.y &&
			segment.origin.z >= aabb.min.z && segment.origin.z <= aabb.max.z);
	}

	float tmin = 0.0f;
	float tmax = 1.0f;

	// X軸
	if (segment.diff.x != 0.0f) {
		float txmin = (aabb.min.x - segment.origin.x) / segment.diff.x;
		float txmax = (aabb.max.x - segment.origin.x) / segment.diff.x;
		tmin = std::max(tmin, std::min(txmin, txmax));
		tmax = std::min(tmax, std::max(txmin, txmax));
		if (tmin > tmax) return false;
	} else {
		if (segment.origin.x < aabb.min.x || segment.origin.x > aabb.max.x) return false;
	}

	// Y軸
	if (segment.diff.y != 0.0f) {
		float tymin = (aabb.min.y - segment.origin.y) / segment.diff.y;
		float tymax = (aabb.max.y - segment.origin.y) / segment.diff.y;
		tmin = std::max(tmin, std::min(tymin, tymax));
		tmax = std::min(tmax, std::max(tymin, tymax));
		if (tmin > tmax) return false;
	} else {
		if (segment.origin.y < aabb.min.y || segment.origin.y > aabb.max.y) return false;
	}

	// Z軸
	if (segment.diff.z != 0.0f) {
		float tzmin = (aabb.min.z - segment.origin.z) / segment.diff.z;
		float tzmax = (aabb.max.z - segment.origin.z) / segment.diff.z;
		tmin = std::max(tmin, std::min(tzmin, tzmax));
		tmax = std::min(tmax, std::max(tzmin, tzmax));
		if (tmin > tmax) return false;
	} else {
		if (segment.origin.z < aabb.min.z || segment.origin.z > aabb.max.z) return false;
	}

	return (tmin <= tmax && tmax >= 0.0f && tmin <= 1.0f);
}

/*-----------------------------------------------------------------------*/
// AABBと点
/*-----------------------------------------------------------------------*/
bool Collision::IsCollision(const AABB& aabb, const Vector3& point) {
	return (point.x >= aabb.min.x && point.x <= aabb.max.x &&
		point.y >= aabb.min.y && point.y <= aabb.max.y &&
		point.z >= aabb.min.z && point.z <= aabb.max.z);
}

/*-----------------------------------------------------------------------*/
// 線分と平面
/*-----------------------------------------------------------------------*/
bool Collision::IsCollision(const Segment& seg, const PlaneMath& plane) {
	float dot = Dot(plane.normal, seg.diff);
	if (dot == 0.0f) return false;
	float t = (plane.distance - Dot(seg.origin, plane.normal)) / dot;
	return (t >= 0.0f && t <= 1.0f);
}

/*-----------------------------------------------------------------------*/
// 三角形と線分
/*-----------------------------------------------------------------------*/
bool Collision::IsCollision(const TriangleMath& tri, const Segment& seg) {
	// 三角形の中心座標
	Vector3 center = {
		(tri.vertices[0].x + tri.vertices[1].x + tri.vertices[2].x) / 3.0f,
		(tri.vertices[0].y + tri.vertices[1].y + tri.vertices[2].y) / 3.0f,
		(tri.vertices[0].z + tri.vertices[1].z + tri.vertices[2].z) / 3.0f
	};

	// 三角形の存在する平面を作成
	PlaneMath plane;
	plane.distance = Distance(center, { 0.0f, 0.0f, 0.0f });
	plane.normal = Cross(
		Subtract(tri.vertices[1], tri.vertices[0]),
		Subtract(tri.vertices[2], tri.vertices[1])
	);

	if (!IsCollision(seg, plane)) return false;

	// 衝突点
	Vector3 p = MakeCollisionPoint(seg, plane);

	Vector3 v01 = Subtract(tri.vertices[1], tri.vertices[0]);
	Vector3 v12 = Subtract(tri.vertices[2], tri.vertices[1]);
	Vector3 v20 = Subtract(tri.vertices[0], tri.vertices[2]);
	Vector3 v0p = Subtract(p, tri.vertices[0]);
	Vector3 v1p = Subtract(p, tri.vertices[1]);
	Vector3 v2p = Subtract(p, tri.vertices[2]);

	Vector3 cross01 = Cross(v01, v1p);
	Vector3 cross12 = Cross(v12, v2p);
	Vector3 cross20 = Cross(v20, v0p);

	return (Dot(cross01, plane.normal) >= 0.0f &&
		Dot(cross12, plane.normal) >= 0.0f &&
		Dot(cross20, plane.normal) >= 0.0f);
}
