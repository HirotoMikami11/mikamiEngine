#include "Parts/BaseParts.h"
#include "Managers/ImGui/ImGuiManager.h"

void BaseParts::Initialize(DirectXCommon* dxCommon, const Vector3& position) {
	directXCommon_ = dxCommon;

	// キューブオブジェクトの生成
	gameObject_ = std::make_unique<Object3D>();
	gameObject_->Initialize(directXCommon_, "cube", "white2x2");

	// トランスフォームの設定
	Vector3Transform transform;
	transform.scale = { 1.0f, 1.0f, 1.0f };  // 1.0f × 1.0f × 1.0f
	transform.rotate = { 0.0f, 0.0f, 0.0f };
	transform.translate = position;
	gameObject_->SetTransform(transform);

	// コライダーの設定（キューブの半分のサイズ = 0.5f）
	SetRadius(0.5f);

	// デフォルトのデバッグカラー（緑）
	SetDefaultDebugColor({ 0.0f, 1.0f, 0.0f, 1.0f });
	SetDebugColor({ 0.0f, 1.0f, 0.0f, 1.0f });
}

void BaseParts::Update(const Matrix4x4& viewProjectionMatrix) {
	gameObject_->Update(viewProjectionMatrix);


}

void BaseParts::Draw(const Light& directionalLight) {
	// デバッグ表示が有効な場合、コライダーを描画
#ifdef USEIMGUI
	if (isDebugVisible_) {
		DebugLineAdd();
	}
#endif

	gameObject_->Draw(directionalLight);
}

void BaseParts::ImGui(const char* label) {
#ifdef USEIMGUI
	if (ImGui::TreeNode(label)) {
		// HP情報
		if (ImGui::CollapsingHeader("HP", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Current HP: %.1f / %.1f", currentHP_, maxHP_);
			float hpPercentage = (maxHP_ > 0.0f) ? (currentHP_ / maxHP_) * 100.0f : 0.0f;
			ImGui::ProgressBar(currentHP_ / maxHP_, ImVec2(0.0f, 0.0f), std::format("{:.1f}%%", hpPercentage).c_str());
			ImGui::Checkbox("Is Active", &isActive_);
		}

		// コライダー情報
		if (ImGui::CollapsingHeader("Collider")) {
			ImGui::Text("Radius: %.2f", GetRadius());
			ImGui::Checkbox("Debug Visible", &isDebugVisible_);

			// デフォルトカラーの編集
			Vector4 defaultColor = GetDefaultDebugColor();
			if (ImGui::ColorEdit4("Default Color", reinterpret_cast<float*>(&defaultColor))) {
				SetDefaultDebugColor(defaultColor);
			}

			// 現在のカラーの表示（読み取り専用）
			Vector4 currentColor = GetDebugColor();
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

Vector3 BaseParts::GetPosition() const {
	return gameObject_->GetPosition();
}

void BaseParts::SetPosition(const Vector3& position) {
	gameObject_->SetPosition(position);
}

Vector3 BaseParts::GetRotation() const {
	return gameObject_->GetRotation();
}

void BaseParts::SetRotation(const Vector3& rotation) {
	gameObject_->SetRotation(rotation);
}

void BaseParts::SetRotationY(float rotationY) {
	Vector3 currentRotation = GetRotation();
	currentRotation.y = rotationY;
	SetRotation(currentRotation);
}

void BaseParts::SetScale(const Vector3& scale) {
	gameObject_->SetScale(scale);
}

void BaseParts::SetColor(const Vector4& color) {
	gameObject_->SetColor(color);
}

void BaseParts::SetColor(uint32_t color) {
	gameObject_->SetColor(color);
}

void BaseParts::OnCollision(Collider* other) {
	// 基底クラスでは何もしない（派生クラスでオーバーライド）
	(void)other;
}

Vector3 BaseParts::GetWorldPosition() {
	return GetPosition();
}

void BaseParts::TakeDamage(float damage) {
	if (!isActive_) {
		return;
	}

	currentHP_ -= damage;

	// HP0以下になったら非アクティブに
	if (currentHP_ <= 0.0f) {
		currentHP_ = 0.0f;
		SetActive(false);
	}
}

void BaseParts::SetActive(bool active) {
	isActive_ = active;

	if (!isActive_) {
		// 非アクティブになったら黒に変更
		SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });
		// 衝突属性をObjectsに変更
		SetCollisionAttribute(kCollisionAttributeObjects);
		SetCollisionMask(0x00000000);  // どことも衝突しない

	} else {
		// アクティブになったらデフォルトカラーに戻す
		SetColor(defaultColor_);
	}
}