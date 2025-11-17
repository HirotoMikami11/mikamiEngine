#include "GameOverScene.h"
#include "ImGui/ImGuiManager.h" 
#include "Transition/SceneTransitionHelper.h" 
#include <numbers> 

GameOverScene::GameOverScene()
	: BaseScene("GameOverScene") // シーン名を設定
	, cameraController_(nullptr)
	, dxCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, debugDrawLineSystem_(nullptr)
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
	debugDrawLineSystem_ = Engine::GetInstance()->GetDebugDrawManager();

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


	///*-----------------------------------------------------------------------*///
	///								グリッド線									///
	///*-----------------------------------------------------------------------*///

	// グリッド
	gridLine_ = std::make_unique<GridLine>();
	// 100m、1m間隔、10mごとに黒
	gridLine_->Initialize(dxCommon_,
		GridLineType::XZ,			// グリッドタイプ：XZ平面
		100.0f,						// サイズ
		1.0f,						// 間隔
		10.0f,						// 主要線間隔
		{ 0.5f, 0.5f, 0.5f, 1.0f },	// 通常線：灰色
		{ 0.0f, 0.0f, 0.0f, 1.0f }	// 主要線：黒
	);
	gridLine_->SetName("Main Grid");

	///*-----------------------------------------------------------------------*///
	///									ライト									///
	///*-----------------------------------------------------------------------*///
	directionalLight_.Initialize(dxCommon_, Light::Type::DIRECTIONAL);
}

void GameOverScene::Update() {
	// カメラ更新
	cameraController_->Update();

	// ゲームオブジェクト更新
	UpdateGameObjects();


	// タイトルシーンに移動
	if (Input::GetInstance()->IsKeyTrigger(DIK_SPACE) ||
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


}

void GameOverScene::DrawOffscreen() {

	///
	/// グリッド線をLineSystemに追加する(実際に描画しない)
	/// 
	gridLine_->Draw();

	///
	///3Dゲームオブジェクトの描画（オフスクリーンに描画）
	/// 
	// 球体の描画
	overFont_->Draw(directionalLight_);

	///
	/// パーティクル・スプライトの描画（オフスクリーンに描画）
	/// 

	///
	/// Line描画の一括実行
	///

	debugDrawLineSystem_->Draw(viewProjectionMatrix);
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

	ImGui::Spacing();
	ImGui::Text("Grid Line");
	gridLine_->ImGui();

	ImGui::Spacing();
	// ライトのImGui
	ImGui::Text("Lighting");
	directionalLight_.ImGui("DirectionalLight");

#endif
}

void GameOverScene::Finalize() {
}