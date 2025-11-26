#include "DemoScene.h"
#include "ImGui/ImGuiManager.h" 
#include "GameTimer.h"
#include <numbers> 
///使用するフィールド
#include "GravityField.h"
#include "AccelerationField.h"
#include "ParticleEditor.h"


DemoScene::DemoScene()
	: BaseScene("DemoScene") // シーン名を設定
	, cameraController_(nullptr)
	, particleSystem_(nullptr)
	, particleEditor_(nullptr)
	, dxCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, viewProjectionMatrix{ MakeIdentity4x4() }
	, viewProjectionMatrixSprite{ MakeIdentity4x4() }
{
}

DemoScene::~DemoScene() = default;

void DemoScene::ConfigureOffscreenEffects()
{
	/// オフスクリーンレンダラーのエフェクト設定

	// 全てのエフェクトを無効化
	offscreenRenderer_->DisableAllEffects();

	auto* vignetteEffect = offscreenRenderer_->GetVignetteEffect();
	if (vignetteEffect) {
		vignetteEffect->SetEnabled(true);
	}
}

void DemoScene::Initialize() {
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

void DemoScene::InitializeGameObjects() {

	///*-----------------------------------------------------------------------*///
	///									球体										///
	///*-----------------------------------------------------------------------*///
	Vector3Transform transformSphere{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	sphere_ = std::make_unique<Sphere>();
	sphere_->Initialize(dxCommon_, "sphere", "monsterBall");
	sphere_->SetTransform(transformSphere);

	///*-----------------------------------------------------------------------*///
	///									平面									///
	///*-----------------------------------------------------------------------*///
	Vector3Transform transformPlane{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{3.0f, 1.0f, -8.0f}
	};

	plane_ = std::make_unique<Plane>();
	plane_->Initialize(dxCommon_, "plane", "uvChecker");
	plane_->SetTransform(transformPlane);

	///*-----------------------------------------------------------------------*///
	///								MultiMesh									///
	///*-----------------------------------------------------------------------*///
	Vector3Transform transformMultiMesh{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 3.0f, 0.0f},
		{-5.31f, -0.3f, 3.7f}
	};

	modelMultiMesh_ = std::make_unique<Model3D>();
	modelMultiMesh_->Initialize(dxCommon_, "model_MultiMesh");
	modelMultiMesh_->SetTransform(transformMultiMesh);

	///*-----------------------------------------------------------------------*///
	///								MultiMaterial								///
	///*-----------------------------------------------------------------------*///
	Vector3Transform transformMultiMaterial{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 3.0f, 0.0f},
		{2.23f, -0.3f, 3.7f}
	};

	modelMultiMaterial_ = std::make_unique<Model3D>();
	modelMultiMaterial_->Initialize(dxCommon_, "model_MultiMaterial");
	modelMultiMaterial_->SetTransform(transformMultiMaterial);

	///*-----------------------------------------------------------------------*///
	///			パーティクルシステム - エディタ										///
	///*-----------------------------------------------------------------------*///
#pragma region パーティクル
	// ParticleSystemシングルトンを取得
	particleSystem_ = ParticleSystem::GetInstance();
	particleSystem_->Initialize(dxCommon_);

	// ParticleEditorシングルトンを取得
	particleEditor_ = ParticleEditor::GetInstance();
	particleEditor_->Initialize(dxCommon_);

	// 中央エフェクト（上向き発射 + 上昇フィールド）
	particleEditor_->CreateInstance("CenterEffect", "Center");

	// 左側エフェクト（ランダム速度 + 渦フィールド）
	particleEditor_->CreateInstance("LeftEffect", "Left");

	// 右側エフェクト（左下発射 + 重力フィールド）
	particleEditor_->CreateInstance("RightEffect", "Right");

	//砂煙パーティクル
	particleEditor_->CreateInstance("WalkSmokeEffect", "Smoke1");

#pragma endregion

	///*-----------------------------------------------------------------------*///
	///								矩形Sprite									///
	///*-----------------------------------------------------------------------*///
	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(dxCommon_, "uvChecker", { 50, 50 }, { 100, 100 });

}

