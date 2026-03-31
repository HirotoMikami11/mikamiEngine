#pragma once
#include <memory>
#include "GameObject.h"
#include "Object3D.h"

/// <summary>
/// テスト用プレイヤー
/// - WASD キーで XZ 平面を移動する
/// - Transform（Position / Scale）・Color・MoveSpeed を JsonSettings で管理
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
    /// <summary> JsonSettings に初期値を登録してファイルから読み込む </summary>
    void LoadFromJson();

    /// <summary> JsonSettings の現在値をオブジェクトに適用する </summary>
    void ApplyJsonValues();

    std::unique_ptr<Sphere> model_;

    float moveSpeed_ = 5.0f;

    DirectXCommon* dxCommon_ = nullptr;
    Matrix4x4 viewProjectionMatrix_{};

    // JsonSettings グループ名
    static constexpr const char* kGroupName = "TestPlayer";
};
