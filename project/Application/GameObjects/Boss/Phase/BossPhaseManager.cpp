#include "BossPhaseManager.h"
#include "Boss.h"
#include "BossExplosionEmitter.h"
#include "State/IdleState.h"
#include "State/SplineMoveState.h"
#include "ImGui/ImGuiManager.h"

BossPhaseManager::BossPhaseManager()
	: boss_(nullptr)
	, explosionEmitter_(nullptr)
	, currentPhase_(BossPhase::Phase1)
	, previousPhase_(BossPhase::Phase1)
	, deathSubPhase_(DeathSubPhase::None)
{
}

BossPhaseManager::~BossPhaseManager() = default;

void BossPhaseManager::Initialize(Boss* boss, BossExplosionEmitter* explosionEmitter)
{
	boss_ = boss;
	explosionEmitter_ = explosionEmitter;

	// Phase別設定をロード
	LoadPhaseConfigs();

	// 初期状態はPhase1
	currentPhase_ = BossPhase::Phase1;
	previousPhase_ = BossPhase::Phase1;
	deathSubPhase_ = DeathSubPhase::None;
}

void BossPhaseManager::LoadPhaseConfigs()
{
	// ========================================
	// Phase1の設定
	// ========================================
	phase1Config_.moveSpeedMultiplier = 1.0f;
	phase1Config_.transitionCandidates = {
		{ BossStateType::SplineMove8WayShoot, 2.0f },
		{ BossStateType::SplineMoveRotateShoot, 2.0f },
	};

	// Phase1のSplineMove8WayShootConfig
	phase1Config_.splineMove8WayShootConfig.csvFilePaths = {
		"Resources/CSV/BossMove/Move_1.csv",
		"Resources/CSV/BossMove/Move_2.csv",
		"Resources/CSV/BossMove/Move_3.csv",
	};
	phase1Config_.splineMove8WayShootConfig.shootInterval = 180;
	phase1Config_.splineMove8WayShootConfig.bulletSpeed = 0.2f;
	phase1Config_.splineMove8WayShootConfig.onShootBulletNumber = 4;

	// Phase1のSplineMoveRotateShootConfig
	phase1Config_.splineMoveRotateShootConfig.csvFilePaths = {
		"Resources/CSV/BossMove/RotateShoot_1.csv",
		"Resources/CSV/BossMove/RotateShoot_2.csv",
		"Resources/CSV/BossMove/RotateShoot_3.csv",
	};
	phase1Config_.splineMoveRotateShootConfig.stopControlPointIndex = 2;
	phase1Config_.splineMoveRotateShootConfig.startAngle = -60.0f;
	phase1Config_.splineMoveRotateShootConfig.endAngle = 60.0f;
	phase1Config_.splineMoveRotateShootConfig.rotationSpeed = 2.0f;
	phase1Config_.splineMoveRotateShootConfig.shootInterval = 4;
	phase1Config_.splineMoveRotateShootConfig.bulletSpeed = 0.3f;
	phase1Config_.splineMoveRotateShootConfig.angleIntervalDuration = 30;
	phase1Config_.splineMoveRotateShootConfig.maxRepeatCount = 1;

	// Phase1のSplineMoveConfig
	phase1Config_.splineMoveConfig.csvFilePaths = {
		"Resources/CSV/BossMove/Move_1.csv",
		"Resources/CSV/BossMove/Move_2.csv",
		"Resources/CSV/BossMove/Move_3.csv",
	};

	// ========================================
	// Phase2の設定（Phase1ベース + 変更点）
	// ========================================
	phase2Config_ = phase1Config_;  // まずコピー

	// 速度1.5倍
	phase2Config_.moveSpeedMultiplier = 1.3f;

	// 弾数増加（8 → 12）
	phase2Config_.splineMove8WayShootConfig.onShootBulletNumber = 12;

	// 遷移候補の重み変更
	phase2Config_.transitionCandidates = {
		{ BossStateType::SplineMove8WayShoot, 3.0f },  // 重み増加
		{ BossStateType::SplineMoveRotateShoot, 2.0f },
	};

	// RotateShootの往復回数増加
	phase2Config_.splineMoveRotateShootConfig.maxRepeatCount = 2;
}

const PhaseConfig& BossPhaseManager::GetCurrentPhaseConfig() const
{
	switch (currentPhase_) {
	case BossPhase::Phase1:
		return phase1Config_;
	case BossPhase::Phase2:
		return phase2Config_;
	case BossPhase::Death:
		// Death中はPhase2の設定を継続
		return phase2Config_;
	default:
		return phase1Config_;
	}
}

