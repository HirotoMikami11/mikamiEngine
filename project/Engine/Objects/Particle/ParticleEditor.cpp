#include "ParticleEditor.h"
#include "AccelerationField.h"
#include "GravityField.h"
#include "Logger.h"
#include "ImGui/ImGuiManager.h"
#include <format>
#include <fstream>

ParticleEditor* ParticleEditor::GetInstance()
{
	static ParticleEditor instance;
	return &instance;
}

void ParticleEditor::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
	particleSystem_ = ParticleSystem::GetInstance();

	// プリセットディレクトリを作成（存在しない場合）
	std::filesystem::path dir = kPresetDirectory_;
	if (!std::filesystem::exists(dir)) {
		std::filesystem::create_directories(dir);
		Logger::Log(Logger::GetStream(),
			std::format("[ParticleEditor] Created preset directory: {}\n", kPresetDirectory_));
	}

	Logger::Log(Logger::GetStream(), "[ParticleEditor] Initialized\n");
}

void ParticleEditor::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::Begin("Particle Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Preset Manager");
		ImGui::Separator();

		// 利用可能なプリセット一覧
		std::vector<std::string> presets = GetAvailablePresets();

		ImGui::Text("Available Presets: %zu", presets.size());

		static int selectedPreset = -1;
		static char newPresetName[128] = "";

		// プリセット選択コンボボックス
		if (ImGui::BeginCombo("Select Preset", selectedPreset >= 0 && selectedPreset < presets.size()
			? presets[selectedPreset].c_str() : "None")) {
			for (int i = 0; i < presets.size(); ++i) {
				bool isSelected = (selectedPreset == i);
				if (ImGui::Selectable(presets[i].c_str(), isSelected)) {
					selectedPreset = i;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Separator();

		// 新しいプリセット名入力
		ImGui::InputText("Preset Name", newPresetName, IM_ARRAYSIZE(newPresetName));

		// 保存ボタン
		if (ImGui::Button("Save Current State", ImVec2(200, 0))) {
			if (strlen(newPresetName) > 0) {
				if (SaveCurrentStateAsPreset(newPresetName)) {
					Logger::Log(Logger::GetStream(),
						std::format("[ParticleEditor] Saved preset: {}\n", newPresetName));
					memset(newPresetName, 0, sizeof(newPresetName));
				}
			} else {
				Logger::Log(Logger::GetStream(),
					"[ParticleEditor] Please enter a preset name\n");
			}
		}

		ImGui::Separator();

		// インスタンス作成
		static char instanceName[128] = "";
		ImGui::InputText("Instance Name", instanceName, IM_ARRAYSIZE(instanceName));

		if (ImGui::Button("Create Instance", ImVec2(200, 0))) {
			if (selectedPreset >= 0 && selectedPreset < presets.size() && strlen(instanceName) > 0) {
				auto* instance = CreateInstance(presets[selectedPreset], instanceName);
				if (instance) {
					Logger::Log(Logger::GetStream(),
						std::format("[ParticleEditor] Created instance: {}\n", instanceName));
					memset(instanceName, 0, sizeof(instanceName));
				}
			}
		}

		ImGui::Separator();

		// 現在のインスタンス一覧
		ImGui::Text("Active Instances: %zu", instances_.size());

		if (ImGui::TreeNode("Instance List")) {
			for (auto& [name, instance] : instances_) {
				ImGui::BulletText("%s", name.c_str());
			}
			ImGui::TreePop();
		}

		// プリセット削除ボタン
		if (selectedPreset >= 0 && selectedPreset < presets.size()) {
			ImGui::Separator();
			if (ImGui::Button("Delete Selected Preset", ImVec2(200, 0))) {
				if (DeletePreset(presets[selectedPreset])) {
					Logger::Log(Logger::GetStream(),
						std::format("[ParticleEditor] Deleted preset: {}\n", presets[selectedPreset]));
					selectedPreset = -1;
				}
			}
		}
	}
	ImGui::End();
#endif
}

std::string ParticleEditor::GetPresetFilePath(const std::string& presetName) const
{
	return std::string(kPresetDirectory_) + presetName + ".json";
}

std::string ParticleEditor::MakeUniqueName(const std::string& instanceName, const std::string& localName) const
{
	return instanceName + "_" + localName;
}

BaseField* ParticleEditor::CreateFieldByTypeName(const std::string& typeName, const std::string& fieldName)
{
	if (typeName == "AccelerationField") {
		return particleSystem_->CreateField<AccelerationField>(fieldName);
	} else if (typeName == "GravityField") {
		return particleSystem_->CreateField<GravityField>(fieldName);
	}

	Logger::Log(Logger::GetStream(),
		std::format("[ParticleEditor] Unknown field type: {}\n", typeName));
	return nullptr;
}

std::vector<std::string> ParticleEditor::GetAvailablePresets() const
{
	std::vector<std::string> presets;
	std::filesystem::path dir = kPresetDirectory_;

	if (!std::filesystem::exists(dir)) {
		return presets;
	}

	for (const auto& entry : std::filesystem::directory_iterator(dir)) {
		if (entry.is_regular_file() && entry.path().extension() == ".json") {
			presets.push_back(entry.path().stem().string());
		}
	}

	return presets;
}

bool ParticleEditor::DeletePreset(const std::string& presetName)
{
	std::string filePath = GetPresetFilePath(presetName);

	try {
		if (std::filesystem::exists(filePath)) {
			std::filesystem::remove(filePath);
			Logger::Log(Logger::GetStream(),
				std::format("[ParticleEditor] Deleted preset file: {}\n", filePath));
			return true;
		} else {
			Logger::Log(Logger::GetStream(),
				std::format("[ParticleEditor] Preset file not found: {}\n", filePath));
			return false;
		}
	} catch (const std::exception& e) {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticleEditor] Exception while deleting preset: {}\n", e.what()));
		return false;
	}
}
// ParticleEditor.cpp の続き（パート2）

