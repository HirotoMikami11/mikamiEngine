#pragma once
#include "DirectXCommon.h"
#include "Structures.h"
#include "LightBase.h"

/// <summary>
/// ポイントライトクラス
/// </summary>
class PointLight : public LightBase
{
public:
	PointLight() { type_ = Type::POINT; }
	~PointLight() override = default;

	/// <summary>
	/// ポイントライトを初期化
	/// </summary>
	/// <param name="position">位置</param>
	/// <param name="color">色</param>
	/// <param name="intensity">強度</param>
	/// <param name="radius">影響範囲</param>
	/// <param name="decay">減衰率</param>
	void Initialize(
		const Vector3& position,
		const Vector4& color = { 1.0f, 1.0f, 1.0f, 1.0f },
		float intensity = 1.0f,
		float radius = 10.0f,
		float decay = 2.0f);

	/// <summary>
	/// デフォルト設定で初期化
	/// </summary>
	void SetDefaultSettings() override;

	// Getter
	Vector4 GetColor() const override { return lightData_.color; }
	Vector3 GetPosition() const { return lightData_.position; }
	float GetIntensity() const override { return lightData_.intensity; }
	float GetRadius() const { return lightData_.radius; }
	float GetDecay() const { return lightData_.decay; }
	const PointLightData& GetData() const { return lightData_; }

	// Setter
	void SetColor(const Vector4& color) override { lightData_.color = color; }
	void SetPosition(const Vector3& position) { lightData_.position = position; }
	void SetIntensity(float intensity) override { lightData_.intensity = intensity; }
	void SetRadius(float radius) { lightData_.radius = radius; }
	void SetDecay(float decay) { lightData_.decay = decay; }

	/// <summary>
	/// ImGui用の編集UI
	/// </summary>
	/// <param name="label">UIのラベル</param>
	void ImGui(const std::string& label) override;

	/// <summary>
	/// デバッグ用のライト表示
	/// </summary>
	void DebugLineAdd() override;

	/// <summary>
	/// ライトが有効かどうか
	/// </summary>
	bool IsActive() const { return isActive_; }
	void SetActive(bool active) { isActive_ = active; }

private:
	friend class LightManager;  // LightManagerのみlightID_にアクセス可能

	PointLightData lightData_;
	bool isActive_ = true;
	uint32_t lightID_ = 0;  // 内部管理用ID（LightManagerが設定）
};