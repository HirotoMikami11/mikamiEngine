#pragma once
#include <vector>
#include <memory>
#include "GameObject.h"

class CollisionManager;

/// <summary>
/// ゲームオブジェクトを一括管理するクラス
/// 処理順ソート・Update中の安全な追加・破棄済み自動削除に対応
/// </summary>
class GameObjectManager
{
public:
	GameObjectManager() = default;
	~GameObjectManager() = default;

	/// <summary>
	/// オブジェクトを追加する
	/// Update 中に呼んだ場合は次フレームに統合される
	/// </summary>
	void AddObject(std::unique_ptr<GameObject> obj);

	/// <summary>
	/// タグ順ソート後に全オブジェクトの Initialize() を呼ぶ
	/// BaseScene::Initialize() final から呼ばれる
	/// </summary>
	void InitializeAll();

	/// <summary>
	/// 全オブジェクトを更新する（オブジェクト更新のみ、衝突判定は含まない）
	/// 処理順ソート → Update → 破棄済み削除 → pending 統合 の順で実行
	/// </summary>
	void Update();

	/// <summary>
	/// ICollider を継承するオブジェクトを CollisionManager に登録する
	/// BaseScene::HandleCollisions() から毎フレーム呼ばれる
	/// </summary>
	void AddAllCollidersToManager(CollisionManager* cm);

	/// <summary>
	/// 全オブジェクトの描画 Submit
	/// RenderGroup で描画先が決まる
	/// </summary>
	void Draw();

	/// <summary>
	/// 全オブジェクトの ImGui 描画
	/// </summary>
	void ImGui();

	/// <summary>
	/// 全オブジェクトを破棄して空にする
	/// </summary>
	void Clear();

	size_t GetObjectCount() const { return objects_.size(); }

private:
	void MergePending();
	void RemoveDestroyed();

	std::vector<std::unique_ptr<GameObject>> objects_;
	std::vector<std::unique_ptr<GameObject>> pendingObjects_;  // Update中の追加バッファ
	bool isUpdating_ = false;
};
