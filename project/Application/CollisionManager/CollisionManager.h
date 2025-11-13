#pragma once

#include "Collider.h"
#include <list>
#include <memory>
#include "Engine.h"

class CollisionManager {
public:

	/// <summary>
	/// コンストラクタ
	/// </summary>
	CollisionManager();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~CollisionManager();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// リストのクリア、コライダー登録、衝突判定処理を行う
	/// </summary>
	void Update();

	/// <summary>
	/// コライダーを引数で受け取り、リストに登録する関数
	/// </summary>
	/// <param name="collider">登録するコライダー</param>
	void AddCollider(Collider* collider);

	/// <summary>
	/// コライダーリストをクリアする関数
	/// </summary>
	void ClearColliderList();

	/// <summary>
	/// 衝突判定と応答
	/// </summary>
	void CheckAllCollision();

	/// <summary>
	/// 衝突時の色を設定
	/// </summary>
	void SetHitColor(const uint32_t& color) { hitColor_ = color; }

	/// <summary>
	/// 衝突時の色を取得
	/// </summary>
	uint32_t GetHitColor() const { return hitColor_; }

private:
	/// <summary>
	/// コライダー２つの衝突判定と応答
	/// </summary>
	/// <param name="colliderA">コライダーA</param>
	/// <param name="colliderB">コライダーB</param>
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);

	/// <summary>
	/// 全コライダーの色をデフォルトにリセット
	/// </summary>
	void ResetAllColliderColors();

	// コライダーのリスト
	std::list<Collider*> colliders_;

	// 衝突時の色（デフォルト: 赤）
	uint32_t hitColor_ = 0xFF0000FF;
};