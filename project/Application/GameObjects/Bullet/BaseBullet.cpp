#include "BaseBullet.h"
#include "CollisionConfig.h"

Vector3 BaseBullet::GetWorldPosition() {
	if (gameObject_) {
		return gameObject_->GetPosition();
	}
	return Vector3{ 0.0f, 0.0f, 0.0f };
}

void BaseBullet::OnCollision(Collider* other) {
	if (!other) return;

	// 基本的な衝突処理（派生クラスでオーバーライド可能）
	// デフォルトでは何もしない
}

void BaseBullet::SetToVelocityDirection() {
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
