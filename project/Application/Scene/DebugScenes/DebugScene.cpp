#include "DebugScene.h"
#include "ImGui/ImGuiManager.h" 
#include <numbers> 

DebugScene::DebugScene()
	: BaseScene("DebugScene") // シーン名を設定
	, cameraController_(nullptr)
	, dxCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, viewProjectionMatrix{ MakeIdentity4x4() }
{
}

DebugScene::~DebugScene() = default;

void DebugScene::ConfigureOffscreenEffects()
{
	/// オフスクリーンレンダラーのエフェクト設定

	// 全てのエフェクトを無効化
	offscreenRenderer_->DisableAllEffects();
	// 必要に応じてここでエフェクトを有効化
}

void DebugScene::Initialize() {
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

void DebugScene::InitializeGameObjects() {
	///*-----------------------------------------------------------------------*///
	///									球体										///
	///*-----------------------------------------------------------------------*///

	sphere_ = std::make_unique<DebugSphere>();
	sphere_->Initialize(dxCommon_);


	///*-----------------------------------------------------------------------*///
	///								地面									///
	///*-----------------------------------------------------------------------*///
	Vector3Transform transformTerrain{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	terrain_ = std::make_unique<Model3D>();
	terrain_->Initialize(dxCommon_, "model_terrain");
	terrain_->SetTransform(transformTerrain);
	///*-----------------------------------------------------------------------*///
	///								ライト									///
	///*-----------------------------------------------------------------------*///

	// シーン内で平行光源を取得して編集
	DirectionalLight& dirLight = LightManager::GetInstance()->GetDirectionalLight();
	dirLight.SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });	// 黒い光（無効化）

	// ポイントライト: 青い光
	pointLight_ = LightManager::GetInstance()->AddPointLight(
		{ -5.0f, 2.0f, 0.0f },			//座標
		{ 0.3f, 0.3f, 1.0f, 1.0f },		// 青色
		2.0f,							// 強度
		15.0f,							// 影響範囲
		2.0f							// 減衰率
	);

	// スポットライト: 白い光（真上から下向き）
	spotLight_ = LightManager::GetInstance()->AddSpotLight(
		{ 0.0f, 5.0f, 0.0f },			// 位置（上方）
		{ 90.0f, 0.0f, 0.0f },			// 回転（X軸90度=下向き）
		{ 1.0f, 1.0f, 1.0f, 1.0f },		// 白色
		3.0f,							// 強度
		15.0f,							// 最大距離
		2.0f,							// 減衰率
		30.0f,							// スポット角度（度）
		20.0f							// フォールオフ開始角度（度）
	);

}

void DebugScene::Update() {
	// カメラ更新
	cameraController_->Update();

	// ゲームオブジェクト更新
	UpdateGameObjects();
}

void DebugScene::UpdateGameObjects() {
	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	// 球体の更新
	sphere_->Update(viewProjectionMatrix);
	// 地面の更新
	terrain_->Update(viewProjectionMatrix);
	//ポイントライトの座標を地面に合わせる
	if (pointLight_) {
		pointLight_->SetPosition(terrain_->GetPosition());
	}

}

void DebugScene::DrawOffscreen() {

	///
	///3Dゲームオブジェクトの描画（オフスクリーンに描画）
	/// 
	// 球体の描画
	sphere_->Draw();
	// 地面の描画
	terrain_->Draw();

	///
	/// パーティクル・スプライトの描画（オフスクリーンに描画）
	/// 


}

void DebugScene::DrawBackBuffer() {
	///
	/// 3Dゲームオブジェクトの描画（オフスクリーンの外に描画）
	///


	///
	/// パーティクル・スプライトの描画（オフスクリーンの外に描画）
	/// 

}

void DebugScene::ImGui() {
#ifdef USEIMGUI

	ImGui::Text("Debug Scene");
	ImGui::Separator();

	ImGui::Spacing();
	ImGui::Text("Sphere");
	sphere_->ImGui();

	ImGui::Spacing();
	ImGui::Text("Terrain");
	terrain_->ImGui();
#endif
}

void DebugScene::Finalize() {
}