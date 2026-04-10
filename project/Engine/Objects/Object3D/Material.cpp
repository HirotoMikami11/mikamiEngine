#include "Material.h"

void Material::Initialize() {
	SetLitObjectSettings();
}

void Material::SetDefaultSettings() {
	cpuData_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	cpuData_.enableLighting = 0;
	cpuData_.lightingMode = 0;
	cpuData_.shininess = 30.0f;
	cpuData_.uvTransform = MakeIdentity4x4();
	lightingMode_ = LightingMode::None;
}

void Material::SetLitObjectSettings() {
	cpuData_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	cpuData_.shininess = 30.0f;
	cpuData_.uvTransform = MakeIdentity4x4();
	SetLightingMode(LightingMode::HalfLambert);
}

void Material::SetLightingMode(LightingMode mode) {
	lightingMode_ = mode;
	cpuData_.lightingMode = static_cast<int32_t>(mode);
	cpuData_.enableLighting = static_cast<int32_t>(mode != LightingMode::None);
}

void Material::UpdateUVTransform() {
	Matrix4x4 m = MakeScaleMatrix({ uvScale_.x, uvScale_.y, 0.0f });
	m = Matrix4x4Multiply(m, MakeRotateZMatrix(uvRotateZ_));
	m = Matrix4x4Multiply(m, MakeTranslateMatrix({ uvTranslate_.x, uvTranslate_.y, 0.0f }));
	cpuData_.uvTransform = m;
}

void Material::CopyFrom(const Material& source) {
	SetColor(source.GetColor());
	SetLightingMode(source.GetLightingMode());
	SetShininess(source.GetShininess());
	SetUVTransformScale(source.GetUVTransformScale());
	SetUVTransformRotateZ(source.GetUVTransformRotateZ());
	SetUVTransformTranslate(source.GetUVTransformTranslate());
}
