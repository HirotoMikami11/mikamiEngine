#pragma once

#include "Collider/ICollider.h"
#include "Collider/SphereCollider.h"
#include "Collider/AABBCollider.h"
#include <list>
#include <set>
#include <utility>

/// <summary>
/// 衝突判定マネージャー
/// Enter / Stay / Exit の3段階コールバックに対応
/// </summary>
class CollisionManager {
public:
	CollisionManager();
	~CollisionManager();

	/// <summary>初期化（コライダーリストのクリア）</summary>
	void Initialize();

	/// <summary>更新処理（色リセット → 衝突判定 → Enter/Stay/Exit 通知）</summary>
	void Update();

	/// <summary>コライダーをリストに登録</summary>
	void AddCollider(ICollider* collider);

	/// <summary>コライダーリストをクリア</summary>
	void ClearColliderList();

	void SetHitColor(const uint32_t& color) { hitColor_ = color; }
	uint32_t GetHitColor() const { return hitColor_; }

private:
	/// <summary>全ペアを総当たりして今フレームの衝突ペア集合を構築</summary>
	void CheckAllCollision();

	/// <summary>2コライダー間の衝突を判定してペア集合に追加</summary>
	bool CheckCollisionPair(ICollider* a, ICollider* b);

	/// <summary>全コライダーの色をデフォルトにリセット</summary>
	void ResetAllColliderColors();

	/// <summary>起動時に全型ペアのハンドラを登録する</summary>
	void RegisterCollisionHandlers();

	// --- 型 ---
	using CollisionFunc = bool(*)(ICollider*, ICollider*);
	using ColliderPair  = std::pair<ICollider*, ICollider*>;

	/// <summary>ポインタを常に小さい順に並べてペアを作る（正規化）</summary>
	static ColliderPair MakePair(ICollider* a, ICollider* b) {
		if (a > b) std::swap(a, b);
		return { a, b };
	}

	// コライダーリスト
	std::list<ICollider*> colliders_;

	// 衝突ペアの集合（Enter/Stay/Exit 判定用）
	std::set<ColliderPair> currentPairs_;
	std::set<ColliderPair> prevPairs_;

	// 2次元 dispatch table（行=typeA, 列=typeB、常に typeA<=typeB）
	static constexpr int kTypeCount = static_cast<int>(ColliderType::Count);
	CollisionFunc dispatchTable_[kTypeCount][kTypeCount] = {};

	// 衝突時の色（デフォルト: 赤）
	uint32_t hitColor_ = 0xFF0000FF;
};
