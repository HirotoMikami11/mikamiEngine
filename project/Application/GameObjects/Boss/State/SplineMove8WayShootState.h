#pragma once
#include "State/BossState.h"
#include <string>

/// <summary>
/// スプライン移動しながら、全Activeパーツから8方向に弾を発射するState
/// SplineMoveStateの機能を内包し、追加で定期的に弾を発射
/// </summary>
class SplineMove8WayShootState : public BossState {
public:
	SplineMove8WayShootState() = default;
	~SplineMove8WayShootState() override = default;

	/// <summary>
	/// コンストラクタ（CSVファイルパス指定）
	/// </summary>
	/// <param name="csvFilePath">スプライン制御点のCSVファイルパス</param>
	explicit SplineMove8WayShootState(const std::string& csvFilePath);

	/// <summary>
	/// 状態の初期化
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 更新処理
	/// スプライン移動 + 定期的に全パーツから8方向弾発射
	/// </summary>
	/// <param name="boss">Bossクラスへのポインタ</param>
	void Update(Boss* boss) override;

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui() override;

	/// <summary>
	/// 状態名を取得
	/// </summary>
	const char* GetStateName() const override { return "SplineMove8WayShoot"; }

	/// <summary>
	/// CSVファイルパスを設定
	/// </summary>
	void SetCSVFilePath(const std::string& csvFilePath) { csvFilePath_ = csvFilePath; }

	/// <summary>
	/// 発射間隔を設定（フレーム数）
	/// </summary>
	void SetShootInterval(int interval) { shootInterval_ = interval; }

	/// <summary>
	/// 弾の速度を設定
	/// </summary>
	void SetBulletSpeed(float speed) { bulletSpeed_ = speed; }

private:
	/// <summary>
	/// CSV読み込みと初期設定
	/// </summary>
	/// <param name="boss">Bossクラスへのポインタ</param>
	/// <returns>成功時true</returns>
	bool LoadAndSetup(Boss* boss);

	/// <summary>
	/// 全Activeパーツから8方向に弾を発射
	/// </summary>
	/// <param name="boss">Bossクラスへのポインタ</param>
	void ShootBulletsFrom8Directions(Boss* boss);

	// CSVファイルパス
	std::string csvFilePath_;

	// 初期化フラグ
	bool isInitialized_ = false;

	// Bossへの参照
	Boss* boss_ = nullptr;

	// 弾発射タイマー
	int shootTimer_ = 0;
	int shootInterval_ = 180;	// 弾発射間隔

	// 弾の速度
	float bulletSpeed_ = 0.2f;	// XZ平面での速度
	//弾の同時発射数
	int onrShootBulletNumber_ = 8;
};
