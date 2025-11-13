#pragma once
#include "Engine.h"
#include "Collider.h"
#include "CollisionConfig.h"

/// <summary>
/// プレイヤーの弾クラス（シンプル版）
/// </summary>
class PlayerBullet : public Collider {
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
	void Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	void Draw(const Light& directionalLight);

	/// <summary>
	/// 死亡フラグを取得
	/// </summary>
	bool IsDead() const { return isDead_; }

	/// <summary>
	/// ワールド座標を取得（オーバーライド）
	/// </summary>
	Vector3 GetWorldPosition() override;

	/// <summary>
	/// 衝突時に呼ばれる関数（オーバーライド）
	/// </summary>
	void OnCollision(Collider* other) override;

private:
	// ゲームオブジェクト
	std::unique_ptr<Object3D> gameObject_;

	// 速度
	Vector3 velocity_;

	// 寿命
	static const int32_t kLifeTime = 60 * 5;	// 弾の寿命（5秒）
	int32_t deathTimer_ = kLifeTime;			// 弾の寿命タイマー

	// 死亡フラグ
	bool isDead_ = false;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	/// <summary>
	/// 速度の方向を向く
	/// </summary>
	void SetToVelocityDirection();
};