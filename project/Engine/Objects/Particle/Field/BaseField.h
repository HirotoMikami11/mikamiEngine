#pragma once
#include <memory>
#include <string>
#include "DirectXCommon.h"
#include "Transform3D.h"
#include "ParticleState.h"
#include "MyFunction.h"
#include "DebugDrawLineSystem.h"

/// <summary>
/// フィールドの基底クラス
/// <para>パーティクルに影響を与えるフィールドの共通インターフェース</para>
/// </summary>
class BaseField
{
public:
	BaseField() = default;
	virtual ~BaseField() = default;

	/// <summary>
	/// フィールドの初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	virtual void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="deltaTime">デルタタイム</param>
	virtual void Update(float deltaTime);

	/// <summary>
	/// パーティクルに効果を適用（純粋仮想関数）
	/// </summary>
	/// <param name="particle">対象のパーティクル</param>
	/// <param name="deltaTime">デルタタイム</param>
	/// <returns>パーティクルを削除すべき場合true</returns>
	virtual bool ApplyEffect(ParticleState& particle, float deltaTime) = 0;

	/// <summary>
	/// 指定した点がフィールド内にあるかチェック（純粋仮想関数）
	/// </summary>
	/// <param name="point">チェックする点の座標</param>
	/// <returns>範囲内ならtrue</returns>
	virtual bool IsInField(const Vector3& point) const = 0;

	/// <summary>
	/// デバッグ描画
	/// </summary>
	virtual void AddLineDebug();

	/// <summary>
	/// ImGui用のデバッグ表示（派生クラスでオーバーライド可能）
	/// </summary>
	virtual void ImGui();

	// フィールド設定
	Transform3D& GetTransform() { return fieldTransform_; }
	const Transform3D& GetTransform() const { return fieldTransform_; }

	void SetEnabled(bool enabled) { isEnabled_ = enabled; }
	bool IsEnabled() const { return isEnabled_; }

	// デバッグ表示設定
	void SetShowDebugVisualization(bool show) { showDebugVisualization_ = show; }
	bool IsShowDebugVisualization() const { return showDebugVisualization_; }

	void SetDebugColor(const Vector4& color) { debugColor_ = color; }
	const Vector4& GetDebugColor() const { return debugColor_; }

	// 状態取得
	const std::string& GetName() const { return name_; }
	void SetName(const std::string& name) { name_ = name; }

	// フィールドタイプ名（デバッグ用）
	virtual const char* GetTypeName() const = 0;

protected:
	/// <summary>
	/// デバッグ描画用の形状を作成（派生クラスで実装）
	/// </summary>
	virtual void CreateDebugShape() = 0;

	// 共通メンバ
	Transform3D fieldTransform_;				// フィールドのトランスフォーム
	bool isEnabled_ = true;						// フィールドが有効か

	// デバッグ描画
	bool showDebugVisualization_ = false;
	Vector4 debugColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };	// 白色（デフォルト）

	std::string name_ = "BaseField";

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;
	DebugDrawLineSystem* debugDrawLineSystem_ = nullptr;

};