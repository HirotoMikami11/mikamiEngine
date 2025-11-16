#include "BossBullet.h"

BossBullet::BossBullet()
{
}

BossBullet::~BossBullet() = default;

void BossBullet::Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity) {
	directXCommon_ = dxCommon;
	velocity_ = velocity;
	deathTimer_ = kLifeTime;
	isDead_ = false;

	// ゲームオブジェクト（球体）の初期化
	gameObject_ = std::make_unique<Object3D>();
	gameObject_->Initialize(directXCommon_, "playerBullet", "white2x2");

	// トランスフォームの設定
	Vector3Transform transform;
	transform.scale = { 1.0f, 1.0f, 1.0f };		// スケール（プレイヤーより少し大きい）
	transform.rotate = { 0.0f, 0.0f, 0.0f };
	transform.translate = position;
	gameObject_->SetTransform(transform);

	// 弾の色を紫色に設定
	gameObject_->SetColor(0xFF00FFFF);

	// 速度の方向を向くように回転
	SetToVelocityDirection();

	// 衝突判定設定
	SetRadius(1.0f);  // Colliderの半径をセット

	// 攻撃力を設定
	SetAttackPower(15.0f);

	// 衝突属性の設定
	SetCollisionAttribute(kCollisionAttributeEnemyBullet);
	// プレイヤーとプレイヤー弾とオブジェクトに衝突
	SetCollisionMask(kCollisionAttributePlayer | kCollisionAttributePlayerBullet | kCollisionAttributeObjects);
}

void BossBullet::Update(const Matrix4x4& viewProjectionMatrix) {
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

void BossBullet::Draw(const Light& directionalLight) {
	// デバッグ表示が有効な場合、コライダーを描画
#ifdef USEIMGUI
	DebugLineAdd();
#endif
	if (!isDead_) {
		gameObject_->Draw(directionalLight);
	}
}

void BossBullet::OnCollision(Collider* other) {
	if (!other) return;

	// 衝突相手がプレイヤー、プレイヤー弾、またはオブジェクトの属性を持つかチェック
	uint32_t otherAttribute = other->GetCollisionAttribute();
	if ((otherAttribute & kCollisionAttributePlayer) ||
		(otherAttribute & kCollisionAttributePlayerBullet) ||
		(otherAttribute & kCollisionAttributeObjects)) {
		// 弾は衝突すると消滅
		isDead_ = true;
	}
}
