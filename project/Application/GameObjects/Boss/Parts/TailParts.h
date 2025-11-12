#pragma once
#include "Parts/BaseParts.h"

/// <summary>
/// 尻尾パーツ（緑）
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
};