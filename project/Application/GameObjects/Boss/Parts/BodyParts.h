#pragma once
#include "Parts/BaseParts.h"

/// <summary>
/// 体パーツ（白）
/// </summary>
class BodyParts : public BaseParts {
public:
	BodyParts() = default;
	~BodyParts() override = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position) override;
};