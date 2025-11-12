#pragma once

// 前方宣言
class Boss;

/// <summary>
/// Boss State基底クラス
/// </summary>
class BossState {
public:
	virtual ~BossState() = default;

	/// <summary>
	/// 状態の初期化
	/// </summary>
	virtual void Initialize() = 0;

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="boss">Bossクラスへのポインタ</param>
	virtual void Update(Boss* boss) = 0;

	/// <summary>
	/// ImGui表示
	/// </summary>
	virtual void ImGui() = 0;

	/// <summary>
	/// 状態名を取得
	/// </summary>
	virtual const char* GetStateName() const = 0;
};