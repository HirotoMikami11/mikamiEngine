#include "BossStateManager.h"
#include "Boss.h"
#include "Phase/BossPhaseManager.h"
#include "State/IdleState.h"
#include "State/SplineMoveState.h"
#include "State/SplineMove8WayShootState.h"
#include "State/SplineMoveRotateShootState.h"
#include "Random/Random.h"
#include "ImGui/ImGuiManager.h"
#include <algorithm>

BossStateManager::BossStateManager()
	: aiEnabled_(true)
	, transitionCount_(0)
{
}

void BossStateManager::Initialize(const PhaseConfig& config)
{
	// PhaseConfigから設定を受け取る
	UpdateConfig(config);
}

void BossStateManager::UpdateConfig(const PhaseConfig& config)
{
	// 遷移候補を更新
	transitionCandidates_ = config.transitionCandidates;

	// 各StateConfigを更新
	splineMove8WayShootConfig_ = config.splineMove8WayShootConfig;
	splineMoveRotateShootConfig_ = config.splineMoveRotateShootConfig;
	splineMoveConfig_ = config.splineMoveConfig;
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
		auto state = std::make_unique<SplineMove8WayShootState>(
			selectedCSV,
			splineMove8WayShootConfig_.shootInterval,
			splineMove8WayShootConfig_.bulletSpeed,
			splineMove8WayShootConfig_.onShootBulletNumber
		);

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

	ImGui::Spacing();

	// 遷移候補の表示と編集
	if (ImGui::CollapsingHeader("Current Transition Candidates")) {
		for (size_t i = 0; i < transitionCandidates_.size(); ++i) {
			auto& candidate = transitionCandidates_[i];

			ImGui::PushID(static_cast<int>(i));

			// Stateタイプ名
			const char* stateNames[] = { "Idle", "SplineMove", "SplineMove8WayShoot", "SplineMoveRotateShoot" };
			int currentType = static_cast<int>(candidate.stateType);
			ImGui::Text("%s", stateNames[currentType]);
			ImGui::SameLine();

			// 重み
			ImGui::Text("Weight: %.1f", candidate.weight);

			ImGui::PopID();
		}
	}

	ImGui::Spacing();

	// 現在の設定表示
	if (ImGui::CollapsingHeader("Current State Configurations")) {
		// SplineMove8WayShoot設定
		if (ImGui::TreeNode("SplineMove8WayShoot Config")) {
			ImGui::Text("CSV Files: %zu", splineMove8WayShootConfig_.csvFilePaths.size());
			ImGui::Text("Shoot Interval: %d", splineMove8WayShootConfig_.shootInterval);
			ImGui::Text("Bullet Speed: %.2f", splineMove8WayShootConfig_.bulletSpeed);
			ImGui::Text("Bullet Number: %d", splineMove8WayShootConfig_.onShootBulletNumber);
			ImGui::TreePop();
		}

		// SplineMoveRotateShoot設定
		if (ImGui::TreeNode("SplineMoveRotateShoot Config")) {
			ImGui::Text("CSV Files: %zu", splineMoveRotateShootConfig_.csvFilePaths.size());
			ImGui::Text("Stop Control Point Index: %d", splineMoveRotateShootConfig_.stopControlPointIndex);
			ImGui::Text("Start Angle: %.1f", splineMoveRotateShootConfig_.startAngle);
			ImGui::Text("End Angle: %.1f", splineMoveRotateShootConfig_.endAngle);
			ImGui::Text("Rotation Speed: %.2f", splineMoveRotateShootConfig_.rotationSpeed);
			ImGui::Text("Shoot Interval: %d", splineMoveRotateShootConfig_.shootInterval);
			ImGui::Text("Max Repeat Count: %d", splineMoveRotateShootConfig_.maxRepeatCount);
			ImGui::TreePop();
		}
	}
#endif
}