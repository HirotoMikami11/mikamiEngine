#include "ResourceLoader.h"
#include "ImGui/ImGuiManager.h"

ResourceLoader* ResourceLoader::GetInstance() {
	static ResourceLoader instance;
	return &instance;
}

void ResourceLoader::Initialize() {
	// 各Managerの参照を取得（既に初期化済みであること前提）
	textureManager_ = TextureManager::GetInstance();
	modelManager_ = ModelManager::GetInstance();
	audioManager_ = AudioManager::GetInstance();

	// Managerが初期化されているか確認
	assert(textureManager_ != nullptr && "TextureManager must be initialized before ResourceLoader");
	assert(modelManager_ != nullptr && "ModelManager must be initialized before ResourceLoader");
	assert(audioManager_ != nullptr && "AudioManager must be initialized before ResourceLoader");

	// リソース定義を登録
	RegisterAllResources();

	Logger::Log(Logger::GetStream(), "ResourceLoader initialized !!\n");
}

void ResourceLoader::Finalize() {
	// 定義データのクリア
	textures_.clear();
	models_.clear();
	audios_.clear();

	resourcesLoaded_ = false;

	// 参照をクリア（解放はしない、各Managerが自分で管理）
	textureManager_ = nullptr;
	modelManager_ = nullptr;
	audioManager_ = nullptr;

	Logger::Log(Logger::GetStream(), "ResourceLoader finalized\n");
}

void ResourceLoader::RegisterAllResources() {
	///*-----------------------------------------------------------------------*///
	///								テクスチャの定義								///
	///*-----------------------------------------------------------------------*///
	textures_ = {
		// 汎用テクスチャ
		{"resources/Texture/uvChecker.png", "uvChecker"},
		{"resources/Texture/monsterBall.png", "monsterBall"},
		{"resources/Texture/white2x2.png", "white"},
		{"resources/Texture/circle.png", "circle"},
		{"resources/Texture/gaugeLight.png", "gaugeLight"},
		{"resources/Texture/field.png", "field"},
		{"resources/Texture/sousa.png", "sousa"},
		{"resources/Texture/pause1.png", "pause1"},
		{"resources/Texture/pause2.png", "pause2"}

		// テクスチャここに追加
		// {"resources/Texture/example.png", "example"},
	};

	///*-----------------------------------------------------------------------*///
	///								モデルの定義									///
	///*-----------------------------------------------------------------------*///
	models_ = {
		// プリミティブモデル
		{"", "", "sphere", true, MeshType::SPHERE},
		{"", "", "triangle", true, MeshType::TRIANGLE},
		{"", "", "plane", true, MeshType::PLANE},

		// OBJモデル
		{"resources/Model/Cube", "Cube.obj", "cube", false},
		{"resources/Model/MultiMesh", "multiMesh.obj", "model_MultiMesh", false},
		{"resources/Model/MultiMaterial", "multiMaterial.obj", "model_MultiMaterial", false},
		{"resources/Model/Goldfish", "Mesh_Goldfish.obj", "modelfish", false},
		{"resources/Model/terrain", "terrain.obj", "model_terrain", false},
		{"resources/Model/Player", "Player.obj", "model_Player", false},
		{"resources/Model/PlayerBullet", "playerBullet.obj", "playerBullet", false},
		{"resources/Model/FontModel", "TitleFont2.obj", "titleFont", false},
		{"resources/Model/FontModel", "GameOverFont.obj", "overFont", false},
		{"resources/Model/FontModel", "GameClearFont.obj", "clearFont", false},
		{"resources/Model/FontModel", "PressA.obj", "pressAFont", false},
		{"resources/Model/titleWallRock", "titleWallRock.obj", "titleWall", false},
		{"resources/Model/wallRock", "wallRock.obj", "wall", false},
		{"resources/Model/fieldRock", "fieldRock.obj", "field", false},
		{"resources/Model/titleFieldRock", "titleFieldRock.obj", "titleField", false},
		{"resources/Model/torch", "torch.obj", "torch", false},
		{"resources/Model/Boss", "Boss_Body.obj", "Boss_Body", false},
		{"resources/Model/Boss", "Boss_Head.obj", "Boss_Head", false},
		{"resources/Model/Boss", "Boss_Tail.obj", "Boss_Tail", false},
		{"resources/Model/wallRock", "Boss_Tail.obj", "Boss_Tail", false},
		{"resources/Model/treasureBox", "treasureBox.obj", "treasureBox", false},


		// モデルはここに追加
		// {"resources/Model/NewModel", "newModel.obj", "model_New", false},
	};

	///*-----------------------------------------------------------------------*///
	///							音声の定義										///
	///*-----------------------------------------------------------------------*///
	audios_ = {
		// 音声ここに追加
		 //{"resources/Audio/Alarm01.wav", "Alarm"},
		 //{"resources/Audio/Bgm01.mp3", "BGM"},
		 //{"resources/Audio/Se01.mp3", "SE"},
		 {"resources/Audio/Explosion.mp3", "Explosion"},
		 {"resources/Audio/Smoke.mp3", "Smoke"},
		 {"resources/Audio/PressA.mp3", "PressA"},
		 {"resources/Audio/GameBGM.mp3", "GameBGM"},
		 {"resources/Audio/TitleBGM.mp3", "TitleBGM"},
		 {"resources/Audio/ClearBGM.mp3", "ClearBGM"},
		 {"resources/Audio/TitleVoice.mp3", "TitleVoice"},
		 {"resources/Audio/PlayerHit.mp3", "PlayerHit"},
		 {"resources/Audio/EnemyHit.mp3", "EnemyHit"},
		 {"resources/Audio/EnemyHitMuteki.mp3", "EnemyHitMuteki"},
		 {"resources/Audio/PauseMove.mp3", "PauseMove"},
		 {"resources/Audio/PlayerShot.mp3", "PlayerShot"},
		 {"resources/Audio/EnemyShot.mp3", "EnemyShot"},
		 {"resources/Audio/EnemyShot2.mp3", "EnemyShot2"},

	};
}

