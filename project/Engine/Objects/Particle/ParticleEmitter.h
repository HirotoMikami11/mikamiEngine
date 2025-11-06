#pragma once
#include <memory>
#include <string>
#include "DirectXCommon.h"
#include "Transform3D.h"
#include "ParticleState.h"
#include "Random/Random.h"
#include "MyFunction.h"
#include "LineRenderer.h"

// 前方宣言
class ParticleGroup;

/// <summary>
/// パーティクルエミッター
/// <para>発生タイミングと射出機能のみを担当</para>
/// <para>ParticleSystemを介してParticleGroupにパーティクルを追加する</para>
/// </summary>
class ParticleEmitter
{
public:
	ParticleEmitter() = default;
	~ParticleEmitter() = default;

	/// <summary>
	/// エミッターの初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="targetGroupName">ターゲットグループ名</param>
	void Initialize(DirectXCommon* dxCommon, const std::string& targetGroupName);

	/// <summary>
	/// 更新処理（パーティクルの発生制御のみ）
	/// </summary>
	/// <param name="deltaTime">デルタタイム</param>
	/// <param name="targetGroup">ターゲットグループへのポインタ（ParticleSystemから渡される）</param>
	void Update(float deltaTime, ParticleGroup* targetGroup);

	/// <summary>
	/// デバッグ描画（AABB表示）
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void DrawDebug(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// ImGui用のデバッグ表示
	/// </summary>
	void ImGui();

	// エミッター設定
	Transform3D& GetTransform() { return emitterTransform_; }
	const Transform3D& GetTransform() const { return emitterTransform_; }

	void SetEmitCount(uint32_t count) { emitCount_ = count; }
	uint32_t GetEmitCount() const { return emitCount_; }

	void SetFrequency(float frequency) { emitFrequency_ = frequency; }
	float GetFrequency() const { return emitFrequency_; }

	void SetEmitEnabled(bool enabled) { isEmitting_ = enabled; }
	bool IsEmitting() const { return isEmitting_; }

	// パーティクル初期設定
	void SetParticleLifeTimeRange(float min, float max) {
		particleLifeTimeMin_ = min;
		particleLifeTimeMax_ = max;
	}
	void SetParticleVelocityRange(float range) { velocityRange_ = range; }

	// 発生範囲（AABB）の設定
	void SetSpawnArea(const AABB& aabb) { spawnArea_ = aabb; FixAABBMinMax(spawnArea_); }
	const AABB& GetSpawnArea() const { return spawnArea_; }

	/// <summary>
	/// 発生範囲をサイズで設定（中心からの±offset）
	/// </summary>
	void SetSpawnAreaSize(const Vector3& size);

	// デバッグ表示設定
	void SetShowDebugAABB(bool show) { showDebugAABB_ = show; }
	bool IsShowDebugAABB() const { return showDebugAABB_; }
	void SetDebugAABBColor(const Vector4& color) { debugAABBColor_ = color; }

	// 状態取得
	const std::string& GetName() const { return name_; }
	void SetName(const std::string& name) { name_ = name; }

	const std::string& GetTargetGroupName() const { return targetGroupName_; }
	void SetTargetGroupName(const std::string& groupName) { targetGroupName_ = groupName; }

private:
	/// <summary>
	/// パーティクルを発生させる
	/// </summary>
	/// <param name="deltaTime">デルタタイム</param>
	/// <param name="targetGroup">ターゲットグループ</param>
	void EmitParticles(float deltaTime, ParticleGroup* targetGroup);

	/// <summary>
	/// 新しいパーティクルを生成
	/// </summary>
	/// <returns>生成されたパーティクル</returns>
	ParticleState CreateNewParticle();

	/// <summary>
	/// AABBのデバッグ線を作成
	/// </summary>
	void CreateDebugAABBLines();

	// エミッター設定
	Transform3D emitterTransform_;			// エミッターのトランスフォーム
	uint32_t emitCount_ = 5;				// 1回の発生で生成するパーティクル数
	float emitFrequency_ = 1.0f;			// 発生頻度（秒）
	float frequencyTimer_ = 0.0f;			// 発生頻度用タイマー
	bool isEmitting_ = true;				// パーティクルを発生させるか

	// パーティクル初期設定
	float particleLifeTimeMin_ = 1.0f;		// パーティクル寿命の最小値
	float particleLifeTimeMax_ = 3.0f;		// パーティクル寿命の最大値
	float velocityRange_ = 1.0f;			// 初期速度の範囲

	// 発生範囲(AABBで1の範囲)
	AABB spawnArea_ = {
		{-0.5f, -0.5f, -0.5f},	// min
		{0.5f, 0.5f, 0.5f}		// max
	};

	// デバッグ描画
	std::unique_ptr<LineRenderer> debugLineRenderer_;
	bool showDebugAABB_ = false;
	Vector4 debugAABBColor_ = { 1.0f, 0.0f, 0.0f, 1.0f };	// 赤色

	std::string name_ = "ParticleEmitter";
	std::string targetGroupName_ = "";		// ターゲットグループ名

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;
};