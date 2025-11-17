#include "BaseField.h"
#include "ImGui/ImGuiManager.h"

void BaseField::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;

	// フィールドトランスフォームを初期化
	fieldTransform_.Initialize(dxCommon);
	fieldTransform_.SetDefaultTransform();

	// DebugDrawLineSystemを取得
	debugDrawLineSystem_ = DebugDrawLineSystem::GetInstance();
}

void BaseField::Update(float deltaTime)
{
	// フィールドトランスフォームの更新
	Matrix4x4 dummyMatrix = MakeIdentity4x4();
	fieldTransform_.UpdateMatrix(dummyMatrix);
}

void BaseField::AddLineDebug()
{
	if (!showDebugVisualization_ ) {
		return;
	}

#ifdef USEIMGUI
	// デバッグ形状を作成
	CreateDebugShape();
#endif
}

void BaseField::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode(name_.c_str())) {
		// 基本情報
		if (ImGui::CollapsingHeader("Field Info", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Type: %s", GetTypeName());
			ImGui::Checkbox("Is Enabled", &isEnabled_);

			// フィールド位置
			Vector3 pos = fieldTransform_.GetPosition();
			if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
				fieldTransform_.SetPosition(pos);
			}

			ImGui::Separator();
		}

		// デバッグ描画設定
		if (ImGui::CollapsingHeader("Debug Visualization")) {
			ImGui::Checkbox("Show Visualization", &showDebugVisualization_);
			ImGui::ColorEdit4("Color", &debugColor_.x);
		}

		ImGui::TreePop();
	}
#endif
}