#include "TitleScene.h"
#include "ImGui/ImGuiManager.h" 
#include "Transition/SceneTransitionHelper.h" 
#include <numbers> 

TitleScene::TitleScene()
	: BaseScene("TitleScene") // シーン名を設定
	, cameraController_(nullptr)
	, dxCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, viewProjectionMatrix{ MakeIdentity4x4() }
{
}

TitleScene::~TitleScene() = default;

void TitleScene::ConfigureOffscreenEffects()
{
	/// オフスクリーンレンダラーのエフェクト設定

	// 全てのエフェクトを無効化
	offscreenRenderer_->DisableAllEffects();
	// 必要に応じてここでエフェクトを有効化
}

void TitleScene::Initialize() {
	// システム参照の取得
	dxCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();

	///*-----------------------------------------------------------------------*///
	///								カメラの初期化									///
	///*-----------------------------------------------------------------------*///
	cameraController_ = CameraController::GetInstance();
	// 座標を指定して初期化
	Vector3 initialPosition = { 0.0f, 0.0f, -10.0f };
	cameraController_->Initialize(dxCommon_, initialPosition);
	cameraController_->SetActiveCamera("normal");
	// ゲームオブジェクト初期化
	InitializeGameObjects();
	//ポストエフェクトの初期設定
	ConfigureOffscreenEffects();
}

void TitleScene::InitializeGameObjects() {
	///*-----------------------------------------------------------------------*///
	///								フォント										///
	///*-----------------------------------------------------------------------*///

	Vector3 titleRogoPos = { 0.0f,0.8f,-1.74f };
	titleRogo_ = std::make_unique<ModelFont>();
	titleRogo_->Initialize(dxCommon_, "titleFont", titleRogoPos);

	Vector3 pressAPos = { 0.0f,-2.04f,4.5f };
	pressA_ = std::make_unique<ModelFont>();
	pressA_->Initialize(dxCommon_, "pressAFont", pressAPos);

	///*-----------------------------------------------------------------------*///
	///									ライト									///
	///*-----------------------------------------------------------------------*///
}

void TitleScene::Update() {
	// カメラ更新
	cameraController_->Update();

	// ゲームオブジェクト更新
	UpdateGameObjects();

	// ゲームシーンに移動
	if (!TransitionManager::GetInstance()->IsTransitioning() &&
		Input::GetInstance()->IsKeyTrigger(DIK_SPACE) ||
		Input::GetInstance()->IsGamePadButtonTrigger(Input::GamePadButton::A)) {
		// フェードを使った遷移
		SceneTransitionHelper::FadeToScene("GameScene", 1.0f);
		return; // 以降の処理をスキップ
	}

}

void TitleScene::UpdateGameObjects() {
	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	// 球体の更新
	titleRogo_->Update(viewProjectionMatrix);
	pressA_->Update(viewProjectionMatrix);

}

void TitleScene::DrawOffscreen() {

	///
	///3Dゲームオブジェクトの描画（オフスクリーンに描画）
	/// 
	// 球体の描画
	titleRogo_->Draw();
	pressA_->Draw();

	///
	/// パーティクル・スプライトの描画（オフスクリーンに描画）
	/// 

}

void TitleScene::DrawBackBuffer() {
	///
	/// 3Dゲームオブジェクトの描画（オフスクリーンの外に描画）
	///


	///
	/// パーティクル・スプライトの描画（オフスクリーンの外に描画）
	/// 

}

void TitleScene::ImGui() {
#ifdef USEIMGUI

	ImGui::Text("Debug Scene");
	ImGui::Separator();

	ImGui::Spacing();
	ImGui::Text("titleRogo");
	titleRogo_->ImGui();
	pressA_->ImGui();
#endif
}

void TitleScene::Finalize() {
}