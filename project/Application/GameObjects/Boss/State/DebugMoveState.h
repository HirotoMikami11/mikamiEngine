#pragma once
#include "State/BossState.h"
#include "MyMath.h"

using namespace MyMath;

/// <summary>
/// デバッグ移動状態（指定座標に向かって移動）
/// </summary>
class DebugMoveState : public BossState {
public:
	DebugMoveState() = default;
	~DebugMoveState() override = default;

	/// <summary>
	/// 状態の初期化
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 更新処理
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
	const char* GetStateName() const override { return "DebugMove"; }

	/// <summary>
	/// 目標座標を設定
	/// </summary>
	void SetTargetPosition(const Vector3& target) { targetPosition_ = target; }

	/// <summary>
	/// 目標座標を取得
	/// </summary>
	Vector3 GetTargetPosition() const { return targetPosition_; }

private:
	Vector3 targetPosition_ = { 0.0f, 0.5f, 10.0f }; // 目標座標
	const float arrivalThreshold_ = 0.5f; // 到着判定距離
};