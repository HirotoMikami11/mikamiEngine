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
	if (ImGui::Begin("Particle Editor", nullptr, ImGuiWindowFlags_None)) {
		// タブバー
		if (ImGui::BeginTabBar("EditorTabs", ImGuiTabBarFlags_None)) {

			// Createタブ
			if (ImGui::BeginTabItem("Create")) {
				ShowCreateTab();
				ImGui::EndTabItem();
			}

			// Editタブ
			if (ImGui::BeginTabItem("Edit")) {
				ShowEditTab();
				ImGui::EndTabItem();
			}

			// Presetsタブ
			if (ImGui::BeginTabItem("Presets")) {
				ShowPresetsTab();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}
	ImGui::End();

	// ダイアログ表示（モーダル）
	if (showCreateGroupDialog_) {
		ShowCreateGroupDialog();
	}
	if (showCreateEmitterDialog_) {
		ShowCreateEmitterDialog();
	}
	if (showCreateFieldDialog_) {
		ShowCreateFieldDialog();
	}
#endif
}

// ========================================
// Createタブ
// ========================================

void ParticleEditor::ShowCreateTab()
{
#ifdef USEIMGUI
	ImGui::Text("Create New Objects");
	ImGui::Separator();
	ImGui::Spacing();

	// Create Group ボタン
	if (ImGui::Button("+ Create Group", ImVec2(250, 40))) {
		showCreateGroupDialog_ = true;
		// ダイアログをリセット
		strcpy_s(groupCreationData_.name, "NewGroup");
		strcpy_s(groupCreationData_.modelTag, "plane");
		strcpy_s(groupCreationData_.textureName, "circle");
		groupCreationData_.maxParticles = 100;
		groupCreationData_.useBillboard = true;
	}
	ImGui::Spacing();

	// Create Emitter ボタン
	if (ImGui::Button("+ Create Emitter", ImVec2(250, 40))) {
		showCreateEmitterDialog_ = true;
		// ダイアログをリセット
		strcpy_s(emitterCreationData_.name, "NewEmitter");
		emitterCreationData_.selectedGroupIndex = 0;
	}
	ImGui::Spacing();

	// Create Field ボタン
	if (ImGui::Button("+ Create Field", ImVec2(250, 40))) {
		showCreateFieldDialog_ = true;
		// ダイアログをリセット
		strcpy_s(fieldCreationData_.name, "NewField");
		fieldCreationData_.fieldTypeIndex = 0;
	}

	ImGui::Spacing();
	ImGui::Separator();

	// 統計情報
	ImGui::Text("Current State:");
	ImGui::BulletText("Groups: %zu", particleSystem_->GetGroupCount());
	ImGui::BulletText("Emitters: %zu", particleSystem_->GetEmitterCount());
	ImGui::BulletText("Fields: %zu", particleSystem_->GetFieldCount());
#endif
}

void ParticleEditor::ShowCreateGroupDialog()
{
#ifdef USEIMGUI
	ImGui::OpenPopup("Create Group");

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Create Group", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Create New Particle Group");
		ImGui::Separator();
		ImGui::Spacing();

		// 入力フィールド
		ImGui::InputText("Group Name", groupCreationData_.name, 128);
		ImGui::InputText("Model Tag", groupCreationData_.modelTag, 128);
		ImGui::InputText("Texture", groupCreationData_.textureName, 128);
		ImGui::SliderInt("Max Particles", &groupCreationData_.maxParticles, 10, 1000);
		ImGui::Checkbox("Use Billboard", &groupCreationData_.useBillboard);

		ImGui::Spacing();
		ImGui::Separator();

		// ボタン
		if (ImGui::Button("Create", ImVec2(120, 0))) {
			// グループ作成
			bool success = particleSystem_->CreateGroup(
				groupCreationData_.name,
				groupCreationData_.modelTag,
				groupCreationData_.maxParticles,
				groupCreationData_.textureName,
				groupCreationData_.useBillboard
			);

			if (success) {
				Logger::Log(Logger::GetStream(),
					std::format("[ParticleEditor] Created group: {}\n", groupCreationData_.name));
				showCreateGroupDialog_ = false;
				ImGui::CloseCurrentPopup();
			} else {
				Logger::Log(Logger::GetStream(),
					std::format("[ParticleEditor] Failed to create group: {}\n", groupCreationData_.name));
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			showCreateGroupDialog_ = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
#endif
}

void ParticleEditor::ShowCreateEmitterDialog()
{
#ifdef USEIMGUI
	ImGui::OpenPopup("Create Emitter");

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Create Emitter", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Create New Particle Emitter");
		ImGui::Separator();
		ImGui::Spacing();

		// エミッター名
		ImGui::InputText("Emitter Name", emitterCreationData_.name, 128);

		// ターゲットグループ選択
		std::vector<std::string> groupNames = particleSystem_->GetAllGroupNames();

		if (groupNames.empty()) {
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "No groups available!");
			ImGui::Text("Please create a group first.");
		} else {
			ImGui::Text("Target Group:");
			if (ImGui::BeginCombo("##TargetGroup",
				emitterCreationData_.selectedGroupIndex < groupNames.size()
				? groupNames[emitterCreationData_.selectedGroupIndex].c_str()
				: "")) {

				for (int i = 0; i < groupNames.size(); ++i) {
					bool isSelected = (emitterCreationData_.selectedGroupIndex == i);
					if (ImGui::Selectable(groupNames[i].c_str(), isSelected)) {
						emitterCreationData_.selectedGroupIndex = i;
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
		}

		ImGui::Spacing();
		ImGui::Separator();

		// ボタン
		bool canCreate = !groupNames.empty();

		if (!canCreate) {
			ImGui::BeginDisabled();
		}

		if (ImGui::Button("Create", ImVec2(120, 0))) {
			if (emitterCreationData_.selectedGroupIndex < groupNames.size()) {
				std::string targetGroup = groupNames[emitterCreationData_.selectedGroupIndex];

				ParticleEmitter* emitter = particleSystem_->CreateEmitter(
					emitterCreationData_.name,
					targetGroup
				);

				if (emitter) {
					Logger::Log(Logger::GetStream(),
						std::format("[ParticleEditor] Created emitter: {} -> {}\n",
							emitterCreationData_.name, targetGroup));
					showCreateEmitterDialog_ = false;
					ImGui::CloseCurrentPopup();
				} else {
					Logger::Log(Logger::GetStream(),
						std::format("[ParticleEditor] Failed to create emitter: {}\n",
							emitterCreationData_.name));
				}
			}
		}

		if (!canCreate) {
			ImGui::EndDisabled();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			showCreateEmitterDialog_ = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
#endif
}

void ParticleEditor::ShowCreateFieldDialog()
{
#ifdef USEIMGUI
	ImGui::OpenPopup("Create Field");

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Create Field", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Create New Field");
		ImGui::Separator();
		ImGui::Spacing();

		// フィールド名
		ImGui::InputText("Field Name", fieldCreationData_.name, 128);

		// フィールドタイプ選択
		ImGui::Text("Field Type:");
		ImGui::Combo("##FieldType", &fieldCreationData_.fieldTypeIndex,
			fieldCreationData_.fieldTypes, 2);

		ImGui::Spacing();
		ImGui::Separator();

		// ボタン
		if (ImGui::Button("Create", ImVec2(120, 0))) {
			std::string typeName = fieldCreationData_.fieldTypes[fieldCreationData_.fieldTypeIndex];

			BaseField* field = CreateFieldByTypeName(typeName, fieldCreationData_.name);

			if (field) {
				Logger::Log(Logger::GetStream(),
					std::format("[ParticleEditor] Created field: {} ({})\n",
						fieldCreationData_.name, typeName));
				showCreateFieldDialog_ = false;
				ImGui::CloseCurrentPopup();
			} else {
				Logger::Log(Logger::GetStream(),
					std::format("[ParticleEditor] Failed to create field: {}\n",
						fieldCreationData_.name));
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			showCreateFieldDialog_ = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
#endif
}
// ParticleEditor.cpp - パート2: Editタブ

// ========================================
// Editタブ
// ========================================

void ParticleEditor::ShowEditTab()
{
#ifdef USEIMGUI
	// 2カラムレイアウト
	ImGui::BeginChild("ObjectListPanel", ImVec2(250, 0), true);
	ShowObjectListPanel();
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("EditPanel", ImVec2(0, 0), true);
	ShowEditPanel();
	ImGui::EndChild();
#endif
}

void ParticleEditor::ShowObjectListPanel()
{
#ifdef USEIMGUI
	ImGui::Text("Objects");
	ImGui::Separator();
	ImGui::Spacing();

	// 選択操作ボタン（フェーズ2）
	if (ImGui::Button("Select All", ImVec2(115, 0))) {
		SelectAll();
	}
	ImGui::SameLine();
	if (ImGui::Button("Deselect All", ImVec2(115, 0))) {
		DeselectAll();
	}

	// 選択数表示
	size_t selectedCount = GetSelectedCount();
	if (selectedCount > 0) {
		ImGui::Text("Selected: %zu", selectedCount);
	} else {
		ImGui::TextDisabled("Selected: 0");
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// グループリスト
	ShowGroupList();

	ImGui::Spacing();

	// エミッターリスト
	ShowEmitterList();

	ImGui::Spacing();

	// フィールドリスト
	ShowFieldList();
#endif
}

void ParticleEditor::ShowGroupList()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode("Groups")) {
		std::vector<std::string> groupNames = particleSystem_->GetAllGroupNames();

		if (groupNames.empty()) {
			ImGui::TextDisabled("No groups");
		} else {
			for (const auto& name : groupNames) {
				// チェックボックス（フェーズ2）
				bool isChecked = IsGroupSelected(name);
				if (ImGui::Checkbox(("##check_group_" + name).c_str(), &isChecked)) {
					if (isChecked) {
						selectedGroups_.insert(name);
					} else {
						selectedGroups_.erase(name);
					}
				}

				ImGui::SameLine();

				// 編集選択
				bool isSelected = (currentEditingType_ == EditingType::Group &&
					currentEditingObject_ == name);

				// Selectableでオブジェクトを選択可能に
				if (ImGui::Selectable(name.c_str(), isSelected)) {
					currentEditingType_ = EditingType::Group;
					currentEditingObject_ = name;
				}

				// 右クリックメニュー
				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::MenuItem("Delete")) {
						particleSystem_->RemoveGroup(name);

						// 削除したオブジェクトが選択中だった場合、選択解除
						if (currentEditingObject_ == name) {
							currentEditingType_ = EditingType::None;
							currentEditingObject_ = "";
						}

						// チェックボックスの状態もクリア
						selectedGroups_.erase(name);

						Logger::Log(Logger::GetStream(),
							std::format("[ParticleEditor] Deleted group: {}\n", name));
					}
					ImGui::EndPopup();
				}
			}
		}

		ImGui::TreePop();
	}
#endif
}

void ParticleEditor::ShowEmitterList()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode("Emitters")) {
		std::vector<std::string> emitterNames = particleSystem_->GetAllEmitterNames();

		if (emitterNames.empty()) {
			ImGui::TextDisabled("No emitters");
		} else {
			for (const auto& name : emitterNames) {
				// チェックボックス（フェーズ2）
				bool isChecked = IsEmitterSelected(name);
				if (ImGui::Checkbox(("##check_emitter_" + name).c_str(), &isChecked)) {
					if (isChecked) {
						selectedEmitters_.insert(name);
					} else {
						selectedEmitters_.erase(name);
					}
				}

				ImGui::SameLine();

				// 編集選択
				bool isSelected = (currentEditingType_ == EditingType::Emitter &&
					currentEditingObject_ == name);

				// Selectableでオブジェクトを選択可能に
				if (ImGui::Selectable(name.c_str(), isSelected)) {
					currentEditingType_ = EditingType::Emitter;
					currentEditingObject_ = name;
				}

				// ターゲットグループ情報を表示
				ParticleEmitter* emitter = particleSystem_->GetEmitter(name);
				if (emitter) {
					ImGui::SameLine();
					ImGui::TextDisabled("-> %s", emitter->GetTargetGroupName().c_str());
				}

				// 右クリックメニュー
				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::MenuItem("Delete")) {
						particleSystem_->RemoveEmitter(name);

						// 削除したオブジェクトが選択中だった場合、選択解除
						if (currentEditingObject_ == name) {
							currentEditingType_ = EditingType::None;
							currentEditingObject_ = "";
						}

						// チェックボックスの状態もクリア
						selectedEmitters_.erase(name);

						Logger::Log(Logger::GetStream(),
							std::format("[ParticleEditor] Deleted emitter: {}\n", name));
					}
					ImGui::EndPopup();
				}
			}
		}

		ImGui::TreePop();
	}
