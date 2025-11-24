#include "JsonSettings.h"
#include "Logger.h"

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#include <fstream>
#include <iostream>
#include <windows.h>
#include <format>

JsonSettings* JsonSettings::GetInstance()
{
	static JsonSettings instance;
	return &instance;
}

void JsonSettings::ImGui()
{
	if (!ImGui::Begin("Global Variables", nullptr, ImGuiWindowFlags_MenuBar))
	{
		ImGui::End();
		return;
	}


	ImGui::DragFloat("dragSensitivity", &dragSensitivity_, 0.01f, 0.001f, 10.0f, "%.3f");


	// トップレベルグループをツリーとして表示
	for (auto& [groupName, group] : datas_)
	{
		if (ImGui::TreeNode(groupName.c_str()))
		{
			DrawGroupRecursive({ groupName }, group);
			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void JsonSettings::DrawGroupRecursive(const std::vector<std::string>& groupPath, Group& group)
{
	// groupPathの末尾が現在のグループ名
	const std::string& groupName = groupPath.back();

	if (ImGui::TreeNode(groupName.c_str()))
	{
		// items を表示（std::variantを使用）
		for (auto& [itemName, value] : group.items)
		{
			std::string label = itemName;

			// std::visitを使って型ごとに処理
			std::visit([&](auto&& val) {
				using T = std::decay_t<decltype(val)>;

				if constexpr (std::is_same_v<T, int32_t>) {
					ImGui::DragInt(label.c_str(), &val, (float)dragSensitivity_);
				} else if constexpr (std::is_same_v<T, float>) {
					ImGui::DragFloat(label.c_str(), &val, dragSensitivity_);
				} else if constexpr (std::is_same_v<T, bool>) {
					ImGui::Checkbox(label.c_str(), &val);
				} else if constexpr (std::is_same_v<T, Vector2>) {
					ImGui::DragFloat2(label.c_str(), reinterpret_cast<float*>(&val), dragSensitivity_);
				} else if constexpr (std::is_same_v<T, Vector3>) {
					ImGui::DragFloat3(label.c_str(), reinterpret_cast<float*>(&val), dragSensitivity_);
				} else if constexpr (std::is_same_v<T, Vector4>) {
					ImGui::ColorEdit4(label.c_str(), reinterpret_cast<float*>(&val));
				}
				}, value);
		}

		ImGui::Spacing();

		// サブグループを再帰的に描画
		for (auto& [subGroupName, subGroup] : group.subGroups)
		{
			// 次の階層パスを作成して再帰呼び出し
			std::vector<std::string> nextGroupPath = groupPath;
			nextGroupPath.push_back(subGroupName);
			DrawGroupRecursive(nextGroupPath, subGroup);
		}

		// セーブボタン
		if (ImGui::Button(("Save [" + groupName + "]").c_str()))
		{
			SaveFile(groupPath);
			std::string message = std::format("Updated group '{}' in {}.json", groupName, groupPath[0]);
			MessageBoxA(nullptr, message.c_str(), "JsonSettings", 0);
		}

		ImGui::TreePop();
	}
}

void JsonSettings::CreateGroup(const std::vector<std::string>& groupPath)
{
	if (groupPath.empty()) return;

	Group* current = &datas_[groupPath[0]];
	for (size_t i = 1; i < groupPath.size(); ++i)
	{
		current = &current->subGroups[groupPath[i]];
	}
}

const JsonSettings::Group* JsonSettings::FindGroup(const std::vector<std::string>& groupPath) const
{
	if (groupPath.empty()) return nullptr;

	auto it = datas_.find(groupPath[0]);
	if (it == datas_.end()) return nullptr;

	const Group* current = &it->second;
	for (size_t i = 1; i < groupPath.size(); ++i)
	{
		auto itSub = current->subGroups.find(groupPath[i]);
		if (itSub == current->subGroups.end()) return nullptr;
		current = &itSub->second;
	}
	return current;
}

JsonSettings::Group& JsonSettings::FindOrCreateGroup(const std::vector<std::string>& groupPath)
{
	assert(!groupPath.empty());
	Group* current = &datas_[groupPath[0]];
	for (size_t i = 1; i < groupPath.size(); ++i)
	{
		current = &current->subGroups[groupPath[i]];
	}
	return *current;
}

void JsonSettings::RemoveItem(const std::vector<std::string>& groupPath, const std::string& key)
{
	// メモリ上のグループを探す
	Group& targetGroup = FindOrCreateGroup(groupPath);

	// items マップから key に該当するイテレータを探す
	auto it = targetGroup.items.find(key);

	// 見つかったら items マップから削除する
	if (it != targetGroup.items.end())
	{
		targetGroup.items.erase(it);
	}
}

void JsonSettings::SaveFile(const std::vector<std::string>& groupPath)
{
	if (groupPath.empty()) {
		Logger::Log(Logger::GetStream(), "[JsonSettings] SaveFile: groupPath is empty\n");
		return;
	}

	// ファイル名はパスの先頭要素から決まる
	const std::string& topLevelName = groupPath[0];
	std::string filePathStr = std::string(kDirectoryPath_) + topLevelName + ".json";
	std::filesystem::path filePath = filePathStr;

	// プログラム内のメモリから更新対象のGroupオブジェクトを見つける
	Group* targetGroup = &datas_[topLevelName];
	for (size_t i = 1; i < groupPath.size(); ++i) {
		auto it = targetGroup->subGroups.find(groupPath[i]);
		if (it == targetGroup->subGroups.end())
		{
			Logger::Log(Logger::GetStream(),
				std::format("[JsonSettings] SaveFile: Group path not found in memory: {}\n", groupPath[i]));
			assert(false && "Group path not found in memory.");
			return;
		}
		targetGroup = &it->second;
	}

	// 既存のJSONファイルを読み込む（なければ新規作成の準備）
	json rootJson;
	std::ifstream ifs(filePath);
	if (ifs.is_open()) {
		ifs >> rootJson;
		ifs.close();
	}

	// JSONデータ内で、更新したい階層まで移動する
	json* currentJsonNode = &rootJson[topLevelName];
	for (size_t i = 1; i < groupPath.size(); ++i) {
		currentJsonNode = &(*currentJsonNode)[groupPath[i]];
	}

	// メモリ上のGroupをJSONに変換し、対象の階層を丸ごと上書きする
	*currentJsonNode = GroupToJson(*targetGroup);

	// 更新したJSONデータ全体をファイルに書き戻す
	std::ofstream ofs(filePath);
	if (ofs.fail()) {
		Logger::Log(Logger::GetStream(),
			std::format("[JsonSettings] SaveFile: Failed to open file for saving: {}\n", filePathStr));
		MessageBoxA(nullptr, "Failed to open file for saving.", "JsonSettings", MB_OK);
		assert(false);
		return;
	}

	ofs << std::setw(4) << rootJson << std::endl;
	ofs.close();

	// 保存成功ログ
	Logger::Log(Logger::GetStream(),
		std::format("[JsonSettings] Successfully saved: {}\n", filePathStr));
}

json JsonSettings::GroupToJson(const Group& group) const
{
	json j;

	// items を JSON に変換（std::visitを使用）
	for (const auto& [key, value] : group.items)
	{
		std::visit([&](auto&& val) {
			using T = std::decay_t<decltype(val)>;

			if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, float> || std::is_same_v<T, bool>) {
				j[key] = val;
			} else if constexpr (std::is_same_v<T, Vector2>) {
				j[key] = { val.x, val.y };
			} else if constexpr (std::is_same_v<T, Vector3>) {
				j[key] = { val.x, val.y, val.z };
			} else if constexpr (std::is_same_v<T, Vector4>) {
				j[key] = { val.x, val.y, val.z, val.w };
			}
			}, value);
	}

	// subGroups を再帰的に JSON に変換
	for (const auto& [subGroupName, subGroup] : group.subGroups)
	{
		j[subGroupName] = GroupToJson(subGroup);
	}

	return j;
}

void JsonSettings::LoadFiles()
{
	std::filesystem::path dir = kDirectoryPath_;

	if (!std::filesystem::exists(dir))
	{
		return;
	}

	for (const auto& entry : std::filesystem::directory_iterator(dir))
	{
		if (!entry.is_regular_file()) continue;

		const std::filesystem::path& filePath = entry.path();
		if (filePath.extension() != ".json") continue;

		std::string filename = filePath.stem().string();

		// ドットがある場合、最初のドット以降はカット
		size_t dotPos = filename.find('.');
		if (dotPos != std::string::npos)
		{
			filename = filename.substr(0, dotPos);
		}

		LoadFile(filename);
	}
}

void JsonSettings::LoadFile(const std::string& groupName)
{
	std::string filePath = std::string(kDirectoryPath_) + groupName + ".json";
	std::ifstream ifs(filePath);
	if (ifs.fail())
	{
		MessageBoxA(nullptr, "Failed to open data file for write.", "JsonSettings", 0);
		assert(false);
		return;
	}

	json root;
	ifs >> root;
	ifs.close();

	json::iterator itGroup = root.find(groupName);
	assert(itGroup != root.end());

	// 最上位グループ名から再帰的に読み込み開始
	LoadGroupRecursive({ groupName }, *itGroup);
}

void JsonSettings::LoadGroupRecursive(const std::vector<std::string>& groupPath, const json& jGroup)
{
	for (auto it = jGroup.begin(); it != jGroup.end(); ++it)
	{
		const std::string& key = it.key();
		const json& value = it.value();

		if (value.is_object())
		{
			// サブグループの場合は再帰的に処理
			std::vector<std::string> nextGroupPath = groupPath;
			nextGroupPath.push_back(key);
			LoadGroupRecursive(nextGroupPath, value);
		} else
		{
			// 値の場合は型に応じて SetValue 呼び出し

			if (value.is_number_integer())
			{
				SetValue(groupPath, key, value.get<int32_t>());
			} else if (value.is_number_float())
			{
				SetValue(groupPath, key, static_cast<float>(value.get<double>()));
			} else if (value.is_boolean())
			{
				SetValue(groupPath, key, value.get<bool>());
			} else if (value.is_array())
			{
				if (value.size() == 2)
				{
					SetValue(groupPath, key, Vector2{ value[0], value[1] });
				} else if (value.size() == 3)
				{
					SetValue(groupPath, key, Vector3{ value[0], value[1], value[2] });
				} else if (value.size() == 4)
				{
					SetValue(groupPath, key, Vector4{ value[0], value[1], value[2], value[3] });
				}
			}
		}
	}
}