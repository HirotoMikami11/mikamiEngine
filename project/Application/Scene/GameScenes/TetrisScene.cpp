#include "TetrisScene.h"
#include "ImGui/ImGuiManager.h" 
#include <numbers> 

TetrisScene::TetrisScene()
	: BaseScene("TetrisScene") // シーン名を設定
	, cameraController_(nullptr)
	, dxCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, viewProjectionMatrix{ MakeIdentity4x4() }
{
}

TetrisScene::~TetrisScene() = default;

void TetrisScene::ConfigureOffscreenEffects()
{
	/// オフスクリーンレンダラーのエフェクト設定

	// 全てのエフェクトを無効化
	offscreenRenderer_->DisableAllEffects();
	// 必要に応じてここでエフェクトを有効化
}

void TetrisScene::Initialize() {
	// システム参照の取得
	dxCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();

	///*-----------------------------------------------------------------------*///
	///								カメラの初期化									///
	///*-----------------------------------------------------------------------*///
	cameraController_ = CameraController::GetInstance();
	// 座標と回転を指定して初期化
	Vector3 initialPosition = { 0.0f, 6.8f, -18.0f };
	Vector3 initialRotation = { 0.4f, 0.0f, 0.0f };
	cameraController_->Initialize(dxCommon_, initialPosition, initialRotation);
	cameraController_->SetActiveCamera("normal");

	// ゲームオブジェクト初期化
	InitializeGameObjects();
	//ポストエフェクトの初期設定
	ConfigureOffscreenEffects();
}

void TetrisScene::InitializeGameObjects() {

	///*-----------------------------------------------------------------------*///
	///								ライト									///
	///*-----------------------------------------------------------------------*///

	// シーン内で平行光源を取得して編集
	DirectionalLight& dirLight = LightManager::GetInstance()->GetDirectionalLight();
	dirLight.SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });	// 黒い光（無効化）

}

void TetrisScene::Update() {
	// カメラ更新
	cameraController_->Update();

	// ゲームオブジェクト更新
	UpdateGameObjects();
}

void TetrisScene::UpdateGameObjects() {
	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();

}

void TetrisScene::DrawOffscreen() {

	///
	///3Dゲームオブジェクトの描画（オフスクリーンに描画）
	/// 

	///
	/// パーティクル・スプライトの描画（オフスクリーンに描画）
	/// 


}

void TetrisScene::DrawBackBuffer() {
	///
	/// 3Dゲームオブジェクトの描画（オフスクリーンの外に描画）
	///


	///
	/// パーティクル・スプライトの描画（オフスクリーンの外に描画）
	/// 

}

void TetrisScene::ImGui() {
#ifdef USEIMGUI

	ImGui::Text("Debug Scene");
#endif
}

void TetrisScene::Finalize() {
}