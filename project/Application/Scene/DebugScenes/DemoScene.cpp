#include "DemoScene.h"
#include "ImGui/ImGuiManager.h"
#include "GameTimer.h"
#include <numbers>
#include "GravityField.h"
#include "AccelerationField.h"
#include "ParticleEditor.h"


DemoScene::DemoScene()
	: BaseScene("DemoScene")
{
}

DemoScene::~DemoScene() = default;

void DemoScene::ConfigureOffscreenEffects()
{
	offscreenRenderer_->DisableAllEffects();

	auto* vignetteEffect = offscreenRenderer_->GetVignetteEffect();
	if (vignetteEffect) {
		vignetteEffect->SetEnabled(true);
	}
}

void DemoScene::OnInitialize()
{
	// システム参照の取得
	dxCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();

	particleSystem_ = ParticleSystem::GetInstance();
	particleSystem_->Initialize(dxCommon_);

	particleEditor_ = ParticleEditor::GetInstance();
	particleEditor_->Initialize(dxCommon_);

	///								カメラの初期化									///

	cameraController_ = CameraController::GetInstance();
	Vector3 initialPosition = { 0.0f, 6.8f, -18.0f };
	Vector3 initialRotation = { 0.4f, 0.0f, 0.0f };
	cameraController_->Initialize(dxCommon_, initialPosition, initialRotation);
	cameraController_->SetActiveCamera("normal");

	///							Manager外オブジェクトの初期化						///

	sphere_ = std::make_unique<Sphere>();
	sphere_->Initialize(dxCommon_, "sphere", "monsterBall");
	sphere_->SetTransform({ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} });

	plane_ = std::make_unique<Plane>();
	plane_->Initialize(dxCommon_, "plane", "uvChecker");
	plane_->SetTransform({ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {3.0f,1.0f,-8.0f} });

	modelMultiMesh_ = std::make_unique<Model3D>();
	modelMultiMesh_->Initialize(dxCommon_, "model_MultiMesh");
	modelMultiMesh_->SetTransform({ {1.0f,1.0f,1.0f}, {0.0f,3.0f,0.0f}, {-5.31f,-0.3f,3.7f} });

	modelMultiMaterial_ = std::make_unique<Model3D>();
	modelMultiMaterial_->Initialize(dxCommon_, "model_MultiMaterial");
	modelMultiMaterial_->SetTransform({ {1.0f,1.0f,1.0f}, {0.0f,3.0f,0.0f}, {2.23f,-0.3f,3.7f} });

	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(dxCommon_, "uvChecker", { 50, 50 }, { 100, 100 });

	///								パーティクル								///
	particleEditor_->CreateInstance("CenterEffect", "Center");
	particleEditor_->CreateInstance("LeftEffect", "Left");
	particleEditor_->CreateInstance("RightEffect", "Right");


	ConfigureOffscreenEffects();
}

void DemoScene::OnUpdate()
{
	GameTimer& gameTimer = GameTimer::GetInstance();
	float gameDeltaTime = gameTimer.GetDeltaTime();

	cameraController_->Update();

	// 球体を回転させる
	sphere_->AddRotation({ 0.0f, DegToRad(30), 0.0f });

	// 行列更新
	viewProjectionMatrix_ = cameraController_->GetViewProjectionMatrix();
	viewProjectionMatrixSprite_ = cameraController_->GetViewProjectionMatrixSprite();

	sphere_->Update(viewProjectionMatrix_);
	sprite_->Update(viewProjectionMatrixSprite_);

	// アルファ値を時間で変化させる
	float a = (sinf(gameTimer.GetGameTime() * 2.0f) + 1.0f) / 2.0f;
	sprite_->SetColor({ 1.0f, 1.0f, 1.0f, a });

	// 平面の移動
	Input* input = Input::GetInstance();
	Vector3 planePosition = plane_->GetPosition();
	constexpr float moveSpeed = 1.5f;
	if (input->IsKeyDown(DIK_A)) planePosition.x -= moveSpeed * gameDeltaTime;
	if (input->IsKeyDown(DIK_D)) planePosition.x += moveSpeed * gameDeltaTime;
	plane_->SetPosition(planePosition);


	plane_->Update(viewProjectionMatrix_);
	modelMultiMesh_->Update(viewProjectionMatrix_);
	modelMultiMaterial_->Update(viewProjectionMatrix_);
	particleSystem_->Update(viewProjectionMatrix_);

}

void DemoScene::OnDrawOffscreen()
{
	sphere_->Draw();
	plane_->Draw();
	modelMultiMesh_->Draw();
	modelMultiMaterial_->Draw();
	particleSystem_->Draw();
}

void DemoScene::OnDrawBackBuffer()
{
	sprite_->Draw();
}

void DemoScene::OnImGui()
{
#ifdef USEIMGUI
	ImGui::Text("Sphere");
	sphere_->ImGui();

	ImGui::Spacing();
	ImGui::Text("Sprite");
	sprite_->ImGui();

	ImGui::Spacing();
	ImGui::Text("plane");
	plane_->ImGui();

	ImGui::Spacing();
	ImGui::Text("ModelMultiMesh");
	modelMultiMesh_->ImGui();

	ImGui::Spacing();
	ImGui::Text("ModelMultiMaterial");
	modelMultiMaterial_->ImGui();

	ImGui::Spacing();
	ImGui::Text("Particle Editor");
	particleEditor_->ImGui();
#endif
}

void DemoScene::OnFinalize()
{
	if (particleSystem_) {
		particleSystem_->Clear();
	}
	if (particleEditor_) {
		particleEditor_->DestroyAllInstance();
	}
}
