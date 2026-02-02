#pragma once
#include "BaseEnemyParts.h"

/// <summary>
/// EnemyWormの体パーツ
/// </summary>
class WormBodyParts : public BaseEnemyParts {
public:
	WormBodyParts() = default;
	~WormBodyParts() override = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position,
		const std::string& modelName = "Boss_Body",
		const std::string& textureName = "white2x2") override;

	/// <summary>
	/// 衝突時の処理
	/// </summary>
	void OnCollision(Collider* other) override;
};
