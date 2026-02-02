#pragma once
#include "BaseEnemyParts.h"

/// <summary>
/// EnemyWormの頭パーツ
/// </summary>
class WormHeadParts : public BaseEnemyParts {
public:
	WormHeadParts() = default;
	~WormHeadParts() override = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position,
		const std::string& modelName = "Boss_Head",
		const std::string& textureName = "white2x2") override;

	/// <summary>
	/// 衝突時の処理
	/// </summary>
	void OnCollision(Collider* other) override;
};
