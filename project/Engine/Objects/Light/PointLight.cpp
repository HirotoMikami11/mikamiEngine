#include "PointLight.h"
#include "Engine.h"
#include "ImGui/ImGuiManager.h" 

void PointLight::Initialize(
	const Vector3& position,
	const Vector4& color,
	float intensity,
	float radius,
	float decay)
{
	lightData_.position = position;
	lightData_.color = color;
	lightData_.intensity = intensity;
	lightData_.radius = radius;
	lightData_.decay = decay;
	lightData_.padding[0] = 0.0f;
	lightData_.padding[1] = 0.0f;
	isActive_ = true;
}

void PointLight::SetDefaultSettings()
{
	// デフォルト設定
	lightData_.color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白色
	lightData_.position = { 0.0f, 2.0f, 0.0f }; // 原点から少し上
	lightData_.intensity = 1.0f; // 強度1.0
	lightData_.radius = 10.0f; // 影響範囲10.0
	lightData_.decay = 2.0f; // 減衰率2.0
	lightData_.padding[0] = 0.0f;
	lightData_.padding[1] = 0.0f;
	isActive_ = true;
}

void PointLight::ImGui(const std::string& label)
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

		// ライトの強度
		if (ImGui::DragFloat("Intensity", &lightData_.intensity, 0.01f, 0.0f, 10.0f)) {

		}

		// 影響範囲
		if (ImGui::DragFloat("Radius", &lightData_.radius, 0.1f, 0.1f, 100.0f)) {

		}

		// 減衰率
		if (ImGui::DragFloat("Decay", &lightData_.decay, 0.1f, 0.1f, 10.0f)) {

		}

		// ライトの種類表示（読み取り専用）
		ImGui::Text("Type: Point");

		ImGui::TreePop();
	}

#endif
}

void PointLight::DebugLineAdd()
{
#ifdef USEIMGUI
	// DebugDrawLineSystemのインスタンスを取得
	DebugDrawLineSystem* debugDrawLineSystem = Engine::GetInstance()->GetDebugDrawManager();
	if (!debugDrawLineSystem) {
		return;
	}

	// スポットライトの形にラインを描画
	debugDrawLineSystem->DrawSphere(
		lightData_.position,
		lightData_.radius,
		{ 1.0f, 1.0f, 1.0f, 1.0f },
		12);
#endif
}
