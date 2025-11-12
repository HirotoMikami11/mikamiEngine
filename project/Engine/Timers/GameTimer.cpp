#define NOMINMAX
#include "GameTimer.h"
#include "Managers/ImGui/ImGuiManager.h"
#include <algorithm>

GameTimer::GameTimer()
	: realDeltaTime_(0.0f)
	, gameDeltaTime_(0.0f)
	, realTime_(0.0f)
	, gameTime_(0.0f)
	, timeScale_(1.0f)
	, previousTimeScale_(1.0f)
	, isPaused_(false)
	, slowMotionScale_(0.3f)
{
}

void GameTimer::Update(float realDeltaTime) {
	// 実時間のデルタタイムを保存
	realDeltaTime_ = realDeltaTime;

	// 一時停止中の場合、ゲーム内時間は進めない
	if (isPaused_) {
		gameDeltaTime_ = 0.0f;
	} else {
		// タイムスケールを適用してゲーム内デルタタイムを計算
		gameDeltaTime_ = realDeltaTime_ * timeScale_;
	}

	// 経過時間を更新
	realTime_ += realDeltaTime_;
	gameTime_ += gameDeltaTime_;
}

void GameTimer::SetTimeScale(float scale) {
	// タイムスケールは負数にならないようにクランプ
	timeScale_ = std::max(0.0f, scale);

	// 一時停止状態を解除（タイムスケールが0の場合は実質停止だが、フラグは解除）
	if (timeScale_ > 0.0f && isPaused_) {
		isPaused_ = false;
	}
}

void GameTimer::Pause() {
	if (!isPaused_) {
		previousTimeScale_ = timeScale_;	// 現在のスケールを保存
		isPaused_ = true;
	}
}

void GameTimer::Resume() {
	if (isPaused_) {
		isPaused_ = false;
		// タイムスケールが0の場合は前回の値に戻す
		if (timeScale_ == 0.0f) {
			timeScale_ = previousTimeScale_;
		}
	}
}

void GameTimer::TogglePause() {
	if (isPaused_) {
		Resume();
	} else {
		Pause();
	}
}

void GameTimer::StartSlowMotion(float scale) {
	slowMotionScale_ = std::max(0.0f, std::min(1.0f, scale));	// 0.0f～1.0fにクランプ
	SetTimeScale(slowMotionScale_);
}

void GameTimer::ResetToNormalSpeed() {
	SetTimeScale(1.0f);
	if (isPaused_) {
		Resume();
	}
}

void GameTimer::Reset() {
	realTime_ = 0.0f;
	gameTime_ = 0.0f;
	realDeltaTime_ = 0.0f;
	gameDeltaTime_ = 0.0f;
	timeScale_ = 1.0f;
	isPaused_ = false;
}

void GameTimer::ImGui() {
#ifdef USEIMGUI

	if (ImGui::CollapsingHeader("GameTimer")) {

		// 状態表示
		ImGui::Separator();
		ImGui::Text("Game Delta: %.6f s (%.2f ms)", gameDeltaTime_, gameDeltaTime_ * 1000.0f);
		ImGui::Text("Time Scale: %.2fx", timeScale_);
		ImGui::Text("Paused: %s", isPaused_ ? "Yes" : "No");

		// 速度インジケーター
		if (timeScale_ == 0.0f || isPaused_) {
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Time: STOPPED");
		} else if (timeScale_ < 0.5f) {
			ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Time: VERY SLOW");
		} else if (timeScale_ < 1.0f) {
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "Time: SLOW");
		} else if (timeScale_ == 1.0f) {
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "Time: NORMAL");
		} else {
			ImGui::TextColored(ImVec4(0, 0.5f, 1, 1), "Time: FAST");
		}

		ImGui::Separator();
		ImGui::Spacing();
		// タイムスケール制御
		ImGui::Text("Time Scale Control");

		// タイムスケールスライダー
		float tempScale = timeScale_;
		if (ImGui::SliderFloat("Time Scale", &tempScale, 0.0f, 3.0f, "%.2f")) {
			SetTimeScale(tempScale);
		}

		ImGui::Spacing();
		ImGui::Separator();
		// 一時停止制御
		ImGui::Text("Pause Control");
		if (isPaused_) {
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Status: PAUSED");
			if (ImGui::Button("Resume")) {
				TogglePause();
			}
		} else {
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "Status: Running");
			if (ImGui::Button("Pause")) {
				TogglePause();
			}
		}
		ImGui::Spacing();
		ImGui::Separator();
		// スローモーション設定
		ImGui::Text("Slow Motion");
		ImGui::SliderFloat("Slow Motion Scale", &slowMotionScale_, 0.1f, 1.0f, "%.2f");
		if (ImGui::Button("Start Slow Motion")) {
			StartSlowMotion(slowMotionScale_);
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset to Normal")) {
			ResetToNormalSpeed();
		}




	}

#endif
}