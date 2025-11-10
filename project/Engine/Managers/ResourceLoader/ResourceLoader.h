#pragma once
#include <string>
#include <vector>

#include "Managers/Texture/TextureManager.h"
#include "Managers/Model/ModelManager.h"
#include "Managers/Audio/AudioManager.h"
#include "Logger.h"

/// <summary>
/// テクスチャ情報
/// </summary>
struct TextureInfo {
	std::string filePath;
	std::string tag;
};

/// <summary>
/// モデル情報
/// </summary>
struct ModelInfo {
	std::string directoryPath;
	std::string filename;
	std::string tag;
	bool isPrimitive = false;
	MeshType meshType = MeshType::SPHERE;
};

/// <summary>
/// 音声情報
/// </summary>
struct AudioInfo {
	std::string filePath;
	std::string tag;
};

/// <summary>
/// 全リソースの読み込みを一元管理するクラス
/// 
/// 責任範囲：
/// - リソース定義の集約（どのリソースを読み込むか）
/// - 読み込み指示の発行（各Managerへの依頼）
/// - 統合ビューの提供（ImGuiで全体表示）
/// 
/// 責任外：
/// - Managerの初期化（Engineが担当）
/// - リソースの実体管理（各Managerが担当）
/// </summary>
class ResourceLoader {
public:
	static ResourceLoader* GetInstance();

	/// <summary>
	/// 初期化
	/// 注意：この時点で各Managerは既に初期化済みであること
	/// </summary>
	void Initialize();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 全リソースの読み込み
	/// 定義されたリソースを各Managerに読み込ませる
	/// </summary>
	void LoadAllResources();

	/// <summary>
	/// ImGui表示（統合ビュー）
	/// 各Managerの詳細はManager側で表示
	/// </summary>
	void ImGui();

	/// <summary>
	/// リソースが読み込み済みかどうか
	/// </summary>
	bool IsResourcesLoaded() const { return resourcesLoaded_; }

private:
	ResourceLoader() = default;
	~ResourceLoader() = default;
	ResourceLoader(const ResourceLoader&) = delete;
	ResourceLoader& operator=(const ResourceLoader&) = delete;

	/// <summary>
	/// 全リソースの定義を登録
	/// ここに全てのテクスチャ・モデル・音声を定義
	/// </summary>
	void RegisterAllResources();

	/// <summary>
	/// 登録されたリソースの読み込み実行
	/// </summary>
	bool LoadResources();

	// Manager参照（初期化は行わない、参照するのみ）
	TextureManager* textureManager_ = nullptr;
	ModelManager* modelManager_ = nullptr;
	AudioManager* audioManager_ = nullptr;

	// 全リソースの定義（読み込むべきリソースのリスト）
	std::vector<TextureInfo> textures_;
	std::vector<ModelInfo> models_;
	std::vector<AudioInfo> audios_;

	// 読み込み完了フラグ
	bool resourcesLoaded_ = false;
};