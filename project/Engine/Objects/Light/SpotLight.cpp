#include "SpotLight.h"
#include "ImGui/ImGuiManager.h"
#include "MyMath.h"
#include "Engine.h"
#include <algorithm>

void SpotLight::Initialize(
	const Vector3& position,
	const Vector3& rotation,
	const Vector4& color,
	float intensity,
	float distance,
	float decay,
	float angle,
	float falloffStart)
{
	lightData_.position = position;
	rotation_ = rotation;
	lightData_.color = color;
	lightData_.intensity = intensity;
	lightData_.distance = distance;
	lightData_.decay = decay;
	angle_ = angle;
	falloffStart_ = falloffStart;

	// 角度の安全性を確保（angleがfalloffStartより大きいことを保証）
	if (angle_ - falloffStart_ < MIN_ANGLE_DIFFERENCE) {
		falloffStart_ = angle_ - MIN_ANGLE_DIFFERENCE;
		// falloffStartが負にならないように
		if (falloffStart_ < 0.0f) {
			falloffStart_ = 0.0f;
			angle_ = MIN_ANGLE_DIFFERENCE;
		}
	}

	// 方向ベクトルを計算
	UpdateDirection();

	// cos値を計算
	lightData_.cosAngle = std::cos(DegToRad(angle_));
	lightData_.cosFalloffStart = std::cos(DegToRad(falloffStart_));

	lightData_.padding = 0.0f;
	isActive_ = true;
}

void SpotLight::SetDefaultSettings()
{
	// デフォルト設定
	lightData_.color = { 1.0f, 1.0f, 1.0f, 1.0f };	// 白色
	lightData_.position = { 0.0f, 5.0f, 0.0f };		// 上方
	rotation_ = { 90.0f, 0.0f, 0.0f };				// 下向き（X軸90度回転）
	lightData_.intensity = 1.0f;
	lightData_.distance = 15.0f;
	lightData_.decay = 2.0f;
	angle_ = 30.0f;
	falloffStart_ = 20.0f;

	// 方向ベクトルを計算
	UpdateDirection();

	// cos値を計算
	lightData_.cosAngle = std::cos(DegToRad(angle_));
	lightData_.cosFalloffStart = std::cos(DegToRad(falloffStart_));

	lightData_.padding = 0.0f;
	isActive_ = true;
}

void SpotLight::SetRotation(const Vector3& rotation)
{
	rotation_ = rotation;
	UpdateDirection();
}

void SpotLight::SetAngle(float angle)
{
	angle_ = angle;

	// angleがfalloffStartより大きく（外側に）なるように自動調整
	if (angle_ - falloffStart_ < MIN_ANGLE_DIFFERENCE) {
		falloffStart_ = angle_ - MIN_ANGLE_DIFFERENCE;
		// falloffStartが負にならないように
		if (falloffStart_ < 0.0f) {
			falloffStart_ = 0.0f;
			angle_ = MIN_ANGLE_DIFFERENCE;
		}
		lightData_.cosFalloffStart = std::cos(DegToRad(falloffStart_));
	}

	// cos値を更新
	lightData_.cosAngle = std::cos(DegToRad(angle_));
}

void SpotLight::SetFalloffStart(float falloffStart)
{
	falloffStart_ = falloffStart;

	// angleがfalloffStartより大きく（外側に）なるように自動調整
	if (angle_ - falloffStart_ < MIN_ANGLE_DIFFERENCE) {
		angle_ = falloffStart_ + MIN_ANGLE_DIFFERENCE;
		lightData_.cosAngle = std::cos(DegToRad(angle_));
	}

	// cos値を更新
	lightData_.cosFalloffStart = std::cos(DegToRad(falloffStart_));
}

void SpotLight::UpdateDirection()
{
	// 回転行列を作成（度数法をラジアンに変換）
	Vector3 rotationRad = {
		DegToRad(rotation_.x),
		DegToRad(rotation_.y),
		DegToRad(rotation_.z)
	};

	Matrix4x4 rotationMatrix = MakeRotateXYZMatrix(rotationRad);

	// 初期方向ベクトル（Z軸正方向）を回転
	Vector3 forward = { 0.0f, 0.0f, 1.0f };
	lightData_.direction = Normalize(Transform(forward, rotationMatrix));
}


void SpotLight::DebugLineAdd()
{
#ifdef USEIMGUI
	// DebugDrawLineSystemのインスタンスを取得
	DebugDrawLineSystem* debugDrawLineSystem = Engine::GetInstance()->GetDebugDrawManager();
	if (!debugDrawLineSystem) {
		return;
	}

	// スポットライトの形にラインを描画
	debugDrawLineSystem->DrawCone(
		lightData_.position,
		lightData_.direction,
		lightData_.distance,
		DegToRad(angle_),
		{ 1.0f, 1.0f, 1.0f, 1.0f }, // 黄色
		12);
#endif

}

void SpotLight::ImGui(const std::string& label)
{
#ifdef USEIMGUI

	if (ImGui::TreeNode(label.c_str())) {
		// 有効/無効切り替え
		ImGui::Checkbox("Active", &isActive_);

		ImGui::Separator();

		// ライトの色
		if (ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&lightData_.color.x))) {

		}

		// ライトの位置
		if (ImGui::DragFloat3("Position", &lightData_.position.x, 0.1f)) {

		}

		// ライトの回転（Euler角、度数法）
		Vector3 rotationDeg = rotation_;
		if (ImGui::DragFloat3("Rotation (deg)", &rotationDeg.x, 1.0f, -180.0f, 180.0f)) {
			SetRotation(rotationDeg);
		}

		// 現在の方向ベクトルを表示（読み取り専用）
		ImGui::Text("Direction: (%.3f, %.3f, %.3f)",
			lightData_.direction.x,
			lightData_.direction.y,
			lightData_.direction.z);

		ImGui::Separator();

		// ライトの強度
		if (ImGui::DragFloat("Intensity", &lightData_.intensity, 0.01f, 0.0f, 10.0f)) {

		}

		// 最大距離
		if (ImGui::DragFloat("Distance", &lightData_.distance, 0.1f, 0.1f, 100.0f)) {

		}

		// 減衰率
		if (ImGui::DragFloat("Decay", &lightData_.decay, 0.1f, 0.1f, 10.0f)) {

		}

		ImGui::Separator();

		// スポット角度（外側の境界、度数法）
		if (ImGui::SliderFloat("Spot Angle (outer, deg)", &angle_, 1.0f, 90.0f)) {
			SetAngle(angle_);
		}

		// フォールオフ開始角度（内側の境界、度数法）
		if (ImGui::SliderFloat("Falloff Start (inner, deg)", &falloffStart_, 0.0f, 89.0f)) {
			SetFalloffStart(falloffStart_);
		}

		// 説明テキスト
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
			"Note: inner < outer (falloffStart < angle)");

		// cos値を表示（読み取り専用）
		ImGui::Text("Cos Angle (outer): %.3f", lightData_.cosAngle);
		ImGui::Text("Cos Falloff Start (inner): %.3f", lightData_.cosFalloffStart);

		ImGui::Separator();

		// ライトの種類表示（読み取り専用）
		ImGui::Text("Type: Spot");

		ImGui::TreePop();
	}

#endif
}