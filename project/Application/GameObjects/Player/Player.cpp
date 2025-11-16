#include "Player.h"
#include "Input.h"
#include "GameTimer.h"
#include "Managers/ImGui/ImGuiManager.h"
#include <numbers>

Player::Player()
	: directXCommon_(nullptr)
	, input_(nullptr)
	, velocity_({ 0.0f, 0.0f, 0.0f })
	, fireTimer_(0)
{
}

Player::~Player() = default;

void Player::Initialize(DirectXCommon* dxCommon, const Vector3& position) {
	// システム参照の保存
	directXCommon_ = dxCommon;
	input_ = Input::GetInstance();

	// 球体の生成と初期化
	gameObject_ = std::make_unique<Object3D>();
	gameObject_->Initialize(directXCommon_, "model_Player", "white2x2");

	// トランスフォームの設定
	Vector3Transform transform;
	transform.scale = { 0.5f, 0.5f, 0.5f };		// スケール0.5f
	transform.rotate = { 0.0f, 0.0f, 0.0f };
	transform.translate = position;
	gameObject_->SetTransform(transform);

	// 色を青に設定
	gameObject_->SetColor(0x0000FFFF);

	// 速度の初期化
	velocity_ = { 0.0f, 0.0f, 0.0f };

	// 衝突判定の設定
	SetRadius(0.5f);  // 半径を0.5fに設定
	SetCollisionAttribute(kCollisionAttributePlayer);	// 自分の属性をPlayerに設定
	SetCollisionMask(kCollisionAttributeEnemy);			// Enemyと衝突するように設定
}

void Player::Update(const Matrix4x4& viewProjectionMatrix) {
	// 移動処理
	ProcessMovement();

	// 回転処理
	ProcessRotation();

	// 弾の発射処理
	ProcessFire();

	// GameTimerからデルタタイムを取得
	GameTimer& gameTimer = GameTimer::GetInstance();
	float deltaTime = gameTimer.GetDeltaTime();

	// 速度を位置に反映
	Vector3 currentPosition = gameObject_->GetPosition();
	currentPosition.x += velocity_.x * deltaTime;
	currentPosition.z += velocity_.z * deltaTime;
	gameObject_->SetPosition(currentPosition);

	// 弾の更新
	for (auto& bullet : bullets_) {
		bullet->Update(viewProjectionMatrix);
	}

	// 死んだ弾を削除
	DeleteBullets();

	// 行列更新
	gameObject_->Update(viewProjectionMatrix);

	// デバッグ表示が有効な場合、コライダーを描画
#ifdef USEIMGUI
	DebugLineAdd();
#endif
}

