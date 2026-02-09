#pragma once
#define NOMINMAX
#include <algorithm>
#include "Engine.h"
#include "CollisionManager/CollisionConfig.h"  // 衝突属性のフラグを定義

/// <summary>
/// コライダーの種類
/// </summary>
enum class ColliderType {
	SPHERE,	// 球体
	AABB,	// AABB
	OBB,	// OBB（未実装）
	Capsule	// カプセル（未実装）
};


/// <summary>
/// 衝突判定オブジェクト
/// </summary>
class Collider {
public:
	/// <summary>
	/// 仮想デストラクタ
	/// </summary>
	virtual ~Collider() = default;

	/// <summary>
	/// 衝突時に呼ばれる関数（相手のコライダー情報を受け取る）
	/// </summary>
	virtual void OnCollision(Collider* other) = 0;

	/// <summary>
	/// ワールド座標を取得する関数
	/// </summary>
	virtual Vector3 GetWorldPosition() = 0;

	/// <summary>
	/// デバッグ用のコライダー表示
	/// </summary>
	virtual void DebugLineAdd();

	// コライダーの種類を取得
	ColliderType GetColliderType() const { return colliderType_; }
	// コライダーの種類を設定
	void SetColliderType(ColliderType type) { colliderType_ = type; }

	// 半径を取得（Sphere用）
	virtual float GetRadius() const { return radius_; }

	// 半径を設定（Sphere用）
	virtual void SetRadius(float radius) { radius_ = radius; }

	// AABBを取得
	virtual AABB GetAABB() const { return aabb_; }

	// AABBを設定
	virtual void SetAABB(const AABB& aabb) {
		aabb_ = aabb;
		FixAABBMinMax(aabb_);
	}

	// AABBをサイズで設定（中心からのオフセット）
	void SetAABBSize(const Vector3& size) {
		Vector3 center = GetWorldPosition();
		Vector3 halfSize = { size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };
		aabb_.min = center - halfSize;
		aabb_.max = center + halfSize;
		FixAABBMinMax(aabb_);
	}

	/// 衝突属性の取得設定

	// 衝突属性(自分)を取得
	uint32_t GetCollisionAttribute() const { return collisionAttribute_; }
	// 衝突属性(自分)を設定
	void SetCollisionAttribute(uint32_t attribute) { collisionAttribute_ = attribute; }

	// 衝突マスク(相手)を取得
	uint32_t GetCollisionMask() const { return collisionMask_; }
	// 衝突マスク(相手)を設定
	void SetCollisionMask(uint32_t mask) { collisionMask_ = mask; }

	// 表示フラグ
	bool IsColliderVisible() const { return isColliderVisible_; }
	void SetColliderVisible(bool visible) { isColliderVisible_ = visible; }

	//色
	uint32_t GetColliderColor() const { return currentColliderColor_; }
	void SetColliderColor(const uint32_t& color) { currentColliderColor_ = color; }
	uint32_t GetDefaultColliderColor() const { return defaultColliderColor_; }
	void SetDefaultColliderColor(const uint32_t& color) { defaultColliderColor_ = color; }

	// 色をデフォルトにリセット
	void ResetColliderColor() { currentColliderColor_ = defaultColliderColor_; }

protected:
	// コライダーの種類（デフォルトはSphere）
	ColliderType colliderType_ = ColliderType::SPHERE;

	// 衝突半径（Sphere用）
	float radius_ = 1.0f; // 当たり判定用半径

	// AABB（軸並行境界箱用）
	AABB aabb_ = { {-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f} };

	// 衝突属性(自分)
	uint32_t collisionAttribute_ = 0xffffffff;
	// 衝突マスク(相手)
	uint32_t collisionMask_ = 0xffffffff;

	// デバッグ表示設定
	bool isColliderVisible_ = true;					// デバッグ表示フラグ
	uint32_t defaultColliderColor_ = 0x00FF00FF;	// デフォルトデバッグカラー（緑）
	uint32_t currentColliderColor_ = 0x00FF00FF;	// 現在のデバッグカラー
};