void DemoScene::Update() {
	// カメラ更新
	cameraController_->Update();

	// ゲームオブジェクト更新
	UpdateGameObjects();
}

void DemoScene::UpdateGameObjects() {
	//GameTimerからゲーム内デルタタイムを取得
	GameTimer& gameTimer = GameTimer::GetInstance();
	float gameDeltaTime = gameTimer.GetDeltaTime();

	//球体を回転させる
	float rotationSpeed = DegToRad(30);
	sphere_->AddRotation({ 0.0f, rotationSpeed, 0.0f });

	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	viewProjectionMatrixSprite = cameraController_->GetViewProjectionMatrixSprite();

	// 球体の更新
	sphere_->Update(viewProjectionMatrix);
	// スプライトの更新
	sprite_->Update(viewProjectionMatrixSprite);

	//平面の移動
	Input* input = Input::GetInstance();
	Vector3 planePosition = plane_->GetPosition();
	float moveSpeed = 1.5f;

	if (input->IsKeyDown(DIK_A)) {
		planePosition.x -= moveSpeed * gameDeltaTime;  // 左移動
	}
	if (input->IsKeyDown(DIK_D)) {
		planePosition.x += moveSpeed * gameDeltaTime;  // 右移動
	}

	//// 座標を変更したい場合は以下のようにアクセス可能
	//auto* centerInstance = particleEditor_->GetInstance("Center");
	//centerInstance->GetEmitter("CenterEmitter")->GetTransform().SetPosition(sphere_->GetPosition());

	plane_->SetPosition(planePosition);
	plane_->Update(viewProjectionMatrix);

	// マルチメッシュモデルの更新
	modelMultiMesh_->Update(viewProjectionMatrix);
	// マルチマテリアルモデルの更新
	modelMultiMaterial_->Update(viewProjectionMatrix);

	// パーティクルシステムの更新
	particleSystem_->Update(viewProjectionMatrix);
}

void DemoScene::DrawOffscreen() {

	///
	///3Dゲームオブジェクトの描画（オフスクリーンに描画）
	/// 
	// 球体の描画
	sphere_->Draw();
	//平面の描画
	plane_->Draw();
	//マルチメッシュモデルの描画
	modelMultiMesh_->Draw();
	//マルチマテリアルモデルの描画
	modelMultiMaterial_->Draw();

	///
	/// パーティクル・スプライトの描画（オフスクリーンに描画）
	/// 
	// パーティクルシステムの描画（全グループ）
	particleSystem_->Draw();

}

void DemoScene::DrawBackBuffer() {
	///
	/// 3Dゲームオブジェクトの描画（オフスクリーンの外に描画）
	///


	///
	/// パーティクル・スプライトの描画（オフスクリーンの外に描画）
	/// 
	sprite_->Draw();
}

void DemoScene::ImGui() {

#ifdef USEIMGUI	//debug,development時にのみ有効なマクロ

	// 球体のImGui
	ImGui::Text("Sphere");
	sphere_->ImGui();

	ImGui::Spacing();
	// スプライトのImGui
	ImGui::Text("Sprite");
	sprite_->ImGui();

	ImGui::Spacing();
	ImGui::Text("plane");
	plane_->ImGui();


	ImGui::Spacing();
	// マルチメッシュモデルのImGui
	ImGui::Text("ModelMultiMesh");
	modelMultiMesh_->ImGui();

	ImGui::Spacing();
	// マルチマテリアルモデルのImGui
	ImGui::Text("ModelMultiMaterial");
	modelMultiMaterial_->ImGui();

	ImGui::Spacing();
	// パーティクルエディタ（統合UI）
	ImGui::Text("Particle Editor");
	particleEditor_->ImGui();

#endif
}

void DemoScene::Finalize() {
	// unique_ptrで自動的に解放される
	// シーン終了時にパーティクルシステムをクリア
	if (particleSystem_) {
		particleSystem_->Clear();
	}
	// シーン終了時にパーティクルインスタンスを全削除
	if (particleEditor_) {
		particleEditor_->DestroyAllInstance();
	}
}