void Player::ProcessMovement() {
	// GameTimerからデルタタイムを取得
	GameTimer& gameTimer = GameTimer::GetInstance();
	float deltaTime = gameTimer.GetDeltaTime();

	// 入力を統合（キーボード + ゲームパッド）
	Vector2 moveInput = { 0.0f, 0.0f };

	// WASDキー入力
	if (input_->IsKeyDown(DIK_W)) {
		moveInput.y += 1.0f;  // 前進
	}
	if (input_->IsKeyDown(DIK_S)) {
		moveInput.y -= 1.0f;  // 後退
	}
	if (input_->IsKeyDown(DIK_A)) {
		moveInput.x -= 1.0f;  // 左移動
	}
	if (input_->IsKeyDown(DIK_D)) {
		moveInput.x += 1.0f;  // 右移動
	}

	// 左スティック入力（移動用）
	if (input_->IsGamePadConnected(0)) {
		float stickX = input_->GetAnalogStick(Input::AnalogStick::LEFT_X, 0);
		float stickY = input_->GetAnalogStick(Input::AnalogStick::LEFT_Y, 0);
		moveInput.x += stickX;
		moveInput.y += stickY;
	}

	// X軸（左右）の移動処理
	if (std::abs(moveInput.x) > 0.0f) {
		Vector3 acceleration = { 0.0f, 0.0f, 0.0f };

		if (moveInput.x > 0.0f) {
			// 右移動
			if (velocity_.x < 0.0f) {
				// 速度と逆方向に入力中は急ブレーキ
				velocity_.x *= (1.0f - acceleration_);
			}
			acceleration.x += acceleration_;
		} else {
			// 左移動
			if (velocity_.x > 0.0f) {
				// 速度と逆方向に入力中は急ブレーキ
				velocity_.x *= (1.0f - acceleration_);
			}
			acceleration.x -= acceleration_;
		}

		// 加速減速
		velocity_.x += acceleration.x;
		// 最大速度制限
		velocity_.x = std::clamp(velocity_.x, -limitRunSpeed_, limitRunSpeed_);
	} else {
		// 非入力時は移動減衰をかける
		velocity_.x *= (1.0f - attenuation_);
	}

	// Z軸（前後）の移動処理
	if (std::abs(moveInput.y) > 0.0f) {
		Vector3 acceleration = { 0.0f, 0.0f, 0.0f };

		if (moveInput.y > 0.0f) {
			// 前進
			if (velocity_.z < 0.0f) {
				// 速度と逆方向に入力中は急ブレーキ
				velocity_.z *= (1.0f - acceleration_);
			}
			acceleration.z += acceleration_;
		} else {
			// 後退
			if (velocity_.z > 0.0f) {
				// 速度と逆方向に入力中は急ブレーキ
				velocity_.z *= (1.0f - acceleration_);
			}
			acceleration.z -= acceleration_;
		}

		// 加速減速
		velocity_.z += acceleration.z;
		// 最大速度制限
		velocity_.z = std::clamp(velocity_.z, -limitRunSpeed_, limitRunSpeed_);
	} else {
		// 非入力時は移動減衰をかける
		velocity_.z *= (1.0f - attenuation_);
	}

	// 速度が極小の場合はゼロにする（完全停止）
	if (std::abs(velocity_.x) < 0.001f) {
		velocity_.x = 0.0f;
	}
	if (std::abs(velocity_.z) < 0.001f) {
		velocity_.z = 0.0f;
	}
}

void Player::ProcessRotation() {
	// 入力を統合（キーボード + ゲームパッド）
	Vector2 rotateInput = { 0.0f, 0.0f };

	// 矢印キー入力（向き制御）
	if (input_->IsKeyDown(DIK_UP)) {
		rotateInput.y += 1.0f;
	}
	if (input_->IsKeyDown(DIK_DOWN)) {
		rotateInput.y -= 1.0f;
	}
	if (input_->IsKeyDown(DIK_LEFT)) {
		rotateInput.x -= 1.0f;
	}
	if (input_->IsKeyDown(DIK_RIGHT)) {
		rotateInput.x += 1.0f;
	}

	// 右スティック入力（向き制御用）
	if (input_->IsGamePadConnected(0)) {
		float stickX = input_->GetAnalogStick(Input::AnalogStick::RIGHT_X, 0);
		float stickY = input_->GetAnalogStick(Input::AnalogStick::RIGHT_Y, 0);
		rotateInput.x = stickX;
		rotateInput.y = stickY;
	}

	// 回転の適用
	if (std::abs(rotateInput.x) > 0.0f || std::abs(rotateInput.y) > 0.0f) {
		// 「上が0.0f」になるように調整
		float targetAngle = std::atan2(rotateInput.x, rotateInput.y);

		Vector3 currentRotation = gameObject_->GetRotation();
		currentRotation.y = targetAngle;	// Y軸回転で方向を決定

		gameObject_->SetRotation(currentRotation);
	}
}

