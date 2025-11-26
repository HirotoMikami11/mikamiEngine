#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "DirectXCommon.h"
#include "Structures.h"
#include "LightBase.h"

/// <summary>
/// 平行光源クラス
/// </summary>
class DirectionalLight : public LightBase
{
public:
	DirectionalLight() { type_ = Type::DIRECTIONAL; }
	~DirectionalLight() override = default;

	/// <summary>
	/// 平行光源を初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// デフォルト設定で初期化
	/// </summary>
	void SetDefaultSettings() override;

	// Getter
	Vector4 GetColor() const override { return lightData_.color; }
	Vector3 GetDirection() const { return lightData_.direction; }
	float GetIntensity() const override { return lightData_.intensity; }
	const DirectionalLightData& GetData() const { return lightData_; }

	// Setter
	void SetColor(const Vector4& color) override { lightData_.color = color; }
	void SetDirection(const Vector3& direction) { lightData_.direction = direction; }
	void SetIntensity(float intensity) override { lightData_.intensity = intensity; }

	/// <summary>
	/// ImGui用の編集UI
	/// </summary>
	/// <param name="label">UIのラベル</param>
	void ImGui(const std::string& label) override;

private:
	DirectionalLightData lightData_;
};
