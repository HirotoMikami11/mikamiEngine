#include "DebugScene.h"
#include "ImGui/ImGuiManager.h" 
#include <numbers> 

DebugScene::DebugScene()
	: BaseScene("DebugScene") // シーン名を設定
	, cameraController_(nullptr)
	, dxCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, debugDrawLineSystem_(nullptr)
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
	debugDrawLineSystem_ = Engine::GetInstance()->GetDebugDrawManager();

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
	Vector3Transform transformSphere{
		{1.0f, 1.0f, 1.0f},
		{0.0f, -std::numbers::pi_v<float>*0.5f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	sphere_ = std::make_unique<Sphere>();
	sphere_->Initialize(dxCommon_, "sphere", "monsterBall");
	sphere_->SetTransform(transformSphere);

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


}

void DebugScene::DrawOffscreen() {

	///
	/// グリッド線をLineSystemに追加する(実際に描画しない)
	/// 
	gridLine_->Draw();

	///
	///3Dゲームオブジェクトの描画（オフスクリーンに描画）
	/// 
	// 球体の描画
	sphere_->Draw(directionalLight_);
	// 地面の描画
	terrain_->Draw(directionalLight_);

	///
	/// パーティクル・スプライトの描画（オフスクリーンに描画）
	/// 
	

	///
	/// Line描画の一括実行
	///

	debugDrawLineSystem_->Draw(viewProjectionMatrix);
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

	ImGui::Spacing();
	ImGui::Text("Grid Line");
	gridLine_->ImGui();

	ImGui::Spacing();
	// ライトのImGui
	ImGui::Text("Lighting");
	directionalLight_.ImGui("DirectionalLight");

#endif
}

void DebugScene::Finalize() {
}