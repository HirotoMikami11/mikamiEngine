#include "TestWall.h"
#include "Engine.h"
#include "CameraController.h"

void TestWall::Initialize()
{
	tag_ = ObjectTag::TestObject;

	dxCommon_ = Engine::GetInstance()->GetDirectXCommon();

	// モデル生成（Cube モデルで壁を表現）
	model_ = std::make_unique<Model3D>();
	model_->Initialize(dxCommon_, "cube", "white");
	model_->SetPosition({ 5.0f, 1.0f, 0.0f });

	// JsonBinder でパラメータをバインド
	binder_ = std::make_unique<JsonBinder>("TestWall");
	binder_->BindTransform3D("Transform", &model_->GetTransform());
	binder_->Bind("AABBSizeX", &aabbSize_.x, 2.0f);
	binder_->Bind("AABBSizeY", &aabbSize_.y, 2.0f);
	binder_->Bind("AABBSizeZ", &aabbSize_.z, 2.0f);

	// コライダー設定
	SetAABBSize(aabbSize_);
	SetCollisionAttribute(kCollisionAttributeObjects);
	SetCollisionMask(~kCollisionAttributeObjects);
}

void TestWall::Update()
{
	viewProjectionMatrix_ = CameraController::GetInstance()->GetViewProjectionMatrix();
	model_->Update(viewProjectionMatrix_);
}

void TestWall::Draw()
{
	model_->Draw();
	DebugLineAdd();
}

void TestWall::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode("TestWall")) {
		binder_->ImGui();
		// AABB サイズを毎フレーム同期（ImGui反映）
		SetAABBSize(aabbSize_);
		ImGui::TreePop();
	}
#endif
}

void TestWall::Finalize()
{
}

Vector3 TestWall::GetWorldPosition()
{
	return model_->GetPosition();
}
