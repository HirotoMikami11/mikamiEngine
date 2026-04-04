#include "TestBullet.h"
#include "CameraController.h"
#include "GameTimer.h"
#include "CollisionManager/CollisionConfig.h"

void TestBullet::Setup(DirectXCommon* dxCommon,
	const Vector3& pos,
	const Vector3& direction,
	float speed,
	float radius,
	float lifetime)
{
	tag_      = ObjectTag::Bullet;
	dxCommon_ = dxCommon;

	// オレンジ色の球モデル
	model_ = std::make_unique<Sphere>();
	model_->Initialize(dxCommon_, "sphere", "white");
	model_->SetPosition(pos);
	model_->SetScale({ radius, radius, radius });
	model_->GetMaterial().SetColor({ 1.0f, 0.5f, 0.0f, 1.0f });
	model_->SetLightingMode(LightingMode::HalfLambert);

	// 速度ベクトル = 方向 × 速度
	velocity_      = direction * speed;
	remainingTime_ = lifetime;

	// コライダー設定
	SetRadius(radius);
	SetCollisionAttribute(kCollisionAttributePlayerBullet);
	SetCollisionMask(kCollisionAttributePlayer);	// プレイヤーにのみ当たる
}

void TestBullet::Update()
{
	const float dt = GameTimer::GetInstance().GetDeltaTime();

	// 移動
	Vector3 pos = model_->GetPosition();
	pos += velocity_ * dt;
	model_->SetPosition(pos);

	// 生存タイマー減算
	remainingTime_ -= dt;
	if (remainingTime_ <= 0.0f) {
		Destroy();
	}

	viewProjectionMatrix_ = CameraController::GetInstance()->GetViewProjectionMatrix();
	model_->Update(viewProjectionMatrix_);
}

void TestBullet::DrawOffscreen()
{
	model_->Draw();
	DebugLineAdd();		// コライダーワイヤーフレーム表示
}

void TestBullet::Finalize() {}

Vector3 TestBullet::GetWorldPosition()
{
	return model_->GetPosition();
}

void TestBullet::OnCollisionEnter(ICollider* other)
{
	// プレイヤー属性のコライダーに当たったら即消滅
	if (other->GetCollisionAttribute() & kCollisionAttributePlayer) {
		Destroy();
	}
}
