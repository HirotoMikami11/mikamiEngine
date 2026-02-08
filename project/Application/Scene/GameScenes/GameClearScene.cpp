#include "GameClearScene.h"
#include "ImGui/ImGuiManager.h" 
#include "Transition/SceneTransitionHelper.h" 
#include <numbers> 

GameClearScene::GameClearScene()
	: BaseScene("GameClearScene") // シーン名を設定
	, cameraController_(nullptr)
	, dxCommon_(nullptr)
	, particleSystem_(nullptr)
	, particleEditor_(nullptr)
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
	auto* depthFogEffect = offscreenRenderer_->GetDepthFogEffect();
	if (depthFogEffect) {
		depthFogEffect->SetEnabled(true);
		depthFogEffect->SetFogDistance(0.1f, 110.0f); // 深度フォグの距離を設定
		depthFogEffect->SetFogColor({ 0.055f,0.0f,0.0f,1.0f });

	}


	//被写界深度
	auto* depthOfFieldEffect = offscreenRenderer_->GetDepthOfFieldEffect();
	if (depthOfFieldEffect) {
		depthOfFieldEffect->SetEnabled(true);
		depthOfFieldEffect->SetFocusDistance(6.0f);
		depthOfFieldEffect->SetFocusRange(10.0f);
	}

	//二値化
	auto* BinarizationEffect = offscreenRenderer_->GetBinarizationEffect();
	if (BinarizationEffect) {
		BinarizationEffect->SetEnabled(true);
		BinarizationEffect->ApplyPreset(BinarizationPostEffect::EffectPreset::BLACK_WHITE);
		BinarizationEffect->SetThreshold(0.035f);
	}


	// ビネット
	auto* vignetteEffect = offscreenRenderer_->GetVignetteEffect();
	if (vignetteEffect) {
		vignetteEffect->SetEnabled(true);
		vignetteEffect->ApplyPreset(VignettePostEffect::EffectPreset::INTENSE);
		vignetteEffect->SetVignetteStrength(0.505f);
	}
}

void GameClearScene::Initialize() {
	// システム参照の取得
	dxCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();

	// ParticleSystemシングルトンを取得
	particleSystem_ = ParticleSystem::GetInstance();
	particleSystem_->Initialize(dxCommon_);

	// ParticleEditorシングルトンを取得
	particleEditor_ = ParticleEditor::GetInstance();
	particleEditor_->Initialize(dxCommon_);
	///*-----------------------------------------------------------------------*///
	///								カメラの初期化									///
	///*-----------------------------------------------------------------------*///
	cameraController_ = CameraController::GetInstance();
	// 座標と回転を指定して初期化
	Vector3 initialPosition = { 0.0f, 4.0f, -37.5f };
	Vector3 initialRotation = { 0.0f, 0.0f, 0.0f };
	cameraController_->Initialize(dxCommon_, initialPosition, initialRotation);
	cameraController_->SetActiveCamera("normal");

	// ゲームオブジェクト初期化
	InitializeGameObjects();
	//ポストエフェクトの初期設定
	ConfigureOffscreenEffects();

	//BGM
	BGMHandle_ = AudioManager::GetInstance()->Play("ClearBGM", true, 0.3f);
}

void GameClearScene::InitializeGameObjects() {
	///*-----------------------------------------------------------------------*///
	///								フォント										///
	///*-----------------------------------------------------------------------*///

	Vector3 clearPos = { 0.0f,4.64f,-27.9f };
	clearFont_ = std::make_unique<ModelFont>();
	clearFont_->Initialize(dxCommon_, "clearFont", clearPos);
	Vector3 pressAPos = { 0.0f, 2.74f, -25.4f };
	pressA_ = std::make_unique<ModelFont>();
	pressA_->Initialize(dxCommon_, "pressAFont", pressAPos);

	///地面
	field_ = std::make_unique<GameField>();
	field_->Initialize(dxCommon_);

	///宝箱
	treasureBox_ = std::make_unique<TreasureBox>();
	treasureBox_->Initialize(dxCommon_);

	///*-----------------------------------------------------------------------*///
	///								ライト									///
	///*-----------------------------------------------------------------------*///

	// シーン内で平行光源を取得して編集
	DirectionalLight& dirLight = LightManager::GetInstance()->GetDirectionalLight();
	dirLight.SetIntensity(0.35f);

}

void GameClearScene::Update() {
	// カメラ更新
	cameraController_->Update();

	// ゲームオブジェクト更新
	UpdateGameObjects();

	// パーティクルシステムの更新
	particleSystem_->Update(viewProjectionMatrix);

	// タイトルシーンに移動
	if (!TransitionManager::GetInstance()->IsTransitioning() &&
		Input::GetInstance()->IsKeyTrigger(DIK_SPACE) ||
		Input::GetInstance()->IsGamePadButtonTrigger(Input::GamePadButton::A)) {
		if (!AudioManager::GetInstance()->IsPlayingByTag("PressA"))
		{
			//押したおと
			AudioManager::GetInstance()->Play("PressA", false, 0.5f);
		}
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
	field_->Update(viewProjectionMatrix);
	treasureBox_->Update(viewProjectionMatrix);
}

void GameClearScene::DrawOffscreen() {

	///
	///3Dゲームオブジェクトの描画（オフスクリーンに描画）
	/// 
	// フォントの描画

	field_->Draw();
	treasureBox_->Draw();


	///
	/// パーティクル・スプライトの描画（オフスクリーンに描画）
	/// 
	// パーティクルシステムの描画（全グループ）
	particleSystem_->Draw();
}

void GameClearScene::DrawBackBuffer() {
	///
	/// 3Dゲームオブジェクトの描画（オフスクリーンの外に描画）
	///
	clearFont_->Draw();
	pressA_->Draw();

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
	ImGui::Spacing();
	treasureBox_->ImGui();

#endif
}

void GameClearScene::Finalize() {

	// シーン終了時にパーティクルシステムをクリア
	if (particleSystem_) {
		particleSystem_->Clear();
	}
	// シーン終了時にパーティクルインスタンスを全削除
	if (particleEditor_) {
		particleEditor_->DestroyAllInstance();
	}

	// BGM停止
	AudioManager::GetInstance()->Stop(BGMHandle_);
}