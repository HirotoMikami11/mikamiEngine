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

	// 全スポットライトをクリア
	ClearSpotLights();

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

SpotLight* LightManager::AddSpotLight(
	const Vector3& position,
	const Vector3& rotation,
	const Vector4& color,
	float intensity,
	float distance,
	float decay,
	float angle,
	float falloffStart)
{
	// 上限チェック
	if (spotLights_.size() >= MAX_SPOT_LIGHTS) {
		return nullptr;
	}

	// ID割り当て（単調増加）
	uint32_t id = nextLightID_++;

	// ライト作成
	auto light = std::make_unique<SpotLight>();
	light->Initialize(position, rotation, color, intensity, distance, decay, angle, falloffStart);
	light->lightID_ = id;  // 内部管理用IDを設定

	SpotLight* ptr = light.get();
	spotLights_[id] = std::move(light);

	Logger::Log(Logger::GetStream(),
		std::format("LightManager: Added spot light ID={} ({}/{})\n",
			id, spotLights_.size(), MAX_SPOT_LIGHTS));

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
		return;
	}

	// ポインタ一致確認（安全性チェック）
	if (it->second.get() != light) {
		return;
	}

	pointLights_.erase(it);
}

void LightManager::RemoveSpotLight(SpotLight* light)
{
	if (!light) {
		Logger::Log(Logger::GetStream(),
			"LightManager: Warning - Attempted to remove nullptr light\n");
		return;
	}

	uint32_t id = light->lightID_;

	// ID検証
	auto it = spotLights_.find(id);
	if (it == spotLights_.end()) {
		return;
	}

	// ポインタ一致確認（安全性チェック）
	if (it->second.get() != light) {
		return;
	}

	spotLights_.erase(it);
}

void LightManager::ClearPointLights()
{
	pointLights_.clear();
	Logger::Log(Logger::GetStream(), "LightManager: Cleared all point lights\n");
}

void LightManager::ClearSpotLights()
{
	spotLights_.clear();
	Logger::Log(Logger::GetStream(), "LightManager: Cleared all spot lights\n");
}

