#pragma once
#include "Parts/BaseParts.h"

/// <summary>
/// 頭パーツ（黄色）
/// </summary>
class HeadParts : public BaseParts {
public:
	HeadParts() = default;
	~HeadParts() override = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position) override;
};