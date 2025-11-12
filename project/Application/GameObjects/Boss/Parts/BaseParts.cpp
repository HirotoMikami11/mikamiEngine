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
}

void BaseParts::Update(const Matrix4x4& viewProjectionMatrix) {
	gameObject_->Update(viewProjectionMatrix);
}

void BaseParts::Draw(const Light& directionalLight) {
	gameObject_->Draw(directionalLight);
}

void BaseParts::ImGui(const char* label) {
#ifdef USEIMGUI
	if (ImGui::TreeNode(label)) {
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