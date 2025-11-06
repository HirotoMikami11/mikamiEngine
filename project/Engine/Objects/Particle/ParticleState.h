#pragma once
#include "MyFunction.h"

/// <summary>
/// 個々のパーティクルの状態を保持する構造体
/// パーティクル一個分ののデータ
/// </summary>
struct ParticleState {
	Vector3Transform transform;	// 大きさ、回転、位置
	Vector3 velocity;			// 速度
	Vector4 color;				// 色
	float lifeTime;				//寿命
	float currentTime;			//経過時間
	// デフォルトコンストラクタ
	ParticleState()
		: transform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }
		, velocity{ 0.0f, 0.0f, 0.0f }
		, color{ 1.0f,1.0f,1.0f,1.0f }
		, lifeTime{ 0.0f }
		, currentTime{ 0.0f }
	{
	}
};

/// <summary>
/// パーティクルのGPUに送るデータ
/// </summary>
struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};