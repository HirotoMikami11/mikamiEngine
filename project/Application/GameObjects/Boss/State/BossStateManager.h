#pragma once
#include <memory>
#include <vector>
#include <string>
#include "BossStateConfig.h"

// 前方宣言
class Boss;
class BossState;

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
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 各Stateの設定をロード
	/// </summary>
	void LoadConfigurations();

	/// <summary>
	/// State遷移候補を設定
	/// </summary>
	/// <param name="candidates">遷移候補のリスト</param>
	void SetTransitionCandidates(const std::vector<StateTransitionCandidate>& candidates);

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

	// 設定の取得
	const SplineMove8WayShootConfig& GetSplineMove8WayShootConfig() const {
		return splineMove8WayShootConfig_;
	}
	const SplineMoveRotateShootConfig& GetSplineMoveRotateShootConfig() const {
		return splineMoveRotateShootConfig_;
	}
	const SplineMoveConfig& GetSplineMoveConfig() const {
		return splineMoveConfig_;
	}

	// 設定の変更
	void SetSplineMove8WayShootConfig(const SplineMove8WayShootConfig& config) {
		splineMove8WayShootConfig_ = config;
	}
	void SetSplineMoveRotateShootConfig(const SplineMoveRotateShootConfig& config) {
		splineMoveRotateShootConfig_ = config;
	}
	void SetSplineMoveConfig(const SplineMoveConfig& config) {
		splineMoveConfig_ = config;
	}

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

	// 各Stateの設定
	SplineMove8WayShootConfig splineMove8WayShootConfig_;
	SplineMoveRotateShootConfig splineMoveRotateShootConfig_;
	SplineMoveConfig splineMoveConfig_;

	// State遷移候補リスト
	std::vector<StateTransitionCandidate> transitionCandidates_;

	// AI有効フラグ
	bool aiEnabled_ = false;

	// デバッグ用
	int transitionCount_ = 0;
};