#pragma once
#include <string>
#include <vector>
#include <json.hpp>
#include "MyFunction.h"

using json = nlohmann::json;

/// <summary>
/// パーティクルグループのデータ
/// </summary>
struct ParticleGroupData
{
	std::string groupName;
	std::string modelTag;
	uint32_t maxParticles;
	std::string textureName;
	bool useBillboard;

	// JSON変換
	json ToJson() const;
	static ParticleGroupData FromJson(const json& j);
};

/// <summary>
/// パーティクルエミッターのデータ
/// </summary>
struct ParticleEmitterData
{
	std::string emitterName;
	std::string targetGroupName;

	// Transform
	Vector3 position;
	Vector3 rotation;
	Vector3 scale;

	// Emit設定
	uint32_t emitCount;
	float emitFrequency;
	bool isEmitting;

	// パーティクル寿命
	float particleLifeTimeMin;
	float particleLifeTimeMax;

	// 速度設定（新方式）
	Vector3 emitDirection;
	float initialSpeed;
	float spreadAngle;
	bool useDirectionalEmit;

	// 速度設定（旧方式）
	float velocityRange;

	// スケール設定
	Vector3 particleScaleMin;
	Vector3 particleScaleMax;

	// 回転設定
	Vector3 particleRotateMin;
	Vector3 particleRotateMax;

	// エミッター寿命
	float emitterLifeTime;
	bool emitterLifeTimeLoop;
	bool useEmitterLifeTime;

	// 発生範囲
	AABB spawnArea;

	// デバッグ
	bool showDebugAABB;
	Vector4 debugAABBColor;


	// 寿命に応じた色(Color Over Lifetime)
	bool enableColorOverLifetime = false;
	Vector4 particleStartColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 particleEndColor = { 1.0f, 1.0f, 1.0f, 0.0f };

	// 寿命に応じたサイズ(Size Over Lifetime)
	bool enableSizeOverLifetime = false;
	Vector3 particleStartScale = { 1.0f, 1.0f, 1.0f };
	Vector3 particleEndScale = { 1.0f, 1.0f, 1.0f };

	// 回転(Rotation)
	bool enableRotation = false;
	Vector3 rotationSpeed = { 0.0f, 0.0f, 0.0f };

	// JSON変換
	json ToJson() const;
	static ParticleEmitterData FromJson(const json& j);
};

/// <summary>
/// フィールドのデータ
/// </summary>
struct ParticleFieldData
{
	std::string fieldName;
	std::string fieldType; // "AccelerationField", "GravityField" など

	// Transform
	Vector3 position;
	Vector3 rotation;
	Vector3 scale;

	// 共通設定
	bool isEnabled;
	bool showDebugVisualization;
	Vector4 debugColor;

	// フィールド固有のパラメータ（JSON形式で保存）
	json parameters;

	// JSON変換
	json ToJson() const;
	static ParticleFieldData FromJson(const json& j);
};

/// <summary>
/// パーティクルプリセット全体のデータ
/// </summary>
struct ParticlePresetData
{
	std::string presetName;
	std::vector<ParticleGroupData> groups;
	std::vector<ParticleEmitterData> emitters;
	std::vector<ParticleFieldData> fields;

	// JSON変換
	json ToJson() const;
	static ParticlePresetData FromJson(const json& j);

	// ファイル操作
	bool SaveToFile(const std::string& filePath) const;
	static ParticlePresetData LoadFromFile(const std::string& filePath);
};