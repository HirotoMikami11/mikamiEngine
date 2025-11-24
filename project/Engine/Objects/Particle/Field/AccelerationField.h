#pragma once
#include "BaseField.h"
#include "DebugDrawLineSystem.h"

/// <summary>
/// 加速度フィールド
/// <para>AABB範囲内のパーティクルに加速度を与える</para>
/// </summary>
class AccelerationField : public BaseField
{
public:
	AccelerationField() = default;
	~AccelerationField() override = default;

	/// <summary>
	/// フィールドの初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon) override;

	/// <summary>
	/// パーティクルに効果を適用
	/// </summary>
	/// <param name="particle">対象のパーティクル</param>
	/// <param name="deltaTime">デルタタイム</param>
	/// <returns>パーティクルを削除すべき場合true（加速度フィールドは常にfalse）</returns>
	bool ApplyEffect(ParticleState& particle, float deltaTime) override;

	/// <summary>
	/// 指定した点がフィールド内にあるかチェック
	/// </summary>
	/// <param name="point">チェックする点の座標</param>
	/// <returns>範囲内ならtrue</returns>
	bool IsInField(const Vector3& point) const override;

	/// <summary>
	/// ImGui用のデバッグ表示
	/// </summary>
	void ImGui() override;

	/// <summary>
	/// フィールドタイプ名
	/// </summary>
	const char* GetTypeName() const override { return "AccelerationField"; }

	/// <summary>
	/// パラメータをJSONにシリアライズ
	/// </summary>
	json SerializeParameters() const override;

	/// <summary>
	/// JSONからパラメータをデシリアライズ
	/// </summary>
	void DeserializeParameters(const json& j) override;

	// 加速度設定
	void SetAcceleration(const Vector3& accel) { acceleration_ = accel; }
	const Vector3& GetAcceleration() const { return acceleration_; }

	// 効果範囲（AABB）の設定（ローカル座標）
	void SetArea(const AABB& aabb) { area_ = aabb; FixAABBMinMax(area_); }
	const AABB& GetArea() const { return area_; }

	/// <summary>
	/// 効果範囲をサイズで設定（中心からの±offset、ローカル座標）
	/// </summary>
	void SetAreaSize(const Vector3& size);

	/// <summary>
	/// ワールド座標でのAABBを取得
	/// </summary>
	AABB GetWorldAABB() const;

protected:
	/// <summary>
	/// AABBのデバッグ線を作成
	/// </summary>
	void CreateDebugShape() override;

private:
	// 加速度フィールド固有のパラメータ
	Vector3 acceleration_ = { 0.0f, 1.0f, 0.0f };	// 加速度（デフォルト: 上向き）

	// 効果範囲（ローカル座標でのAABB）
	AABB area_ = {
		{-1.0f, -1.0f, -1.0f},	// min
		{1.0f, 1.0f, 1.0f}		// max
	};
};