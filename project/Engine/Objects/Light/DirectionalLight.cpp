#include "DirectionalLight.h"
#include "ImGui/ImGuiManager.h" 

void DirectionalLight::Initialize(DirectXCommon* dxCommon)
{
	// デフォルト設定で初期化
	SetDefaultSettings();
}

void DirectionalLight::SetDefaultSettings()
{
	// デフォルト設定
	lightData_.color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白色
	lightData_.direction = { 0.0f, -1.0f, 0.0f }; // 上から下方向
	lightData_.intensity = 1.0f; // 強度1.0
}

void DirectionalLight::ImGui(const std::string& label)
{
#ifdef USEIMGUI

	if (ImGui::TreeNode(label.c_str())) {
		// ライトの色
		if (ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&lightData_.color.x))) {

		}

		// ライトの方向
		if (ImGui::DragFloat3("Direction", &lightData_.direction.x, 0.01f, -1.0f, 1.0f)) {

		}

		// ライトの強度
		if (ImGui::DragFloat("Intensity", &lightData_.intensity, 0.01f, 0.0f, 10.0f)) {

		}

		// ライトの種類表示（読み取り専用）
		ImGui::Text("Type: Directional");

		ImGui::TreePop();
	}

#endif
}