#endif
}

void ParticleEditor::ShowFieldList()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode("Fields")) {
		std::vector<std::string> fieldNames = particleSystem_->GetAllFieldNames();

		if (fieldNames.empty()) {
			ImGui::TextDisabled("No fields");
		} else {
			for (const auto& name : fieldNames) {
				// チェックボックス（フェーズ2）
				bool isChecked = IsFieldSelected(name);
				if (ImGui::Checkbox(("##check_field_" + name).c_str(), &isChecked)) {
					if (isChecked) {
						selectedFields_.insert(name);
					} else {
						selectedFields_.erase(name);
					}
				}

				ImGui::SameLine();

				// 編集選択
				bool isSelected = (currentEditingType_ == EditingType::Field &&
					currentEditingObject_ == name);

				// Selectableでオブジェクトを選択可能に
				if (ImGui::Selectable(name.c_str(), isSelected)) {
					currentEditingType_ = EditingType::Field;
					currentEditingObject_ = name;
				}

				// フィールドタイプ情報を表示
				BaseField* field = particleSystem_->GetField(name);
				if (field) {
					ImGui::SameLine();
					ImGui::TextDisabled("(%s)", field->GetTypeName());
				}

				// 右クリックメニュー
				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::MenuItem("Delete")) {
						particleSystem_->RemoveField(name);

						// 削除したオブジェクトが選択中だった場合、選択解除
						if (currentEditingObject_ == name) {
							currentEditingType_ = EditingType::None;
							currentEditingObject_ = "";
						}

						// チェックボックスの状態もクリア
						selectedFields_.erase(name);

						Logger::Log(Logger::GetStream(),
							std::format("[ParticleEditor] Deleted field: {}\n", name));
					}
					ImGui::EndPopup();
				}
			}
		}

		ImGui::TreePop();
	}
