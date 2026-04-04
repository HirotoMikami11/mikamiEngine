#include "DebugScene.h"
#include "ImGui/ImGuiManager.h"
#include <numbers>

DebugScene::DebugScene()
	: BaseScene("DebugScene")
{
}

DebugScene::~DebugScene() = default;

void DebugScene::ConfigureOffscreenEffects()
{
	offscreenRenderer_->DisableAllEffects();
}

void DebugScene::OnInitialize()
{
	// システム参照の取得
	dxCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();

	///								カメラの初期化									///

	cameraController_ = CameraController::GetInstance();
	Vector3 initialPosition = { 0.0f, 6.8f, -18.0f };
	Vector3 initialRotation = { 0.4f, 0.0f, 0.0f };
	cameraController_->Initialize(dxCommon_, initialPosition, initialRotation);
	cameraController_->SetActiveCamera("normal");


	///							objectManagerに追加								///


	auto testPlayer = std::make_unique<TestPlayer>();
	testPlayer_ = testPlayer.get();
	gameObjectManager_.AddObject(std::move(testPlayer));

	auto testWall = std::make_unique<TestWall>();
	testWall_ = testWall.get();
	gameObjectManager_.AddObject(std::move(testWall));

	auto testObject = std::make_unique<TestObject>();
	testObject_ = testObject.get();
	gameObjectManager_.AddObject(std::move(testObject));

	// TestShooter は testPlayer_ への参照が必要なので最後に生成
	auto testShooter = std::make_unique<TestShooter>(&gameObjectManager_, testPlayer_);
	testShooter_ = testShooter.get();
	gameObjectManager_.AddObject(std::move(testShooter));


	///							Manager外オブジェクトの初期化						///

	terrain_ = std::make_unique<Model3D>();
	terrain_->Initialize(dxCommon_, "model_terrain");
	terrain_->SetTransform({ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} });

	gltfPlane_ = std::make_unique<Model3D>();
	gltfPlane_->Initialize(dxCommon_, "glftPlane");
	gltfPlane_->SetTransform({ {1.0f,1.0f,1.0f}, {0.0f, std::numbers::pi_v<float>, 0.0f}, {-1.5f,2.0f,0.0f} });

	objPlane_ = std::make_unique<Model3D>();
	objPlane_->Initialize(dxCommon_, "objPlane");
	objPlane_->SetTransform({ {1.0f,1.0f,1.0f}, {0.0f, std::numbers::pi_v<float>, 0.0f}, {1.5f,2.0f,0.0f} });


	///									ライト									///

	DirectionalLight& dirLight = LightManager::GetInstance()->GetDirectionalLight();
	dirLight.SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });

	pointLight_ = LightManager::GetInstance()->AddPointLight(
		{ -5.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f, 1.0f }, 2.0f, 2.5f, 1.6f);

	spotLight_ = LightManager::GetInstance()->AddSpotLight(
		{ 0.0f, 5.0f, 0.0f }, { 90.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f }, 3.0f, 15.0f, 2.0f, 30.0f, 20.0f);

	rectLight_ = LightManager::GetInstance()->AddRectLight(
		{ -2.4f, 0.1f, -5.0f }, { 90.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f, 1.0f }, 1.0f, 4.5f, 2.1f, 10.0f);

	ConfigureOffscreenEffects();
}

void DebugScene::OnUpdate()
{
	// カメラ更新
	cameraController_->Update();
	viewProjectionMatrix_ = cameraController_->GetViewProjectionMatrix();

	// Manager外オブジェクトの更新
	terrain_->Update(viewProjectionMatrix_);
	gltfPlane_->Update(viewProjectionMatrix_);
	objPlane_->Update(viewProjectionMatrix_);
}

void DebugScene::OnDrawOffscreen()
{
	// Manager外オブジェクトの3D描画
	terrain_->Draw();
	gltfPlane_->Draw();
	objPlane_->Draw();
}

void DebugScene::OnImGui()
{
#ifdef USEIMGUI
	ImGui::Spacing();
	ImGui::Text("terrain");
	terrain_->ImGui();

	ImGui::Spacing();
	ImGui::Text("gltfPlane");
	gltfPlane_->ImGui();

	ImGui::Spacing();
	ImGui::Text("objPlane");
	objPlane_->ImGui();
#endif
}

void DebugScene::OnFinalize()
{
	// 生ポインタの nullptr 化
	testPlayer_ = nullptr;
	testWall_ = nullptr;
	testObject_ = nullptr;
	testShooter_ = nullptr;
}
