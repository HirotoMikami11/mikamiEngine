#pragma once
#include "Parts/BaseParts.h"

/// <summary>
/// 頭パーツ（黄色）
/// 常にObjects属性で、衝突判定はあるがダメージを受けない
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
	/// <param name="modelName">使用するモデル</param>
	/// <param name="textureName">使用するテクスチャ名</param>
	virtual void Initialize(DirectXCommon* dxCommon, const Vector3& position,
		const std::string& modelName = "Boss_Body",
		const std::string& textureName = "white2x2") override;

	/// <summary>
	/// 衝突時の処理（頭はダメージを受けない）
	/// </summary>
	void OnCollision(Collider* other) override;
};