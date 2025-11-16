#pragma once
#include "Engine.h"
#include "Collider.h"

/// <summary>
/// 弾の基底クラス（抽象クラス）
/// PlayerBullet と BossBullet の共通機能を定義
/// </summary>
class BaseBullet : public Collider {
public:
	BaseBullet() = default;
	virtual ~BaseBullet() = default;

	/// <summary>
	/// 初期化（純粋仮想関数）
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	/// <param name="velocity">速度ベクトル</param>
	virtual void Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity) = 0;

	/// <summary>
	/// 更新（純粋仮想関数）
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	virtual void Update(const Matrix4x4& viewProjectionMatrix) = 0;

	/// <summary>
	/// 描画（純粋仮想関数）
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	virtual void Draw(const Light& directionalLight) = 0;

	/// <summary>
	/// 死亡フラグを取得
	/// </summary>
	virtual bool IsDead() const { return isDead_; }

	/// <summary>
	/// ワールド座標を取得（オーバーライド）
	/// </summary>
	Vector3 GetWorldPosition() override;

	/// <summary>
	/// 衝突時に呼ばれる関数（オーバーライド）
	/// 派生クラスでカスタマイズ可能
	/// </summary>
	void OnCollision(Collider* other) override;

protected:
	/// <summary>
	/// 速度の方向を向く
	/// </summary>
	void SetToVelocityDirection();

	// ゲームオブジェクト
	std::unique_ptr<Object3D> gameObject_;

	// 速度
	Vector3 velocity_ = { 0.0f, 0.0f, 0.0f };

	// 寿命タイマー
	int32_t deathTimer_ = 0;

	// 死亡フラグ
	bool isDead_ = false;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;
};
