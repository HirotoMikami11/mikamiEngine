#pragma once
#include "DirectXCommon.h"
#include "Structures.h"
#include "LightBase.h"

/// <summary>
/// エリアライト（矩形ライト）クラス
/// </summary>
class RectLight : public LightBase
{
public:
	RectLight() { type_ = Type::RECT; }
	~RectLight() override = default;

	/// <summary>
	/// エリアライトを初期化
	/// </summary>
	/// <param name="position">矩形の中心位置</param>
	/// <param name="rotation">矩形の回転（Euler角、度数法）</param>
	/// <param name="color">色</param>
	/// <param name="intensity">強度</param>
	/// <param name="width">矩形の幅</param>
	/// <param name="height">矩形の高さ</param>
	/// <param name="decay">減衰率</param>
	void Initialize(
		const Vector3& position,
		const Vector3& rotation = { 0.0f, 0.0f, 0.0f },
		const Vector4& color = { 1.0f, 1.0f, 1.0f, 1.0f },
		float intensity = 1.0f,
		float width = 2.0f,
		float height = 2.0f,
		float decay = 2.0f);

	/// <summary>
	/// デフォルト設定で初期化
	/// </summary>
	void SetDefaultSettings() override;

	// Getter
	Vector4 GetColor() const override { return lightData_.color; }
	Vector3 GetPosition() const { return lightData_.position; }
	Vector3 GetRotation() const { return rotation_; }
	Vector3 GetNormal() const { return lightData_.normal; }
	Vector3 GetTangent() const { return lightData_.tangent; }
	Vector3 GetBitangent() const { return lightData_.bitangent; }
	float GetIntensity() const override { return lightData_.intensity; }
	float GetWidth() const { return lightData_.width; }
	float GetHeight() const { return lightData_.height; }
	float GetDecay() const { return lightData_.decay; }
	const RectLightData& GetData() const { return lightData_; }

	// Setter
	void SetColor(const Vector4& color) override { lightData_.color = color; }
	void SetPosition(const Vector3& position) { lightData_.position = position; }
	void SetRotation(const Vector3& rotation);
	void SetIntensity(float intensity) override { lightData_.intensity = intensity; }
	void SetWidth(float width) { lightData_.width = width; }
	void SetHeight(float height) { lightData_.height = height; }
	void SetDecay(float decay) { lightData_.decay = decay; }

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
	/// 回転角度から法線とタンジェントベクトルを計算
	/// </summary>
	void UpdateVectors();

private:
	RectLightData lightData_;
	Vector3 rotation_ = { 0.0f, 0.0f, 0.0f };	// 回転（Euler角、度数法）
	bool isActive_ = true;
	uint32_t lightID_ = 0;  // 内部管理用ID（LightManagerが設定）
};