#include "DemoScene.h"
#include "Managers/ImGui/ImGuiManager.h" 

///使用するフィールド
#include "GravityField.h"
#include "AccelerationField.h"


DemoScene::DemoScene()
	: BaseScene("DemoScene") // シーン名を設定
	, cameraController_(nullptr)
	, particleSystem_(nullptr)
	, directXCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, debugDrawLineSystem_(nullptr)
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


	//auto* depthFogEffect = offscreenRenderer_->GetDepthFogEffect();
	//if (depthFogEffect) {
	//	depthFogEffect->SetEnabled(true);
	//	depthFogEffect->SetFogDistance(0.2f, 40.0f); // 深度フォグの距離を設定
	//}
	//auto* depthOfFieldEffect = offscreenRenderer_->GetDepthOfFieldEffect();
	//if (depthOfFieldEffect) {
	//	depthOfFieldEffect->SetEnabled(true);
	//}
	auto* vignetteEffect = offscreenRenderer_->GetVignetteEffect();
	if (vignetteEffect) {
		vignetteEffect->SetEnabled(true);
	}

}

void DemoScene::Initialize() {
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();
	debugDrawLineSystem_ = Engine::GetInstance()->GetDebugDrawManager();

	///*-----------------------------------------------------------------------*///
	///								カメラの初期化									///
	///*-----------------------------------------------------------------------*///
	cameraController_ = CameraController::GetInstance();
	// 座標と回転を指定して初期化
	Vector3 initialPosition = { 0.0f, 6.8f, -18.0f };
	Vector3 initialRotation = { 0.4f, 0.0f, 0.0f };
	cameraController_->Initialize(directXCommon_, initialPosition, initialRotation);
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
	sphere_->Initialize(directXCommon_, "sphere", "monsterBall");
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
	plane_->Initialize(directXCommon_, "plane", "uvChecker");
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
	modelMultiMesh_->Initialize(directXCommon_, "model_MultiMesh");
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
	modelMultiMaterial_->Initialize(directXCommon_, "model_MultiMaterial");
	modelMultiMaterial_->SetTransform(transformMultiMaterial);

	///*-----------------------------------------------------------------------*///
	///								グリッド線									///
	///*-----------------------------------------------------------------------*///

	// グリッド
	gridLine_ = std::make_unique<GridLine>();
	// 100m、1m間隔、10mごとに黒
	gridLine_->Initialize(directXCommon_,
		GridLineType::XZ,			// グリッドタイプ：XZ平面
		100.0f,						// サイズ
		1.0f,						// 間隔
		10.0f,						// 主要線間隔
		{ 0.5f, 0.5f, 0.5f, 1.0f },	// 通常線：灰色
		{ 0.0f, 0.0f, 0.0f, 1.0f }	// 主要線：黒
	);
	gridLine_->SetName("Main Grid");

	///*-----------------------------------------------------------------------*///
	///					パーティクルシステムの初期化							///
	///*-----------------------------------------------------------------------*///
#pragma region パーティクル
	// ParticleSystemシングルトンを取得
	particleSystem_ = ParticleSystem::GetInstance();
	particleSystem_->Initialize(directXCommon_);

	///パーティクルグループを作成

	//円形パーティクル
	particleSystem_->CreateGroup(
		"CircleParticles",	// グループ名
		"plane",			// モデル
		400,				// 最大パーティクル数
		"circle",			// テクスチャ
		true				// ビルボードON
	);

	//四角形パーティクル
	particleSystem_->CreateGroup(
		"SquareParticles",	// グループ名
		"plane",			// モデル
		100,				// 最大パーティクル数
		"uvChecker",		// テクスチャ
		true				// ビルボードON
	);

	///エミッター作成

	//中央
	ParticleEmitter* centerEmitter = particleSystem_->CreateEmitter(
		"CenterEmitter",	// エミッター名
		"CircleParticles"	// ターゲットグループ名
	);
	if (centerEmitter) {
		centerEmitter->GetTransform().SetPosition({ 0.0f, 0.0f, 0.0f });
		centerEmitter->SetEmitCount(3);
		centerEmitter->SetFrequency(0.2f);
		centerEmitter->SetEmitEnabled(true);
		centerEmitter->SetParticleLifeTimeRange(2.0f, 4.0f);
		centerEmitter->SetParticleVelocityRange(1.5f);
		// AABB発生範囲を設定
		centerEmitter->SetSpawnAreaSize({ 0.5f, 0.5f, 0.5f });	// 1x1x1の範囲
		// デバッグ表示を有効化
		centerEmitter->SetShowDebugAABB(true);
	}

	//左側
	ParticleEmitter* leftEmitter = particleSystem_->CreateEmitter(
		"LeftEmitter",		// エミッター名
		"CircleParticles"	// ターゲットグループ名
	);
	if (leftEmitter) {
		leftEmitter->GetTransform().SetPosition({ -5.0f, 2.0f, 0.0f });
		leftEmitter->SetEmitCount(5);
		leftEmitter->SetFrequency(0.5f);
		leftEmitter->SetEmitEnabled(true);
		leftEmitter->SetParticleLifeTimeRange(1.0f, 2.5f);
		leftEmitter->SetParticleVelocityRange(2.0f);
		// AABB発生範囲を設定（大きめ）
		leftEmitter->SetSpawnAreaSize({ 1.0f, 1.0f, 1.0f });	// 2x2x2の範囲
		// デバッグ表示を有効化
		leftEmitter->SetShowDebugAABB(true);
	}

	//右側
	ParticleEmitter* rightEmitter = particleSystem_->CreateEmitter(
		"RightEmitter",		// エミッター名
		"SquareParticles"	// ターゲットグループ名
	);
	if (rightEmitter) {
		rightEmitter->GetTransform().SetPosition({ 5.0f, 2.0f, 0.0f });
		rightEmitter->SetEmitCount(2);
		rightEmitter->SetFrequency(0.3f);
		rightEmitter->SetEmitEnabled(true);
		rightEmitter->SetParticleLifeTimeRange(1.5f, 3.0f);
		rightEmitter->SetParticleVelocityRange(1.0f);
		// AABB発生範囲を設定(Y方向に長め設定)
		AABB customArea;
		customArea.min = { -0.3f, -0.3f, -0.3f };
		customArea.max = { 0.3f, 2.0f, 0.3f };
		rightEmitter->SetSpawnArea(customArea);
		// デバッグ表示を有効化
		rightEmitter->SetShowDebugAABB(true);
		rightEmitter->SetDebugAABBColor({ 0.0f, 0.5f, 1.0f, 1.0f });	// 水色
	}

	///*-----------------------------------------------------------------------*///
	///								加速度フィールド							///
	///*-----------------------------------------------------------------------*///

	// 上昇フィールド（中央）
	auto* upwardField = particleSystem_->CreateField<AccelerationField>("UpwardField");
	if (upwardField) {
		upwardField->GetTransform().SetPosition({ 0.0f, 1.0f, 0.0f });
		upwardField->SetAcceleration({ 0.0f, 2.0f, 0.0f });  // 上向きの加速度
		upwardField->SetAreaSize({ 2.0f, 2.0f, 2.0f });  // 4x4x4の範囲
		upwardField->SetEnabled(true);
		upwardField->SetShowDebugVisualization(true);
	}

	// 渦フィールド（左側）
	auto* vortexField = particleSystem_->CreateField<AccelerationField>("VortexField");
	if (vortexField) {
		vortexField->GetTransform().SetPosition({ -5.0f, 3.0f, 0.0f });
		vortexField->SetAcceleration({ 1.0f, 0.5f, 0.0f });  // 右上向きの加速度
		vortexField->SetAreaSize({ 1.5f, 1.5f, 1.5f });  // 3x3x3の範囲
		vortexField->SetEnabled(true);
		vortexField->SetShowDebugVisualization(true);
	}

	///*-----------------------------------------------------------------------*///
	///								重力フィールド									///
	///*-----------------------------------------------------------------------*///

	// 重力フィールド（右側）
	auto* gravityField = particleSystem_->CreateField<GravityField>("GravityField");
	if (gravityField) {
		gravityField->GetTransform().SetPosition({ 5.0f, 3.0f, 0.0f });
		gravityField->SetGravityStrength(8.0f);		// 重力の強さ
		gravityField->SetEffectRadius(4.0f);		// 効果範囲（球体）
		gravityField->SetDeleteRadius(0.5f);		// 削除範囲
		gravityField->SetEnabled(true);
		gravityField->SetShowDebugVisualization(true);
		gravityField->SetDebugColor({ 1.0f, 0.0f, 1.0f, 1.0f });  // マゼンタ
	}

#pragma endregion

	///*-----------------------------------------------------------------------*///
	///								矩形Sprite									///
	///*-----------------------------------------------------------------------*///
	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(directXCommon_, "uvChecker", { 50, 50 }, { 100, 100 });

	///*-----------------------------------------------------------------------*///
	///									ライト									///
	///*-----------------------------------------------------------------------*///
	directionalLight_.Initialize(directXCommon_, Light::Type::DIRECTIONAL);
}

