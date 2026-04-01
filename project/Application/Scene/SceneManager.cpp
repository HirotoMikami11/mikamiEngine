#include "SceneManager.h"
#include <cassert>

#include "ImGui/ImGuiManager.h"
#include "Logger.h"
#include "ObjectID/ObjectIDManager.h"

SceneManager* SceneManager::GetInstance() {
	static SceneManager instance;
	return &instance;
}

void SceneManager::Initialize() {
	Logger::Log(Logger::GetStream(), "SceneManager initialized\n");
}

void SceneManager::Update() {
	// シーン切り替えの処理
	ProcessSceneChange();

	// 現在のシーンの更新
	if (currentScene_) {
		currentScene_->Update();
	}
}

void SceneManager::DrawOffscreen() {
	// 現在のシーンの3D描画（オフスクリーン内）
	if (currentScene_) {
		currentScene_->DrawOffscreen();
	}
}

void SceneManager::DrawBackBuffer() {
	// 現在のシーンのUI描画（オフスクリーン外）
	if (currentScene_) {
		currentScene_->DrawBackBuffer();
	}
}

void SceneManager::Finalize() {
	// 現在のシーンの終了処理
	if (currentScene_) {
		currentScene_->Finalize();
		currentScene_ = nullptr;
	}

	// 全シーンのクリア
	scenes_.clear();
	currentSceneName_.clear();
}

void SceneManager::RegisterScene(const std::string& sceneName, std::unique_ptr<BaseScene> scene) {
	assert(scene != nullptr);
	scenes_[sceneName] = std::move(scene);
	Logger::Log(Logger::GetStream(), std::format("registered scene: {}\n", sceneName));
}

void SceneManager::UnregisterScene(const std::string& sceneName) {
	auto it = scenes_.find(sceneName);
	if (it != scenes_.end()) {
		// 現在のシーンを削除する場合は現在のシーンをクリア
		if (currentSceneName_ == sceneName) {
			if (currentScene_) {
				currentScene_->Finalize();
			}
			currentScene_ = nullptr;
			currentSceneName_.clear();
		}
		scenes_.erase(it);
	}
}

bool SceneManager::ChangeScene(const std::string& sceneName) {
	if (!HasScene(sceneName)) {
		return false;
	}

	std::string previousSceneName = currentSceneName_;

	// 現在のシーンの処理（オブジェクトのみ解放）
	if (currentScene_) {
		currentScene_->Finalize();
		// オブジェクト初期化フラグのみリセット（リソースフラグは保持）
		currentScene_->SetInitialized(false);
	}

	// シーンを変えるときにIDを全てリセット
	ObjectIDManager::GetInstance()->ResetAllCounters();
	// ライトを平行光源しか存在しない状態にリセット
	LightManager::GetInstance()->ResetToDefault();

	// 新しいシーンの設定
	currentScene_ = scenes_[sceneName].get();
	currentSceneName_ = sceneName;

	// リソースは既に読み込み済みなので、オブジェクト初期化のみ実行
	if (!currentScene_->IsInitialized()) {
		Logger::Log(Logger::GetStream(), std::format("Initializing objects for scene: {}\n", sceneName));
		currentScene_->Initialize();
		currentScene_->SetInitialized(true);
		Logger::Log(Logger::GetStream(), std::format("Objects initialized for scene: {}\n", sceneName));
	}


	// コールバック実行
	if (sceneChangeCallback_) {
		sceneChangeCallback_(previousSceneName, currentSceneName_);
	}

	return true;
}

void SceneManager::SetNextScene(const std::string& sceneName) {
	if (HasScene(sceneName)) {
		nextSceneName_ = sceneName;
		sceneChangeRequested_ = true;
	}
}

void SceneManager::ResetScene(const std::string& sceneName) {
	auto it = scenes_.find(sceneName);
	if (it != scenes_.end()) {
		// 現在のシーンをリセットする場合
		if (currentSceneName_ == sceneName && currentScene_) {
			currentScene_->Finalize();

			// シーンを変えるときにIDを全てリセット
			ObjectIDManager::GetInstance()->ResetAllCounters();
			// ライトを平行光源しか存在しない状態にリセット
			LightManager::GetInstance()->ResetToDefault();

			currentScene_->Initialize();
			currentScene_->SetInitialized(true);
		} else {
			// 非アクティブなシーンをリセット
			it->second->Finalize();
			it->second->SetInitialized(false);
		}
	}
}

void SceneManager::ResetCurrentScene() {
	if (currentScene_ && !currentSceneName_.empty()) {
		ResetScene(currentSceneName_);
	}
}

bool SceneManager::HasScene(const std::string& sceneName) const {
	return scenes_.find(sceneName) != scenes_.end();
}

void SceneManager::ProcessSceneChange() {
	if (sceneChangeRequested_) {
		ChangeScene(nextSceneName_);
		sceneChangeRequested_ = false;
		nextSceneName_.clear();
	}
}

void SceneManager::ImGui() {
#ifdef USEIMGUI
	ImGui::Begin("シーンマネージャー");

	// シーン管理UI
	DrawScenesUI();

	// 現在のシーンのUI
	DrawCurrentSceneUI();

	ImGui::End();
#endif
}

void SceneManager::DrawScenesUI() {
#ifdef USEIMGUI

	// 登録されているシーン一覧とボタン
	ImGui::Separator();
	MyImGui::CenterText("シーン一覧");
	ImGui::Separator();

	for (const auto& [sceneName, scene] : scenes_) {
		ImGui::PushID(sceneName.c_str());

		// 現在のシーンかどうかで色を変える
		bool isCurrent = (currentSceneName_ == sceneName);
		if (isCurrent) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.8f, 0.2f, 0.8f));
		}

		// シーン切り替えボタン
		if (ImGui::Button(sceneName.c_str(), ImVec2(120, 0))) {
			if (!isCurrent) {
				SetNextScene(sceneName);
			}
		}
		// リセットボタン
		ImGui::SameLine();
		if (ImGui::SmallButton(("リセット##" + sceneName).c_str())) {
			ResetScene(sceneName);
		}
		if (isCurrent) {
			ImGui::PopStyleColor(2);
		} else {
		}
		ImGui::PopID();
	}

	// 現在のシーンのリセットボタン
	if (currentScene_) {
		ImGui::Spacing();
		if (ImGui::Button("現在のシーンをリセット", ImVec2(200, 0))) {
			ResetCurrentScene();
		}
	}
	// シーン切り替え要求があるかの表示
	if (sceneChangeRequested_) {
		ImGui::Spacing();
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Next Scene: %s", nextSceneName_.c_str());
		ImGui::Text("(Will change next frame)");
	}
#endif
}

void SceneManager::DrawCurrentSceneUI() {
#ifdef USEIMGUI
	ImGui::Begin("現在のシーン情報");
	if (currentScene_) {
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "シーン名: %s", currentSceneName_.c_str());
		ImGui::Separator();
		// 現在のシーンのImGuiを呼び出し
		currentScene_->ImGui();
	} else {
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "現在有効なシーンが存在しません");
	}

	ImGui::End();
#endif
}