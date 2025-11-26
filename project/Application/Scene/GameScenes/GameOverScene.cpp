#include "GameOverScene.h"
#include "ImGui/ImGuiManager.h" 
#include "Transition/SceneTransitionHelper.h" 
#include <numbers> 

GameOverScene::GameOverScene()
	: BaseScene("GameOverScene") // シーン名を設定
	, cameraController_(nullptr)
	, dxCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, viewProjectionMatrix{ MakeIdentity4x4() }
{
}

GameOverScene::~GameOverScene() = default;

void GameOverScene::ConfigureOffscreenEffects()
{
	/// オフスクリーンレンダラーのエフェクト設定

	// 全てのエフェクトを無効化
	offscreenRenderer_->DisableAllEffects();
	// 必要に応じてここでエフェクトを有効化
		//auto* depthFogEffect = offscreenRenderer_->GetDepthFogEffect();
	//if (depthFogEffect) {
	//	depthFogEffect->SetEnabled(true);
	//	depthFogEffect->SetFogDistance(33.3f, 142.0f); // 深度フォグの距離を設定
	//	depthFogEffect->SetFogColor({0.23f,0.23f,0.23f,1.0f});
	//
	//}
}

void GameOverScene::Initialize() {
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

void GameOverScene::InitializeGameObjects() {
	///*-----------------------------------------------------------------------*///
	///								フォント										///
	///*-----------------------------------------------------------------------*///

	Vector3 overFontPos = { 0.0f,0.9f,0.0f };
	overFont_ = std::make_unique<ModelFont>();
	overFont_->Initialize(dxCommon_, "overFont", overFontPos);

	Vector3 pressAPos = { 0.0f, -2.04f, 4.5f };
	pressA_ = std::make_unique<ModelFont>();
	pressA_->Initialize(dxCommon_, "pressAFont", pressAPos);

}

void GameOverScene::Update() {
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

void GameOverScene::UpdateGameObjects() {
	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	// 球体の更新
	overFont_->Update(viewProjectionMatrix);
	pressA_->Update(viewProjectionMatrix);


}

void GameOverScene::DrawOffscreen() {

	///
	///3Dゲームオブジェクトの描画（オフスクリーンに描画）
	/// 
	// 球体の描画
	overFont_->Draw();
	pressA_->Draw();

	///
	/// パーティクル・スプライトの描画（オフスクリーンに描画）
	/// 
}

void GameOverScene::DrawBackBuffer() {
	///
	/// 3Dゲームオブジェクトの描画（オフスクリーンの外に描画）
	///


	///
	/// パーティクル・スプライトの描画（オフスクリーンの外に描画）
	/// 

}

void GameOverScene::ImGui() {
#ifdef USEIMGUI

	ImGui::Text("Debug Scene");
	ImGui::Separator();

	ImGui::Spacing();
	ImGui::Text("overFont");
	overFont_->ImGui();
	pressA_->ImGui();

#endif
}

void GameOverScene::Finalize() {
}