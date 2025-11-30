#pragma once
#include <memory>
#include <vector>
#include <string>
#include "BossStateConfig.h"

// 前方宣言
class Boss;
class BossState;
struct PhaseConfig;

/// <summary>
/// Stateの種類
/// </summary>
enum class BossStateType {
	Idle,
	SplineMove,
	SplineMove8WayShoot,
	SplineMoveRotateShoot,
};

/// <summary>
/// State遷移の候補
/// </summary>
struct StateTransitionCandidate {
	BossStateType stateType;	// 遷移先のStateタイプ
	float weight;				// 重み（確率）
};

/// <summary>
/// BossのState遷移を管理するクラス
/// AIの挙動パターンを制御
/// </summary>
class BossStateManager {
public:
	BossStateManager();
	~BossStateManager() = default;

	/// <summary>
	/// 初期化（PhaseConfigから設定を受け取る）
	/// </summary>
	/// <param name="config">Phase別設定</param>
	void Initialize(const PhaseConfig& config);

	/// <summary>
	/// Phase設定を更新（Phase切り替え時に呼ばれる）
	/// </summary>
	/// <param name="config">新しいPhase設定</param>
	void UpdateConfig(const PhaseConfig& config);

	/// <summary>
	/// 次のStateをランダムに選択して遷移
	/// </summary>
	/// <param name="boss">Bossクラスへのポインタ</param>
	void TransitionToRandomState(Boss* boss);

	/// <summary>
	/// 特定のStateに遷移
	/// </summary>
	/// <param name="boss">Bossクラスへのポインタ</param>
	/// <param name="stateType">遷移先のStateタイプ</param>
	void TransitionToState(Boss* boss, BossStateType stateType);

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

	/// <summary>
	/// AI動作を有効/無効にする
	/// </summary>
	void SetAIEnabled(bool enabled) { aiEnabled_ = enabled; }
	bool IsAIEnabled() const { return aiEnabled_; }

private:
	/// <summary>
	/// 重み付きランダム選択
	/// </summary>
	/// <returns>選択されたStateタイプ</returns>
	BossStateType SelectRandomStateByWeight();

	/// <summary>
	/// 指定されたStateを生成
	/// </summary>
	/// <param name="stateType">Stateタイプ</param>
	/// <returns>生成されたState</returns>
	std::unique_ptr<BossState> CreateState(BossStateType stateType);

	// 現在の設定（PhaseConfigから受け取る）
	SplineMove8WayShootConfig splineMove8WayShootConfig_;
	SplineMoveRotateShootConfig splineMoveRotateShootConfig_;
	SplineMoveConfig splineMoveConfig_;

	// State遷移候補リスト（PhaseConfigから受け取る）
	std::vector<StateTransitionCandidate> transitionCandidates_;

	// AI有効フラグ
	bool aiEnabled_ = false;

	// デバッグ用
	int transitionCount_ = 0;
};