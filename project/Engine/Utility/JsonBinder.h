#pragma once
#include <string>
#include <vector>
#include <functional>
#include "MyMath.h"
#include "Transform3D.h"
#include "Material.h"

/// <summary>
/// JsonSettings ラッパー - 変数バインド + ImGui 自動化
///
/// 使い方:
///   binder_ = std::make_unique<JsonBinder>("MyObject");
///   binder_->BindTransform3D("Transform", &model_->GetTransform());
///   binder_->BindMaterial("Material", &model_->GetMaterial());
///   binder_->Bind("MoveSpeed", &moveSpeed_, 5.0f);
///
///   // ImGui() で全項目を一括表示 + Save ボタン
///   binder_->ImGui();
///
/// 設計:
///   - コンストラクタで CreateGroup + LoadFile を自動実行
///   - Bind() 呼び出し毎に AddItem（デフォルト登録）→ 現在値適用 → ImGui ラムダ登録
///   - BindTransform3D / BindMaterial は CollapsingHeader で区分けして表示
///   - サブグループは JsonSettings の subGroups として保存・読み込み
/// </summary>
class JsonBinder
{
public:
    /// <summary>
    /// コンストラクタ
    /// CreateGroup + LoadFile を自動実行する（ファイルなしなら何もしない）
    /// </summary>
    explicit JsonBinder(const std::string& groupName);

    //-----------------------------------------------------------------------
    // 基本型 Bind（個別パラメータ）
    //-----------------------------------------------------------------------

    void Bind(const std::string& key, float*   ptr, float   defaultValue, float speed = 0.1f);
    void Bind(const std::string& key, int32_t* ptr, int32_t defaultValue, float speed = 1.0f);
    void Bind(const std::string& key, bool*    ptr, bool    defaultValue);
    void Bind(const std::string& key, Vector2* ptr, const Vector2& defaultValue, float speed = 0.1f);
    void Bind(const std::string& key, Vector3* ptr, const Vector3& defaultValue, float speed = 0.1f);
    void Bind(const std::string& key, Vector4* ptr, const Vector4& defaultValue, float speed = 0.1f);

    /// <summary> Vector4 を RGBA カラーとして ColorEdit4 で表示する </summary>
    void BindColor(const std::string& key, Vector4* ptr, const Vector4& defaultValue);

    //-----------------------------------------------------------------------
    // 複合型 Bind（CollapsingHeader で区分け）
    //-----------------------------------------------------------------------

    /// <summary>
    /// Transform3D を Position / Rotation / Scale として CollapsingHeader でバインド
    /// JsonSettings サブグループ { groupName, sectionName } に保存
    /// </summary>
    void BindTransform3D(const std::string& sectionName, Transform3D* transform);

    /// <summary>
    /// Material を Color + LightingMode として CollapsingHeader でバインド
    /// JsonSettings サブグループ { groupName, sectionName } に保存
    /// </summary>
    void BindMaterial(const std::string& sectionName, Material* material);

    //-----------------------------------------------------------------------
    // 表示・保存
    //-----------------------------------------------------------------------

    /// <summary>
    /// 登録された全項目を ImGui で表示する
    /// 変更時は JsonSettings に自動反映される
    /// 末尾に Save ボタンを表示する
    /// </summary>
    void ImGui();

    /// <summary> JsonSettings::SaveFile を呼ぶ </summary>
    void Save();

private:
    std::string groupName_;

    // 登録順に描画ラムダを保持
    std::vector<std::function<void()>> drawItems_;
};
