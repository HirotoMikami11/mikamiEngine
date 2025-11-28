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

		// ========================================
		// モード表示
		// ========================================
		if (currentMode_ == EditorMode::Preset) {
			ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
				"MODE: Editing Preset [%s]", editingPresetName_.c_str());
			ImGui::Separator();
		} else {
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f),
				"MODE: Instance Editor");
			ImGui::Separator();
		}

		// タブバー
		if (ImGui::BeginTabBar("EditorTabs", ImGuiTabBarFlags_None)) {


			/// Preset Manager タブ
			if (ImGui::BeginTabItem("Preset Manager")) {
				ShowPresetManagerTab();
				ImGui::EndTabItem();
			}


			/// Create タブ
			if (ImGui::BeginTabItem("Create")) {
				ShowCreateTab();
				ImGui::EndTabItem();
			}


			/// Edit タブ
			if (ImGui::BeginTabItem("Edit")) {
				ShowEditTab();
				ImGui::EndTabItem();
			}

			/// Instances タブ
			if (ImGui::BeginTabItem("Instances")) {
				ShowInstancesTab();
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

	// ========================================
	// 選択保存セクション
	// ========================================
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::TreeNode("Save Selection")) {
		size_t selectedCount = GetSelectedCount();

		if (selectedCount > 0) {
			ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
				"%zu object(s) selected", selectedCount);
			ImGui::Spacing();

			static char selectedPresetName[128] = "";
			ImGui::InputTextWithHint("##SaveSelectedPreset",
				"Preset name...", selectedPresetName, 128);

			if (ImGui::Button("Save As Preset", ImVec2(-1, 0))) {
				if (strlen(selectedPresetName) > 0) {
					if (SaveSelectedAsPreset(selectedPresetName)) {
						selectedPresetName[0] = '\0';  // クリア
						DeselectAll();  // 選択解除
						Logger::Log(Logger::GetStream(),
							"[ParticleEditor] Successfully saved selection as preset\n");
					}
				} else {
					Logger::Log(Logger::GetStream(),
						"[ParticleEditor] Please enter a preset name\n");
				}
			}

			ImGui::Spacing();
			ImGui::TextWrapped("Saves only selected objects. Dependencies are included automatically.");
		} else {
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
				"No objects selected");
			ImGui::Spacing();
			ImGui::TextWrapped("Select objects above to save them as a preset.");
		}

		ImGui::TreePop();
	}
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

// ========================================
// Preset Manager タブ
// ========================================

