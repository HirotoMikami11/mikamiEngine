#include "BossStateManager.h"
#include "Boss.h"
#include "State/IdleState.h"
#include "State/SplineMoveState.h"
#include "State/SplineMove8WayShootState.h"
#include "State/SplineMoveRotateShootState.h"
#include "Random/Random.h"
#include "ImGui/ImGuiManager.h"
#include <algorithm>

BossStateManager::BossStateManager()
	: aiEnabled_(false)
	, transitionCount_(0)
{
}

void BossStateManager::Initialize() {
	// デフォルトの設定をロード
	LoadConfigurations();

	// デフォルトの遷移候補を設定
	std::vector<StateTransitionCandidate> defaultCandidates = {
		{ BossStateType::SplineMove, 2.0f },
		{ BossStateType::SplineMove8WayShoot, 3.0f },
		{ BossStateType::SplineMoveRotateShoot, 2.0f },
	};
	SetTransitionCandidates(defaultCandidates);
}

void BossStateManager::LoadConfigurations() {
	// SplineMove8WayShootStateの設定
	splineMove8WayShootConfig_.csvFilePaths = {
		"Resources/CSV/BossMove/Move_1.csv",
		"Resources/CSV/BossMove/Move_2.csv",
		"Resources/CSV/BossMove/Move_3.csv",
	};
	// SplineMoveRotateShootStateの設定
	splineMoveRotateShootConfig_.csvFilePaths = {
		"Resources/CSV/BossMove/RotateShoot_1.csv",
		"Resources/CSV/BossMove/RotateShoot_2.csv",
		"Resources/CSV/BossMove/RotateShoot_3.csv",
	};
	// SplineMoveStateの設定
	splineMoveConfig_.csvFilePaths = {
		"Resources/CSV/BossMove/Move_1.csv",
		"Resources/CSV/BossMove/Move_2.csv",
		"Resources/CSV/BossMove/Move_3.csv",
	};
}

void BossStateManager::SetTransitionCandidates(const std::vector<StateTransitionCandidate>& candidates) {
	transitionCandidates_ = candidates;
}

void BossStateManager::TransitionToRandomState(Boss* boss) {
	if (!boss || !aiEnabled_) {
		return;
	}

	// 遷移候補が空の場合はIdleに遷移
	if (transitionCandidates_.empty()) {
		boss->ChangeState(std::make_unique<IdleState>());
		return;
	}

	// ランダムにStateを選択
	BossStateType selectedType = SelectRandomStateByWeight();

	// 選択されたStateに遷移
	TransitionToState(boss, selectedType);

	// 遷移回数をカウント
	transitionCount_++;
}

void BossStateManager::TransitionToState(Boss* boss, BossStateType stateType) {
	if (!boss) {
		return;
	}

	// Stateを生成して遷移
	std::unique_ptr<BossState> newState = CreateState(stateType);
	if (newState) {
		boss->ChangeState(std::move(newState));
	}
}

BossStateType BossStateManager::SelectRandomStateByWeight() {
	// 重みの合計を計算
	float totalWeight = 0.0f;
	for (const auto& candidate : transitionCandidates_) {
		totalWeight += candidate.weight;
	}

	// ランダムな値を生成（0.0 ~ totalWeight）
	float randomValue = Random::GetInstance().GenerateFloat(0.0f, totalWeight);

	// 重み付きランダム選択
	float cumulativeWeight = 0.0f;
	for (const auto& candidate : transitionCandidates_) {
		cumulativeWeight += candidate.weight;
		if (randomValue <= cumulativeWeight) {
			return candidate.stateType;
		}
	}

	// デフォルト（最初の候補）
	return transitionCandidates_[0].stateType;
}

std::unique_ptr<BossState> BossStateManager::CreateState(BossStateType stateType) {
	switch (stateType) {
	case BossStateType::Idle:
		return std::make_unique<IdleState>();

	case BossStateType::SplineMove:
	{
		// CSVファイルをランダムに選択
		if (splineMoveConfig_.csvFilePaths.empty()) {
			return std::make_unique<IdleState>();
		}
		int index = Random::GetInstance().GenerateInt(0, static_cast<int>(splineMoveConfig_.csvFilePaths.size()) - 1);
		std::string selectedCSV = splineMoveConfig_.csvFilePaths[index];
		return std::make_unique<SplineMoveState>(selectedCSV);
	}

	case BossStateType::SplineMove8WayShoot:
	{
		// CSVファイルをランダムに選択
		if (splineMove8WayShootConfig_.csvFilePaths.empty()) {
			return std::make_unique<IdleState>();
		}
		int index = Random::GetInstance().GenerateInt(0, static_cast<int>(splineMove8WayShootConfig_.csvFilePaths.size()) - 1);
		std::string selectedCSV = splineMove8WayShootConfig_.csvFilePaths[index];

		// Stateを生成し、設定を適用
		auto state = std::make_unique<SplineMove8WayShootState>(selectedCSV);
		state->SetShootInterval(splineMove8WayShootConfig_.shootInterval);
		state->SetBulletSpeed(splineMove8WayShootConfig_.bulletSpeed);
		return state;
	}

	case BossStateType::SplineMoveRotateShoot:
	{
		// CSVファイルをランダムに選択
		if (splineMoveRotateShootConfig_.csvFilePaths.empty()) {
			return std::make_unique<IdleState>();
		}
		int index = Random::GetInstance().GenerateInt(0, static_cast<int>(splineMoveRotateShootConfig_.csvFilePaths.size()) - 1);
		std::string selectedCSV = splineMoveRotateShootConfig_.csvFilePaths[index];

		// Stateを生成（設定は全てコンストラクタで渡す）
		return std::make_unique<SplineMoveRotateShootState>(
			selectedCSV,
			splineMoveRotateShootConfig_.stopControlPointIndex,
			splineMoveRotateShootConfig_.startAngle,
			splineMoveRotateShootConfig_.endAngle,
			splineMoveRotateShootConfig_.rotationSpeed,
			splineMoveRotateShootConfig_.shootInterval,
			splineMoveRotateShootConfig_.bulletSpeed,
			splineMoveRotateShootConfig_.angleIntervalDuration,
			splineMoveRotateShootConfig_.maxRepeatCount
		);
	}

	default:
		return std::make_unique<IdleState>();
	}
}

