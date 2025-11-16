#pragma once
#include "BaseBullet.h"
#include "CollisionConfig.h"

/// <summary>
/// プレイヤーの弾クラス（シンプル版）
/// BaseBullet を継承
/// </summary>
class PlayerBullet : public BaseBullet {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	PlayerBullet();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~PlayerBullet();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	/// <param name="velocity">速度ベクトル</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity) override;

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix) override;

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	void Draw(const Light& directionalLight) override;

	/// <summary>
	/// 衝突時に呼ばれる関数（オーバーライド）
	/// </summary>
	void OnCollision(Collider* other) override;

private:
	// 寿命
	static const int32_t kLifeTime = 60 * 5;	// 弾の寿命（5秒）
};
