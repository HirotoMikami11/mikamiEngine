#include "GameScene.h"
#include "ImGui/ImGuiManager.h"
#include "SceneTransitionHelper.h"
#include <numbers>

GameScene::GameScene()
	: BaseScene("GameScene")
{
}

GameScene::~GameScene() = default;

void GameScene::ConfigureOffscreenEffects()
{
	offscreenRenderer_->DisableAllEffects();
}

void GameScene::OnInitialize()
{
	// システム参照の取得
	dxCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();

	///*-----------------------------------------------------------------------*///
	///								カメラの初期化									///
	///*-----------------------------------------------------------------------*///
	cameraController_ = CameraController::GetInstance();
	Vector3 initialPosition = { 0.0f, 6.8f, -18.0f };
	Vector3 initialRotation = { 0.4f, 0.0f, 0.0f };
	cameraController_->Initialize(dxCommon_, initialPosition, initialRotation);
	cameraController_->SetActiveCamera("normal");

	///*-----------------------------------------------------------------------*///
	///									ライト									///
	///*-----------------------------------------------------------------------*///
	DirectionalLight& dirLight = LightManager::GetInstance()->GetDirectionalLight();
	dirLight.SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });

	ConfigureOffscreenEffects();
}

void GameScene::OnUpdate()
{
	cameraController_->Update();
	viewProjectionMatrix_ = cameraController_->GetViewProjectionMatrix();
}

void GameScene::OnDraw()
{
}

void GameScene::OnDrawBackBuffer()
{
}

void GameScene::OnImGui()
{
#ifdef USEIMGUI
	if (ImGui::Button("Change Scene")) {
		//debugSceneに切り替え
		SceneTransitionHelper::FadeToScene("DemoScene", 1.0f);
	}

#endif
}
