#pragma once
#include "BaseEnemyParts.h"

/// <summary>
/// EnemyWormの尻尾パーツ
/// </summary>
class WormTailParts : public BaseEnemyParts {
public:
	WormTailParts() = default;
	~WormTailParts() override = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position,
		const std::string& modelName = "Boss_Tail",
		const std::string& textureName = "white2x2") override;

	/// <summary>
	/// 衝突時の処理
	/// </summary>
	void OnCollision(Collider* other) override;
};