void ResourceLoader::LoadAllResources() {
	if (resourcesLoaded_) {
		Logger::Log(Logger::GetStream(), "Resources already loaded. Skipping.\n");
		return;
	}

	Logger::Log(Logger::GetStream(), "Loading all resources\n");

	bool success = LoadResources();

	if (success) {
		resourcesLoaded_ = true;
		Logger::Log(Logger::GetStream(), "All resources loaded successfully!\n");
	} else {
		Logger::Log(Logger::GetStream(), "Warning: Some resources failed to load\n");
	}
}

bool ResourceLoader::LoadResources() {
	bool allSuccess = true;
	int textureSuccess = 0;
	int textureFailed = 0;
	int modelSuccess = 0;
	int modelFailed = 0;
	int audioSuccess = 0;
	int audioFailed = 0;

	///*-----------------------------------------------------------------------*///
	///								テクスチャの読み込み							///
	///*-----------------------------------------------------------------------*///
	Logger::Log(Logger::GetStream(), "Loading textures...\n");
	for (const auto& texInfo : textures_) {
		if (textureManager_->LoadTexture(texInfo.filePath, texInfo.tag)) {
			textureSuccess++;
		} else {
			Logger::Log(Logger::GetStream(),
				std::format("  [FAILED] Texture: {} (tag: {})\n",
					texInfo.filePath, texInfo.tag));
			textureFailed++;
			allSuccess = false;
		}
	}

	///*-----------------------------------------------------------------------*///
	///								モデルの読み込み								///
	///*-----------------------------------------------------------------------*///
	Logger::Log(Logger::GetStream(), "Loading models...\n");
	for (const auto& modelInfo : models_) {
		bool success = false;

		if (modelInfo.isPrimitive) {
			success = modelManager_->LoadPrimitive(modelInfo.meshType, modelInfo.tag);
		} else {
			success = modelManager_->LoadModel(
				modelInfo.directoryPath,
				modelInfo.filename,
				modelInfo.tag
			);
		}

		if (success) {
			modelSuccess++;
		} else {
			Logger::Log(Logger::GetStream(),
				std::format("  [FAILED] Model: {} (tag: {})\n",
					modelInfo.filename.empty() ? "Primitive" : modelInfo.filename,
					modelInfo.tag));
			modelFailed++;
			allSuccess = false;
		}
	}

	///*-----------------------------------------------------------------------*///
	///								音声の読み込み									///
	///*-----------------------------------------------------------------------*///
	Logger::Log(Logger::GetStream(), "Loading audios...\n");
	for (const auto& audioInfo : audios_) {
		// AudioManager::LoadAudio は現在エラーチェックを返さないため、
		// 成功として扱う（必要に応じてAudioManager側を改善）
		audioManager_->LoadAudio(audioInfo.filePath, audioInfo.tag);
		audioSuccess++;
	}

	///*-----------------------------------------------------------------------*///
	///								読み込み結果									///
	///*-----------------------------------------------------------------------*///
	Logger::Log(Logger::GetStream(), "\nResource Loading Summary\n");
	Logger::Log(Logger::GetStream(),
		std::format("Textures: {}/{} succeeded, {} failed\n",
			textureSuccess, textures_.size(), textureFailed));
	Logger::Log(Logger::GetStream(),
		std::format("Models:   {}/{} succeeded, {} failed\n",
			modelSuccess, models_.size(), modelFailed));
	Logger::Log(Logger::GetStream(),
		std::format("Audios:   {}/{} succeeded, {} failed\n",
			audioSuccess, audios_.size(), audioFailed));
	Logger::Log(Logger::GetStream(), "\n");

	return allSuccess;
}

