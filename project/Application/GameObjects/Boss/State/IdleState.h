#pragma once
#include "State/BossState.h"

/// <summary>
/// 待機状態
/// </summary>
class IdleState : public BossState {
public:
	IdleState() = default;
	~IdleState() override = default;

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
	const char* GetStateName() const override { return "Idle"; }
};