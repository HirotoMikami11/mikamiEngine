#include "TestShooter.h"
#include "TestBullet.h"
#include "GameObjectManager.h"
#include "DebugObject/TestPlayer/TestPlayer.h"
#include "Engine.h"
#include "Input.h"
#include "CameraController.h"

TestShooter::TestShooter(GameObjectManager* objectManager, TestPlayer* target)
	: objectManager_(objectManager)
	, target_(target)
{
}

void TestShooter::Initialize()
{
	tag_      = ObjectTag::TestObject;
	dxCommon_ = Engine::GetInstance()->GetDirectXCommon();

	// 赤いキューブモデル（小さめにして "砲台" 感を出す）
	model_ = std::make_unique<Model3D>();
	model_->Initialize(dxCommon_, "cube", "white");
	model_->SetPosition({ -5.0f, 1.0f, 0.0f });
	model_->SetScale({ 0.6f, 0.6f, 0.6f });
	model_->GetMaterial().SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
	model_->SetLightingMode(LightingMode::HalfLambert);

	// JsonBinder でパラメータをバインド
	binder_ = std::make_unique<JsonBinder>("TestShooter");
	binder_->BindTransform3D("Transform",   &model_->GetTransform());
	binder_->Bind("BulletLifetime", &bulletLifetime_, 3.0f);
	binder_->Bind("BulletSpeed",    &bulletSpeed_,    10.0f);
	binder_->Bind("BulletRadius",   &bulletRadius_,   0.3f);
}

void TestShooter::Update()
{
	auto* input = Input::GetInstance();

	// SHIFT（左右どちらでも）+ 1 押した瞬間に発射
	if ((input->IsKeyDown(DIK_LSHIFT) || input->IsKeyDown(DIK_RSHIFT))
		&& input->IsKeyTrigger(DIK_1)) {
		Shoot();
	}

	viewProjectionMatrix_ = CameraController::GetInstance()->GetViewProjectionMatrix();
	model_->Update(viewProjectionMatrix_);
}

void TestShooter::Draw()
{
	model_->Draw();
}

void TestShooter::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode("TestShooter")) {
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[SHIFT + 1] : 自機に向けて弾を発射");
		ImGui::Spacing();
		binder_->ImGui();
		ImGui::TreePop();
	}
#endif
}

void TestShooter::Finalize() {}

void TestShooter::Shoot()
{
	if (!target_) return;

	const Vector3 shooterPos = model_->GetPosition();
	const Vector3 targetPos  = target_->GetWorldPosition();
	const Vector3 diff       = targetPos - shooterPos;

	// 距離ゼロなら発射しない（除算ガード）
	if (Length(diff) < 0.0001f) return;

	const Vector3 dir = Normalize(diff);

	auto bullet = std::make_unique<TestBullet>();
	bullet->Setup(dxCommon_, shooterPos, dir, bulletSpeed_, bulletRadius_, bulletLifetime_);
	objectManager_->AddObject(std::move(bullet));
}
