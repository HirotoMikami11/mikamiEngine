#include "TextureManager.h"
#include "ImGui/ImGuiManager.h"

// シングルトンインスタンス
TextureManager* TextureManager::GetInstance() {
	static TextureManager instance;
	return &instance;
}

TextureManager::~TextureManager() {
	Finalize();
}

void TextureManager::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// 初期化できたらログを出す
	Logger::Log(Logger::GetStream(), "Complete TextureManager initialized !!\n");
}

void TextureManager::Finalize() {
	// 全てのテクスチャを解放
	UnloadAll();
	dxCommon_ = nullptr;
}
bool TextureManager::LoadTexture(const std::string& filename, const std::string& tagName) {

	// 既に同じタグ名で登録されている場合はスキップ（成功として扱う）
	if (HasTexture(tagName)) {
		Logger::Log(Logger::GetStream(), std::format("Texture with tag '{}' already exists. Skipping load.\n", tagName));
		return true; // 既存のテクスチャを使用
	}

	// DescriptorHeapManagerからSRVを割り当て
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (!descriptorManager) {
		// DescriptorManagerが無効な場合はログを出力
		Logger::Log(Logger::GetStream(), "DescriptorManager is null\n");
		return false;
	}

	// 使用可能なSRVスロットを割り当て
	// Allotaterで自動であいてるスロットに確保する
	auto descriptorHandle = descriptorManager->AllocateSRV();
	if (!descriptorHandle.isValid) {
		// SRVのヒープが最大値の場合はログを出力して失敗
		Logger::Log(Logger::GetStream(), std::format("Failed to allocate SRV for texture '{}': No available slots.\n", filename));
		return false;
	}

	// 新しいテクスチャを作成し、既に割り当てられたハンドルを使用
	auto texture = std::make_unique<Texture>();
	// テクスチャをロード
	if (!texture->LoadTextureWithHandle(filename, dxCommon_, descriptorHandle)) {
		// ロードに失敗した場合はSRVを解放
		descriptorManager->ReleaseSRV(descriptorHandle.index);
		return false;
	}

	// マップに登録
	textures_[tagName] = std::move(texture);

	Logger::Log(Logger::GetStream(), std::format("Texture '{}' loaded successfully with tag '{}' (SRV Index: {})\n", filename, tagName, descriptorHandle.index));
	return true;
}

DirectX::TexMetadata TextureManager::GetTextureMetadata(const std::string& tagName) const {
	auto it = textures_.find(tagName);
	if (it != textures_.end()) {
		return it->second->GetMetadata();
	}

	// テクスチャが見つからない場合は空のメタデータにして返す
	Logger::Log(Logger::GetStream(),
		std::format("Texture with tag '{}' not found for metadata retrieval.\n", tagName));
	assert(it == textures_.end());
	return DirectX::TexMetadata{};
}

Texture* TextureManager::GetTexture(const std::string& tagName) {
	auto it = textures_.find(tagName);
	if (it != textures_.end()) {
		return it->second.get();
	}

	// テクスチャが見つからない場合はnullptr
	Logger::Log(Logger::GetStream(),
		std::format("Texture with tag '{}' not found.\n", tagName));
	return nullptr;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetTextureHandle(const std::string& tagName) {
	Texture* texture = GetTexture(tagName);
	if (texture) {
		return texture->GetGPUHandle();
	}

	// デフォルトハンドル（無効な値）を返す
	D3D12_GPU_DESCRIPTOR_HANDLE invalidHandle = {};
	return invalidHandle;
}

void TextureManager::UnloadTexture(const std::string& tagName) {
	auto textureIt = textures_.find(tagName);
	if (textureIt != textures_.end()) {
		// テクスチャをアンロード（内部でSRVも解放される）
		textureIt->second->Unload(dxCommon_);

		// マップから削除
		textures_.erase(textureIt);

		Logger::Log(Logger::GetStream(), std::format("Texture with tag '{}' unloaded.\n", tagName));
	}
}

void TextureManager::UnloadAll() {
	// 全てのテクスチャを解放
	for (const auto& pair : textures_) {
		Logger::Log(Logger::GetStream(),
			std::format("Unloading texture: {}\n", pair.first));
		pair.second->Unload(dxCommon_);
	}

	textures_.clear();

	Logger::Log(Logger::GetStream(), "All textures unloaded.\n");
}

bool TextureManager::HasTexture(const std::string& tagName) const {
	// 指定されたタグ名のテクスチャが存在するかチェック
	return textures_.find(tagName) != textures_.end();
}

std::vector<std::string> TextureManager::GetTextureTagList() const {
	std::vector<std::string> tagList;
	tagList.reserve(textures_.size()); // メモリの効率化

	for (const auto& pair : textures_) {
		tagList.push_back(pair.first);
	}
	// アルファベット順にソート
	std::sort(tagList.begin(), tagList.end());

	return tagList;
}


uint32_t TextureManager::GetAvailableSRVCount() const {
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (descriptorManager) {
		return descriptorManager->GetAvailableCount(DescriptorHeapManager::HeapType::SRV);
	}
	return 0;
}

uint32_t TextureManager::GetUsedSRVCount() const {
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (descriptorManager) {
		return descriptorManager->GetUsedCount(DescriptorHeapManager::HeapType::SRV);
	}
	return 0;
}

void TextureManager::ImGui() {
#ifdef USEIMGUI
	// テクスチャの総数
	ImGui::Text("Total Textures: %zu", textures_.size());

	// SRVスロットの使用状況
	uint32_t usedSRV = GetUsedSRVCount();
	uint32_t availableSRV = GetAvailableSRVCount();
	ImGui::Text("SRV Slots: %u used / %u available", usedSRV, availableSRV);

	ImGui::Separator();

	// テクスチャが存在しない場合
	if (textures_.empty()) {
		ImGui::TextDisabled("No textures loaded");
		return;
	}

	// テクスチャ一覧の表示
	if (ImGui::TreeNode("Texture List")) {
		// アルファベット順にソート済みのリストを取得
		auto tagList = GetTextureTagList();

		for (const auto& tag : tagList) {
			ImGui::PushID(tag.c_str());

			// テクスチャの存在確認（念のため）
			bool exists = HasTexture(tag);
			if (exists) {
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[OK]");
			} else {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[ERROR]");
			}


			ImGui::SameLine();

			// ツリーノードで詳細表示
			if (ImGui::TreeNode(tag.c_str())) {
				Texture* texture = GetTexture(tag);
				if (texture) {
					// ファイルパス表示
					ImGui::Text("Path: %s", texture->GetFilePath().c_str());

					// メタデータ表示
					const auto& metadata = texture->GetMetadata();
					ImGui::Text("Size: %llux%llu", metadata.width, metadata.height);
					ImGui::Text("Mip Levels: %zu", metadata.mipLevels);
					ImGui::Text("SRV Index: %u", texture->GetSRVIndex());

					// アンロードボタン
					if (ImGui::SmallButton("Unload")) {
						UnloadTexture(tag);
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