#include "RectLight.h"
#include "ImGui/ImGuiManager.h"
#include "MyMath.h"

void RectLight::Initialize(
	const Vector3& position,
	const Vector3& rotation,
	const Vector4& color,
	float intensity,
	float width,
	float height,
	float decay)
{
	lightData_.position = position;
	rotation_ = rotation;
	lightData_.color = color;
	lightData_.intensity = intensity;
	lightData_.width = width;
	lightData_.height = height;
	lightData_.decay = decay;

	// 法線とタンジェントベクトルを計算
	UpdateVectors();

	isActive_ = true;
}

void RectLight::SetDefaultSettings()
{
	// デフォルト設定
	lightData_.color = { 1.0f, 1.0f, 1.0f, 1.0f };	// 白色
	lightData_.position = { 0.0f, 3.0f, 0.0f };		// 少し上方
	rotation_ = { 90.0f, 0.0f, 0.0f };				// 下向き（X軸90度回転）
	lightData_.intensity = 1.0f;
	lightData_.width = 2.0f;
	lightData_.height = 2.0f;
	lightData_.decay = 2.0f;

	// 法線とタンジェントベクトルを計算
	UpdateVectors();

	isActive_ = true;
}

void RectLight::SetRotation(const Vector3& rotation)
{
	rotation_ = rotation;
	UpdateVectors();
}

void RectLight::UpdateVectors()
{
	// 回転行列を作成（度数法をラジアンに変換）
	Vector3 rotationRad = {
		DegToRad(rotation_.x),
		DegToRad(rotation_.y),
		DegToRad(rotation_.z)
	};

	Matrix4x4 rotationMatrix = MakeRotateXYZMatrix(rotationRad);

	// 初期法線ベクトル（Z軸正方向 = 前方）
	Vector3 forward = { 0.0f, 0.0f, 1.0f };
	lightData_.normal = Normalize(Transform(forward, rotationMatrix));

	// 初期タンジェントベクトル（X軸正方向 = 右方向）
	Vector3 right = { 1.0f, 0.0f, 0.0f };
	lightData_.tangent = Normalize(Transform(right, rotationMatrix));

	// バイタンジェントベクトル（Y軸正方向 = 上方向）
	Vector3 up = { 0.0f, 1.0f, 0.0f };
	lightData_.bitangent = Normalize(Transform(up, rotationMatrix));
}

void RectLight::ImGui(const std::string& label)
{
#ifdef USEIMGUI

	if (ImGui::TreeNode(label.c_str())) {
		// 有効/無効切り替え
		ImGui::Checkbox("Active", &isActive_);

		ImGui::Separator();

		// ライトの色
		if (ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&lightData_.color.x))) {

		}

		// ライトの位置
		if (ImGui::DragFloat3("Position", &lightData_.position.x, 0.1f)) {

		}

		// ライトの回転（Euler角、度数法）
		Vector3 rotationDeg = rotation_;
		if (ImGui::DragFloat3("Rotation (deg)", &rotationDeg.x, 1.0f, -180.0f, 180.0f)) {
			SetRotation(rotationDeg);
		}

		// 現在の法線ベクトルを表示（読み取り専用）
		ImGui::Text("Normal: (%.3f, %.3f, %.3f)",
			lightData_.normal.x,
			lightData_.normal.y,
			lightData_.normal.z);

		ImGui::Separator();

		// ライトの強度
		if (ImGui::DragFloat("Intensity", &lightData_.intensity, 0.01f, 0.0f, 10.0f)) {

		}

		// 矩形のサイズ
		if (ImGui::DragFloat("Width", &lightData_.width, 0.1f, 0.1f, 20.0f)) {

		}

		if (ImGui::DragFloat("Height", &lightData_.height, 0.1f, 0.1f, 20.0f)) {

		}

		// 減衰率
		if (ImGui::DragFloat("Decay", &lightData_.decay, 0.1f, 0.1f, 10.0f)) {

		}

		ImGui::Separator();

		// ライトの種類表示（読み取り専用）
		ImGui::Text("Type: Area (Rectangular)");

		// 矩形の面積を表示
		float area = lightData_.width * lightData_.height;
		ImGui::Text("Area: %.2f units²", area);

		ImGui::TreePop();
	}

#endif
}