void ParticleEditor::ShowPresetManagerTab()
{
#ifdef USEIMGUI
	ImGui::Text("Preset Management");
	ImGui::Separator();
	ImGui::Spacing();

	// プリセット一覧を取得
	std::vector<std::string> presets = GetAvailablePresets();

	if (presets.empty()) {
		ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
			"No presets available");
		ImGui::Spacing();
		ImGui::TextWrapped("Create a preset by saving the current state in the Edit tab, "
			"or by creating and configuring particles in the Create tab.");
		return;
	}

	// 選択されたプリセットのインデックス
	static int selectedPreset = 0;
	if (selectedPreset >= static_cast<int>(presets.size())) {
		selectedPreset = 0;
	}

	// 左側：プリセットリスト
	ImGui::BeginChild("PresetList", ImVec2(250, 0), true);
	{
		ImGui::Text("Available Presets");
		ImGui::Separator();

		for (size_t i = 0; i < presets.size(); ++i) {
			bool isSelected = (selectedPreset == static_cast<int>(i));

			// プリセット編集中のものは特別な色で表示
			if (currentMode_ == EditorMode::Preset && presets[i] == editingPresetName_) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
				if (ImGui::Selectable((presets[i] + " (Editing)").c_str(), isSelected)) {
					selectedPreset = static_cast<int>(i);
				}
				ImGui::PopStyleColor();
			} else {
				if (ImGui::Selectable(presets[i].c_str(), isSelected)) {
					selectedPreset = static_cast<int>(i);
				}
			}
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	// 右側：プリセット操作
	ImGui::BeginChild("PresetActions", ImVec2(0, 0), true);
	{
		const std::string& presetName = presets[selectedPreset];

		ImGui::Text("Selected: %s", presetName.c_str());
		ImGui::Separator();
		ImGui::Spacing();

		// ========================================
		// プリセット編集モードでない場合
		// ========================================
		if (currentMode_ == EditorMode::Instance) {

			// 【新規】プリセットを直接編集
			if (ImGui::Button("Edit This Preset", ImVec2(-1, 40))) {
				OpenPresetForEdit(presetName);
			}
			ImGui::TextWrapped("Opens the preset for direct editing. "
				"Changes will be saved to the original preset.");

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// 【既存】インスタンスとして生成
			ImGui::Text("Create Instance");
			static char instanceName[128] = "";
			ImGui::InputTextWithHint("Instance Name", "Enter unique name...", instanceName, 128);

			if (ImGui::Button("Create Instance", ImVec2(-1, 40))) {
				if (strlen(instanceName) > 0) {
					if (CreateInstance(presetName, instanceName)) {
						instanceName[0] = '\0';  // クリア
					}
				} else {
					Logger::Log(Logger::GetStream(),
						"[ParticleEditor] Please enter an instance name\n");
				}
			}
			ImGui::TextWrapped("Creates a new instance that can be "
				"placed and configured independently.");

		}
		// ========================================
		// プリセット編集モードの場合
		// ========================================
		else {
			ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
				"Currently Editing: %s", editingPresetName_.c_str());

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// 保存ボタン
			if (ImGui::Button("Save Preset", ImVec2(-1, 40))) {
				SaveEditingPreset();
			}
			ImGui::TextWrapped("Saves changes to the preset '%s'", editingPresetName_.c_str());

			ImGui::Spacing();

			// 別名保存
			ImGui::Text("Save As New Preset");
			static char newName[128] = "";
			ImGui::InputTextWithHint("New Preset Name", "Enter new name...", newName, 128);

			if (ImGui::Button("Save As New Preset", ImVec2(-1, 40))) {
				if (strlen(newName) > 0) {
					if (SaveEditingPresetAs(newName)) {
						newName[0] = '\0';
					}
				} else {
					Logger::Log(Logger::GetStream(),
						"[ParticleEditor] Please enter a new preset name\n");
				}
			}
			ImGui::TextWrapped("Creates a new preset with the current settings");

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// 閉じるボタン
			if (ImGui::Button("Close Editor", ImVec2(-1, 40))) {
				ClosePresetEditor();
			}
			ImGui::TextWrapped("Closes the preset editor and returns to instance mode");
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// ========================================
		// 削除（常に表示）
		// ========================================
		ImGui::Text("Danger Zone");
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
		if (ImGui::Button("Delete Preset", ImVec2(-1, 40))) {
			ImGui::OpenPopup("DeletePresetConfirm");
		}
		ImGui::PopStyleColor(2);

		// 削除確認ダイアログ
		if (ImGui::BeginPopupModal("DeletePresetConfirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Delete preset '%s'?", presetName.c_str());
			ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
				"This action cannot be undone!");
			ImGui::Spacing();

			if (ImGui::Button("Yes, Delete", ImVec2(120, 0))) {
				// 編集中のプリセットを削除する場合は、先にエディタを閉じる
				if (currentMode_ == EditorMode::Preset && presetName == editingPresetName_) {
					ClosePresetEditor();
				}

				if (DeletePreset(presetName)) {
					// 削除成功時、選択を先頭にリセット
					selectedPreset = 0;
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}
	ImGui::EndChild();
#endif
}

// ========================================
// Instances タブ
// ========================================

void ParticleEditor::ShowInstancesTab()
{
#ifdef USEIMGUI
	ImGui::Text("Instance Management");
	ImGui::Separator();
	ImGui::Spacing();

	// インスタンス一覧
	if (instances_.empty()) {
		ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
			"No instances created");
		ImGui::Spacing();
		ImGui::TextWrapped("Create instances from presets in the Preset Manager tab.");
		return;
	}

	// 選択されたインスタンス
	static std::string selectedInstance;

	// 左側：インスタンスリスト
	ImGui::BeginChild("InstanceList", ImVec2(250, 0), true);
	{
		ImGui::Text("Active Instances");
		ImGui::Separator();

		for (const auto& [instanceName, instance] : instances_) {
			// プリセット編集用の特別なインスタンスは表示しない
			if (instanceName == kPresetEditInstanceName_) {
				continue;
			}

			bool isSelected = (selectedInstance == instanceName);
			if (ImGui::Selectable(instanceName.c_str(), isSelected)) {
				selectedInstance = instanceName;
			}
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	// 右側：インスタンス操作
	ImGui::BeginChild("InstanceActions", ImVec2(0, 0), true);
	{
		if (!selectedInstance.empty() && instances_.find(selectedInstance) != instances_.end()) {
			ImGui::Text("Selected: %s", selectedInstance.c_str());
			ImGui::Separator();
			ImGui::Spacing();

			ParticlePresetInstance* instance = instances_[selectedInstance].get();

			// 位置設定
			ImGui::Text("Instance Position");
			static Vector3 instancePos = { 0, 0, 0 };
			if (ImGui::DragFloat3("Position", &instancePos.x, 0.1f)) {
				instance->SetPositionOffset(instancePos);
			}

			ImGui::Spacing();

			// 有効/無効切り替え
			static bool instanceEnabled = true;
			if (ImGui::Checkbox("Enable Instance", &instanceEnabled)) {
				instance->SetEnabled(instanceEnabled);
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// プリセットとして保存
			ImGui::Text("Save As Preset");
			static char presetSaveName[128] = "";
			ImGui::InputTextWithHint("Preset Name", "Enter preset name...", presetSaveName, 128);

			if (ImGui::Button("Save Instance As Preset", ImVec2(-1, 40))) {
				if (strlen(presetSaveName) > 0) {
					if (SaveInstanceAsPreset(selectedInstance, presetSaveName)) {
						presetSaveName[0] = '\0';  // クリア
					}
				} else {
					Logger::Log(Logger::GetStream(),
						"[ParticleEditor] Please enter a preset name\n");
				}
			}
			ImGui::TextWrapped("Saves the current instance configuration as a preset. "
				"Instance prefixes will be automatically removed.");

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// 削除
			ImGui::Text("Danger Zone");
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
			if (ImGui::Button("Destroy Instance", ImVec2(-1, 40))) {
				ImGui::OpenPopup("DestroyInstanceConfirm");
			}
			ImGui::PopStyleColor(2);

			// 削除確認ダイアログ
			if (ImGui::BeginPopupModal("DestroyInstanceConfirm", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("Destroy instance '%s'?", selectedInstance.c_str());
				ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
					"All particles and settings will be removed!");
				ImGui::Spacing();

				if (ImGui::Button("Yes, Destroy", ImVec2(120, 0))) {
					DestroyInstance(selectedInstance);
					selectedInstance.clear();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0))) {
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

		} else {
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
				"Select an instance from the list");
		}
	}
	ImGui::EndChild();
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

	// パーティクル寿命を実際の値から取得
	data.particleLifeTimeMin = emitter->GetParticleLifeTimeMin();
	data.particleLifeTimeMax = emitter->GetParticleLifeTimeMax();

	// 速度設定
	data.emitDirection = emitter->GetEmitDirection();
	data.initialSpeed = emitter->GetInitialSpeed();
	data.spreadAngle = emitter->GetSpreadAngle();
	data.useDirectionalEmit = emitter->IsUseDirectionalEmit();

	// 速度範囲も実際の値から取得
	data.velocityRange = emitter->GetParticleVelocityRange();

	// スケール・回転も実際の値から取得
	data.particleScaleMin = emitter->GetParticleScaleMin();
	data.particleScaleMax = emitter->GetParticleScaleMax();
	data.particleRotateMin = emitter->GetParticleRotateMin();
	data.particleRotateMax = emitter->GetParticleRotateMax();

	// エミッター寿命
	data.emitterLifeTime = emitter->GetEmitterLifeTime();
	data.emitterLifeTimeLoop = emitter->IsEmitterLifeTimeLoop();
	data.useEmitterLifeTime = emitter->IsUseEmitterLifeTime();

	// 発生範囲
	data.spawnArea = emitter->GetSpawnArea();

	// デバッグ
	data.showDebugAABB = emitter->IsShowDebugAABB();
	data.debugAABBColor = emitter->GetDebugAABBColor();

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
			ApplyEmitterData(emitter, emitterData);
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
			ApplyFieldData(field, fieldData);
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

void ParticleEditor::DestroyAllInstance()
{

	//全てのインスタンスを削除
	for (auto& [name, instance] : instances_) {
		if (instance) {
			instance->Destroy();
		}
	}

	//全てeraseしてからクリアする
	instances_.clear();
	Logger::Log(Logger::GetStream(),
		"[ParticleEditor] Destroyed all instances.\n");
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

// ========================================
// 名前正規化ユーティリティ
// ========================================

std::string ParticleEditor::RemoveInstancePrefix(const std::string& objectName, const std::string& instanceName) const
{
	// プレフィックスパターン：「インスタンス名_」
	std::string prefix = instanceName + "_";

	// プレフィックスが存在する場合は除去
	if (objectName.find(prefix) == 0) {
		return objectName.substr(prefix.length());
	}

	// プレフィックスが無い場合はそのまま返す
	return objectName;
}

void ParticleEditor::NormalizePresetNames(ParticlePresetData& data, const std::string& instanceName) const
{
	// グループ名を正規化
	for (auto& group : data.groups) {
		group.groupName = RemoveInstancePrefix(group.groupName, instanceName);
	}

	// エミッター名とターゲットグループ名を正規化
	for (auto& emitter : data.emitters) {
		emitter.emitterName = RemoveInstancePrefix(emitter.emitterName, instanceName);
		emitter.targetGroupName = RemoveInstancePrefix(emitter.targetGroupName, instanceName);
	}

	// フィールド名を正規化
	for (auto& field : data.fields) {
		field.fieldName = RemoveInstancePrefix(field.fieldName, instanceName);
	}
}

// ========================================
// ヘルパー関数: データ適用
// ========================================

void ParticleEditor::ApplyEmitterData(ParticleEmitter* emitter, const ParticleEmitterData& data)
{
	// Transform設定
	emitter->GetTransform().SetPosition(data.position);
	emitter->GetTransform().SetRotation(data.rotation);
	emitter->GetTransform().SetScale(data.scale);

	// Emit設定
	emitter->SetEmitCount(data.emitCount);
	emitter->SetFrequency(data.emitFrequency);
	emitter->SetEmitEnabled(data.isEmitting);
	emitter->SetParticleLifeTimeRange(data.particleLifeTimeMin, data.particleLifeTimeMax);

	// 速度設定
	emitter->SetUseDirectionalEmit(data.useDirectionalEmit);
	if (data.useDirectionalEmit) {
		emitter->SetEmitDirection(data.emitDirection);
		emitter->SetInitialSpeed(data.initialSpeed);
		emitter->SetSpreadAngle(data.spreadAngle);
	} else {
		emitter->SetParticleVelocityRange(data.velocityRange);
	}

	// スケール・回転設定
	emitter->SetParticleScaleRange(data.particleScaleMin, data.particleScaleMax);
	emitter->SetParticleRotateRange(data.particleRotateMin, data.particleRotateMax);

	// エミッター寿命
	emitter->SetUseEmitterLifeTime(data.useEmitterLifeTime);
	emitter->SetEmitterLifeTime(data.emitterLifeTime);
	emitter->SetEmitterLifeTimeLoop(data.emitterLifeTimeLoop);

	// 発生範囲
	emitter->SetSpawnArea(data.spawnArea);

	// デバッグ
	emitter->SetShowDebugAABB(data.showDebugAABB);
	emitter->SetDebugAABBColor(data.debugAABBColor);

	// 特殊効果
	emitter->SetColorOverLifetime(data.enableColorOverLifetime, data.particleStartColor, data.particleEndColor);
	emitter->SetSizeOverLifetime(data.enableSizeOverLifetime, data.particleStartScale, data.particleEndScale);
	emitter->SetRotation(data.enableRotation, data.rotationSpeed);
}

void ParticleEditor::ApplyFieldData(BaseField* field, const ParticleFieldData& data)
{
	// Transform設定
	field->GetTransform().SetPosition(data.position);
	field->GetTransform().SetRotation(data.rotation);
	field->GetTransform().SetScale(data.scale);

	// 共通設定
	field->SetEnabled(data.isEnabled);
	field->SetShowDebugVisualization(data.showDebugVisualization);
	field->SetDebugColor(data.debugColor);

	// フィールド固有のパラメータ
	field->DeserializeParameters(data.parameters);
}

// ========================================
// プリセット編集モード
// ========================================

bool ParticleEditor::OpenPresetForEdit(const std::string& presetName)
{
	// 既に編集中の場合は確認
	if (currentMode_ == EditorMode::Preset) {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticleEditor] Already editing preset '{}'. Close current editor first.\n",
				editingPresetName_));
		return false;
	}

	// 既存のインスタンスがある場合は警告を出す
	if (!instances_.empty()) {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticleEditor] Warning: {} instance(s) exist. They will remain active during preset editing.\n",
				instances_.size()));
	}

	// プリセット編集用の特別なインスタンスを作成
	ParticlePresetInstance* instance = CreateInstance(presetName, kPresetEditInstanceName_);

	if (!instance) {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticleEditor] Failed to open preset '{}' for editing\n", presetName));
		return false;
	}

	// モードを切り替え
	currentMode_ = EditorMode::Preset;
	editingPresetName_ = presetName;

	Logger::Log(Logger::GetStream(),
		std::format("[ParticleEditor] Opened preset '{}' for editing\n", presetName));

	return true;
}