bool ParticleEditor::SaveCurrentStateAsPreset(const std::string& presetName)
{
	ParticlePresetData preset;
	preset.presetName = presetName;

	// 全グループを保存
	for (const std::string& groupName : particleSystem_->GetAllGroupNames()) {
		ParticleGroup* group = particleSystem_->GetGroup(groupName);
		if (!group) continue;

		ParticleGroupData groupData;
		groupData.groupName = groupName;
		groupData.modelTag = group ? group->GetModelTag() : "";
		groupData.maxParticles = group->GetMaxParticleCount();
		groupData.textureName = group->GetTextureName();
		groupData.useBillboard = group->UseBillboard();

		preset.groups.push_back(groupData);
	}

	// 全エミッターを保存
	for (const std::string& emitterName : particleSystem_->GetAllEmitterNames()) {
		ParticleEmitter* emitter = particleSystem_->GetEmitter(emitterName);
		if (!emitter) continue;

		ParticleEmitterData emitterData;
		emitterData.emitterName = emitterName;
		emitterData.targetGroupName = emitter->GetTargetGroupName();

		// Transform
		emitterData.position = emitter->GetTransform().GetPosition();
		emitterData.rotation = emitter->GetTransform().GetRotation();
		emitterData.scale = emitter->GetTransform().GetScale();

		// Emit設定
		emitterData.emitCount = emitter->GetEmitCount();
		emitterData.emitFrequency = emitter->GetFrequency();
		emitterData.isEmitting = emitter->IsEmitting();

		// パーティクル寿命（GetterがないのでEmitterから直接は取得できないため、デフォルト値を使用）
		// 注: 実際の実装では、Emitterにゲッターを追加するか、アクセス可能にする必要があります
		emitterData.particleLifeTimeMin = 1.0f;  // TODO: Getterを追加
		emitterData.particleLifeTimeMax = 3.0f;  // TODO: Getterを追加

		// 速度設定
		emitterData.emitDirection = emitter->GetEmitDirection();
		emitterData.initialSpeed = emitter->GetInitialSpeed();
		emitterData.spreadAngle = emitter->GetSpreadAngle();
		emitterData.useDirectionalEmit = emitter->IsUseDirectionalEmit();
		emitterData.velocityRange = 1.0f;  // TODO: Getterを追加

		// スケール・回転（TODO: Getterを追加）
		emitterData.particleScaleMin = { 1.0f, 1.0f, 1.0f };
		emitterData.particleScaleMax = { 1.0f, 1.0f, 1.0f };
		emitterData.particleRotateMin = { 0.0f, 0.0f, 0.0f };
		emitterData.particleRotateMax = { 0.0f, 0.0f, 0.0f };

		// エミッター寿命
		emitterData.emitterLifeTime = emitter->GetEmitterLifeTime();
		emitterData.emitterLifeTimeLoop = emitter->IsEmitterLifeTimeLoop();
		emitterData.useEmitterLifeTime = emitter->IsUseEmitterLifeTime();

		// 発生範囲
		emitterData.spawnArea = emitter->GetSpawnArea();

		// デバッグ
		emitterData.showDebugAABB = emitter->IsShowDebugAABB();
		emitterData.debugAABBColor = { 1.0f, 0.0f, 0.0f, 1.0f };  // TODO: Getterを追加

		preset.emitters.push_back(emitterData);
	}

	// 全フィールドを保存
	for (const std::string& fieldName : particleSystem_->GetAllFieldNames()) {
		BaseField* field = particleSystem_->GetField(fieldName);
		if (!field) continue;

		ParticleFieldData fieldData;
		fieldData.fieldName = fieldName;
		fieldData.fieldType = field->GetTypeName();

		// Transform
		fieldData.position = field->GetTransform().GetPosition();
		fieldData.rotation = field->GetTransform().GetRotation();
		fieldData.scale = field->GetTransform().GetScale();

		// 共通設定
		fieldData.isEnabled = field->IsEnabled();
		fieldData.showDebugVisualization = field->IsShowDebugVisualization();
		fieldData.debugColor = field->GetDebugColor();

		// フィールド固有のパラメータ
		fieldData.parameters = field->SerializeParameters();

		preset.fields.push_back(fieldData);
	}

	// ファイルに保存
	std::string filePath = GetPresetFilePath(presetName);
	return preset.SaveToFile(filePath);
}