void LightManager::ImGui()
{
#ifdef USEIMGUI

	ImGui::Begin("Light Manager");

	// 平行光源
	directionalLight_.ImGui("Directional Light");

	ImGui::Separator();

	// ===== ポイントライト =====
	// 使用状況表示
	int activePointLights = 0;
	for (const auto& [id, light] : pointLights_) {
		if (light && light->IsActive()) {
			activePointLights++;
		}
	}

	ImGui::Text("Point Lights: %d / %d (Active: %d)",
		static_cast<int>(pointLights_.size()),
		MAX_POINT_LIGHTS,
		activePointLights);

	ImGui::Separator();

	// 削除対象を記録（ループ中に削除できないため）
	PointLight* pointLightToRemove = nullptr;

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
					pointLightToRemove = light.get();
				}

				ImGui::PopStyleColor(3);

				ImGui::TreePop();
			}
		}
	}

	// ループ外で削除（イテレーション中の削除を避ける）
	if (pointLightToRemove) {
		RemovePointLight(pointLightToRemove);
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
			"Maximum point light limit reached");
	}

	ImGui::Separator();
	ImGui::Spacing();

	// ===== スポットライト =====
	// 使用状況表示
	int activeSpotLights = 0;
	for (const auto& [id, light] : spotLights_) {
		if (light && light->IsActive()) {
			activeSpotLights++;
		}
	}

	ImGui::Text("Spot Lights: %d / %d (Active: %d)",
		static_cast<int>(spotLights_.size()),
		MAX_SPOT_LIGHTS,
		activeSpotLights);

	ImGui::Separator();

	// 削除対象を記録（ループ中に削除できないため）
	SpotLight* spotLightToRemove = nullptr;

	// 使用中のスポットライトのみ表示
	for (auto& [id, light] : spotLights_) {
		if (light) {
			std::string label = std::format("Spot Light [ID:{}]", id);

			if (ImGui::TreeNode(label.c_str())) {
				// ライトの編集UI
				light->ImGui(label);

				// 削除ボタン
				ImGui::Separator();
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));

				if (ImGui::Button("Remove This Light", ImVec2(-1, 0))) {
					spotLightToRemove = light.get();
				}

				ImGui::PopStyleColor(3);

				ImGui::TreePop();
			}
		}
	}

	// ループ外で削除（イテレーション中の削除を避ける）
	if (spotLightToRemove) {
		RemoveSpotLight(spotLightToRemove);
	}

	ImGui::Separator();

	// 追加ボタン（上限チェック）
	if (spotLights_.size() >= MAX_SPOT_LIGHTS) {
		ImGui::BeginDisabled();
	}

	if (ImGui::Button("Add Spot Light", ImVec2(-1, 0))) {
		AddSpotLight({ 0.0f, 5.0f, 0.0f }, { 90.0f, 0.0f, 0.0f });
	}

	if (spotLights_.size() >= MAX_SPOT_LIGHTS) {
		ImGui::EndDisabled();
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
			"Maximum spot light limit reached");
	}

	ImGui::End();

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
	std::vector<PointLight*> activePointLights;
	activePointLights.reserve(pointLights_.size());

	for (auto& [id, light] : pointLights_) {
		if (light && light->IsActive()) {
			activePointLights.push_back(light.get());
		}
	}

	// GPU送信上限数まで
	int gpuPointLightCount = std::min(static_cast<int>(activePointLights.size()), MAX_POINT_LIGHTS);

	// GPU配列にコピー
	for (int i = 0; i < gpuPointLightCount; ++i) {
		lightingData_->pointLights[i] = activePointLights[i]->GetData();
	}

	// 有効なポイントライト数を設定
	lightingData_->numPointLights = gpuPointLightCount;

	// アクティブなスポットライトを収集
	std::vector<SpotLight*> activeSpotLights;
	activeSpotLights.reserve(spotLights_.size());

	for (auto& [id, light] : spotLights_) {
		if (light && light->IsActive()) {
			activeSpotLights.push_back(light.get());
		}
	}

	// GPU送信上限数まで
	int gpuSpotLightCount = std::min(static_cast<int>(activeSpotLights.size()), MAX_SPOT_LIGHTS);

	// GPU配列にコピー
	for (int i = 0; i < gpuSpotLightCount; ++i) {
		lightingData_->spotLights[i] = activeSpotLights[i]->GetData();
	}

	// 有効なスポットライト数を設定
	lightingData_->numSpotLights = gpuSpotLightCount;

	// パディングをゼロで埋める
	lightingData_->padding1[0] = 0.0f;
	lightingData_->padding1[1] = 0.0f;
	lightingData_->padding1[2] = 0.0f;
	lightingData_->padding2[0] = 0.0f;
	lightingData_->padding2[1] = 0.0f;
	lightingData_->padding2[2] = 0.0f;

	//それぞれ最大数を超えた場合の警告
	if (activePointLights.size() > MAX_POINT_LIGHTS) {
		static bool warnedPoint = false;
		if (!warnedPoint) {
			Logger::Log(Logger::GetStream(),
				std::format("LightManager: Warning - {} active point lights, but only {} sent to GPU\n",
					activePointLights.size(), MAX_POINT_LIGHTS));
			warnedPoint = true;
		}
	}

	if (activeSpotLights.size() > MAX_SPOT_LIGHTS) {
		static bool warnedSpot = false;
		if (!warnedSpot) {
			Logger::Log(Logger::GetStream(),
				std::format("LightManager: Warning - {} active spot lights, but only {} sent to GPU\n",
					activeSpotLights.size(), MAX_SPOT_LIGHTS));
			warnedSpot = true;
		}
	}
}