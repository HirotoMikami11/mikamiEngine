#pragma once
#include "BossSplineTrack.h"

/// <summary>
/// Bossのスプライン移動制御クラス
/// 等間隔移動（距離ベース）でスプライン曲線上を移動
/// </summary>
class BossSplineMovement {
public:
	BossSplineMovement() = default;
	~BossSplineMovement() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="track">BossSplineTrackへのポインタ</param>
	void Initialize(BossSplineTrack* track);

	/// <summary>
	/// 更新処理
	/// デルタタイムと速度から移動量を計算し、進行度を更新
	/// </summary>
	/// <param name="deltaTime">デルタタイム（秒）</param>
	/// <param name="speed">移動速度（単位/秒）</param>
	void Update(float deltaTime, float speed);

	/// <summary>
	/// 位置をリセット（開始地点に戻す）
	/// </summary>
	void ResetPosition();

	/// <summary>
	/// 進行度を設定
	/// </summary>
	/// <param name="progress">進行度 (0.0 ~ 1.0)</param>
	void SetProgress(float progress);

	/// <summary>
	/// 進行度を取得
	/// </summary>
	float GetProgress() const { return t_; }

	/// <summary>
	/// 現在の位置を取得
	/// </summary>
	Vector3 GetCurrentPosition() const;

	/// <summary>
	/// 先読み位置を取得（進行方向の計算用）
	/// </summary>
	/// <param name="lookAheadDistance">先読み距離（スプライン全体の長さに対する割合）</param>
	/// <returns>先読み位置</returns>
	Vector3 GetLookAheadPosition(float lookAheadDistance = 0.01f) const;

	/// <summary>
	/// 進行方向ベクトルを取得
	/// </summary>
	/// <param name="lookAheadDistance">先読み距離</param>
	/// <returns>正規化された進行方向ベクトル</returns>
	Vector3 GetForwardDirection(float lookAheadDistance = 0.01f) const;

	/// <summary>
	/// 終点に到達したかチェック
	/// </summary>
	bool IsAtEnd() const { return isAtEnd_; }

	/// <summary>
	/// 移動中かチェック
	/// </summary>
	bool IsMoving() const { return isMoving_; }

	/// <summary>
	/// 移動を開始
	/// </summary>
	void StartMovement() { isMoving_ = true; isAtEnd_ = false; }

	/// <summary>
	/// 移動を停止
	/// </summary>
	void StopMovement() { isMoving_ = false; }

	/// <summary>
	/// 等間隔移動が有効かチェック
	/// </summary>
	bool IsUniformSpeedEnabled() const { return uniformSpeedEnabled_; }

	/// <summary>
	/// 等間隔移動の有効/無効を設定
	/// </summary>
	/// <param name="enabled">有効にする場合true</param>
	void SetUniformSpeedEnabled(bool enabled);

private:
	BossSplineTrack* track_ = nullptr;

	// 移動パラメータ
	float t_ = 0.0f;				// 現在の進行度 (0.0 ~ 1.0)
	bool isMoving_ = false;			// 移動中フラグ
	bool isAtEnd_ = false;			// 終点到達フラグ
	float distance_;				// 全体の距離
	// 等間隔移動
	bool uniformSpeedEnabled_ = true;	// 等間隔移動を使用（デフォルトON）
};