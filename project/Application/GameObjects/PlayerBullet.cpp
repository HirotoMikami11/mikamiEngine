#include "PlayerBullet.h"

PlayerBullet::PlayerBullet()
	: directXCommon_(nullptr)
	, velocity_({ 0.0f, 0.0f, 0.0f })
	, isDead_(false)
{
}

PlayerBullet::~PlayerBullet() = default;

void PlayerBullet::Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity) {
	directXCommon_ = dxCommon;
	velocity_ = velocity;

	// ゲームオブジェクト（球体）の初期化
	gameObject_ = std::make_unique<Object3D>();
	gameObject_->Initialize(directXCommon_, "playerBullet", "white2x2");

	// トランスフォームの設定
	Vector3Transform transform;
	transform.scale = { 0.1f, 0.1f, 0.1f };
	transform.rotate = { 0.0f, 0.0f, 0.0f };
	transform.translate = position;
	gameObject_->SetTransform(transform);

	// 弾の色を黄色に設定
	gameObject_->SetColor(0xFFFF00FF);

	// 速度の方向を向くように回転
	SetToVelocityDirection();

	// 衝突判定設定
	SetRadius(0.3f);  // Colliderの半径をセット

	// 衝突属性の設定
	SetCollisionAttribute(kCollisionAttributePlayerBullet);
	// 敵とオブジェクトに衝突
	SetCollisionMask(kCollisionAttributeEnemy | kCollisionAttributeObjects);

	// デバッグカラーを黄色に設定
	SetDebugColor({ 1.0f, 1.0f, 0.0f, 1.0f });
}

void PlayerBullet::Update(const Matrix4x4& viewProjectionMatrix) {
	// 座標を移動させる
	Vector3 currentPos = gameObject_->GetPosition();
	currentPos = Add(currentPos, velocity_);
	gameObject_->SetPosition(currentPos);

	// タイマーを減らす
	deathTimer_--;
	if (deathTimer_ <= 0) {
		isDead_ = true;
	}

	// ゲームオブジェクトの更新
	gameObject_->Update(viewProjectionMatrix);


}

void PlayerBullet::Draw(const Light& directionalLight) {
	// デバッグ表示が有効な場合、コライダーを描画
#ifdef USEIMGUI
	if (IsDebugVisible()) {
		DebugLineAdd();
	}
#endif
	if (!isDead_) {
		gameObject_->Draw(directionalLight);
	}
}

Vector3 PlayerBullet::GetWorldPosition() {
	if (gameObject_) {
		return gameObject_->GetPosition();
	}
	return Vector3{ 0.0f, 0.0f, 0.0f };
}

void PlayerBullet::OnCollision(Collider* other) {
	if (!other) return;

	// 衝突相手が敵またはオブジェクトの属性を持つかチェック
	uint32_t otherAttribute = other->GetCollisionAttribute();
	if ((otherAttribute & kCollisionAttributeEnemy) || (otherAttribute & kCollisionAttributeObjects)) {
		// 弾は衝突すると消滅
		isDead_ = true;
	}
}

void PlayerBullet::SetToVelocityDirection() {
	if (gameObject_) {
		// 速度の方向を向くように回転
		Vector3 rotation = gameObject_->GetRotation();

		// Y軸周り角度（水平回転）
		rotation.y = std::atan2(velocity_.x, velocity_.z);

		// 横軸方向の長さを求める
		float XZLength = std::sqrt(velocity_.x * velocity_.x + velocity_.z * velocity_.z);

		// X軸周り角度（垂直回転）
		rotation.x = std::atan2(-velocity_.y, XZLength);

		gameObject_->SetRotation(rotation);
	}
}