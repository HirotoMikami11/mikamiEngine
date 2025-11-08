#include "Material.h"

void Material::Initialize(DirectXCommon* dxCommon) {
	// マテリアル用のリソースを作成
	materialResource_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(MaterialData));
	// マテリアルデータにマップ
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	// デフォルト設定で初期化
	SetLitObjectSettings();
}

void Material::SetDefaultSettings() {
	// デフォルト設定
	// ライティング無効、白色、UV変換は単位行列
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	lightingMode_ = LightingMode::None;
	materialData_->enableLighting = false;
	materialData_->lightingMode = 0;
	shininess_ = 30.0f;
	materialData_->shininess = shininess_;
	materialData_->padding = 0.0f;
	materialData_->uvTransform = MakeIdentity4x4();
}

void Material::SetLitObjectSettings() {
	// ライト付きオブジェクト用設定
	// ライティング有効、白色、UV変換は単位行列
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	SetLightingMode(LightingMode::HalfLambert);
	shininess_ = 30.0f;
	materialData_->shininess = shininess_;
	materialData_->padding = 0.0f;
	materialData_->uvTransform = MakeIdentity4x4();
}

void Material::SetLightingMode(LightingMode mode) {
	// ライティングモードを設定
	lightingMode_ = mode;
	materialData_->lightingMode = static_cast<int32_t>(mode);

	switch (mode) {
		//ライティングなし
	case LightingMode::None:
		materialData_->enableLighting = false;
		break;

		// ランバート反射
	case LightingMode::Lambert:
		materialData_->enableLighting = true;
		break;

		// ハーフランバート反射
	case LightingMode::HalfLambert:
		materialData_->enableLighting = true;
		break;

		// Phong鏡面反射
	case LightingMode::PhongSpecular:
		materialData_->enableLighting = true;
		break;
	}
}

void Material::UpdateUVTransform() {
	Matrix4x4 uvTransformMatrix = MakeScaleMatrix({ uvScale_.x, uvScale_.y, 0.0f });
	uvTransformMatrix = Matrix4x4Multiply(uvTransformMatrix, MakeRotateZMatrix(uvRotateZ_));
	uvTransformMatrix = Matrix4x4Multiply(uvTransformMatrix, MakeTranslateMatrix({ uvTranslate_.x, uvTranslate_.y, 0.0f }));
	materialData_->uvTransform = uvTransformMatrix;
}

void Material::CopyFrom(const Material& source) {
	SetColor(source.GetColor());
	SetLightingMode(source.GetLightingMode());
	SetShininess(source.GetShininess());
	SetUVTransformScale(source.GetUVTransformScale());
	SetUVTransformRotateZ(source.GetUVTransformRotateZ());
	SetUVTransformTranslate(source.GetUVTransformTranslate());
}