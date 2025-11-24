#pragma once
#include <memory>
#include <string>
#include <numbers>
#include "DirectXCommon.h"
#include "Transform3D.h"
#include "ParticleState.h"
#include "Random/Random.h"
#include "MyFunction.h"
#include "DebugDrawLineSystem.h"

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
	void AddLineDebug();

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

	// 新方式：方向指定発射
	void SetEmitDirection(const Vector3& direction) {
		emitDirection_ = Normalize(direction);
	}
	const Vector3& GetEmitDirection() const { return emitDirection_; }

	void SetInitialSpeed(float speed) { initialSpeed_ = speed; }
	float GetInitialSpeed() const { return initialSpeed_; }

	void SetSpreadAngle(float angleDegrees) { spreadAngle_ = angleDegrees; }
	float GetSpreadAngle() const { return spreadAngle_; }

	void SetUseDirectionalEmit(bool use) { useDirectionalEmit_ = use; }
	bool IsUseDirectionalEmit() const { return useDirectionalEmit_; }

	// Scale/Rotate設定
	void SetParticleScaleRange(const Vector3& min, const Vector3& max) {
		particleScaleMin_ = min;
		particleScaleMax_ = max;
	}
	const Vector3& GetParticleScaleMin() const { return particleScaleMin_; }
	const Vector3& GetParticleScaleMax() const { return particleScaleMax_; }

	void SetParticleRotateRange(const Vector3& min, const Vector3& max) {
		particleRotateMin_ = min;
		particleRotateMax_ = max;
	}
	const Vector3& GetParticleRotateMin() const { return particleRotateMin_; }
	const Vector3& GetParticleRotateMax() const { return particleRotateMax_; }

	// ========================================
	// Color Over Lifetime
	// ========================================

	/// <summary>
	/// 色の時間変化を設定
	/// </summary>
	void SetColorOverLifetime(bool enable, const Vector4& startColor, const Vector4& endColor) {
		enableColorOverLifetime_ = enable;
		particleStartColor_ = startColor;
		particleEndColor_ = endColor;
	}

	void SetEnableColorOverLifetime(bool enable) { enableColorOverLifetime_ = enable; }
	bool IsEnableColorOverLifetime() const { return enableColorOverLifetime_; }

	void SetParticleStartColor(const Vector4& color) { particleStartColor_ = color; }
	const Vector4& GetParticleStartColor() const { return particleStartColor_; }

	void SetParticleEndColor(const Vector4& color) { particleEndColor_ = color; }
	const Vector4& GetParticleEndColor() const { return particleEndColor_; }

	// ========================================
	// Size Over Lifetime
	// ========================================

	/// <summary>
	/// サイズの時間変化を設定
	/// </summary>
	void SetSizeOverLifetime(bool enable, const Vector3& startScale, const Vector3& endScale) {
		enableSizeOverLifetime_ = enable;
		particleStartScale_ = startScale;
		particleEndScale_ = endScale;
	}

	void SetEnableSizeOverLifetime(bool enable) { enableSizeOverLifetime_ = enable; }
	bool IsEnableSizeOverLifetime() const { return enableSizeOverLifetime_; }

	void SetParticleStartScale(const Vector3& scale) { particleStartScale_ = scale; }
	const Vector3& GetParticleStartScale() const { return particleStartScale_; }

	void SetParticleEndScale(const Vector3& scale) { particleEndScale_ = scale; }
	const Vector3& GetParticleEndScale() const { return particleEndScale_; }

	// ========================================
	// Rotation
	// ========================================

	/// <summary>
	/// 回転を設定
	/// </summary>
	void SetRotation(bool enable, const Vector3& rotationSpeed) {
		enableRotation_ = enable;
		rotationSpeed_ = rotationSpeed;
	}

	void SetEnableRotation(bool enable) { enableRotation_ = enable; }
	bool IsEnableRotation() const { return enableRotation_; }

	void SetRotationSpeed(const Vector3& speed) { rotationSpeed_ = speed; }
	const Vector3& GetRotationSpeed() const { return rotationSpeed_; }

	// 発生範囲
	void SetSpawnArea(const AABB& aabb) {
		spawnArea_ = aabb;
		FixAABBMinMax(spawnArea_);
	}
	const AABB& GetSpawnArea() const { return spawnArea_; }
	/// <summary>
	/// 発生範囲をサイズで設定（中心からの±offset）
	/// </summary>
	void SetSpawnAreaSize(const Vector3& size);

	/// <summary>
	/// ワールド座標でのAABBを取得
	/// </summary>
	AABB GetWorldAABB() const;
	
	// デバッグ描画
	void SetShowDebugAABB(bool show) { showDebugAABB_ = show; }
	bool IsShowDebugAABB() const { return showDebugAABB_; }

	void SetDebugAABBColor(const Vector4& color) { debugAABBColor_ = color; }
	const Vector4& GetDebugAABBColor() const { return debugAABBColor_; }

	// 状態取得
	const std::string& GetName() const { return name_; }
	void SetName(const std::string& name) { name_ = name; }

	const std::string& GetTargetGroupName() const { return targetGroupName_; }

	// エミッター寿命
	void SetEmitterLifeTime(float time) { emitterLifeTime_ = time; }
	float GetEmitterLifeTime() const { return emitterLifeTime_; }

	void SetEmitterLifeTimeLoop(bool loop) { emitterLifeTimeLoop_ = loop; }
	bool IsEmitterLifeTimeLoop() const { return emitterLifeTimeLoop_; }

	float GetEmitterCurrentTime() const { return emitterCurrentTime_; }
	bool IsEmitterAlive() const {
		return !useEmitterLifeTime_ || emitterLifeTimeLoop_ || emitterCurrentTime_ < emitterLifeTime_;
	}

	void SetUseEmitterLifeTime(bool use) { useEmitterLifeTime_ = use; }
	bool IsUseEmitterLifeTime() const { return useEmitterLifeTime_; }

	// エミッター時間をリセット
	void ResetEmitterTime() { emitterCurrentTime_ = 0.0f; }

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

	// エミッター設定
	Transform3D emitterTransform_;			// エミッターのトランスフォーム
	uint32_t emitCount_ = 5;				// 1回の発生で生成するパーティクル数
	float emitFrequency_ = 1.0f;			// 発生頻度（秒）
	float frequencyTimer_ = 0.0f;			// 発生頻度用タイマー
	bool isEmitting_ = true;				// パーティクルを発生させるか

	// パーティクル初期設定
	float particleLifeTimeMin_ = 1.0f;		// パーティクル寿命の最小値
	float particleLifeTimeMax_ = 3.0f;		// パーティクル寿命の最大値

	// 速度設定（新方式）
	Vector3 emitDirection_ = { 0.0f, 1.0f, 0.0f };	// 発射方向（正規化される）
	float initialSpeed_ = 1.0f;						// 初速度の大きさ
	float spreadAngle_ = 30.0f;						// 散らばり角度（度数法）
	bool useDirectionalEmit_ = false;				// 方向指定発射を使用するか

	// 旧方式（互換性のため残す）
	float velocityRange_ = 1.0f;			// 初期速度の範囲（useDirectionalEmit_=falseの時に使用）

	// Scale設定
	Vector3 particleScaleMin_ = { 1.0f, 1.0f, 1.0f };	// パーティクルスケールの最小値
	Vector3 particleScaleMax_ = { 1.0f, 1.0f, 1.0f };	// パーティクルスケールの最大値

	// Rotate設定
	Vector3 particleRotateMin_ = { 0.0f, 0.0f, 0.0f };	// パーティクル回転の最小値
	Vector3 particleRotateMax_ = { 0.0f, 0.0f, 0.0f };	// パーティクル回転の最大値

	// ========================================
	// Color Over Lifetime
	// ========================================
	bool enableColorOverLifetime_ = false;					// 色変化を使用するか
	Vector4 particleStartColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };	// 開始色
	Vector4 particleEndColor_ = { 1.0f, 1.0f, 1.0f, 0.0f };		// 終了色（デフォルト：フェードアウト）

	// ========================================
	// Size Over Lifetime
	// ========================================
	bool enableSizeOverLifetime_ = false;				// サイズ変化を使用するか
	Vector3 particleStartScale_ = { 1.0f, 1.0f, 1.0f };	// 開始スケール
	Vector3 particleEndScale_ = { 1.0f, 1.0f, 1.0f };		// 終了スケール

	// ========================================
	// Rotation
	// ========================================
	bool enableRotation_ = false;					// 回転を使用するか
	Vector3 rotationSpeed_ = { 0.0f, 0.0f, 0.0f };	// 回転速度（度/秒）

	// エミッター寿命設定
	float emitterLifeTime_ = 5.0f;			// エミッター寿命（秒）
	float emitterCurrentTime_ = 0.0f;		// エミッター経過時間
	bool emitterLifeTimeLoop_ = false;		// 寿命がループするか
	bool useEmitterLifeTime_ = false;		// エミッター寿命を使用するか

	// 発生範囲(AABBで1の範囲)
	AABB spawnArea_ = {
		{-0.5f, -0.5f, -0.5f},	// min
		{0.5f, 0.5f, 0.5f}		// max
	};

	// デバッグ描画
	bool showDebugAABB_ = false;
	Vector4 debugAABBColor_ = { 1.0f, 0.0f, 0.0f, 1.0f };	// 赤色

	std::string name_ = "ParticleEmitter";
	std::string targetGroupName_ = "";		// ターゲットグループ名

	// システム参照
	DirectXCommon* dxCommon_ = nullptr;
	DebugDrawLineSystem* debugDrawLineSystem_ = nullptr;
};