bool ParticleEditor::SaveEditingPreset()
{
	if (currentMode_ != EditorMode::Preset) {
		Logger::Log(Logger::GetStream(),
			"[ParticleEditor] Not in preset editing mode\n");
		return false;
	}

	// 現在のプリセット名で保存（上書き）
	return SaveEditingPresetAs(editingPresetName_);
}

bool ParticleEditor::SaveEditingPresetAs(const std::string& newPresetName)
{
	if (currentMode_ != EditorMode::Preset) {
		Logger::Log(Logger::GetStream(),
			"[ParticleEditor] Not in preset editing mode\n");
		return false;
	}

	// プリセット編集用インスタンスを取得
	ParticlePresetInstance* instance = GetInstance(kPresetEditInstanceName_);
	if (!instance) {
		Logger::Log(Logger::GetStream(),
			"[ParticleEditor] Preset editor instance not found\n");
		return false;
	}

	// インスタンスからプリセットとして保存（名前正規化を実施）
	bool success = SaveInstanceAsPreset(kPresetEditInstanceName_, newPresetName);

	if (success) {
		// 新しい名前で保存した場合、編集中のプリセット名を更新
		if (newPresetName != editingPresetName_) {
			Logger::Log(Logger::GetStream(),
				std::format("[ParticleEditor] Saved preset '{}' as '{}'\n",
					editingPresetName_, newPresetName));
			editingPresetName_ = newPresetName;
		} else {
			Logger::Log(Logger::GetStream(),
				std::format("[ParticleEditor] Saved preset '{}'\n", newPresetName));
		}
	}

	return success;
}