void Player::ProcessFire() {
	// 発射タイマーを減らす
	if (fireTimer_ > 0) {
		fireTimer_--;
	}

	// 入力チェック：方向キーまたは右スティック
	bool isInputting = false;

	// 矢印キー入力チェック
	if (input_->IsKeyDown(DIK_UP) || input_->IsKeyDown(DIK_DOWN) ||
		input_->IsKeyDown(DIK_LEFT) || input_->IsKeyDown(DIK_RIGHT)) {
		isInputting = true;
	}

	// 右スティック入力チェック
	if (input_->IsGamePadConnected(0)) {
		float stickX = input_->GetAnalogStick(Input::AnalogStick::RIGHT_X, 0);
		float stickY = input_->GetAnalogStick(Input::AnalogStick::RIGHT_Y, 0);
		// スティックが倒されているか（デッドゾーンを考慮）
		if (std::abs(stickX) > 0.1f || std::abs(stickY) > 0.1f) {
			isInputting = true;
		}
	}

	// 入力があり、発射タイマーが0なら弾を発射
	if (isInputting && fireTimer_ <= 0) {
		// プレイヤーの位置と向きを取得
		Vector3 position = gameObject_->GetPosition();
		Vector3 rotation = gameObject_->GetRotation();

		// 向いている方向に弾を発射
		Vector3 velocity = {
			std::sinf(rotation.y) * bulletSpeed_,
			0.0f,
			std::cosf(rotation.y) * bulletSpeed_
		};

		// 弾を生成
		std::unique_ptr<PlayerBullet> bullet = std::make_unique<PlayerBullet>();
		bullet->Initialize(directXCommon_, position, velocity);
		bullets_.push_back(std::move(bullet));

		// 発射タイマーをリセット
		fireTimer_ = fireInterval_;
	}
}

void Player::DeleteBullets() {
	// 死んだ弾を削除
	bullets_.remove_if([](const std::unique_ptr<PlayerBullet>& bullet) {
		return bullet->IsDead();
		});
}

void Player::Draw(const Light& directionalLight) {
	// 球体の描画
	gameObject_->Draw(directionalLight);

	// 弾の描画
	for (auto& bullet : bullets_) {
		bullet->Draw(directionalLight);
	}
}

void Player::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode("Player")) {

		// 移動パラメータ 
		if (ImGui::CollapsingHeader("Movement")) {
			ImGui::DragFloat3("Velocity", &velocity_.x, 0.01f, -10.0f, 10.0f, "%.2f");
			ImGui::DragFloat("Acceleration", &acceleration_, 0.01f, 0.0f, 2.0f, "%.2f");
			ImGui::DragFloat("Max Run Speed", &limitRunSpeed_, 0.1f, 0.0f, 20.0f, "%.2f");
			ImGui::DragFloat("Attenuation", &attenuation_, 0.01f, 0.0f, 1.0f, "%.2f");
		}

		// 回転パラメータ
		if (ImGui::CollapsingHeader("Rotation")) {
			ImGui::DragFloat("Rotation Speed", &rotationSpeed_, 0.01f, 0.0f, 1.0f, "%.2f");
		}

		// 弾パラメータ
		if (ImGui::CollapsingHeader("Bullet")) {
			ImGui::DragFloat("Bullet Speed", &bulletSpeed_, 0.01f, 0.01f, 2.0f, "%.2f");
			ImGui::DragInt("Fire Interval (frames)", &fireInterval_, 1, 1, 60);
			ImGui::Text("Bullets Count: %zu", bullets_.size());
			ImGui::Text("Fire Timer: %d", fireTimer_);
		}

		// 衝突情報 
		if (ImGui::CollapsingHeader("Collision")) {
			ImGui::Text("Collision Radius: %.2f", GetRadius());
			ImGui::Checkbox("Show Collider", &isColliderVisible_);
		}
		// ゲームオブジェクトのImGui
		gameObject_->ImGui();
		ImGui::TreePop();
	}
#endif
}

void Player::OnCollision(Collider* other) {
	// 衝突時の処理
	// Enemyとの衝突処理
	uint32_t otherAttribute = other->GetCollisionAttribute();

	if (otherAttribute & kCollisionAttributeEnemy) {
		// Enemyとの衝突処理
		// 例: ダメージを受けるなど（今回は特に処理なし）
	}
}

Vector3 Player::GetWorldPosition() {
	// ゲームオブジェクトのワールド座標を返す
	return gameObject_->GetPosition();
}