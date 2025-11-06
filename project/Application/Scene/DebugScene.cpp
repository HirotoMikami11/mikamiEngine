#include "DebugScene.h"
#include "Managers/ImGui/ImGuiManager.h" 


DebugScene::DebugScene()
	: BaseScene("DebugScene") // シーン名を設定
	, cameraController_(nullptr)
	, directXCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, textureManager_(nullptr)
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
	///						パーティクルシステムとエミッター							///
	///*-----------------------------------------------------------------------*///

	// パーティクルシステムの作成（パーティクルの管理・更新・描画）
	particleSystem_ = std::make_unique<Particle>();
	particleSystem_->Initialize(directXCommon_, "plane", 200, "circle");
	particleSystem_->SetName("Particle System");
	particleSystem_->SetBillboard(true);

	// エミッター1: 中央から上に向かって発生するエミッター
	{
		auto emitter = std::make_unique<ParticleEmitter>();
		emitter->Initialize(directXCommon_, particleSystem_.get());
		emitter->SetName("Center Emitter");

		// エミッターの位置
		emitter->GetTransform().SetPosition({ 0.0f, 0.0f, 0.0f });

		// 発生設定
		emitter->SetEmitCount(3);			// 1回で3個
		emitter->SetFrequency(0.2f);		// 0.2秒ごと
		emitter->SetEmitEnabled(true);

		// パーティクル設定
		emitter->SetParticleLifeTimeRange(2.0f, 4.0f);	// 寿命2-4秒
		emitter->SetParticleVelocityRange(1.5f);		// 速度範囲
		emitter->SetParticleSpawnRange(0.3f);			// 生成範囲

		emitters_.push_back(std::move(emitter));
	}

	// エミッター2: 左側から発生するエミッター
	{
		auto emitter = std::make_unique<ParticleEmitter>();
		emitter->Initialize(directXCommon_, particleSystem_.get());
		emitter->SetName("Left Emitter");

		// エミッターの位置
		emitter->GetTransform().SetPosition({ -5.0f, 2.0f, 0.0f });

		// 発生設定
		emitter->SetEmitCount(5);			// 1回で5個
		emitter->SetFrequency(0.5f);		// 0.5秒ごと
		emitter->SetEmitEnabled(true);

		// パーティクル設定
		emitter->SetParticleLifeTimeRange(1.0f, 2.5f);	// 寿命1-2.5秒
		emitter->SetParticleVelocityRange(2.0f);		// 速度範囲
		emitter->SetParticleSpawnRange(0.5f);			// 生成範囲

		emitters_.push_back(std::move(emitter));
	}

	// エミッター3: 右側から発生するエミッター
	{
		auto emitter = std::make_unique<ParticleEmitter>();
		emitter->Initialize(directXCommon_, particleSystem_.get());
		emitter->SetName("Right Emitter");

		// エミッターの位置
		emitter->GetTransform().SetPosition({ 5.0f, 2.0f, 0.0f });

		// 発生設定
		emitter->SetEmitCount(2);			// 1回で2個
		emitter->SetFrequency(0.3f);		// 0.3秒ごと
		emitter->SetEmitEnabled(true);

		// パーティクル設定
		emitter->SetParticleLifeTimeRange(1.5f, 3.0f);	// 寿命1.5-3秒
		emitter->SetParticleVelocityRange(1.0f);		// 速度範囲
		emitter->SetParticleSpawnRange(0.2f);			// 生成範囲

		emitters_.push_back(std::move(emitter));
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

	// エミッターの更新（パーティクルの発生制御）
	for (auto& emitter : emitters_) {
		emitter->Update(1.0f / 60.0f);
	}

	// パーティクルシステムの更新
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
	ImGui::Text("=== Particle System ===");
	// パーティクルシステム
	particleSystem_->ImGui();

	ImGui::Spacing();
	ImGui::Text("=== Emitters ===");
	// エミッター
	for (auto& emitter : emitters_) {
		emitter->ImGui();
	}


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
	// unique_ptrで自動的に解放される
}