void ParticleEditor::ClosePresetEditor()
{
	if (currentMode_ != EditorMode::Preset) {
		Logger::Log(Logger::GetStream(),
			"[ParticleEditor] Not in preset editing mode\n");
		return;
	}

	// 編集用インスタンスを削除
	DestroyInstance(kPresetEditInstanceName_);

	// モードをリセット
	currentMode_ = EditorMode::Instance;
	editingPresetName_.clear();

	Logger::Log(Logger::GetStream(),
		"[ParticleEditor] Closed preset editor\n");
}

bool ParticleEditor::IsInPresetEditMode() const
{
	return currentMode_ == EditorMode::Preset;
}

// ========================================
// インスタンス保存（名前正規化対応）
// ========================================

bool ParticleEditor::SaveInstanceAsPreset(const std::string& instanceName, const std::string& presetName)
{
	// インスタンスの存在確認
	if (!GetInstance(instanceName)) {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticleEditor] Instance '{}' not found\n", instanceName));
		return false;
	}

	// プリセットデータを作成
	ParticlePresetData preset;
	preset.presetName = presetName;

	// プレフィックス準備
	std::string prefix = instanceName + "_";

	// インスタンスに属するオブジェクトを収集
	for (const auto& name : particleSystem_->GetAllGroupNames()) {
		if (name.find(prefix) == 0) {
			if (auto* obj = particleSystem_->GetGroup(name)) {
				preset.groups.push_back(CreateGroupData(obj));
			}
		}
	}

	for (const auto& name : particleSystem_->GetAllEmitterNames()) {
		if (name.find(prefix) == 0) {
			if (auto* obj = particleSystem_->GetEmitter(name)) {
				preset.emitters.push_back(CreateEmitterData(obj));
			}
		}
	}

	for (const auto& name : particleSystem_->GetAllFieldNames()) {
		if (name.find(prefix) == 0) {
			if (auto* obj = particleSystem_->GetField(name)) {
				preset.fields.push_back(CreateFieldData(obj));
			}
		}
	}

	// 名前を正規化
	NormalizePresetNames(preset, instanceName);

	// 保存
	if (preset.SaveToFile(GetPresetFilePath(presetName))) {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticleEditor] Saved instance '{}' as preset '{}'\n",
				instanceName, presetName));
		return true;
	}
	return false;
}