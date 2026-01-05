#include "GameScene.h"
#include "ImGui/ImGuiManager.h" 
#include "Transition/SceneTransitionHelper.h" 
#include <numbers> 

GameScene::GameScene()
	: BaseScene("GameScene") // シーン名を設定
	, cameraController_(nullptr)
	, dxCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, particleSystem_(nullptr)
	, particleEditor_(nullptr)
	, viewProjectionMatrix{ MakeIdentity4x4() }
	, viewProjectionMatrixSprite{ MakeIdentity4x4() }
{
}

GameScene::~GameScene() = default;

void GameScene::ConfigureOffscreenEffects()
{
	/// オフスクリーンレンダラーのエフェクト設定

	// 全てのエフェクトを無効化
	offscreenRenderer_->DisableAllEffects();

	auto* depthFogEffect = offscreenRenderer_->GetDepthFogEffect();
	if (depthFogEffect) {
		depthFogEffect->SetEnabled(true);
		depthFogEffect->SetFogDistance(0.1f, 119.0f); // 深度フォグの距離を設定
		depthFogEffect->SetFogColor({ 0.0f,0.0f,0.0f,1.0f });

	}
}

void GameScene::Initialize() {
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
	Vector3 initialPosition = { 0.5f, 49.5f, -75.7f };
	Vector3 initialRotation = { 0.57f, 0.0f, 0.0f };
	cameraController_->Initialize(dxCommon_, initialPosition, initialRotation);
	cameraController_->SetActiveCamera("normal");

	// 衝突マネージャーの生成
	collisionManager_ = std::make_unique<CollisionManager>();
	// 衝突マネージャーの初期化
	collisionManager_->Initialize();

	// ゲームオブジェクト初期化
	InitializeGameObjects();
	//ポストエフェクトの初期設定
	ConfigureOffscreenEffects();

	// 操作説明
	sousa_ = std::make_unique<Sprite>();
	sousa_->Initialize(
		dxCommon_,
		"sousa",
		{640.0f,360.0f},
		{1280.0f,720.0f}
	);
}

void GameScene::InitializeGameObjects() {

	///自機
	player_ = std::make_unique<Player>();
	player_->Initialize(dxCommon_, { 0.0f, 0.5f, 0.0f });


	///地面
	ground_ = std::make_unique<Ground>();
	ground_->Initialize(dxCommon_, { 0.0f,-0.51f,0.0f });

	wall_ = std::make_unique<Wall>();
	wall_->Initialize(dxCommon_);

	torch_ = std::make_unique<Torch>();
	torch_->Initialize(dxCommon_);

	///ボス
	boss_ = std::make_unique<Boss>();
	boss_->Initialize(dxCommon_, { -10.0f,1.5f, 0.0f });
	///*-----------------------------------------------------------------------*///
	///			パーティクル										///
	///*-----------------------------------------------------------------------*///
#pragma region パーティクル

	//フィールドパーティクル
	particleEditor_->CreateInstance("FieldEffect", "Field_circle");


#pragma endregion
	///*-----------------------------------------------------------------------*///
	///								ライト									///
	///*-----------------------------------------------------------------------*///

	// シーン内で平行光源を取得して編集
	DirectionalLight& dirLight = LightManager::GetInstance()->GetDirectionalLight();
	dirLight.SetIntensity(0.35f);
	// 地面ライトの生成
	groundLight_ = std::make_unique<GroundLight>();
	groundLight_->Initialize(LightManager::GetInstance());
}

void GameScene::Update() {

	// GameTimerからデルタタイムを取得
	GameTimer& gameTimer = GameTimer::GetInstance();
	
	//ポーズ中は処理を行わない
	if (!gameTimer.IsPaused()) {
		// カメラ更新
		cameraController_->Update();

		// ゲームオブジェクト更新
		UpdateGameObjects();

		//当たり判定の更新
		UpdateCollison();


		// パーティクルシステムの更新
		particleSystem_->Update(viewProjectionMatrix);

		//クリア・デス判定

		if (boss_->GetCurrentPhase() == BossPhase::Death &&
			boss_->GetDeathSubPhase() == DeathSubPhase::Complete) {
			// フェードを使った遷移
			SceneTransitionHelper::FadeToScene("GameClearScene", 1.0f);
			return; // 以降の処理をスキップ
		}

		if (player_->IsDeathSequenceComplete()) {
			// フェードを使った遷移
			SceneTransitionHelper::FadeToScene("GameOverScene", 1.0f);
			return; // 以降の処理をスキップ
		}

	}

}