#endif
}

void ParticleEditor::ShowEditPanel()
{
#ifdef USEIMGUI
	if (currentEditingType_ == EditingType::None || currentEditingObject_.empty()) {
		ImGui::TextDisabled("Select an object to edit");
		ImGui::Spacing();
		ImGui::Text("Tip: Click on an object in the list");
		ImGui::Text("or right-click to delete.");
		return;
	}

	// 選択されたオブジェクトの編集UI
	ImGui::Text("Editing: %s", currentEditingObject_.c_str());
	ImGui::Separator();
	ImGui::Spacing();

	switch (currentEditingType_) {
	case EditingType::Group: {
		ParticleGroup* group = particleSystem_->GetGroup(currentEditingObject_);
		if (group) {
			// 既存のImGui()を呼び出す
			group->ImGui();
		} else {
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Group not found!");
			currentEditingType_ = EditingType::None;
			currentEditingObject_ = "";
		}
		break;
	}

	case EditingType::Emitter: {
		ParticleEmitter* emitter = particleSystem_->GetEmitter(currentEditingObject_);
		if (emitter) {
			// 既存のImGui()を呼び出す
			emitter->ImGui();
		} else {
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Emitter not found!");
			currentEditingType_ = EditingType::None;
			currentEditingObject_ = "";
		}
		break;
	}

	case EditingType::Field: {
		BaseField* field = particleSystem_->GetField(currentEditingObject_);
		if (field) {
			// 既存のImGui()を呼び出す
			field->ImGui();
		} else {
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Field not found!");
			currentEditingType_ = EditingType::None;
			currentEditingObject_ = "";
		}
		break;
	}

	default:
		break;
	}
#endif
}
// ParticleEditor.cpp - パート3: Presetsタブとユーティリティ

