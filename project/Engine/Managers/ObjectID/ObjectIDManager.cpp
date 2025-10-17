#include "ObjectIDManager.h"
#include "Logger.h"
#include "Managers/ImGui/ImGuiManager.h"


ObjectIDManager* ObjectIDManager::GetInstance() {
	static ObjectIDManager instance;
	return &instance;
}

std::string ObjectIDManager::GenerateName(const std::string& baseName, const std::string& typeName) {
	// typeNameが空の場合はbaseNameを使用
	std::string type = typeName.empty() ? baseName : typeName;
	//次のIDを取得
	int id = GetNextID(type);
	//種類と番号を返す
	return std::format("{}_{}", baseName, id);
}


std::string ObjectIDManager::GenerateNameModel(const std::string& baseName, const std::string& typeName)
{
	// typeNameが空の場合はbaseNameを使用
	std::string type = typeName.empty() ? baseName : typeName;
	//次のIDを取得
	int id = GetNextID(type);
	//種類と番号を返す
	return std::format("Model3D_{}({})",id, baseName );
}

int ObjectIDManager::GetCurrentCount(const std::string& typeName) const {
	//現在のカウントを数えて返す
	auto it = counters_.find(typeName);
	return (it != counters_.end()) ? it->second : 0;
}

void ObjectIDManager::ResetCounter(const std::string& typeName) {
	//特定の種類のカウンター(ID番号)をリセットする

	auto it = counters_.find(typeName);
	if (it != counters_.end()) {
		it->second = 0;
		Logger::Log(Logger::GetStream(), std::format("ObjectIDManager: Reset counter for '{}'\n", typeName));
	}

}

void ObjectIDManager::ResetAllCounters() {
	//全てのカウンター（ID）の数をリセットする
	for (auto& [typeName, counter] : counters_) {
		counter = 0;
	}
	Logger::Log(Logger::GetStream(), "ObjectIDManager: All counters have been reset\n");
}

void ObjectIDManager::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode("Object ID Manager")) {
		ImGui::Text("Object Type Counters:");
		ImGui::Separator();

		for (const auto& [typeName, counter] : counters_) {
			ImGui::Text("%s: %d", typeName.c_str(), counter);
		}

		ImGui::Separator();
		if (ImGui::Button("Reset All Counters")) {
			ResetAllCounters();
		}

		ImGui::TreePop();
	}
#endif
}

void ObjectIDManager::RegisterType(const std::string& typeName) {
	//タイプを追加する
	if (counters_.find(typeName) == counters_.end()) {
		//見つからなかったら追加
		counters_[typeName] = 0;
		Logger::Log(Logger::GetStream(), std::format("ObjectIDManager: Registered type '{}'\n", typeName));
	}
}

int ObjectIDManager::GetNextID(const std::string& typeName) {
	// タイプが未登録の場合は自動登録
	RegisterType(typeName);

	// カウンターをインクリメントして返す
	return ++counters_[typeName];
}