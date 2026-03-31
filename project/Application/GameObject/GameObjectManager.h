#pragma once
#include <vector>
#include <memory>
#include "GameObject.h"

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
    /// Initialize() 済みのオブジェクトを渡すこと
    /// Update 中に呼んだ場合は次フレームに統合される
    /// </summary>
    void AddObject(std::unique_ptr<GameObject> obj);

    /// <summary>
    /// 全オブジェクトを更新する
    /// 処理順ソート → Update → 破棄済み削除 → pending 統合 の順で実行
    /// </summary>
    void Update();

    /// <summary>
    /// 全オブジェクトの 3D 描画（DrawOffscreen を呼ぶ）
    /// </summary>
    void DrawOffscreen();

    /// <summary>
    /// 全オブジェクトの UI 描画（DrawBackBuffer を呼ぶ）
    /// </summary>
    void DrawBackBuffer();

    /// <summary>
    /// 全オブジェクトのデバッグ描画（DrawDebug を呼ぶ）
    /// </summary>
    void ImGui();

    /// <summary>
    /// 全オブジェクトを破棄して空にする（シーン終了時に呼ぶ）
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
