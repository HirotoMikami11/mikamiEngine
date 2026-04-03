#include "TestObject.h"
#include "Engine.h"
#include "CameraController.h"

void TestObject::Initialize()
{
	tag_ = ObjectTag::TestObject;

	dxCommon_ = Engine::GetInstance()->GetDirectXCommon();

	// モデル生成
	model_ = std::make_unique<Sphere>();
	model_->Initialize(dxCommon_, "sphere", "white");
	model_->SetPosition({ -5.0f, 1.0f, 0.0f });
	model_->SetLightingMode(LightingMode::PhongSpecular);

	// JsonBinder でパラメータをバインド
	binder_ = std::make_unique<JsonBinder>("TestObject");
	binder_->BindTransform3D("Transform", &model_->GetTransform());
	binder_->BindMaterial("Material", &model_->GetMaterial());
	binder_->Bind("ColliderRadius", &radius_, 1.0f);

	// コライダー設定
	SetCollisionAttribute(kCollisionAttributeObjects);
	SetCollisionMask(~kCollisionAttributeObjects);
}

void TestObject::Update()
{
	viewProjectionMatrix_ = CameraController::GetInstance()->GetViewProjectionMatrix();
	model_->Update(viewProjectionMatrix_);
}

void TestObject::DrawOffscreen()
{
	model_->Draw();
	DebugLineAdd();
}

void TestObject::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode("TestObject")) {
		binder_->ImGui();
		ImGui::TreePop();
	}
#endif
}

void TestObject::Finalize()
{
}

Vector3 TestObject::GetWorldPosition()
{
	return model_->GetPosition();
}