// ========================================
// Presetsタブ
// ========================================

void ParticleEditor::ShowPresetsTab()
{
#ifdef USEIMGUI
	ImGui::Text("Preset Management");
	ImGui::Separator();
	ImGui::Spacing();

	// 利用可能なプリセット一覧
	std::vector<std::string> presets = GetAvailablePresets();

	ImGui::Text("Available Presets: %zu", presets.size());
	ImGui::Spacing();

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

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// 新しいプリセット名入力
	ImGui::InputText("Preset Name", newPresetName, 128);
	ImGui::Spacing();

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

	// 選択的保存ボタン（フェーズ2）
	ImGui::SameLine();
	bool hasSelection = GetSelectedCount() > 0;
	if (!hasSelection) {
		ImGui::BeginDisabled();
	}

	if (ImGui::Button("Save Selected", ImVec2(200, 0))) {
		if (strlen(newPresetName) > 0) {
			if (SaveSelectedAsPreset(newPresetName)) {
				Logger::Log(Logger::GetStream(),
					std::format("[ParticleEditor] Saved selected preset: {}\n", newPresetName));
				memset(newPresetName, 0, sizeof(newPresetName));
			}
		} else {
			Logger::Log(Logger::GetStream(),
				"[ParticleEditor] Please enter a preset name\n");
		}
	}

	if (!hasSelection) {
		ImGui::EndDisabled();
	}

	// 選択状態の表示（フェーズ2）
	ImGui::Spacing();
	if (hasSelection) {
		ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f),
			"Selected: %zu groups, %zu emitters, %zu fields",
			selectedGroups_.size(), selectedEmitters_.size(), selectedFields_.size());

		// 依存関係の警告
		bool hasEmittersWithoutGroups = false;
		for (const auto& emitterName : selectedEmitters_) {
			ParticleEmitter* emitter = particleSystem_->GetEmitter(emitterName);
			if (emitter) {
				std::string targetGroup = emitter->GetTargetGroupName();
				if (selectedGroups_.find(targetGroup) == selectedGroups_.end()) {
					hasEmittersWithoutGroups = true;
					break;
				}
			}
		}

		if (hasEmittersWithoutGroups) {
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
				"⚠ Warning: Some emitters' target groups");
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
				"  will be auto-added when saving.");
		}
	} else {
		ImGui::TextDisabled("No objects selected.");
		ImGui::TextDisabled("Use checkboxes in Edit tab to select.");
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// インスタンス作成
	static char instanceName[128] = "";
	ImGui::Text("Create Instance from Preset");
	ImGui::InputText("Instance Name", instanceName, 128);

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

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// 現在のインスタンス一覧
	ImGui::Text("Active Instances: %zu", instances_.size());

	if (ImGui::TreeNode("Instance List")) {
		for (auto& [name, instance] : instances_) {
			ImGui::BulletText("%s", name.c_str());
		}
		ImGui::TreePop();
	}

	ImGui::Spacing();

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
#endif
}

