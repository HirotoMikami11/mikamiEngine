#include "TitleScene.h"
#include "ImGui/ImGuiManager.h" 
#include "Transition/SceneTransitionHelper.h" 
#include <numbers> 

TitleScene::TitleScene()
	: BaseScene("TitleScene") // シーン名を設定
	, cameraController_(nullptr)
	, dxCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, particleSystem_(nullptr)
	, particleEditor_(nullptr)
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
	auto* depthFogEffect = offscreenRenderer_->GetDepthFogEffect();
	if (depthFogEffect) {
		depthFogEffect->SetEnabled(true);
		depthFogEffect->SetFogDistance(0.1f, 76.0f); // 深度フォグの距離を設定
		depthFogEffect->SetFogColor({ 0.0f,0.0f,0.0f,1.0f });

	}
}

void TitleScene::Initialize() {
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
	// 座標を指定して初期化
	Vector3 initialPosition = { 0.0f, 4.6f, -50.0f };
	cameraController_->Initialize(dxCommon_, initialPosition);
	cameraController_->SetActiveCamera("normal");

	cameraMoveSpeed_ = 0.1f;

	// ゲームオブジェクト初期化
	InitializeGameObjects();
	//ポストエフェクトの初期設定
	ConfigureOffscreenEffects();

	//BGM
	BGMHandle_ = AudioManager::GetInstance()->Play("TitleBGM", true, 0.3f);

}

void TitleScene::InitializeGameObjects() {
	///*-----------------------------------------------------------------------*///
	///								モデル										///
	///*-----------------------------------------------------------------------*///

	Vector3 titleRogoPos = { 0.0f,5.36f,cameraController_->GetCamera("normal")->GetPosition().z + 10.0f };
	titleRogo_ = std::make_unique<ModelFont>();
	titleRogo_->Initialize(dxCommon_, "titleFont", titleRogoPos);

	Vector3 pressAPos = { 0.0f,3.410f,cameraController_->GetCamera("normal")->GetPosition().z + (50.0f - 36.15f) };
	pressA_ = std::make_unique<ModelFont>();
	pressA_->Initialize(dxCommon_, "pressAFont", pressAPos);



	///地面
	titleField_ = std::make_unique<TitleField>();
	titleField_->Initialize(dxCommon_);


	//フィールドパーティクル
	particleEditor_->CreateInstance("FieldEffect", "Field_circle");

	///*-----------------------------------------------------------------------*///
	///								ライト									///
	///*-----------------------------------------------------------------------*///

	// シーン内で平行光源を取得して編集
	DirectionalLight& dirLight = LightManager::GetInstance()->GetDirectionalLight();
	dirLight.SetDirection(Vector3{ 0.0f,1.0f,0.0f });
	dirLight.SetIntensity(1.0f);
}

void TitleScene::Update() {
	// カメラ更新
	cameraController_->Update();
	cameraController_->GetCamera("normal")->SetPosition(
		{ 0.0f,4.6f,
		cameraController_->GetCamera("normal")->GetPosition().z + cameraMoveSpeed_ }
	);

	// ゲームオブジェクト更新
	UpdateGameObjects();

	// パーティクルシステムの更新
	particleSystem_->Update(viewProjectionMatrix);
	// ゲームシーンに移動
	if (!TransitionManager::GetInstance()->IsTransitioning() &&
		Input::GetInstance()->IsKeyTrigger(DIK_SPACE) ||
		Input::GetInstance()->IsGamePadButtonTrigger(Input::GamePadButton::A)) {
		//押したおと
		AudioManager::GetInstance()->Play("PressA", false, 0.5f);
		// フェードを使った遷移
		SceneTransitionHelper::FadeToScene("GameScene", 1.0f);
		return; // 以降の処理をスキップ
	}

}

void TitleScene::UpdateGameObjects() {
	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();


	Vector3 titleRogoPos = { 0.0f,5.36f,cameraController_->GetCamera("normal")->GetPosition().z + 10.0f };
	titleRogo_->SetPosition(titleRogoPos);

	Vector3 pressAPos = { 0.0f,3.410f,cameraController_->GetCamera("normal")->GetPosition().z + (50.0f - 36.15f) };
	pressA_->SetPosition(pressAPos);


	// 球体の更新
	titleRogo_->Update(viewProjectionMatrix);
	pressA_->Update(viewProjectionMatrix);

	// カメラのZ座標を取得してTitleFieldに渡す
	float cameraZ = cameraController_->GetCamera("normal")->GetPosition().z;
	titleField_->Update(viewProjectionMatrix, cameraZ);


}

void TitleScene::DrawOffscreen() {

	///
	///3Dゲームオブジェクトの描画（オフスクリーンに描画）
	/// 
	// 球体の描画
	titleRogo_->Draw();
	pressA_->Draw();

	titleField_->Draw();

	///
	/// パーティクル・スプライトの描画（オフスクリーンに描画）
	/// 
		// パーティクルシステムの描画（全グループ）
	particleSystem_->Draw();
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

	titleField_->ImGui();
#endif
}

void TitleScene::Finalize() {
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