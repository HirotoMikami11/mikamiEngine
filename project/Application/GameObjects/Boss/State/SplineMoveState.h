#pragma once
#include "State/BossState.h"
#include <string>

/// <summary>
/// スプライン移動State
/// CSVから制御点を読み込み、スプライン曲線に沿って移動する
/// </summary>
class SplineMoveState : public BossState {
public:
	SplineMoveState() = default;
	~SplineMoveState() override = default;

	/// <summary>
	/// コンストラクタ（CSVファイルパス指定）
	/// </summary>
	/// <param name="csvFilePath">スプライン制御点のCSVファイルパス</param>
	explicit SplineMoveState(const std::string& csvFilePath);

	/// <summary>
	/// 状態の初期化
	/// CSV読み込み → 最初の座標にワープ → 次のポイントの方向を向く
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 更新処理
	/// スプライン曲線に沿って移動し、終点でIdleStateに遷移
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
	const char* GetStateName() const override { return "SplineMove"; }

	/// <summary>
	/// CSVファイルパスを設定
	/// </summary>
	void SetCSVFilePath(const std::string& csvFilePath) { csvFilePath_ = csvFilePath; }

	/// <summary>
	/// CSVファイルパスを取得
	/// </summary>
	const std::string& GetCSVFilePath() const { return csvFilePath_; }

	/// <summary>
	/// 初期化が成功したかチェック
	/// </summary>
	bool IsInitialized() const { return isInitialized_; }

private:
	/// <summary>
	/// CSV読み込みと初期設定
	/// </summary>
	/// <param name="boss">Bossクラスへのポインタ</param>
	/// <returns>成功時true</returns>
	bool LoadAndSetup(Boss* boss);

	/// <summary>
	/// 最初の座標にワープし、位置履歴をクリア
	/// </summary>
	/// <param name="boss">Bossクラスへのポインタ</param>
	void WarpToStartPosition(Boss* boss);

	/// <summary>
	/// 次のポイントの方向を向く
	/// </summary>
	/// <param name="boss">Bossクラスへのポインタ</param>
	void LookAtNextPoint(Boss* boss);

	// CSVファイルパス
	std::string csvFilePath_;

	// 初期化フラグ
	bool isInitialized_ = false;

	// Bossへの参照（Update内で使用）
	Boss* boss_ = nullptr;
};