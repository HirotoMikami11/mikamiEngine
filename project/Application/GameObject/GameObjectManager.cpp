#include "GameObjectManager.h"
#include <algorithm>

void GameObjectManager::AddObject(std::unique_ptr<GameObject> obj)
{
    if (isUpdating_) {
        // Update中は pending に積んで次フレームに統合
        pendingObjects_.push_back(std::move(obj));
    } else {
        objects_.push_back(std::move(obj));
    }
}

void GameObjectManager::Update()
{
    // 処理順でソート（小さいほど先）
    std::stable_sort(objects_.begin(), objects_.end(),
        [](const std::unique_ptr<GameObject>& a, const std::unique_ptr<GameObject>& b) {
            return a->GetUpdateOrder() < b->GetUpdateOrder();
        });

    isUpdating_ = true;
    for (auto& obj : objects_) {
        if (!obj->IsDestroyed()) {
            obj->Update();
        }
    }
    isUpdating_ = false;

    // 破棄済みを Finalize して削除
    RemoveDestroyed();

    // Update 中に追加されたオブジェクトを統合
    MergePending();
}

void GameObjectManager::DrawOffscreen()
{
    for (auto& obj : objects_) {
        if (!obj->IsDestroyed()) {
            obj->DrawOffscreen();
        }
    }
}

void GameObjectManager::DrawBackBuffer()
{
    for (auto& obj : objects_) {
        if (!obj->IsDestroyed()) {
            obj->DrawBackBuffer();
        }
    }
}

void GameObjectManager::ImGui()
{
    for (auto& obj : objects_) {
        if (!obj->IsDestroyed()) {
            obj->ImGui();
        }
    }
}

void GameObjectManager::Clear()
{
    for (auto& obj : objects_) {
        obj->Finalize();
    }
    objects_.clear();
    pendingObjects_.clear();
}

void GameObjectManager::MergePending()
{
    if (pendingObjects_.empty()) return;

    for (auto& obj : pendingObjects_) {
        objects_.push_back(std::move(obj));
    }
    pendingObjects_.clear();
}

void GameObjectManager::RemoveDestroyed()
{
    for (auto& obj : objects_) {
        if (obj->IsDestroyed()) {
            obj->Finalize();
        }
    }

    objects_.erase(
        std::remove_if(objects_.begin(), objects_.end(),
            [](const std::unique_ptr<GameObject>& obj) {
                return obj->IsDestroyed();
            }),
        objects_.end()
    );
}
