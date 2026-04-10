#pragma once

#include <vector>
#include <string>

#include "MyFunction.h"
#include "Structures.h"
#include "Logger.h"
#include <cassert>
using namespace MyMath;

/// <summary>
/// ライティングモードの定義
/// </summary>
enum class LightingMode {
	None = 0,			// ライティングなし
	Lambert = 1,		// ランバート反射
	HalfLambert = 2,	// ハーフランバート反射
	PhongSpecular = 3	// Phong鏡面反射
};

/// <summary>
/// 一つのマテリアルデータクラス
/// </summary>
class Material {
public:
	Material() = default;
	~Material() = default;

	/// <summary>
	/// マテリアルを初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// デフォルト設定で初期化（ライティング無効、白色）
	/// </summary>
	void SetDefaultSettings();

	/// <summary>
	/// ライト付きオブジェクト用の設定（ライティング有効、白色）
	/// </summary>
	void SetLitObjectSettings();

	/// <summary>
	/// UVTransformの行列更新
	/// </summary>
	void UpdateUVTransform();

	/// <summary>
	/// 他のマテリアルからの設定をコピー
	/// </summary>
	/// <param name="source">コピー元のマテリアル</param>
	void CopyFrom(const Material& source);

	// Getter
	Vector4      GetColor()              const { return cpuData_.color; }
	LightingMode GetLightingMode()       const { return lightingMode_; }
	float        GetShininess()          const { return cpuData_.shininess; }
	Matrix4x4    GetUVTransform()        const { return cpuData_.uvTransform; }
	Vector2      GetUVTransformScale()   const { return uvScale_; }
	float        GetUVTransformRotateZ() const { return uvRotateZ_; }
	Vector2      GetUVTransformTranslate() const { return uvTranslate_; }

	/// <summary>
	/// Draw() 内で UploadRingBuffer に書き込むための MaterialData を返す
	/// </summary>
	MaterialData BuildMaterialData() const { return cpuData_; }

	// Setter
	void SetColor(const Vector4& color) { cpuData_.color = color; }
	void SetLightingMode(LightingMode mode);
	void SetShininess(float shininess) { cpuData_.shininess = shininess; }
	void SetUVTransform(const Matrix4x4& uvTransform) { cpuData_.uvTransform = uvTransform; }
	void SetUVTransformScale(const Vector2& uvScale) { uvScale_ = uvScale;         UpdateUVTransform(); }
	void SetUVTransformRotateZ(float uvRotateZ) { uvRotateZ_ = uvRotateZ;     UpdateUVTransform(); }
	void SetUVTransformTranslate(const Vector2& uvTranslate) { uvTranslate_ = uvTranslate; UpdateUVTransform(); }

private:
	// CPU マテリアルデータ
	MaterialData cpuData_{
		{ 1.0f, 1.0f, 1.0f, 1.0f },  // color
		0,                             // enableLighting
		0,                             // lightingMode
		30.0f,                         // shininess
		0.0f,                          // padding
		MakeIdentity4x4()              // uvTransform
	};

	// ライティングモード
	LightingMode lightingMode_ = LightingMode::None;

	// UV 計算用中間変数（UpdateUVTransform()で更新）
	Vector2 uvTranslate_ = { 0.0f, 0.0f };
	Vector2 uvScale_ = { 1.0f, 1.0f };
	float uvRotateZ_ = 0.0f;
};