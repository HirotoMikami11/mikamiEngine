#define NOMINMAX
#include "LightManager.h"
#include "ImGui/ImGuiManager.h"
#include <format>
#include <algorithm>

LightManager* LightManager::GetInstance()
{
	static LightManager instance;
	return &instance;
}

void LightManager::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;

	// LightingData用のリソースを作成
	lightingResource_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(LightingData));

	// LightingDataにマップ
	lightingResource_->Map(0, nullptr, reinterpret_cast<void**>(&lightingData_));

	// デフォルト状態で初期化
	ResetToDefault();

	Logger::Log(Logger::GetStream(), "LightManager: Initialized successfully\n");
}

void LightManager::ResetToDefault()
{
	// 平行光源をデフォルト設定で初期化
	directionalLight_.SetDefaultSettings();

	// 全ポイントライトをクリア
	ClearPointLights();

	// LightingDataを更新
	UpdateLightingData();

	Logger::Log(Logger::GetStream(), "LightManager: Reset to default (Directional light only)\n");
}

void LightManager::Update()
{
	// LightingDataを更新
	UpdateLightingData();
}

PointLight* LightManager::AddPointLight(
	const Vector3& position,
	const Vector4& color,
	float intensity,
	float radius,
	float decay)
{
	// 上限チェック
	if (pointLights_.size() >= MAX_POINT_LIGHTS) {
		Logger::Log(Logger::GetStream(),
			std::format("LightManager: Cannot add point light - max limit reached ({}/{})\n",
				MAX_POINT_LIGHTS, MAX_POINT_LIGHTS));
		return nullptr;
	}

	// ID割り当て（単調増加）
	uint32_t id = nextLightID_++;

	// ライト作成
	auto light = std::make_unique<PointLight>();
	light->Initialize(position, color, intensity, radius, decay);
	light->lightID_ = id;  // 内部管理用IDを設定

	PointLight* ptr = light.get();
	pointLights_[id] = std::move(light);

	Logger::Log(Logger::GetStream(),
		std::format("LightManager: Added point light ID={} ({}/{})\n",
			id, pointLights_.size(), MAX_POINT_LIGHTS));

	return ptr;
}

void LightManager::RemovePointLight(PointLight* light)
{
	if (!light) {
		Logger::Log(Logger::GetStream(),
			"LightManager: Warning - Attempted to remove nullptr light\n");
		return;
	}

	uint32_t id = light->lightID_;

	// ID検証
	auto it = pointLights_.find(id);
	if (it == pointLights_.end()) {
		Logger::Log(Logger::GetStream(),
			std::format("LightManager: Warning - Light ID={} not found (possibly already deleted)\n", id));
		return;
	}

	// ポインタ一致確認（安全性チェック）
	if (it->second.get() != light) {
		Logger::Log(Logger::GetStream(),
			std::format("LightManager: Error - Pointer mismatch for ID={}\n", id));
		return;
	}

	pointLights_.erase(it);

	Logger::Log(Logger::GetStream(),
		std::format("LightManager: Removed light ID={} ({}/{})\n",
			id, pointLights_.size(), MAX_POINT_LIGHTS));
}

void LightManager::ClearPointLights()
{
	pointLights_.clear();
	Logger::Log(Logger::GetStream(), "LightManager: Cleared all point lights\n");
}

void LightManager::ImGui()
{
#ifdef USEIMGUI

	if (ImGui::TreeNode("Light Manager")) {
		// 平行光源
		directionalLight_.ImGui("Directional Light");

		ImGui::Separator();

		// 使用状況表示
		int activeCount = 0;
		for (const auto& [id, light] : pointLights_) {
			if (light && light->IsActive()) {
				activeCount++;
			}
		}

		ImGui::Text("Point Lights: %d / %d (Active: %d)",
			static_cast<int>(pointLights_.size()),
			MAX_POINT_LIGHTS,
			activeCount);

		ImGui::Separator();

		// 削除対象を記録（ループ中に削除できないため）
		PointLight* lightToRemove = nullptr;

		// 使用中のポイントライトのみ表示
		for (auto& [id, light] : pointLights_) {
			if (light) {
				std::string label = std::format("Point Light [ID:{}]", id);

				if (ImGui::TreeNode(label.c_str())) {
					// ライトの編集UI
					light->ImGui(label);

					// 削除ボタン
					ImGui::Separator();
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));

					if (ImGui::Button("Remove This Light", ImVec2(-1, 0))) {
						lightToRemove = light.get();
					}

					ImGui::PopStyleColor(3);

					ImGui::TreePop();
				}
			}
		}

		// ループ外で削除（イテレーション中の削除を避ける）
		if (lightToRemove) {
			RemovePointLight(lightToRemove);
		}

		ImGui::Separator();

		// 追加ボタン（上限チェック）
		if (pointLights_.size() >= MAX_POINT_LIGHTS) {
			ImGui::BeginDisabled();
		}

		if (ImGui::Button("Add Point Light", ImVec2(-1, 0))) {
			AddPointLight({ 0.0f, 2.0f, 0.0f });
		}

		if (pointLights_.size() >= MAX_POINT_LIGHTS) {
			ImGui::EndDisabled();
			ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
				"Maximum light limit reached");
		}

		ImGui::TreePop();
	}

#endif
}

void LightManager::Finalize()
{
	// ComPtrのリセット
	lightingResource_.Reset();
	Logger::Log(Logger::GetStream(), "LightManager: Finalized successfully\n");
}

void LightManager::UpdateLightingData()
{
	if (!lightingData_) {
		return;
	}

	// 平行光源データをコピー
	lightingData_->directionalLight = directionalLight_.GetData();

	// アクティブなポイントライトを収集
	std::vector<PointLight*> activeLights;
	activeLights.reserve(pointLights_.size());

	for (auto& [id, light] : pointLights_) {
		if (light && light->IsActive()) {
			activeLights.push_back(light.get());
		}
	}

	// GPU送信は最大10個まで
	int gpuLightCount = std::min(static_cast<int>(activeLights.size()), MAX_POINT_LIGHTS);

	// GPU配列にコピー
	for (int i = 0; i < gpuLightCount; ++i) {
		lightingData_->pointLights[i] = activeLights[i]->GetData();
	}

	// 有効なポイントライト数を設定
	lightingData_->numPointLights = gpuLightCount;

	// パディングをゼロで埋める
	lightingData_->padding[0] = 0.0f;
	lightingData_->padding[1] = 0.0f;
	lightingData_->padding[2] = 0.0f;

	// 警告：10個を超えた場合（通常は発生しない）
	if (activeLights.size() > MAX_POINT_LIGHTS) {
		static bool warned = false;
		if (!warned) {
			Logger::Log(Logger::GetStream(),
				std::format("LightManager: Warning - {} active lights, but only {} sent to GPU\n",
					activeLights.size(), MAX_POINT_LIGHTS));
			warned = true;
		}
	}
}