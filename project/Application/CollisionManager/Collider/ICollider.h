#pragma once
#define NOMINMAX
#include <algorithm>
#include "Engine.h"
#include "CollisionManager/CollisionConfig.h"

/// <summary>
/// コライダーの種類
/// ※ enum の値は 2次元配列のインデックスとして使用するため連番を維持すること
/// ※ 新しい型は Count の直前に追加すること
/// </summary>
enum class ColliderType {
	SPHERE = 0,		// 球体
	AABB = 1,		// AABB
	OBB = 2,		// OBB(未実装)
	SPRITE = 3,		// 2Dスプライト
	CAPSULE = 4,	// カプセル（未実装）
	Count			// 配列サイズ用
};

/// <summary>
/// コライダー基底インターフェース
/// </summary>
class ICollider {
public:
	virtual ~ICollider() = default;

	/// <summary>衝突し始めたフレームに呼ばれる</summary>
	virtual void OnCollisionEnter(ICollider* other) {}

	/// <summary>衝突し続けている間、毎フレーム呼ばれる</summary>
	virtual void OnCollisionStay(ICollider* other) {}

	/// <summary>衝突が終わったフレームに呼ばれる</summary>
	virtual void OnCollisionExit(ICollider* other) {}

	/// <summary>ワールド座標を取得</summary>
	virtual Vector3 GetWorldPosition() = 0;

	/// <summary>デバッグ描画（派生クラスでオーバーライド）</summary>
	virtual void DebugLineAdd() {}

	// --- 型 ---
	ColliderType GetColliderType() const { return colliderType_; }
	void SetColliderType(ColliderType type) { colliderType_ = type; }

	// --- 衝突属性 / マスク ---
	uint32_t GetCollisionAttribute() const { return collisionAttribute_; }
	void SetCollisionAttribute(uint32_t attribute) { collisionAttribute_ = attribute; }
	uint32_t GetCollisionMask() const { return collisionMask_; }
	void SetCollisionMask(uint32_t mask) { collisionMask_ = mask; }

	// --- デバッグ表示 ---
	bool IsColliderVisible() const { return isColliderVisible_; }
	void SetColliderVisible(bool visible) { isColliderVisible_ = visible; }

	// --- 色 ---
	uint32_t GetColliderColor() const { return currentColliderColor_; }
	void SetColliderColor(const uint32_t& color) { currentColliderColor_ = color; }
	uint32_t GetDefaultColliderColor() const { return defaultColliderColor_; }
	void SetDefaultColliderColor(const uint32_t& color) { defaultColliderColor_ = color; }
	void ResetColliderColor() { currentColliderColor_ = defaultColliderColor_; }

protected:
	ColliderType colliderType_ = ColliderType::SPHERE;
	uint32_t collisionAttribute_ = 0xffffffff;
	uint32_t collisionMask_ = 0xffffffff;
	bool isColliderVisible_ = true;
	uint32_t defaultColliderColor_ = 0x00FF00FF;
	uint32_t currentColliderColor_ = 0x00FF00FF;
};
