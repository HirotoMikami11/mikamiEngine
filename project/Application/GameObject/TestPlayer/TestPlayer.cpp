#include "TestPlayer.h"
#include "Engine.h"
#include "Input.h"
#include "JsonSettings.h"
#include "CameraController.h"
#include "GameTimer.h"
#include "ImGui/ImGuiManager.h"

void TestPlayer::Initialize()
{
	SetTag(ObjectTag::Player);

	dxCommon_ = Engine::GetInstance()->GetDirectXCommon();

	// モデル生成
	model_ = std::make_unique<Sphere>();
	model_->Initialize(dxCommon_, "sphere", "white");
	model_->SetPosition({ 0.0f, 1.0f, 0.0f });
	model_->SetLightingMode(LightingMode::PhongSpecular);

	// JsonSettings に初期値を登録してファイルから読み込む
	auto* js = JsonSettings::GetInstance();
	js->CreateGroup({ kGroupName });
	js->AddItem({ kGroupName }, "Position", model_->GetPosition());
	js->AddItem({ kGroupName }, "Scale", model_->GetScale());
	js->AddItem({ kGroupName }, "Color", model_->GetColor());
	js->AddItem({ kGroupName }, "MoveSpeed", moveSpeed_);

	LoadFromJson();
	ApplyJsonValues();
}

void TestPlayer::Update()
{
	auto* input = Input::GetInstance();
	const float dt = GameTimer::GetInstance().GetDeltaTime();

	// WASD 移動（XZ 平面）
	Vector3 pos = model_->GetPosition();
	if (input->IsKeyDown(DIK_W)) pos.z += moveSpeed_ * dt;
	if (input->IsKeyDown(DIK_S)) pos.z -= moveSpeed_ * dt;
	if (input->IsKeyDown(DIK_A)) pos.x -= moveSpeed_ * dt;
	if (input->IsKeyDown(DIK_D)) pos.x += moveSpeed_ * dt;
	model_->SetPosition(pos);

	// VP 行列更新
	viewProjectionMatrix_ = CameraController::GetInstance()->GetViewProjectionMatrix();
	model_->Update(viewProjectionMatrix_);
}

void TestPlayer::DrawOffscreen()
{
	model_->Draw();
}

void TestPlayer::ImGui()
{
#ifdef USEIMGUI
	auto* js = JsonSettings::GetInstance();

	Vector3 pos = model_->GetPosition();
	if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
		model_->SetPosition(pos);
		js->SetValue({ kGroupName }, "Position", pos);
	}

	Vector3 scale = model_->GetScale();
	if (ImGui::DragFloat3("Scale", &scale.x, 0.01f, 0.01f, 10.0f)) {
		model_->SetScale(scale);
		js->SetValue({ kGroupName }, "Scale", scale);
	}

	Vector4 color = model_->GetColor();
	if (ImGui::ColorEdit4("Color", &color.x)) {
		model_->SetColor(color);
		js->SetValue({ kGroupName }, "Color", color);
	}

	if (ImGui::DragFloat("MoveSpeed", &moveSpeed_, 0.1f, 0.0f, 50.0f)) {
		js->SetValue({ kGroupName }, "MoveSpeed", moveSpeed_);
	}

	if (ImGui::Button("Save (TestPlayer)")) {
		js->SaveFile({ kGroupName });
	}
#endif
}

void TestPlayer::Finalize()
{
}

void TestPlayer::LoadFromJson()
{
	JsonSettings::GetInstance()->LoadFile(kGroupName);
}

void TestPlayer::ApplyJsonValues()
{
	auto* js = JsonSettings::GetInstance();
	const auto* group = js->FindGroup({ kGroupName });
	if (!group) return;

	if (group->items.count("Position"))
		model_->SetPosition(js->GetVector3Value({ kGroupName }, "Position"));
	if (group->items.count("Scale"))
		model_->SetScale(js->GetVector3Value({ kGroupName }, "Scale"));
	if (group->items.count("Color"))
		model_->SetColor(js->GetVector4Value({ kGroupName }, "Color"));
	if (group->items.count("MoveSpeed"))
		moveSpeed_ = js->GetFloatValue({ kGroupName }, "MoveSpeed");
}
