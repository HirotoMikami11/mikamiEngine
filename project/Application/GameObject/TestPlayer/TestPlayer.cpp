#include "TestPlayer.h"
#include "Engine.h"
#include "Input.h"
#include "CameraController.h"
#include "GameTimer.h"

void TestPlayer::Initialize()
{
	SetTag(ObjectTag::Player);

	dxCommon_ = Engine::GetInstance()->GetDirectXCommon();

	// モデル生成（デフォルト値を設定してからバインドする）
	model_ = std::make_unique<Sphere>();
	model_->Initialize(dxCommon_, "sphere", "white");
	model_->SetPosition({ 0.0f, 1.0f, 0.0f });
	model_->SetLightingMode(LightingMode::PhongSpecular);

	// JsonBinder でパラメータをバインド
	// コンストラクタ内で CreateGroup + LoadFile が自動実行される
	binder_ = std::make_unique<JsonBinder>("TestPlayer");
	binder_->BindTransform3D("Transform", &model_->GetTransform());
	binder_->BindMaterial("Material", &model_->GetMaterial());
	binder_->Bind("MoveSpeed", &moveSpeed_, 5.0f);
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
	if (ImGui::TreeNode("TestPlayer")) {
		binder_->ImGui();
		ImGui::TreePop();
	}
#endif
}

void TestPlayer::Finalize()
{
}
