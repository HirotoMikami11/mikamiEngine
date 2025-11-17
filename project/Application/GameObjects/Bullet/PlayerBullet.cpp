#include "PlayerBullet.h"

PlayerBullet::PlayerBullet()
{
}

PlayerBullet::~PlayerBullet() = default;

void PlayerBullet::Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity) {
	dxCommon_ = dxCommon;
	velocity_ = velocity;
	deathTimer_ = kLifeTime;
	isDead_ = false;

	// ゲームオブジェクト（球体）の初期化
	gameObject_ = std::make_unique<Object3D>();
	gameObject_->Initialize(dxCommon_, "playerBullet", "white2x2");

	// トランスフォームの設定
	Vector3Transform transform;
	transform.scale = { 0.3f, 0.3f, 0.3f };		// スケール
	transform.rotate = { 0.0f, 0.0f, 0.0f };
	transform.translate = position;
	gameObject_->SetTransform(transform);

	// 弾の色を赤色に設定
	gameObject_->SetColor(0xFF0000FF);

	// 速度の方向を向くように回転
	SetToVelocityDirection();

	// 衝突判定設定
	SetRadius(0.3f);  // Colliderの半径をセット

	// 攻撃力を設定
	SetAttackPower(5.0f);

	// 衝突属性の設定
	SetCollisionAttribute(kCollisionAttributePlayerBullet);
	// 敵とオブジェクトに衝突
	SetCollisionMask(kCollisionAttributeEnemy | kCollisionAttributeObjects);
}

void PlayerBullet::Update(const Matrix4x4& viewProjectionMatrix) {
	// 座標を移動させる
	Vector3 currentPos = gameObject_->GetPosition();
	currentPos = currentPos + velocity_;
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
	DebugLineAdd();
#endif
	if (!isDead_) {
		gameObject_->Draw(directionalLight);
	}
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