void GameScene::UpdateGameObjects() {
	// 行列更新
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	viewProjectionMatrixSprite = cameraController_->GetViewProjectionMatrixSprite();

	// 自機の更新
	player_->Update(viewProjectionMatrix, viewProjectionMatrixSprite);

	// ボスの更新
	boss_->Update(viewProjectionMatrix, viewProjectionMatrixSprite);

	ground_->Update(viewProjectionMatrix);
	wall_->Update(viewProjectionMatrix);
	torch_->Update(viewProjectionMatrix);
	groundLight_->Update();
	sousa_->Update(viewProjectionMatrixSprite);

}

void GameScene::UpdateCollison()
{

	// 衝突マネージャーのリストをクリアする
	collisionManager_->ClearColliderList();

	// プレイヤーの弾のリストを取得（プール対応）
	std::vector<PlayerBullet*> playerBullets = player_->GetActiveBullets();
	std::vector<BossBullet*> bossBullets = boss_->GetActiveBullets();

	// Player
	collisionManager_->AddCollider(player_.get());
	// Bossのコライダーを追加（各パーツ）
	auto bossColliders = boss_->GetColliders();
	for (auto* collider : bossColliders) {
		collisionManager_->AddCollider(collider);
	}


	// プレイヤーの弾のコライダーを追加
	for (auto* bullet : playerBullets) {
		collisionManager_->AddCollider(bullet);
	}

	// ボスの弾のコライダーを追加
	for (auto* bullet : bossBullets) {
		collisionManager_->AddCollider(bullet);
	}

	// 衝突判定と応答
	collisionManager_->Update();

}

void GameScene::DrawOffscreen() {

	///
	///3Dゲームオブジェクトの描画（オフスクリーンに描画）
	/// 
	// 自機の描画
	player_->Draw();

	// ボスの描画
	boss_->Draw();


	ground_->Draw();
	wall_->Draw();
	torch_->Draw();

	///
	/// パーティクル・スプライトの描画（オフスクリーンに描画）
	/// 

	// パーティクルシステムの描画（全グループ）
	particleSystem_->Draw();
}

void GameScene::DrawBackBuffer() {
	///
	/// 3Dゲームオブジェクトの描画（オフスクリーンの外に描画）
	///


	///
	/// パーティクル・スプライトの描画（オフスクリーンの外に描画）
	/// 

	boss_->DrawUI();
	player_->DrawUI();
	sousa_->Draw();
}

void GameScene::ImGui() {
#ifdef USEIMGUI

	ImGui::Text("Debug Scene");
	ImGui::Separator();

	ImGui::Spacing();
	ImGui::Text("Player");
	player_->ImGui();

	ImGui::Spacing();
	ImGui::Text("Boss");
	boss_->ImGui();


	ImGui::Spacing();
	ImGui::Text("Ground");
	ground_->ImGui();

	ImGui::Spacing();
	ImGui::Text("Wall");
	wall_->ImGui();

	ImGui::Spacing();
	ImGui::Text("Torch");
	torch_->ImGui();

	ImGui::Spacing();
	groundLight_->ImGui();

	ImGui::Spacing();
	// パーティクルエディタ（統合UI）
	ImGui::Text("Particle Editor");
	particleEditor_->ImGui();
	sousa_->ImGui();

#endif
}

void GameScene::Finalize() {

	// シーン終了時にパーティクルシステムをクリア
	if (particleSystem_) {
		particleSystem_->Clear();
	}
	// シーン終了時にパーティクルインスタンスを全削除
	if (particleEditor_) {
		particleEditor_->DestroyAllInstance();
	}

}