void DemoScene::Update() {
	// カメラ更新
	cameraController_->Update();

	// ゲームオブジェクト更新
	UpdateGameObjects();
}

void DemoScene::UpdateGameObjects() {


	// 球体を回転させる
	sphere_->AddRotation({ 0.0f, 0.015f, 0.0f });

	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	viewProjectionMatrixSprite = cameraController_->GetViewProjectionMatrixSprite();

	// 球体の更新
	sphere_->Update(viewProjectionMatrix);
	// スプライトの更新
	sprite_->Update(viewProjectionMatrixSprite);
	// 平面の更新
	plane_->Update(viewProjectionMatrix);

	//マルチメッシュモデルの更新
	modelMultiMesh_->Update(viewProjectionMatrix);
	//マルチマテリアルモデルの更新
	modelMultiMaterial_->Update(viewProjectionMatrix);

	// パーティクルシステムの更新（全グループ・全エミッター）
	particleSystem_->Update(viewProjectionMatrix, 1.0f / 60.0f);
}

void DemoScene::DrawOffscreen() {

	///
	/// グリッド線をLineSystemに追加する(実際に描画しない)
	/// 
	gridLine_->Draw();

	///
	///3Dゲームオブジェクトの描画（オフスクリーンに描画）
	/// 
	// 球体の描画
	sphere_->Draw(directionalLight_);
	//平面の描画
	plane_->Draw(directionalLight_);
	//マルチメッシュモデルの描画
	modelMultiMesh_->Draw(directionalLight_);
	//マルチマテリアルモデルの描画
	modelMultiMaterial_->Draw(directionalLight_);

	///
	/// パーティクル・スプライトの描画（オフスクリーンに描画）
	/// 
	// パーティクルシステムの描画（全グループ）
	particleSystem_->Draw(directionalLight_);

	///
	/// Line描画の一括実行
	///

	debugDrawLineSystem_->Draw(viewProjectionMatrix);

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
	ImGui::Text("Particle System");
	// パーティクルシステム（全グループと全エミッターを表示）
	particleSystem_->ImGui();

	ImGui::Spacing();
	ImGui::Text("Grid Line");
	gridLine_->ImGui();

	ImGui::Spacing();
	// ライトのImGui
	ImGui::Text("Lighting");
	directionalLight_.ImGui("DirectionalLight");


#endif
}

void DemoScene::Finalize() {
	// unique_ptrで自動的に解放される
	// シーン終了時にパーティクルシステムをクリア
	if (particleSystem_) {
		particleSystem_->Clear();
	}
}