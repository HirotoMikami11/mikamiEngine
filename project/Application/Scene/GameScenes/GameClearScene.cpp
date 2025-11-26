#include "GameClearScene.h"
#include "ImGui/ImGuiManager.h" 
#include "Transition/SceneTransitionHelper.h" 
#include <numbers> 

GameClearScene::GameClearScene()
	: BaseScene("GameClearScene") // シーン名を設定
	, cameraController_(nullptr)
	, dxCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, viewProjectionMatrix{ MakeIdentity4x4() }
{
}

GameClearScene::~GameClearScene() = default;

void GameClearScene::ConfigureOffscreenEffects()
{
	/// オフスクリーンレンダラーのエフェクト設定

	// 全てのエフェクトを無効化
	offscreenRenderer_->DisableAllEffects();
	// 必要に応じてここでエフェクトを有効化
}

void GameClearScene::Initialize() {
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

void GameClearScene::InitializeGameObjects() {
	///*-----------------------------------------------------------------------*///
	///								フォント										///
	///*-----------------------------------------------------------------------*///

	Vector3 clearPos = { 0.0f,0.9f,0.0f };
	clearFont_ = std::make_unique<ModelFont>();
	clearFont_->Initialize(dxCommon_, "clearFont", clearPos);
	Vector3 pressAPos = { 0.0f, -2.04f, 4.5f };
	pressA_ = std::make_unique<ModelFont>();
	pressA_->Initialize(dxCommon_, "pressAFont", pressAPos);

}

void GameClearScene::Update() {
	// カメラ更新
	cameraController_->Update();

	// ゲームオブジェクト更新
	UpdateGameObjects();

	// タイトルシーンに移動
	if (!TransitionManager::GetInstance()->IsTransitioning() &&
		Input::GetInstance()->IsKeyTrigger(DIK_SPACE) ||
		Input::GetInstance()->IsGamePadButtonTrigger(Input::GamePadButton::A)) {
		// フェードを使った遷移
		SceneTransitionHelper::FadeToScene("TitleScene", 1.0f);
		return; // 以降の処理をスキップ
	}
}

void GameClearScene::UpdateGameObjects() {
	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	// 球体の更新
	clearFont_->Update(viewProjectionMatrix);
	pressA_->Update(viewProjectionMatrix);
}

void GameClearScene::DrawOffscreen() {

	///
	///3Dゲームオブジェクトの描画（オフスクリーンに描画）
	/// 
	// フォントの描画
	clearFont_->Draw();
	pressA_->Draw();

	///
	/// パーティクル・スプライトの描画（オフスクリーンに描画）
	/// 

	///
	/// デバッグ線の一括描画はEngine::EndDrawOffscreen()で自動実行される
	/// このシーンでは呼び出し不要
	///
}

void GameClearScene::DrawBackBuffer() {
	///
	/// 3Dゲームオブジェクトの描画（オフスクリーンの外に描画）
	///


	///
	/// パーティクル・スプライトの描画（オフスクリーンの外に描画）
	/// 

}

void GameClearScene::ImGui() {
#ifdef USEIMGUI

	ImGui::Text("Game Clear Scene");
	ImGui::Separator();

	ImGui::Spacing();
	ImGui::Text("Fonts");
	clearFont_->ImGui();
	pressA_->ImGui();

#endif
}

void GameClearScene::Finalize() {
}