#pragma once

/// <summary>
/// ゲームオブジェクトの種類
/// 新しい種類を追加したら kObjectUpdateOrder も同じ順番で追加すること
/// </summary>
enum class ObjectTag : int
{
	Default = 0,
	Background,
	Player,
	Manager,
	Enemy,
	Bullet,
	Effect,
	UI,
	Count
};

/// <summary>
/// 処理順テーブル（小さいほど先に処理される）
/// ObjectTag の宣言順と一致させること
/// </summary>
static constexpr int kObjectUpdateOrder[] = {
	0,   // Default
	10,  // Background
	20,  // Player
	25,  // Manager
	30,  // Enemy
	40,  // Bullet
	50,  // Effect
	90,  // UI
};
