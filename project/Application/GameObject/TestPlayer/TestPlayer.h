#pragma once
#include <memory>
#include "GameObject.h"
#include "Object3D.h"
#include "JsonBinder.h"

/// <summary>
/// テスト用プレイヤー
/// - WASD キーで XZ 平面を移動する
/// - JsonBinder で Transform・Material・MoveSpeed を JSON 管理
/// </summary>
class TestPlayer : public GameObject
{
public:
    TestPlayer() = default;
    ~TestPlayer() override = default;

    void Initialize() override;
    void Update() override;
    void DrawOffscreen() override;
    void ImGui() override;
    void Finalize() override;

private:
    std::unique_ptr<Sphere> model_;
    std::unique_ptr<JsonBinder> binder_;

    float moveSpeed_ = 5.0f;

    DirectXCommon* dxCommon_ = nullptr;
    Matrix4x4 viewProjectionMatrix_{};
};
