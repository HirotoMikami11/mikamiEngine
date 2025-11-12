#pragma once
#include "Collider.h"	//衝突判定
#include "CollisionConfig.h"	//衝突属性のフラグを定義する

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

private:
	/// <summary>
	/// 移動処理（XZ平面）
	/// </summary>
	void ProcessMovement();

	/// <summary>
	/// 回転処理（向き制御）
	/// </summary>
	void ProcessRotation();

	// ゲームオブジェクト
	std::unique_ptr<Object3D> gameObject_;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;
	Input* input_ = nullptr;

	// 移動関連
	Vector3 velocity_;					// 速度
	const float kAcceleration = 0.15f;	// 加速度
	const float kLimitRunSpeed = 5.0f;	// 最大速度
	const float kAttenuation = 0.1f;	// 減衰率（非入力時）

	// 回転関連
	const float kRotationSpeed = 0.05f;	// 回転速度
};