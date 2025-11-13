#pragma once
#include "Parts/BaseParts.h"

// 前方宣言
class Boss;

/// <summary>
/// 尻尾パーツ（緑）
/// Phase1: Objects属性
/// Phase2: Enemy属性
/// </summary>
class TailParts : public BaseParts {
public:
	TailParts() = default;
	~TailParts() override = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position) override;

	/// <summary>
	/// 衝突時の処理
	/// </summary>
	void OnCollision(Collider* other) override;

	/// <summary>
	/// Bossへの参照を設定（ダメージ連動のため）
	/// </summary>
	void SetBoss(Boss* boss) { boss_ = boss; }

	/// <summary>
	/// Phase1の属性に設定（Objects）
	/// </summary>
	void SetPhase1Attribute();

	/// <summary>
	/// Phase2の属性に設定（Enemy）
	/// </summary>
	void SetPhase2Attribute();

private:
	Boss* boss_ = nullptr;
};