ParticlePresetData ParticleEditor::LoadPreset(const std::string& presetName)
{
	std::string filePath = GetPresetFilePath(presetName);
	return ParticlePresetData::LoadFromFile(filePath);
}
// ParticleEditor.cpp の続き（パート3）

ParticlePresetInstance* ParticleEditor::CreateInstance(const std::string& presetName, const std::string& instanceName)
{
	// 既に同じ名前のインスタンスが存在するかチェック
	if (instances_.find(instanceName) != instances_.end()) {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticleEditor] Instance '{}' already exists!\n", instanceName));
		return nullptr;
	}

	// プリセットを読み込み
	ParticlePresetData presetData = LoadPreset(presetName);
	if (presetData.presetName.empty()) {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticleEditor] Failed to load preset: {}\n", presetName));
		return nullptr;
	}

	// インスタンスを作成
	auto instance = std::make_unique<ParticlePresetInstance>();
	instance->Initialize(instanceName, particleSystem_);

	// グループを作成
	for (const auto& groupData : presetData.groups) {
		std::string uniqueName = MakeUniqueName(instanceName, groupData.groupName);

		bool success = particleSystem_->CreateGroup(
			uniqueName,
			groupData.modelTag,
			groupData.maxParticles,
			groupData.textureName,
			groupData.useBillboard
		);

		if (success) {
			instance->RegisterGroup(groupData.groupName, uniqueName);
		} else {
			Logger::Log(Logger::GetStream(),
				std::format("[ParticleEditor] Failed to create group: {}\n", uniqueName));
		}
	}

	// エミッターを作成
	for (const auto& emitterData : presetData.emitters) {
		std::string uniqueName = MakeUniqueName(instanceName, emitterData.emitterName);
		std::string targetGroupUniqueName = MakeUniqueName(instanceName, emitterData.targetGroupName);

		ParticleEmitter* emitter = particleSystem_->CreateEmitter(uniqueName, targetGroupUniqueName);
		if (emitter) {
			// Transform設定
			emitter->GetTransform().SetPosition(emitterData.position);
			emitter->GetTransform().SetRotation(emitterData.rotation);
			emitter->GetTransform().SetScale(emitterData.scale);

			// Emit設定
			emitter->SetEmitCount(emitterData.emitCount);
			emitter->SetFrequency(emitterData.emitFrequency);
			emitter->SetEmitEnabled(emitterData.isEmitting);

			// パーティクル寿命
			emitter->SetParticleLifeTimeRange(emitterData.particleLifeTimeMin, emitterData.particleLifeTimeMax);

			// 速度設定
			emitter->SetUseDirectionalEmit(emitterData.useDirectionalEmit);
			if (emitterData.useDirectionalEmit) {
				emitter->SetEmitDirection(emitterData.emitDirection);
				emitter->SetInitialSpeed(emitterData.initialSpeed);
				emitter->SetSpreadAngle(emitterData.spreadAngle);
			} else {
				emitter->SetParticleVelocityRange(emitterData.velocityRange);
			}

			// スケール設定
			emitter->SetParticleScaleRange(emitterData.particleScaleMin, emitterData.particleScaleMax);

			// 回転設定
			emitter->SetParticleRotateRange(emitterData.particleRotateMin, emitterData.particleRotateMax);

			// エミッター寿命
			emitter->SetUseEmitterLifeTime(emitterData.useEmitterLifeTime);
			emitter->SetEmitterLifeTime(emitterData.emitterLifeTime);
			emitter->SetEmitterLifeTimeLoop(emitterData.emitterLifeTimeLoop);

			// 発生範囲
			emitter->SetSpawnArea(emitterData.spawnArea);

			// デバッグ
			emitter->SetShowDebugAABB(emitterData.showDebugAABB);
			emitter->SetDebugAABBColor(emitterData.debugAABBColor);

			instance->RegisterEmitter(emitterData.emitterName, uniqueName);
		} else {
			Logger::Log(Logger::GetStream(),
				std::format("[ParticleEditor] Failed to create emitter: {}\n", uniqueName));
		}
	}

	// フィールドを作成
	for (const auto& fieldData : presetData.fields) {
		std::string uniqueName = MakeUniqueName(instanceName, fieldData.fieldName);

		BaseField* field = CreateFieldByTypeName(fieldData.fieldType, uniqueName);
		if (field) {
			// Transform設定
			field->GetTransform().SetPosition(fieldData.position);
			field->GetTransform().SetRotation(fieldData.rotation);
			field->GetTransform().SetScale(fieldData.scale);

			// 共通設定
			field->SetEnabled(fieldData.isEnabled);
			field->SetShowDebugVisualization(fieldData.showDebugVisualization);
			field->SetDebugColor(fieldData.debugColor);

			// フィールド固有のパラメータ
			field->DeserializeParameters(fieldData.parameters);

			instance->RegisterField(fieldData.fieldName, uniqueName);
		} else {
			Logger::Log(Logger::GetStream(),
				std::format("[ParticleEditor] Failed to create field: {}\n", uniqueName));
		}
	}

	// インスタンスを登録
	ParticlePresetInstance* instancePtr = instance.get();
	instances_[instanceName] = std::move(instance);

	Logger::Log(Logger::GetStream(),
		std::format("[ParticleEditor] Successfully created instance '{}' from preset '{}'\n",
			instanceName, presetName));

	return instancePtr;
}

ParticlePresetInstance* ParticleEditor::GetInstance(const std::string& instanceName)
{
	auto it = instances_.find(instanceName);
	if (it != instances_.end()) {
		return it->second.get();
	}
	return nullptr;
}

void ParticleEditor::DestroyInstance(const std::string& instanceName)
{
	auto it = instances_.find(instanceName);
	if (it != instances_.end()) {
		it->second->Destroy();
		instances_.erase(it);
		Logger::Log(Logger::GetStream(),
			std::format("[ParticleEditor] Destroyed instance: {}\n", instanceName));
	}
}