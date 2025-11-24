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
	float lifeTime;				// 寿命
	float currentTime;			// 経過時間

	// Color Over Lifetime（色の時間変化）
	bool useColorOverLifetime = false;	// 色変化を使用するか
	Vector4 startColor;					// 開始色
	Vector4 endColor;					// 終了色

	// Size Over Lifetime（サイズの時間変化）
	bool useSizeOverLifetime = false;	// サイズ変化を使用するか
	Vector3 startScale;					// 開始スケール
	Vector3 endScale;					// 終了スケール

	// Rotation（回転）
	bool useRotation = false;			// 回転を使用するか
	Vector3 rotationSpeed;				// 回転速度（度/秒）

	// デフォルトコンストラクタ
	ParticleState()
		: transform{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} }
		, velocity{ 0.0f, 0.0f, 0.0f }
		, color{ 1.0f, 1.0f, 1.0f, 1.0f }
		, lifeTime{ 0.0f }
		, currentTime{ 0.0f }
		, useColorOverLifetime{ false }
		, startColor{ 1.0f, 1.0f, 1.0f, 1.0f }
		, endColor{ 1.0f, 1.0f, 1.0f, 0.0f }
		, useSizeOverLifetime{ false }
		, startScale{ 1.0f, 1.0f, 1.0f }
		, endScale{ 1.0f, 1.0f, 1.0f }
		, useRotation{ false }
		, rotationSpeed{ 0.0f, 0.0f, 0.0f }
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