void BossStateManager::ImGui() {
#ifdef USEIMGUI
	ImGui::SeparatorText("Boss State Manager");

	// AI有効/無効
	ImGui::Checkbox("AI Enabled", &aiEnabled_);
	ImGui::Text("Transition Count: %d", transitionCount_);

	ImGui::Separator();

	// 遷移候補の表示と編集
	if (ImGui::TreeNode("Transition Candidates")) {
		for (size_t i = 0; i < transitionCandidates_.size(); ++i) {
			auto& candidate = transitionCandidates_[i];

			ImGui::PushID(static_cast<int>(i));

			// Stateタイプ名
			const char* stateNames[] = { "Idle", "SplineMove", "SplineMove8WayShoot", "SplineMoveRotateShoot" };
			int currentType = static_cast<int>(candidate.stateType);
			ImGui::Combo("Type", &currentType, stateNames, IM_ARRAYSIZE(stateNames));
			candidate.stateType = static_cast<BossStateType>(currentType);

			// 重み
			ImGui::DragFloat("Weight", &candidate.weight, 0.1f, 0.0f, 10.0f);

			ImGui::PopID();
			ImGui::Separator();
		}

		// 候補追加ボタン
		if (ImGui::Button("Add Candidate")) {
			transitionCandidates_.push_back({ BossStateType::SplineMove, 1.0f });
		}

		// 候補削除ボタン
		if (!transitionCandidates_.empty()) {
			ImGui::SameLine();
			if (ImGui::Button("Remove Last")) {
				transitionCandidates_.pop_back();
			}
		}

		ImGui::TreePop();
	}

	ImGui::Separator();

	// 各Stateの設定表示
	if (ImGui::TreeNode("State Configurations")) {

		// SplineMove8WayShoot設定
		if (ImGui::TreeNode("SplineMove8WayShoot Config")) {
			ImGui::Text("CSV Files:");
			for (size_t i = 0; i < splineMove8WayShootConfig_.csvFilePaths.size(); ++i) {
				ImGui::BulletText("%s", splineMove8WayShootConfig_.csvFilePaths[i].c_str());
			}
			ImGui::DragInt("Shoot Interval", &splineMove8WayShootConfig_.shootInterval, 1, 10, 300);
			ImGui::DragFloat("Bullet Speed", &splineMove8WayShootConfig_.bulletSpeed, 0.01f, 0.05f, 1.0f);
			ImGui::DragInt("Bullet Number", &splineMove8WayShootConfig_.onrShootBulletNumber, 1, 4, 16);
			ImGui::TreePop();
		}

		// SplineMoveRotateShoot設定
		if (ImGui::TreeNode("SplineMoveRotateShoot Config")) {
			ImGui::Text("CSV Files:");
			for (size_t i = 0; i < splineMoveRotateShootConfig_.csvFilePaths.size(); ++i) {
				ImGui::BulletText("%s", splineMoveRotateShootConfig_.csvFilePaths[i].c_str());
			}
			ImGui::DragInt("Stop Control Point Index", &splineMoveRotateShootConfig_.stopControlPointIndex, 1, 0, 10);
			ImGui::DragFloat("Start Angle", &splineMoveRotateShootConfig_.startAngle, 1.0f, -180.0f, 180.0f);
			ImGui::DragFloat("End Angle", &splineMoveRotateShootConfig_.endAngle, 1.0f, -180.0f, 180.0f);
			ImGui::DragFloat("Rotation Speed", &splineMoveRotateShootConfig_.rotationSpeed, 0.1f, 0.1f, 10.0f);
			ImGui::DragInt("Shoot Interval", &splineMoveRotateShootConfig_.shootInterval, 1, 1, 60);
			ImGui::DragFloat("Bullet Speed", &splineMoveRotateShootConfig_.bulletSpeed, 0.01f, 0.05f, 1.0f);
			ImGui::DragInt("Angle Interval Duration", &splineMoveRotateShootConfig_.angleIntervalDuration, 1, 10, 120);
			ImGui::DragInt("Max Repeat Count", &splineMoveRotateShootConfig_.maxRepeatCount, 1, 1, 10);
			ImGui::TreePop();
		}

		// SplineMove設定
		if (ImGui::TreeNode("SplineMove Config")) {
			ImGui::Text("CSV Files:");
			for (size_t i = 0; i < splineMoveConfig_.csvFilePaths.size(); ++i) {
				ImGui::BulletText("%s", splineMoveConfig_.csvFilePaths[i].c_str());
			}
			ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	// 手動遷移ボタン
	ImGui::Separator();
	if (ImGui::Button("Transition to Random State (Manual)")) {
		// このボタンはBossクラス側で処理する必要がある
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Use Boss::TransitionToRandomState()");
	}
#endif
}