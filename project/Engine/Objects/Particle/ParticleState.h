#pragma once
#include "MyFunction.h"

/// <summary>
/// 個々のパーティクルの状態を保持する構造体
/// <para>Scale</para>
/// <para>Rotate</para>
/// <para>Translate</para>
/// </summary>
struct ParticleState {
	Vector3Transform transform;	// 大きさ、回転、位置
	Vector3 velocity;			// 速度

	// デフォルトコンストラクタ
	ParticleState()
		: transform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }
		, velocity{ 0.0f, 0.0f, 0.0f }
	{
	}
};