// ========================================
// ユーティリティ関数
// ========================================

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

ParticleGroupData ParticleEditor::CreateGroupData(ParticleGroup* group) const
{
	ParticleGroupData data;
	data.groupName = group->GetName();
	data.modelTag = group->GetModel() ? group->GetModelTag() : "";
	data.maxParticles = group->GetMaxParticleCount();
	data.textureName = group->GetTextureName();
	data.useBillboard = group->UseBillboard();
	return data;
}

ParticleEmitterData ParticleEditor::CreateEmitterData(ParticleEmitter* emitter) const
{
	ParticleEmitterData data;
	data.emitterName = emitter->GetName();
	data.targetGroupName = emitter->GetTargetGroupName();

	// Transform
	data.position = emitter->GetTransform().GetPosition();
	data.rotation = emitter->GetTransform().GetRotation();
	data.scale = emitter->GetTransform().GetScale();

	// Emit設定
	data.emitCount = emitter->GetEmitCount();
	data.emitFrequency = emitter->GetFrequency();
	data.isEmitting = emitter->IsEmitting();

	// パーティクル寿命（現時点でGetterがないため、デフォルト値）
	data.particleLifeTimeMin = 1.0f;
	data.particleLifeTimeMax = 3.0f;

	// 速度設定
	data.emitDirection = emitter->GetEmitDirection();
	data.initialSpeed = emitter->GetInitialSpeed();
	data.spreadAngle = emitter->GetSpreadAngle();
	data.useDirectionalEmit = emitter->IsUseDirectionalEmit();
	data.velocityRange = 1.0f;

	// スケール・回転（現時点でGetterがないため、デフォルト値）
	data.particleScaleMin = { 1.0f, 1.0f, 1.0f };
	data.particleScaleMax = { 1.0f, 1.0f, 1.0f };
	data.particleRotateMin = { 0.0f, 0.0f, 0.0f };
	data.particleRotateMax = { 0.0f, 0.0f, 0.0f };

	// エミッター寿命
	data.emitterLifeTime = emitter->GetEmitterLifeTime();
	data.emitterLifeTimeLoop = emitter->IsEmitterLifeTimeLoop();
	data.useEmitterLifeTime = emitter->IsUseEmitterLifeTime();

	// 発生範囲
	data.spawnArea = emitter->GetSpawnArea();

	// デバッグ
	data.showDebugAABB = emitter->IsShowDebugAABB();
	data.debugAABBColor = { 1.0f, 0.0f, 0.0f, 1.0f };

	// Color Over Lifetime
	data.enableColorOverLifetime = emitter->IsEnableColorOverLifetime();
	data.particleStartColor = emitter->GetParticleStartColor();
	data.particleEndColor = emitter->GetParticleEndColor();

	// Size Over Lifetime
	data.enableSizeOverLifetime = emitter->IsEnableSizeOverLifetime();
	data.particleStartScale = emitter->GetParticleStartScale();
	data.particleEndScale = emitter->GetParticleEndScale();

	// Rotation
	data.enableRotation = emitter->IsEnableRotation();
	data.rotationSpeed = emitter->GetRotationSpeed();

	return data;
}

