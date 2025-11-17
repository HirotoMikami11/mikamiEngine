#include "State/IdleState.h"
#include "Boss.h"
#include "ImGui/ImGuiManager.h"

void IdleState::Initialize() {
	// 待機状態では特に初期化処理なし
}

void IdleState::Update(Boss* boss) {
	// 待機状態では何もしない
	// bossパラメータは使用しないが、インターフェース統一のため残す
	(void)boss;
}

void IdleState::ImGui() {
#ifdef USEIMGUI
	ImGui::Text("State: Idle");
	ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Waiting for next action...");
#endif
}