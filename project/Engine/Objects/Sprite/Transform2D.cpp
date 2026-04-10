#include "Transform2D.h"
#include "ImGui/ImGuiManager.h"

void Transform2D::Initialize()
{
	SetDefaultTransform();
}

void Transform2D::UpdateMatrix(const Matrix4x4& viewProjectionMatrix)
{
	// 2D用のワールド行列を作成（S * R * T の順）
	Matrix4x4 scaleMatrix     = MakeScaleMatrix({ transform_.scale.x, transform_.scale.y, 1.0f });
	Matrix4x4 rotateMatrix    = MakeRotateZMatrix(transform_.rotateZ);
	Matrix4x4 translateMatrix = MakeTranslateMatrix({ transform_.translate.x, transform_.translate.y, 0.0f });

	cpuData_.World = Matrix4x4Multiply(scaleMatrix, rotateMatrix);
	cpuData_.World = Matrix4x4Multiply(cpuData_.World, translateMatrix);

	// ビュープロジェクション行列を掛けてWVP行列を計算
	cpuData_.WVP = Matrix4x4Multiply(cpuData_.World, viewProjectionMatrix);
}

void Transform2D::SetDefaultTransform()
{
	transform_.scale     = { 1.0f, 1.0f };
	transform_.rotateZ   = 0.0f;
	transform_.translate = { 0.0f, 0.0f };

	cpuData_.World                = MakeIdentity4x4();
	cpuData_.WVP                  = MakeIdentity4x4();
	cpuData_.WorldInverseTranspose = MakeIdentity4x4();
}

void Transform2D::ImGui()
{
#ifdef USEIMGUI
	// 2D座標用（XYのみ）
	ImGui::DragFloat2("Position", &transform_.translate.x, 1.0f);

	// Z軸回転のみ
	ImGui::SliderAngle("Rotation", &transform_.rotateZ);

	// 2Dサイズ用（XYのみ）- スケールとして管理
	ImGui::DragFloat2("Size", &transform_.scale.x, 1.0f, 0.1f, 1000.0f);
#endif
}
