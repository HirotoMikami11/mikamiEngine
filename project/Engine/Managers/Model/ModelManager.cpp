#include "ModelManager.h"
#include "ImGui/ImGuiManager.h"
#include <cassert>
#include <algorithm>
#include <vector>

ModelManager* ModelManager::GetInstance() {
	static ModelManager instance;
	return &instance;
}

ModelManager::~ModelManager() {
	Finalize();
}

void ModelManager::Initialize(DirectXCommon* dxCommon) {
	directXCommon_ = dxCommon;
	textureManager_ = TextureManager::GetInstance();

	Logger::Log(Logger::GetStream(), "ModelManager initialized !!\n");
}

void ModelManager::Finalize() {
	// 全てのモデルを解放
	UnloadAll();
	directXCommon_ = nullptr;
	textureManager_ = nullptr;
}

bool ModelManager::LoadModel(const std::string& directoryPath, const std::string& filename, const std::string& tagName) {
	// 既に同じタグ名で登録されている場合はスキップ（成功として扱う）
	if (HasModel(tagName)) {
		Logger::Log(Logger::GetStream(), std::format("Model with tag '{}' already exists. Skipping load.\n", tagName));
		return true;
	}

	// 新しいモデルを作成
	auto model = std::make_unique<Model>();

	// モデルをロード
	if (!model->LoadFromOBJ(directoryPath, filename, directXCommon_)) {
		Logger::Log(Logger::GetStream(), std::format("Failed to load model: {} from {}\n", filename, directoryPath));
		return false;
	}

	// マップに登録
	models_[tagName] = std::move(model);

	Logger::Log(Logger::GetStream(), std::format("Model '{}' loaded successfully with tag '{}'\n", filename, tagName));
	return true;
}

bool ModelManager::LoadPrimitive(MeshType meshType, const std::string& tagName) {
	// 既に同じタグ名で登録されている場合はスキップ（成功として扱う）
	if (HasModel(tagName)) {
		Logger::Log(Logger::GetStream(), std::format("Primitive model with tag '{}' already exists. Skipping load.\n", tagName));
		return true;
	}

	// 新しいプリミティブモデルを作成
	auto model = std::make_unique<Model>();

	// プリミティブモデルをロード
	if (!model->LoadFromPrimitive(meshType, directXCommon_)) {
		Logger::Log(Logger::GetStream(), std::format("Failed to load primitive model: {}\n", Mesh::MeshTypeToString(meshType)));
		return false;
	}

	// マップに登録
	models_[tagName] = std::move(model);

	Logger::Log(Logger::GetStream(), std::format("Primitive model '{}' loaded successfully with tag '{}'\n",
		Mesh::MeshTypeToString(meshType), tagName));
	return true;
}

Model* ModelManager::GetModel(const std::string& tagName) {
	auto it = models_.find(tagName);
	if (it != models_.end()) {
		return it->second.get();
	}

	// モデルが見つからない場合はnullptr
	Logger::Log(Logger::GetStream(), std::format("Model with tag '{}' not found.\n", tagName));
	return nullptr;
}

void ModelManager::UnloadModel(const std::string& tagName) {
	auto modelIt = models_.find(tagName);
	if (modelIt != models_.end()) {
		// モデルをアンロード
		modelIt->second->Unload();

		// マップから削除
		models_.erase(modelIt);

		Logger::Log(Logger::GetStream(), std::format("Model with tag '{}' unloaded.\n", tagName));
	}
}

void ModelManager::UnloadAll() {
	// 全てのモデルを解放
	for (const auto& pair : models_) {
		Logger::Log(Logger::GetStream(), std::format("Unloading model: {}\n", pair.first));
		pair.second->Unload();
	}

	models_.clear();

	Logger::Log(Logger::GetStream(), "All models unloaded.\n");
}

bool ModelManager::HasModel(const std::string& tagName) const {
	return models_.find(tagName) != models_.end();
}

void ModelManager::ImGui() {
#ifdef USEIMGUI
	// モデルの総数
	ImGui::Text("Total Models: %zu", models_.size());

	ImGui::Separator();

	// モデルが存在しない場合
	if (models_.empty()) {
		ImGui::TextDisabled("No models loaded");
		return;
	}

	// モデル一覧の表示
	if (ImGui::TreeNode("Model List")) {
		// タグ名リストをアルファベット順にソート
		std::vector<std::string> tagList;
		tagList.reserve(models_.size());
		for (const auto& pair : models_) {
			tagList.push_back(pair.first);
		}
		std::sort(tagList.begin(), tagList.end());

		for (const auto& tag : tagList) {
			ImGui::PushID(tag.c_str());

			// モデルの存在確認（念のため）
			bool exists = HasModel(tag);
			if (exists) {
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[OK]");
			} else {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[ERROR]");
			}

			ImGui::SameLine();

			// ツリーノードで詳細表示
			if (ImGui::TreeNode(tag.c_str())) {
				Model* model = GetModel(tag);
				if (model) {
					// シンプルな情報表示（メッシュ数とマテリアル数のみ）
					ImGui::Text("Meshes: %zu", model->GetMeshCount());
					ImGui::Text("Materials: %zu", model->GetMaterialCount());

					// アンロードボタン
					ImGui::Spacing();
					if (ImGui::SmallButton("Unload")) {
						UnloadModel(tag);
					}
				}
				ImGui::TreePop();
			}

			ImGui::PopID();
		}

		ImGui::TreePop();
	}

	ImGui::Separator();
#endif
}