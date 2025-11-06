#include "DebugScene.h"
#include "Managers/ImGui/ImGuiManager.h" 


DebugScene::DebugScene()
	: BaseScene("DebugScene") // シーン名を設定
	, cameraController_(nullptr)
	, directXCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, textureManager_(nullptr)
	, particleSystem_(nullptr)
	, viewProjectionMatrix{ MakeIdentity4x4() }
{
}

DebugScene::~DebugScene() = default;


void DebugScene::LoadResources() {
	// リソースの読み込み
	Logger::Log(Logger::GetStream(), "DebugScene: Loading resources...\n");

	// リソースマネージャーの取得
	textureManager_ = TextureManager::GetInstance();

	// テクスチャの事前読み込み（必要に応じて）


	Logger::Log(Logger::GetStream(), "DebugScene: Resources loaded successfully\n");
}

void DebugScene::ConfigureOffscreenEffects()
{
	/// オフスクリーンレンダラーのエフェクト設定

	// 全てのエフェクトを無効化
	offscreenRenderer_->DisableAllEffects();

	// デバッグ用なので基本的にエフェクトは無効のまま
	// 必要に応じてここでエフェクトを有効化
}

void DebugScene::Initialize() {
	// システム参照の取得
	directXCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();

	///*-----------------------------------------------------------------------*///
	///								カメラの初期化									///
	///*-----------------------------------------------------------------------*///
	cameraController_ = CameraController::GetInstance();
	// 座標と回転を指定して初期化
	Vector3 initialPosition = { 0.0f, 6.8f, -18.0f };
	Vector3 initialRotation = { 0.4f, 0.0f, 0.0f };
	cameraController_->Initialize(initialPosition, initialRotation);
	cameraController_->SetActiveCamera("normal");

	// ゲームオブジェクト初期化
	InitializeGameObjects();
	//ポストエフェクトの初期設定
	ConfigureOffscreenEffects();
}

void DebugScene::InitializeGameObjects() {

	///*-----------------------------------------------------------------------*///
	///									平面									///
	///*-----------------------------------------------------------------------*///
	Vector3Transform transformPlane{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 5.0f}
	};

	plane_ = std::make_unique<Plane>();
	plane_->Initialize(directXCommon_, "plane", "uvChecker");
	plane_->SetTransform(transformPlane);

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
	///					パーティクルマネージャーの初期化							///
	///*-----------------------------------------------------------------------*///

	// ParticleSystemシングルトンを取得
	particleSystem_ = ParticleSystem::GetInstance();
	particleSystem_->Initialize(directXCommon_);

	// 【ステップ1】パーティクルグループを作成
	// グループ1: 円形パーティクル（200個まで）
	particleSystem_->CreateGroup(
		"CircleParticles",  // グループ名
		"plane",            // モデル
		200,                // 最大パーティクル数
		"circle",           // テクスチャ
		true                // ビルボードON
	);

	// グループ2: 四角形パーティクル（100個まで）
	particleSystem_->CreateGroup(
		"SquareParticles",  // グループ名
		"plane",            // モデル
		100,                // 最大パーティクル数
		"uvChecker",        // テクスチャ
		true                // ビルボードON
	);

	// 【ステップ2】エミッターを作成

	// エミッター1: 中央（CircleParticlesグループに発生）
	ParticleEmitter* centerEmitter = particleSystem_->CreateEmitter(
		"CenterEmitter",     // エミッター名
		"CircleParticles"    // ターゲットグループ名
	);
	if (centerEmitter) {
		centerEmitter->GetTransform().SetPosition({ 0.0f, 0.0f, 0.0f });
		centerEmitter->SetEmitCount(3);
		centerEmitter->SetFrequency(0.2f);
		centerEmitter->SetEmitEnabled(true);
		centerEmitter->SetParticleLifeTimeRange(2.0f, 4.0f);
		centerEmitter->SetParticleVelocityRange(1.5f);
		centerEmitter->SetParticleSpawnRange(0.3f);
	}

	// エミッター2: 左側（CircleParticlesグループに発生）
	ParticleEmitter* leftEmitter = particleSystem_->CreateEmitter(
		"LeftEmitter",       // エミッター名
		"CircleParticles"    // ターゲットグループ名
	);
	if (leftEmitter) {
		leftEmitter->GetTransform().SetPosition({ -5.0f, 2.0f, 0.0f });
		leftEmitter->SetEmitCount(5);
		leftEmitter->SetFrequency(0.5f);
		leftEmitter->SetEmitEnabled(true);
		leftEmitter->SetParticleLifeTimeRange(1.0f, 2.5f);
		leftEmitter->SetParticleVelocityRange(2.0f);
		leftEmitter->SetParticleSpawnRange(0.5f);
	}

	// エミッター3: 右側（SquareParticlesグループに発生）
	ParticleEmitter* rightEmitter = particleSystem_->CreateEmitter(
		"RightEmitter",      // エミッター名
		"SquareParticles"    // ターゲットグループ名
	);
	if (rightEmitter) {
		rightEmitter->GetTransform().SetPosition({ 5.0f, 2.0f, 0.0f });
		rightEmitter->SetEmitCount(2);
		rightEmitter->SetFrequency(0.3f);
		rightEmitter->SetEmitEnabled(true);
		rightEmitter->SetParticleLifeTimeRange(1.5f, 3.0f);
		rightEmitter->SetParticleVelocityRange(1.0f);
		rightEmitter->SetParticleSpawnRange(0.2f);
	}

	///*-----------------------------------------------------------------------*///
	///									ライト									///
	///*-----------------------------------------------------------------------*///
	directionalLight_.Initialize(directXCommon_, Light::Type::DIRECTIONAL);
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

	// 平面の更新
	plane_->Update(viewProjectionMatrix);

	// グリッド線更新
	gridLine_->Update(viewProjectionMatrix);

	// パーティクルマネージャーの更新（全グループ・全エミッター）
	particleSystem_->Update(viewProjectionMatrix, 1.0f / 60.0f);
}

void DebugScene::DrawOffscreen() {

	///
	/// グリッド線を描画（3D要素）
	/// 
	gridLine_->Draw(viewProjectionMatrix);

	///
	///3Dゲームオブジェクトの描画（オフスクリーンに描画）
	/// 
	// 平面の描画
	plane_->Draw(directionalLight_);


	///
	/// パーティクル・スプライトの描画（オフスクリーンに描画）
	/// 
	// パーティクルマネージャーの描画（全グループ）
	particleSystem_->Draw(directionalLight_);
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
	ImGui::Text("Plane");
	plane_->ImGui();

	ImGui::Spacing();
	ImGui::Text("=== Particle Manager ===");
	// パーティクルマネージャー（全グループと全エミッターを表示）
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

void DebugScene::Finalize() {
	// シーン終了時にパーティクルマネージャーをクリア
	if (particleSystem_) {
		particleSystem_->Clear();
	}
}