void BossPhaseManager::Update()
{
	// Death演出中の場合は専用処理
	if (currentPhase_ == BossPhase::Death) {
		UpdateDeathSequence();
	}
}

void BossPhaseManager::TriggerPhase2Transition()
{
	if (currentPhase_ == BossPhase::Phase1) {
		ChangePhase(BossPhase::Phase2);
	}
}

void BossPhaseManager::TriggerDeathTransition()
{
	if (currentPhase_ == BossPhase::Phase2) {
		ChangePhase(BossPhase::Death);

		// Death演出開始（現在のStateの終了を待つ）
		deathSubPhase_ = DeathSubPhase::WaitingStateFinish;
	}
}

void BossPhaseManager::ChangePhase(BossPhase newPhase)
{
	previousPhase_ = currentPhase_;
	currentPhase_ = newPhase;
}

void BossPhaseManager::NotifyStateFinished()
{
	if (currentPhase_ != BossPhase::Death) {
		return;
	}

	// Death演出の進行
	if (deathSubPhase_ == DeathSubPhase::WaitingStateFinish) {
		// 現在のStateが終了 → 死亡移動へ
		deathSubPhase_ = DeathSubPhase::MovingToDeathPosition;

		if (boss_) {
			auto deathMoveState = std::make_unique<SplineMoveState>(
				"Resources/CSV/BossMove/DeathMove.csv"
			);
			boss_->ChangeState(std::move(deathMoveState));
		}
	} else if (deathSubPhase_ == DeathSubPhase::MovingToDeathPosition) {
		// 死亡移動完了 → 爆発演出へ
		deathSubPhase_ = DeathSubPhase::Exploding;

		if (explosionEmitter_) {
			explosionEmitter_->StartExplosionSequence();
		}

		if (boss_) {
			// 動きを止める
			boss_->ChangeState(std::make_unique<IdleState>());
		}
	}
}

void BossPhaseManager::UpdateDeathSequence()
{
	if (deathSubPhase_ == DeathSubPhase::Exploding) {
		// 爆発完了チェック
		if (explosionEmitter_ && explosionEmitter_->IsExplosionComplete()) {
			deathSubPhase_ = DeathSubPhase::Complete;
		}
	}
}

void BossPhaseManager::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::CollapsingHeader("Phase Manager")) {
		// Phase表示
		const char* phaseNames[] = { "Phase1", "Phase2", "Death" };
		ImGui::Text("Current Phase: %s", phaseNames[static_cast<int>(currentPhase_)]);

		// DeathSubPhase表示
		if (currentPhase_ == BossPhase::Death) {
			const char* subPhaseNames[] = {
				"None", "WaitingStateFinish", "MovingToDeathPosition",
				"Exploding", "Complete"
			};
			ImGui::Text("Death Sub-Phase: %s",
				subPhaseNames[static_cast<int>(deathSubPhase_)]);
		}

		ImGui::Separator();

		// 現在のConfig表示
		const PhaseConfig& config = GetCurrentPhaseConfig();
		ImGui::Text("Move Speed Multiplier: %.2f", config.moveSpeedMultiplier);

		if (ImGui::TreeNode("Transition Candidates")) {
			for (const auto& candidate : config.transitionCandidates) {
				const char* typeNames[] = {
					"Idle", "SplineMove", "SplineMove8WayShoot", "SplineMoveRotateShoot"
				};
				ImGui::Text("%s: Weight %.1f",
					typeNames[static_cast<int>(candidate.stateType)],
					candidate.weight);
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Phase1 Config")) {
			ImGui::Text("Speed Multiplier: %.2f", phase1Config_.moveSpeedMultiplier);
			ImGui::Text("8Way Bullet Count: %d", phase1Config_.splineMove8WayShootConfig.onShootBulletNumber);
			ImGui::Text("Rotate Max Repeat: %d", phase1Config_.splineMoveRotateShootConfig.maxRepeatCount);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Phase2 Config")) {
			ImGui::Text("Speed Multiplier: %.2f", phase2Config_.moveSpeedMultiplier);
			ImGui::Text("8Way Bullet Count: %d", phase2Config_.splineMove8WayShootConfig.onShootBulletNumber);
			ImGui::Text("Rotate Max Repeat: %d", phase2Config_.splineMoveRotateShootConfig.maxRepeatCount);
			ImGui::TreePop();
		}
	}
#endif
}