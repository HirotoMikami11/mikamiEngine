#pragma once
#include "State/BossState.h"
#include <string>

/// <summary>
/// 指定インデックスで停止し弾を発射してから再び移動するState
/// 1. 初期化: CSVから制御点を読み込み、開始位置にワープ
/// 2. 移動: 指定された制御点インデックスまで移動
/// 3. 停止: 制御点位置で停止
/// 4. 回転・発射: 角度1→停止→角度2→停止 を往復（発射は回転中のみ）
/// 5. 移動再開: 制御点から終点まで移動
/// 6. 完了: Idle状態へ遷移
/// </summary>
class SplineMoveRotateShootState : public BossState {
public:

	/// <summary>
	/// メインフェーズ
	/// </summary>
	enum class Phase {
		Initializing,    // 初期化中（制御点読み込み、ワープ）
		MovingToStop,    // 停止位置（制御点）まで移動
		Stopping,        // 停止処理（回転フェーズへの準備）
		Rotating,        // 回転・発射フェーズ
		MovingToEnd,     // 停止位置から終点まで移動
		Completed        // 完了 → Idle へ遷移
	};

	/// <summary>
	/// 回転フェーズのサブステート
	/// startAngle(角度1) ←→ endAngle(角度2) を往復
	/// </summary>
	enum class RotatingSubPhase {
		RotatingToEnd,       // startAngle → endAngle へ回転（発射あり）
		IntervalAtEnd,       // endAngle で待機（発射なし）
		RotatingToStart,     // endAngle → startAngle へ回転（発射あり）
		IntervalAtStart,     // startAngle で待機（発射なし）
	};

	SplineMoveRotateShootState() = default;
	~SplineMoveRotateShootState() override = default;

	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="csvFilePath">スプライン制御点のCSVパス</param>
	/// <param name="stopControlPointIndex">停止する制御点のインデックス（0-based）</param>
	/// <param name="startAngle">回転開始角度（度）</param>
	/// <param name="endAngle">回転終了角度（度）</param>
	/// <param name="rotationSpeed">回転速度（度/フレーム）</param>
	/// <param name="shootInterval">発射間隔（フレーム）</param>
	/// <param name="bulletSpeed">弾の速度</param>
	/// <param name="angleIntervalDuration">角度到達時の停止時間（フレーム）</param>
	/// <param name="maxRepeatCount">往復回数（1往復 = 角度1→角度2→角度1）</param>
	explicit SplineMoveRotateShootState(
		const std::string& csvFilePath,
		int stopControlPointIndex = 2,
		float startAngle = -60.0f,
		float endAngle = 60.0f,
		float rotationSpeed = 2.0f,
		int shootInterval = 4,
		float bulletSpeed = 0.3f,
		int angleIntervalDuration = 30,
		int maxRepeatCount = 1
	);

	/// <summary>
	/// 状態開始時の初期化
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 毎フレーム更新
	/// </summary>
	void Update(Boss* boss) override;

	/// <summary>
	/// ImGuiデバッグ表示
	/// </summary>
	void ImGui() override;

	/// <summary>
	/// 状態名取得
	/// </summary>
	const char* GetStateName() const override { return "SplineMoveRotateShoot"; }

private:
	/// <summary>
	/// CSVからスプラインをロードし、初期位置にワープ
	/// </summary>
	bool LoadAndSetup(Boss* boss);

	/// <summary>
	/// 制御点インデックスから対応するtパラメータを計算
	/// </summary>
	/// <param name="controlPointIndex">制御点のインデックス</param>
	/// <param name="totalControlPoints">制御点の総数</param>
	/// <returns>対応するtパラメータ（0.0～1.0）</returns>
	float CalculateTFromControlPointIndex(int controlPointIndex, size_t totalControlPoints) const;


	// フェーズ別更新処理

	/// <summary>
	/// 初期化フェーズ
	/// </summary>
	void UpdateInitializingPhase(Boss* boss);

	/// <summary>
	/// 停止位置まで移動フェーズ
	/// </summary>
	void UpdateMovingToStopPhase(Boss* boss);

	/// <summary>
	/// 停止処理フェーズ
	/// </summary>
	void UpdateStoppingPhase(Boss* boss);

	/// <summary>
	/// 回転・発射フェーズ
	/// </summary>
	void UpdateRotatingPhase(Boss* boss);

	/// <summary>
	/// 終点まで移動フェーズ
	/// </summary>
	void UpdateMovingToEndPhase(Boss* boss);


	// 回転フェーズのサブ処理

	/// <summary>
	/// startAngleからendAngleへ回転（発射あり）
	/// </summary>
	void UpdateRotatingToEnd(Boss* boss);

	/// <summary>
	/// endAngleで待機（発射なし）
	/// </summary>
	void UpdateIntervalAtEnd(Boss* boss);

	/// <summary>
	/// endAngleからstartAngleへ回転発射
	/// </summary>
	void UpdateRotatingToStart(Boss* boss);

	/// <summary>
	/// startAngle 待機（発射なし）
	/// </summary>
	void UpdateIntervalAtStart(Boss* boss);


	/// <summary>
	/// 頭パーツの向いている方向に弾を発射
	/// </summary>
	void ShootBulletFromHead(Boss* boss);

	/// <summary>
	/// 現在位置と向きを更新
	/// </summary>
	void UpdatePositionAndRotation(Boss* boss);

private:


	// パラメータ
	std::string csvFilePath_;		// スプライン制御点CSVパス
	int stopControlPointIndex_;		// 停止する制御点のインデックス
	float startAngle_;				// 回転開始角度（度）
	float endAngle_;				// 回転終了角度（度）
	float rotationSpeed_;			// 回転速度（度/フレーム）
	int shootInterval_;				// 発射間隔（フレーム）
	float bulletSpeed_;				// 弾の速度
	int angleIntervalDuration_;		// 角度到達時の停止時間（フレーム）
	int maxRepeatCount_;			// 往復回数


	// 状態管理
	Phase currentPhase_ = Phase::Initializing;
	RotatingSubPhase rotatingSubPhase_ = RotatingSubPhase::RotatingToEnd;

	bool isInitialized_ = false;
	Boss* boss_ = nullptr;

	// 移動制御
	float stopProgress_ = 0.0f;			// 停止位置のprogress値
	float currentProgress_ = 0.0f;		// 現在のprogress値

	// 回転制御
	float currentAngle_ = 0.0f;			// 現在の角度（度）
	float baseRotationY_ = 0.0f;		// 停止時の頭部基準角度（Y軸、ラジアン）
	int currentRepeatCount_ = 0;		// 現在の往復回数
	int shootTimer_ = 0;				// 発射タイマー
	int intervalTimer_ = 0;				// インターバルタイマー
};