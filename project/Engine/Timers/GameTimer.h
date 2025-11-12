#pragma once
#include <chrono>

/// <summary>
/// ゲーム内時間管理クラス
/// タイムスケール（時間の速度）を制御して、スロー再生、停止、倍速などを実現
/// FrameTimerから実時間のデルタタイムを取得し、ゲーム内時間に変換する
/// </summary>
class GameTimer {
public:
	/// <summary>
	/// シングルトンインスタンスを取得
	/// </summary>
	static GameTimer& GetInstance() {
		static GameTimer instance;
		return instance;
	}

	/// <summary>
	/// フレーム開始時に呼ぶ（実時間のデルタタイムを受け取る）
	/// </summary>
	/// <param name="realDeltaTime">実時間のデルタタイム（秒）</param>
	void Update(float realDeltaTime);

	/// <summary>
	/// ImGuiでデバッグ情報を表示
	/// </summary>
	void ImGui();

	///*-----------------------------------------------------------------------*///
	///								タイムスケール制御								///
	///*-----------------------------------------------------------------------*///

	/// <summary>
	/// タイムスケールを設定（時間の速度）
	/// </summary>
	/// <param name="scale">
	/// 0.0f = 停止
	/// 0.5f = 半速（スロー）
	/// 1.0f = 通常速度
	/// 2.0f = 2倍速
	/// </param>
	void SetTimeScale(float scale);

	/// <summary>
	/// 現在のタイムスケールを取得
	/// </summary>
	float GetTimeScale() const { return timeScale_; }

	///*-----------------------------------------------------------------------*///
	///								一時停止制御								///
	///*-----------------------------------------------------------------------*///

	/// <summary>
	/// ゲーム内時間を一時停止
	/// </summary>
	void Pause();

	/// <summary>
	/// ゲーム内時間を再開
	/// </summary>
	void Resume();

	/// <summary>
	/// 一時停止状態を切り替え
	/// </summary>
	void TogglePause();

	/// <summary>
	/// 一時停止中かどうか
	/// </summary>
	bool IsPaused() const { return isPaused_; }

	///*-----------------------------------------------------------------------*///
	///								時間取得									///
	///*-----------------------------------------------------------------------*///

	/// <summary>
	/// ゲーム内デルタタイムを取得（タイムスケール適用済み）
	/// ゲームロジックの更新にはこれを使用する
	/// </summary>
	float GetDeltaTime() const { return gameDeltaTime_; }

	/// <summary>
	/// 実時間のデルタタイムを取得（タイムスケール非適用）
	/// UIやエフェクトなど、ゲーム時間に影響されないものに使用
	/// </summary>
	float GetRealDeltaTime() const { return realDeltaTime_; }

	/// <summary>
	/// ゲーム内経過時間を取得（秒単位）
	/// </summary>
	float GetGameTime() const { return gameTime_; }

	/// <summary>
	/// 実経過時間を取得（秒単位）
	/// </summary>
	float GetRealTime() const { return realTime_; }

	///*-----------------------------------------------------------------------*///
	///								便利な機能									///
	///*-----------------------------------------------------------------------*///

	/// <summary>
	/// スローモーションを開始（指定した速度で）
	/// </summary>
	/// <param name="scale">スロー時の速度（例: 0.3f で30%速度）</param>
	void StartSlowMotion(float scale = 0.3f);

	/// <summary>
	/// 通常速度に戻す
	/// </summary>
	void ResetToNormalSpeed();

	/// <summary>
	/// 時間をリセット
	/// </summary>
	void Reset();

private:
	GameTimer();
	~GameTimer() = default;
	GameTimer(const GameTimer&) = delete;
	GameTimer& operator=(const GameTimer&) = delete;

	// 時間情報
	float realDeltaTime_;		// 実時間のデルタタイム（秒）
	float gameDeltaTime_;		// ゲーム内デルタタイム（タイムスケール適用済み）
	float realTime_;			// 実経過時間（秒）
	float gameTime_;			// ゲーム内経過時間（秒）

	// タイムスケール制御
	float timeScale_;			// 時間の速度（0.0f～任意）
	float previousTimeScale_;	// 一時停止前のタイムスケール（復帰用）
	bool isPaused_;				// 一時停止中か

	// デバッグ用
	float slowMotionScale_;		// スローモーション時のスケール値（ImGui表示用）
};