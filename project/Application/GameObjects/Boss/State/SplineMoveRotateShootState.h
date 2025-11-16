#pragma once
#include "State/BossState.h"
#include <string>

/// <summary>
/// スプライン移動 → 停止 → 頭部回転 → 弾発射 を行う State。
/// ・スプライン上を移動し、指定位置で停止
/// ・停止後、startAngle → endAngle を往復回転
/// ・回転中に弾を発射（間隔指定）
/// ・指定往復回数で終了
/// </summary>
class SplineMoveRotateShootState : public BossState {
public:

	/// <summary>
	/// 処理フェーズ
	/// </summary>
	enum class Phase {
		Moving,     // スプライン移動中
		Stopping,   // 停止直後（初期化）
		Rotating,   // 頭部回転（往復あり）
		Shooting,   // （未使用に近い）単発射撃フェーズ
		Completed   // 完了 → Idle へ遷移
	};

	SplineMoveRotateShootState() = default;
	~SplineMoveRotateShootState() override = default;

	/// <summary>
	/// コンストラクタ（動作パラメータ指定版）
	/// </summary>
	/// <param name="csvFilePath">スプライン制御点のCSVパス</param>
	/// <param name="startAngle">回転開始角度（度）</param>
	/// <param name="endAngle">回転終了角度（度）</param>
	/// <param name="rotationSpeed">回転速度（度/フレーム）</param>
	/// <param name="bulletSpeed">撃つ弾の速度</param>
	/// <param name="maxRepeatCount">
	/// 回転往復回数（開始→終了→開始で1往復）。
	/// 実際は 端到達回数 = 往復数 * 2 で制御。
	/// </param>
	explicit SplineMoveRotateShootState(
		const std::string& csvFilePath,
		float startAngle = -60.0f,
		float endAngle = 60.0f,
		float rotationSpeed = 2.0f,
		float bulletSpeed = 0.3f,
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
	/// 初回のみ行う CSV からのスプラインロードとセットアップ
	/// </summary>
	bool LoadAndSetup(Boss* boss);

	/// <summary>
	/// スプライン移動フェーズ
	/// </summary>
	void UpdateMovingPhase(Boss* boss);

	/// <summary>
	///停止フェーズ → 回転フェーズのセットアップ
	/// </summary>
	void UpdateStoppingPhase(Boss* boss);

	/// <summary>
	/// 頭部回転フェーズ（start ↔ end の往復回転）
	/// 回転中に一定間隔で弾を発射
	/// </summary>
	void UpdateRotatingPhase(Boss* boss);

	/// <summary>
	///単発処理フェーズ（消す）
	/// </summary>
	void UpdateShootingPhase(Boss* boss);

	/// <summary>
	/// 頭パーツの向いている方向に弾を発射
	/// </summary>
	void ShootBulletFromHead(Boss* boss);

private:

	// パラメータ

	std::string csvFilePath_;	// スプライン制御点 CSV パス
	Phase currentPhase_ = Phase::Moving;// 現在のフェーズ
	bool isInitialized_ = false;		// 初期化済か？
	Boss* boss_ = nullptr;				// 所属ボス

	float startAngle_;			// 回転開始角度（度）
	float endAngle_;			// 回転終了角度（度）
	float rotationSpeed_;		// 回転速度（度/フレーム）
	float currentAngle_;				// 現在の角度
	float baseRotationY_;				// 停止時の頭部角度(Y軸)
	float bulletSpeed_;			// 弾の速度
	float stopProgress_; // スプライン上で停止する位置（0.0〜1.0）
	float moveProgress_;				// 移動進捗(0〜1)
	int shootInterval_;					// 発射間隔(フレーム)
	int shootTimer_;					// 発射タイマー
	bool hasReachedStop_ = false;		// 停止地点に到達したか？
	//往復回転制御

	int currentRepeatCount_ = 0;         // 端到達回数（往復*2）
	int maxRepeatCount_ = 1;             // 設定往復回数
	bool rotatingForward_ = true;        // true＝start→end、false＝end→start
};
