#pragma once
#include "Structures.h"
#include"DebugDrawLineSystem.h"
#include <string>

/// <summary>
/// ライトの基底クラス
/// </summary>
class LightBase
{
public:
	/// <summary>
	/// ライトの種類
	/// </summary>
	enum class Type {
		DIRECTIONAL,	// 平行光源
		POINT,			// 点光源
		SPOT,			// スポットライト
		RECT			// エリアライト（矩形）
	};

	LightBase() = default;
	virtual ~LightBase() = default;

	/// <summary>
	/// デフォルト設定で初期化
	/// </summary>
	virtual void SetDefaultSettings() = 0;

	// 共通Getter
	virtual Vector4 GetColor() const = 0;
	virtual float GetIntensity() const = 0;
	Type GetType() const { return type_; }

	// 共通Setter
	virtual void SetColor(const Vector4& color) = 0;
	virtual void SetIntensity(float intensity) = 0;

	/// <summary>
	/// デバッグ用のライト表示
	/// </summary>
	virtual void DebugLineAdd() = 0;

	/// <summary>
	/// ImGui用の編集UI
	/// </summary>
	/// <param name="label">UIのラベル</param>
	virtual void ImGui(const std::string& label) = 0;

protected:
	Type type_ = Type::DIRECTIONAL;
};