ParticleFieldData ParticleEditor::CreateFieldData(BaseField* field) const
{
	ParticleFieldData data;
	data.fieldName = field->GetName();
	data.fieldType = field->GetTypeName();

	// Transform
	data.position = field->GetTransform().GetPosition();
	data.rotation = field->GetTransform().GetRotation();
	data.scale = field->GetTransform().GetScale();

	// 共通設定
	data.isEnabled = field->IsEnabled();
	data.showDebugVisualization = field->IsShowDebugVisualization();
	data.debugColor = field->GetDebugColor();

	// フィールド固有のパラメータ
	data.parameters = field->SerializeParameters();

	return data;
}
// ParticleEditor.cpp - パート4: Save/Load/Instance機能

// ========================================
// Save/Load機能
// ========================================

bool ParticleEditor::SaveCurrentStateAsPreset(const std::string& presetName)
{
	ParticlePresetData preset;
	preset.presetName = presetName;

	// 全グループを保存
	for (const std::string& groupName : particleSystem_->GetAllGroupNames()) {
		ParticleGroup* group = particleSystem_->GetGroup(groupName);
		if (!group) continue;

		preset.groups.push_back(CreateGroupData(group));
	}

	// 全エミッターを保存
	for (const std::string& emitterName : particleSystem_->GetAllEmitterNames()) {
		ParticleEmitter* emitter = particleSystem_->GetEmitter(emitterName);
		if (!emitter) continue;

		preset.emitters.push_back(CreateEmitterData(emitter));
	}

	// 全フィールドを保存
	for (const std::string& fieldName : particleSystem_->GetAllFieldNames()) {
		BaseField* field = particleSystem_->GetField(fieldName);
		if (!field) continue;

		preset.fields.push_back(CreateFieldData(field));
	}

	// ファイルに保存
	std::string filePath = GetPresetFilePath(presetName);
	return preset.SaveToFile(filePath);
}

