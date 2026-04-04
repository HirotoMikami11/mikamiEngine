#pragma once
#include <memory>
#include "GameObject.h"
#include "Object3D.h"
#include "CollisionManager/Collider/SphereCollider.h"

/// <summary>
/// TestShooter が発射するデバッグ用弾
/// SphereCollider を持ち、プレイヤーに当たるか一定時間で消滅する
/// </summary>
class TestBullet : public SphereCollider, public GameObject
{
public:
	TestBullet() = default;
	~TestBullet() override = default;

	/// <summary>
	/// 弾のパラメータを設定して初期化する（AddObject 前に呼ぶ）
	/// </summary>
	/// <param name="dxCommon">DirectXCommon</param>
	/// <param name="pos">発射座標</param>
	/// <param name="direction">正規化済み方向ベクトル</param>
	/// <param name="speed">移動速度</param>
	/// <param name="radius">コライダー半径（モデルスケールにも使用）</param>
	/// <param name="lifetime">消滅までの時間（秒）</param>
	void Setup(DirectXCommon* dxCommon,
		const Vector3& pos,
		const Vector3& direction,
		float speed,
		float radius,
		float lifetime);

	void Initialize()    override {}
	void Update()        override;
	void DrawOffscreen() override;
	void Finalize()      override;

	//コライダー設定
	Vector3 GetWorldPosition() override;
	void OnCollisionEnter(ICollider* other)	override;
	void OnCollisionStay(ICollider* other)	override {}
	void OnCollisionExit(ICollider* other)	override {}

	ObjectTag GetTag() const { return tag_; }

private:
	std::unique_ptr<Sphere> model_;

	Vector3 velocity_{};
	float   remainingTime_ = 0.0f;

	DirectXCommon* dxCommon_ = nullptr;
	Matrix4x4 viewProjectionMatrix_{};

	ObjectTag tag_ = ObjectTag::Bullet;
};
