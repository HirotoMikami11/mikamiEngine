#include "BaseEnemyParts.h"
#include "Enemy/EnemyWorm.h"
#include "ImGui/ImGuiManager.h"

void BaseEnemyParts::Initialize(DirectXCommon* dxCommon, const Vector3& position, const std::string& modelName, const std::string& textureName)
{
	dxCommon_ = dxCommon;

	// オブジェクトの生成
	gameObject_ = std::make_unique<Object3D>();
	gameObject_->Initialize(dxCommon_, modelName, textureName);

	// トランスフォームの設定
	Vector3Transform transform;
	transform.scale = { 1.0f, 1.0f, 1.0f };
	transform.rotate = { 0.0f, 0.0f, 0.0f };
	transform.translate = position;
	gameObject_->SetTransform(transform);

	// コライダーの設定（球体、モデルサイズは直径1.0なので半径0.5）
	SetColliderType(ColliderType::SPHERE);
	SetRadius(transform.scale.x * 0.5f);
}

void BaseEnemyParts::Update(const Matrix4x4& viewProjectionMatrix) {
	// ダメージ色タイマーの更新
	UpdateDamageColorTimer();

	gameObject_->Update(viewProjectionMatrix);
}

void BaseEnemyParts::Draw() {
	// 可視性フラグがfalseなら描画しない
	if (!isVisible_) {
		radius_ = 0.0f; // 当たり判定も無効化
		return;
	}

	// デバッグ表示が有効な場合、コライダーを描画
#ifdef USEIMGUI
	DebugLineAdd();
#endif

	gameObject_->Draw();
}

void BaseEnemyParts::ImGui(const char* label) {
#ifdef USEIMGUI
	if (ImGui::TreeNode(label)) {
		// 状態情報
		if (ImGui::CollapsingHeader("Status", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Checkbox("Is Active", &isActive_);
			ImGui::Checkbox("Is Visible", &isVisible_);
		}

		// コライダー情報
		if (ImGui::CollapsingHeader("Collider")) {
			ImGui::Text("Radius: %.2f", GetRadius());
			ImGui::Checkbox("Debug Visible", &isColliderVisible_);

			// デフォルトカラーの編集
			uint32_t defaultColor = GetDefaultColliderColor();
			if (ImGui::ColorEdit4("Default Color", reinterpret_cast<float*>(&defaultColor))) {
				SetDefaultColliderColor(defaultColor);
			}

			// 現在のカラーの表示（読み取り専用）
			uint32_t currentColor = GetColliderColor();
			ImGui::ColorEdit4("Current Color", reinterpret_cast<float*>(&currentColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
			ImGui::SameLine();
			ImGui::TextDisabled("(Read Only)");

			// 衝突属性の表示
			uint32_t attribute = GetCollisionAttribute();
			std::string attributeStr = "";
			if (attribute & kCollisionAttributePlayer) attributeStr += "Player ";
			if (attribute & kCollisionAttributeEnemy) attributeStr += "Enemy ";
			if (attribute & kCollisionAttributeObjects) attributeStr += "Objects ";
			if (attributeStr.empty()) attributeStr = "None";
			ImGui::Text("Attribute: %s", attributeStr.c_str());
		}

		// ゲームオブジェクトのImGui
		gameObject_->ImGui();
		ImGui::TreePop();
	}
#endif
}

Vector3 BaseEnemyParts::GetPosition() const {
	return gameObject_->GetPosition();
}

void BaseEnemyParts::SetPosition(const Vector3& position) {
	gameObject_->SetPosition(position);
}

Vector3 BaseEnemyParts::GetRotation() const {
	return gameObject_->GetRotation();
}

void BaseEnemyParts::SetRotation(const Vector3& rotation) {
	gameObject_->SetRotation(rotation);
}

void BaseEnemyParts::SetRotationY(float rotationY) {
	Vector3 currentRotation = GetRotation();
	currentRotation.y = rotationY;
	SetRotation(currentRotation);
}

Vector3 BaseEnemyParts::GetScale() const {
	return gameObject_->GetScale();
}

void BaseEnemyParts::SetScale(const Vector3& scale) {
	gameObject_->SetScale(scale);
	// スケール変更時にコライダーのサイズも更新
	SetRadius(scale.x * 0.5f);
}

void BaseEnemyParts::SetColor(uint32_t color) {
	gameObject_->SetColor(color);
}

void BaseEnemyParts::OnCollision(Collider* other) {
	// 可視性フラグがfalseなら衝突しない
	if (!isVisible_ || !isActive_) {
		return;
	}

	// 衝突相手の属性を取得
	uint32_t otherAttribute = other->GetCollisionAttribute();

	// プレイヤーの弾との衝突
	if (otherAttribute & kCollisionAttributePlayerBullet) {
		// ダメージを親のEnemyWormに通知
		if (enemy_) {
			float damage = other->GetAttackPower();
			enemy_->TakeDamage(damage);
			
			// ダメージ色に変更
			SetDamageColor();
		}
	}
}

Vector3 BaseEnemyParts::GetWorldPosition() {
	return GetPosition();
}

void BaseEnemyParts::SetDamageColor() {
	// ダメージ色（赤）に設定
	SetColor(kDamageColor);
	// タイマーをリセット
	damageColorTimer_ = kDamageColorDuration;
}

void BaseEnemyParts::UpdateDamageColorTimer() {
	// タイマーが動いている場合
	if (damageColorTimer_ > 0) {
		damageColorTimer_--;

		// タイマーが0になったら元の色に戻す
		if (damageColorTimer_ == 0) {
			if (isActive_) {
				// アクティブならデフォルトカラー
				SetColor(defaultColor_);
			} else {
				// 非アクティブなら黒
				SetColor(0x000000FF);
			}
		}
	}
}
