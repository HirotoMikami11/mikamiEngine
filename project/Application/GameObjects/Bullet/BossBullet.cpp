#include "BossBullet.h"
#include "BossBulletHitEffectPool.h"

BossBullet::BossBullet()
{
}

BossBullet::~BossBullet() = default;

void BossBullet::Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity) {
	dxCommon_ = dxCommon;

	///リソースを生成

	// ゲームオブジェクト（球体）の初期化
	gameObject_ = std::make_unique<Object3D>();
	gameObject_->Initialize(dxCommon_, "playerBullet", "white2x2");

	// トランスフォームの初期設定（ダミー値）
	Vector3Transform transform;
	transform.scale = { 1.0f, 1.0f, 1.0f };
	transform.rotate = { 0.0f, 0.0f, 0.0f };
	transform.translate = { 0.0f, -1000.0f, 0.0f };  // 画面外に配置
	gameObject_->SetTransform(transform);

	// 弾の色を紫色に設定
	gameObject_->SetColor(0xFF00FFFF);

	// 衝突判定設定
	SetRadius(1.0f);
	SetAttackPower(5.0f);
	SetCollisionAttribute(kCollisionAttributeEnemyBullet);
	SetCollisionMask(kCollisionAttributePlayer | kCollisionAttributePlayerBullet | kCollisionAttributeObjects);

	// 初期状態は非アクティブ
	isDead_ = true;
	velocity_ = { 0.0f, 0.0f, 0.0f };
	deathTimer_ = 0;
}

void BossBullet::Activate(const Vector3& position, const Vector3& velocity) {


	// 状態をリセット
	isDead_ = false;
	deathTimer_ = kLifeTime;
	velocity_ = velocity;

	// 位置を設定（Transform3Dのリソースは再利用）
	gameObject_->SetPosition(position);

	// 速度の方向を向くように回転
	SetToVelocityDirection();
}

void BossBullet::Deactivate() {
	//非アクティブ化する
	isDead_ = true;
	velocity_ = { 0.0f, 0.0f, 0.0f };
	deathTimer_ = 0;

	// 画面外に移動（描画されないように）
	gameObject_->SetPosition({ 0.0f, -1000.0f, 0.0f });
}

void BossBullet::Update(const Matrix4x4& viewProjectionMatrix) {
	// 非アクティブなら更新しない
	if (isDead_) {
		return;
	}

	// 座標を移動させる
	Vector3 currentPos = gameObject_->GetPosition();
	currentPos = currentPos + velocity_;
	gameObject_->SetPosition(currentPos);

	// タイマーを減らす
	deathTimer_--;
	if (deathTimer_ <= 0) {
		Deactivate();  // タイマー切れで非アクティブ化
		return;
	}

	// ゲームオブジェクトの更新
	gameObject_->Update(viewProjectionMatrix);
}

void BossBullet::Draw() {
	// 非アクティブなら描画しない
	if (isDead_) {
		return;
	}

	// デバッグ表示が有効な場合、コライダーを描画
#ifdef USEIMGUI
	DebugLineAdd();
#endif

	gameObject_->Draw();
}

void BossBullet::OnCollision(Collider* other) {
	if (!other) return;

	// 非アクティブなら衝突判定しない
	if (isDead_) {
		return;
	}

	// 衝突相手がプレイヤー、プレイヤー弾、またはオブジェクトの属性を持つかチェック
	uint32_t otherAttribute = other->GetCollisionAttribute();
	if ((otherAttribute & kCollisionAttributePlayer) ||
		(otherAttribute & kCollisionAttributePlayerBullet) ||
		(otherAttribute & kCollisionAttributeObjects)) {

		// ヒットエフェクトを発動
		if (hitEffectPool_) {
			// 衝突位置（弾丸の現在位置）
			Vector3 hitPosition = gameObject_->GetPosition();

			// 弾丸の進行方向の逆向き（ヒットエフェクトの方向）
			Vector3 hitDirection = velocity_;
			float length = std::sqrt(hitDirection.x * hitDirection.x +
				hitDirection.y * hitDirection.y +
				hitDirection.z * hitDirection.z);

			// 正規化して逆向きにする
			if (length > 0.0f) {
				hitDirection.x = -hitDirection.x / length;
				hitDirection.y = -hitDirection.y / length;
				hitDirection.z = -hitDirection.z / length;
			}
			
			//自機とぶつかった時だけ効果音
			if(otherAttribute& kCollisionAttributePlayer) {
				AudioManager::GetInstance()->Play("PlayerHit", false, 0.5f);
			}
			// エフェクトを発動
			hitEffectPool_->TriggerEffect(hitPosition, hitDirection);
		}

		// 弾は衝突すると非アクティブ化
		Deactivate();
	}
}