void ResourceLoader::ImGui() {
#ifdef USEIMGUI
	if (ImGui::CollapsingHeader("ResourceLoader")) {
		// 読み込み状態の表示
		ImGui::Text("Status:");
		ImGui::SameLine();
		if (resourcesLoaded_) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Loaded");
		} else {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not Loaded");
		}

		ImGui::Separator();

		///*-------------------------------------------------------------------*///
		///				 実際に読み込まれているリソース数（Manager経由）				///
		///*-------------------------------------------------------------------*///
		ImGui::Text("Loaded Resources (from Managers):");
		ImGui::Indent();

		// TextureManagerから実際の数を取得
		ImGui::Text("Textures: %zu", textureManager_->GetTextureCount());

		// ModelManagerから実際の数を取得
		ImGui::Text("Models:   %zu", modelManager_->GetModelCount());

		// AudioManagerから実際Data数を取得
		ImGui::Text("Audios:   %zu", audioManager_->GetAudioDataCount());

		ImGui::Unindent();

		ImGui::Separator();

		///*-------------------------------------------------------------------*///
		///					各Managerの詳細ImGui（委譲）							///
		///*-------------------------------------------------------------------*///
		ImGui::Text("Manager Details:");

		// TextureManagerのImGuiを呼び出し
		if (ImGui::TreeNode("TextureManager")) {
			textureManager_->ImGui();
			ImGui::TreePop();
		}

		// ModelManagerのImGuiを呼び出し
		if (ImGui::TreeNode("ModelManager")) {
			modelManager_->ImGui();
			ImGui::TreePop();
		}

		// AudioManagerのImGuiを呼び出し
		if (ImGui::TreeNode("AudioManager")) {
			audioManager_->ImGui();
			ImGui::TreePop();
		}

		ImGui::Separator();

		///*-------------------------------------------------------------------*///
		///							再読み込みボタン								///
		///*-------------------------------------------------------------------*///
		if (!resourcesLoaded_) {
			if (ImGui::Button("Load All Resources")) {
				LoadAllResources();
			}
		} else {
			ImGui::TextDisabled("Resources already loaded");
		}
	}
#endif
}