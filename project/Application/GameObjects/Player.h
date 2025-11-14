#pragma once
#include "Collider.h"	//衝突判定
#include "CollisionConfig.h"	//衝突属性のフラグを定義する
#include "PlayerBullet.h"	// 自機弾
#include <list>
#include <memory>

class Player : public Collider
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Player();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Player();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position);

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
	/// ImGui
	/// </summary>
	void ImGui();

	/// <summary>
	/// 衝突時に呼ばれる関数（オーバーライド）
	/// </summary>
	/// <param name="other">衝突相手のコライダー</param>
	void OnCollision(Collider* other) override;

	/// <summary>
	/// ワールド座標を取得（オーバーライド）
	/// </summary>
	Vector3 GetWorldPosition() override;

	/// <summary>
	/// 弾リストを取得
	/// </summary>
	const std::list<std::unique_ptr<PlayerBullet>>& GetBullets() const { return bullets_; }

private:
	/// <summary>
	/// 移動処理（XZ平面）
	/// </summary>
	void ProcessMovement();

	/// <summary>
	/// 回転処理（向き制御）
	/// </summary>
	void ProcessRotation();

	/// <summary>
	/// 弾の発射処理
	/// </summary>
	void ProcessFire();

	/// <summary>
	/// 寿命の尽きた弾を削除
	/// </summary>
	void DeleteBullets();

	// ゲームオブジェクト
	std::unique_ptr<Object3D> gameObject_;

	// 弾リスト
	std::list<std::unique_ptr<PlayerBullet>> bullets_;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;
	Input* input_ = nullptr;

	// 移動関連
	Vector3 velocity_;					// 速度
	// 移動関連
	float acceleration_ = 0.15f;
	float limitRunSpeed_ = 5.0f;
	float attenuation_ = 0.2f;

	// 回転関連
	float rotationSpeed_ = 0.05f;

	// 弾発射関連
	float bulletSpeed_ = 0.2f;
	int fireInterval_ = 5;

	int fireTimer_ = 0;					// 発射タイマー
};