bool ParticleEditor::SaveSelectedAsPreset(const std::string& presetName)
{
	// 選択されているオブジェクトがない場合は警告
	if (GetSelectedCount() == 0) {
		Logger::Log(Logger::GetStream(),
			"[ParticleEditor] Warning: No objects selected. Nothing to save.\n");
		return false;
	}

	ParticlePresetData preset;
	preset.presetName = presetName;

	// 依存関係解決のために、追加したグループ名を追跡
	std::set<std::string> includedGroups;

	// ========================================
	// ステップ1: 選択されたグループを追加
	// ========================================
	for (const auto& groupName : selectedGroups_) {
		ParticleGroup* group = particleSystem_->GetGroup(groupName);
		if (group) {
			preset.groups.push_back(CreateGroupData(group));
			includedGroups.insert(groupName);
		}
	}

	// ========================================
	// ステップ2: 選択されたエミッターを追加 + 依存関係解決
	// ========================================
	for (const auto& emitterName : selectedEmitters_) {
		ParticleEmitter* emitter = particleSystem_->GetEmitter(emitterName);
		if (!emitter) continue;

		// エミッターを追加
		preset.emitters.push_back(CreateEmitterData(emitter));

		// 依存関係チェック: ターゲットグループが含まれているか？
		std::string targetGroupName = emitter->GetTargetGroupName();

		// ターゲットグループがまだ含まれていない場合、自動的に追加
		if (includedGroups.find(targetGroupName) == includedGroups.end()) {
			ParticleGroup* targetGroup = particleSystem_->GetGroup(targetGroupName);
			if (targetGroup) {
				preset.groups.push_back(CreateGroupData(targetGroup));
				includedGroups.insert(targetGroupName);

				// ログで通知
				Logger::Log(Logger::GetStream(),
					std::format("[ParticleEditor] Auto-added group '{}' (required by emitter '{}')\n",
						targetGroupName, emitterName));
			} else {
				// ターゲットグループが存在しない（異常な状態）
				Logger::Log(Logger::GetStream(),
					std::format("[ParticleEditor] Warning: Emitter '{}' references non-existent group '{}'\n",
						emitterName, targetGroupName));
			}
		}
	}

	// ========================================
	// ステップ3: 選択されたフィールドを追加
	// ========================================
	for (const auto& fieldName : selectedFields_) {
		BaseField* field = particleSystem_->GetField(fieldName);
		if (field) {
			preset.fields.push_back(CreateFieldData(field));
		}
	}

	// ========================================
	// ステップ4: ファイルに保存
	// ========================================
	std::string filePath = GetPresetFilePath(presetName);
	bool success = preset.SaveToFile(filePath);

	if (success) {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticleEditor] Saved selected preset '{}': {} groups, {} emitters, {} fields\n",
				presetName, preset.groups.size(), preset.emitters.size(), preset.fields.size()));
	} else {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticleEditor] Failed to save selected preset '{}'\n", presetName));
	}

	return success;
}

ParticlePresetData ParticleEditor::LoadPreset(const std::string& presetName)
{
	std::string filePath = GetPresetFilePath(presetName);
	return ParticlePresetData::LoadFromFile(filePath);
}

// ========================================
// インスタンス作成・管理
// ========================================

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

			// Color Over Lifetime
			emitter->SetColorOverLifetime(
				emitterData.enableColorOverLifetime,
				emitterData.particleStartColor,
				emitterData.particleEndColor
			);

			// Size Over Lifetime
			emitter->SetSizeOverLifetime(
				emitterData.enableSizeOverLifetime,
				emitterData.particleStartScale,
				emitterData.particleEndScale
			);

			// Rotation
			emitter->SetRotation(
				emitterData.enableRotation,
				emitterData.rotationSpeed
			);

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

// ========================================
// 選択操作（フェーズ2）
// ========================================

void ParticleEditor::SelectAll()
{
	// すべてのグループを選択
	selectedGroups_.clear();
	for (const auto& groupName : particleSystem_->GetAllGroupNames()) {
		selectedGroups_.insert(groupName);
	}

	// すべてのエミッターを選択
	selectedEmitters_.clear();
	for (const auto& emitterName : particleSystem_->GetAllEmitterNames()) {
		selectedEmitters_.insert(emitterName);
	}

	// すべてのフィールドを選択
	selectedFields_.clear();
	for (const auto& fieldName : particleSystem_->GetAllFieldNames()) {
		selectedFields_.insert(fieldName);
	}

	Logger::Log(Logger::GetStream(),
		std::format("[ParticleEditor] Selected all objects ({} total)\n", GetSelectedCount()));
}

void ParticleEditor::DeselectAll()
{
	selectedGroups_.clear();
	selectedEmitters_.clear();
	selectedFields_.clear();

	Logger::Log(Logger::GetStream(), "[ParticleEditor] Deselected all objects\n");
}

size_t ParticleEditor::GetSelectedCount() const
{
	return selectedGroups_.size() + selectedEmitters_.size() + selectedFields_.size();
}

bool ParticleEditor::IsGroupSelected(const std::string& groupName) const
{
	return selectedGroups_.find(groupName) != selectedGroups_.end();
}

bool ParticleEditor::IsEmitterSelected(const std::string& emitterName) const
{
	return selectedEmitters_.find(emitterName) != selectedEmitters_.end();
}

bool ParticleEditor::IsFieldSelected(const std::string& fieldName) const
{
	return selectedFields_.find(fieldName) != selectedFields_.end();
}