#pragma once
#include "State/BossStateConfig.h"
#include "State/BossStateManager.h"
#include <memory>
#include <vector>

// 前方宣言
class Boss;
class BossExplosionEmitter;

/// <summary>
/// Bossの大きな段階（Phase）
/// </summary>
enum class BossPhase {
	Phase1,  // 頭と体がアクティブ
	Phase2,  // 頭と尻尾がアクティブ
	Death    // 死亡演出（SubPhaseで細かく管理）
};

/// <summary>
/// Death Phase内の細かい進行状態
/// </summary>
enum class DeathSubPhase {
	None,                   // Deathフェーズではない
	WaitingStateFinish,     // 現在のStateの終了待ち
	MovingToDeathPosition,  // 死亡位置への移動中
	Exploding,              // 爆発演出中
	Complete                // 完了
};

/// <summary>
/// Phase別の設定
/// </summary>
struct PhaseConfig {
	float moveSpeedMultiplier = 1.0f;
	std::vector<StateTransitionCandidate> transitionCandidates;

	SplineMove8WayShootConfig splineMove8WayShootConfig;
	SplineMoveRotateShootConfig splineMoveRotateShootConfig;
	SplineMoveConfig splineMoveConfig;
};

/// <summary>
/// BossのPhase管理クラス
/// Phase遷移、Phase別設定、死亡演出を管理
/// </summary>
class BossPhaseManager {
public:
	BossPhaseManager();
	~BossPhaseManager();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="boss">Bossへのポインタ</param>
	/// <param name="explosionEmitter">爆発エミッターへのポインタ</param>
	void Initialize(Boss* boss, BossExplosionEmitter* explosionEmitter);

	/// <summary>
	/// 更新（Phase遷移チェック、Death演出制御）
	/// </summary>
	void Update();

	/// <summary>
	/// Phase取得
	/// </summary>
	BossPhase GetCurrentPhase() const { return currentPhase_; }

	/// <summary>
	/// DeathSubPhase取得
	/// </summary>
	DeathSubPhase GetDeathSubPhase() const { return deathSubPhase_; }

	/// <summary>
	/// Phase別の設定を取得
	/// </summary>
	const PhaseConfig& GetCurrentPhaseConfig() const;

	/// <summary>
	/// Phase遷移をトリガー（HP管理側から呼ばれる）
	/// </summary>
	void TriggerPhase2Transition();
	void TriggerDeathTransition();

	/// <summary>
	/// 現在のStateが終了したことを通知
	/// </summary>
	void NotifyStateFinished();

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

	/// <summary>
	/// Phaseが変更されたかチェック
	/// </summary>
	bool HasPhaseChanged() const { return currentPhase_ != previousPhase_; }

	/// <summary>
	/// Phase変更フラグをリセット
	/// </summary>
	void ResetPhaseChangeFlag() { previousPhase_ = currentPhase_; }

private:
	/// <summary>
	/// Phase別設定のロード
	/// </summary>
	void LoadPhaseConfigs();

	/// <summary>
	/// Phaseを変更
	/// </summary>
	void ChangePhase(BossPhase newPhase);

	/// <summary>
	/// Death演出の更新
	/// </summary>
	void UpdateDeathSequence();

	Boss* boss_;
	BossExplosionEmitter* explosionEmitter_;

	// 現在のPhase
	BossPhase currentPhase_;
	BossPhase previousPhase_;

	// Death演出の進行状態
	DeathSubPhase deathSubPhase_;

	// Phase別設定
	PhaseConfig phase1Config_;
	PhaseConfig phase2Config_;
};