#pragma once
#include "DirectXCommon.h"
#include "Structures.h"
#include "LightBase.h"

/// <summary>
/// スポットライトクラス
/// </summary>
class SpotLight : public LightBase
{
public:
	SpotLight() { type_ = Type::SPOT; }
	~SpotLight() override = default;

	/// <summary>
	/// スポットライトを初期化
	/// </summary>
	/// <param name="position">位置</param>
	/// <param name="rotation">回転（Euler角、度数法）</param>
	/// <param name="color">色</param>
	/// <param name="intensity">強度</param>
	/// <param name="distance">最大距離</param>
	/// <param name="decay">減衰率</param>
	/// <param name="angle">スポット角度（外側の境界、度数法）</param>
	/// <param name="falloffStart">フォールオフ開始角度（内側の境界、度数法）(angle > falloffStart) であること</param>
	void Initialize(
		const Vector3& position,
		const Vector3& rotation = { 0.0f, 0.0f, 0.0f },
		const Vector4& color = { 1.0f, 1.0f, 1.0f, 1.0f },
		float intensity = 1.0f,
		float distance = 15.0f,
		float decay = 2.0f,
		float angle = 30.0f,
		float falloffStart = 20.0f);

	/// <summary>
	/// デフォルト設定で初期化
	/// </summary>
	void SetDefaultSettings() override;

	// Getter
	Vector4 GetColor() const override { return lightData_.color; }
	Vector3 GetPosition() const { return lightData_.position; }
	Vector3 GetRotation() const { return rotation_; }
	Vector3 GetDirection() const { return lightData_.direction; }
	float GetIntensity() const override { return lightData_.intensity; }
	float GetDistance() const { return lightData_.distance; }
	float GetDecay() const { return lightData_.decay; }
	float GetAngle() const { return angle_; }
	float GetFalloffStart() const { return falloffStart_; }
	float GetCosAngle() const { return lightData_.cosAngle; }
	float GetCosFalloffStart() const { return lightData_.cosFalloffStart; }
	const SpotLightData& GetData() const { return lightData_; }

	// Setter
	void SetColor(const Vector4& color) override { lightData_.color = color; }
	void SetPosition(const Vector3& position) { lightData_.position = position; }
	void SetRotation(const Vector3& rotation);
	void SetIntensity(float intensity) override { lightData_.intensity = intensity; }
	void SetDistance(float distance) { lightData_.distance = distance; }
	void SetDecay(float decay) { lightData_.decay = decay; }
	void SetAngle(float angle);
	void SetFalloffStart(float falloffStart);

	/// <summary>
	/// デバッグ用のライト表示
	/// </summary>
	void DebugLineAdd() override;

	/// <summary>
	/// ImGui用の編集UI
	/// </summary>
	/// <param name="label">UIのラベル</param>
	void ImGui(const std::string& label) override;

	/// <summary>
	/// ライトが有効かどうか
	/// </summary>
	bool IsActive() const { return isActive_; }
	void SetActive(bool active) { isActive_ = active; }

private:
	friend class LightManager;  // LightManagerのみlightID_にアクセス可能

	/// <summary>
	/// 回転角度から方向ベクトルを計算
	/// </summary>
	void UpdateDirection();

private:
	SpotLightData lightData_;
	Vector3 rotation_ = { 0.0f, 0.0f, 0.0f };	// 回転（Euler角、度数法）
	float angle_ = 30.0f;						// スポット角度（外側の境界、度数法）
	float falloffStart_ = 20.0f;				// フォールオフ開始角度（内側の境界、度数法）(angle_ > falloffStart_)
	bool isActive_ = true;
	uint32_t lightID_ = 0;  // 内部管理用ID（LightManagerが設定）

	static constexpr float MIN_ANGLE_DIFFERENCE = 0.01f;  // 最小角度差（度）
};