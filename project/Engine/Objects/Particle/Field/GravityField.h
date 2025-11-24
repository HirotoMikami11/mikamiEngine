#pragma once
#include "BaseField.h"
#include "DebugDrawLineSystem.h"

/// <summary>
/// 重力フィールド
/// <para>球体範囲内のパーティクルをフィールド中心に引き寄せる</para>
/// </summary>
class GravityField : public BaseField
{
public:
	GravityField() = default;
	~GravityField() override = default;

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
	/// <returns>パーティクルを削除すべき場合true</returns>
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
	const char* GetTypeName() const override { return "GravityField"; }

	/// <summary>
	/// パラメータをJSONにシリアライズ
	/// </summary>
	json SerializeParameters() const override;

	/// <summary>
	/// JSONからパラメータをデシリアライズ
	/// </summary>
	void DeserializeParameters(const json& j) override;

	// 重力設定
	void SetGravityStrength(float strength) { gravityStrength_ = strength; }
	float GetGravityStrength() const { return gravityStrength_; }

	// 効果範囲（球体）
	void SetEffectRadius(float radius) { effectRadius_ = radius; }
	float GetEffectRadius() const { return effectRadius_; }

	// 削除範囲（中心に近づきすぎたパーティクルを削除）
	void SetDeleteRadius(float radius) { deleteRadius_ = radius; }
	float GetDeleteRadius() const { return deleteRadius_; }

protected:
	/// <summary>
	/// 球体のデバッグ線を作成
	/// </summary>
	void CreateDebugShape() override;

private:
	// 重力フィールド固有のパラメータ
	float gravityStrength_ = 5.0f;		// 重力の強さ
	float effectRadius_ = 3.0f;			// 効果範囲（球体の半径）
	float deleteRadius_ = 0.3f;			// 削除範囲（中心からの距離）
};