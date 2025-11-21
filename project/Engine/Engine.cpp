#include "Engine.h"

Engine* Engine::GetInstance() {
	static Engine instance;
	return &instance;
}

void Engine::Initialize(const std::wstring& title) {
	// ダンプ初期化
	Dump::Initialize();
	// 基礎システムの初期化
	InitializeBase(title);
	// Manager関連の初期化
	InitializeManagers();

	// ResourceLoader経由で全リソースを読み込み
	resourceLoader_->LoadAllResources();
}

void Engine::InitializeBase(const std::wstring& title) {
	// ウィンドウ初期化
	winApp_ = std::make_unique<WinApp>();
	winApp_->Initialize(title);

	// ログ初期化
	Logger::Initialize();

	// DirectX初期化
	dxCommon_ = std::make_unique<DirectXCommon>();
	dxCommon_->Initialize(winApp_.get());
}

void Engine::InitializeManagers() {


	// 入力マネージャー初期化
	inputManager_ = Input::GetInstance();
	inputManager_->Initialize(winApp_.get());

	// FPSタイマー取得
	frameTimer_ = &FrameTimer::GetInstance();

	// ゲームタイマー取得
	gameTimer_ = &GameTimer::GetInstance();

	// テクスチャマネージャー初期化
	textureManager_ = TextureManager::GetInstance();
	textureManager_->Initialize(dxCommon_.get());

	// モデルマネージャー初期化
	modelManager_ = ModelManager::GetInstance();
	modelManager_->Initialize(dxCommon_.get());

	// オーディオマネージャー初期化
	audioManager_ = AudioManager::GetInstance();
	audioManager_->Initialize();

	// ResourceLoader初期化
	resourceLoader_ = ResourceLoader::GetInstance();
	resourceLoader_->Initialize();

	// スプライトの共通部分を初期化
	SpriteCommon::GetInstance()->Initialize(dxCommon_.get());

	// オブジェクト3Dの共通部分を初期化
	Object3DCommon::GetInstance()->Initialize(dxCommon_.get());

	// パーティクルの共通部分を初期化
	ParticleCommon::GetInstance()->Initialize(dxCommon_.get());

	// カメラコントローラー取得
	cameraController_ = CameraController::GetInstance();

	// ImGui初期化
	imguiManager_ = ImGuiManager::GetInstance();
	imguiManager_->Initialize(winApp_.get(), dxCommon_.get());

	// オフスクリーンレンダラー初期化
	offscreenRenderer_ = std::make_unique<OffscreenRenderer>();
	offscreenRenderer_->Initialize(dxCommon_.get());

	// DebugDrawLineSystem初期化
	debugDrawManager_ = DebugDrawLineSystem::GetInstance();
	debugDrawManager_->Initialize(dxCommon_.get());
}


void Engine::Update() {
	// ウィンドウメッセージの処理
	if (!winApp_->ProsessMessege()) {
		ClosedWindow_ = true; //ウィンドウを閉じる
	}

	// FPS開始
	frameTimer_->BeginFrame();

	// GameTimerの更新(deltaTimeから計算)
	gameTimer_->Update(frameTimer_->GetDeltaTime());

	// 入力更新
	inputManager_->Update();

	// AudioManagerの更新(再生し終わったインスタンスの削除)
	audioManager_->Update();

	/// ImGuiの受付開始
	imguiManager_->Begin();

	// オフスクリーンレンダラーの更新（エフェクト含む)
	offscreenRenderer_->Update(frameTimer_->GetDeltaTime());
}


void Engine::StartDrawOffscreen() {
	/// ImGuiの受付終了
	imguiManager_->End();

	// フレーム開始
	dxCommon_->BeginFrame();

	// デバッグ描画のリセット（前フレームの線分をクリア、グリッド自動生成）
	if (debugDrawManager_) {
		debugDrawManager_->Reset();
	}

	/// オフスクリーンの描画準備（3D描画用）
	offscreenRenderer_->PreDraw();

}

void Engine::EndDrawOffscreen() {

	// デバッグ線の一括描画（カメラコントローラーから行列取得）アプリ―ケーションに持っていく
	if (debugDrawManager_ && cameraController_) {
		debugDrawManager_->Draw(cameraController_->GetViewProjectionMatrix());
	}
	/// オフスクリーンの描画終了
	offscreenRenderer_->PostDraw();
}


void Engine::StartDrawBackBuffer() {

	// 通常描画の描画準備（バックバッファ描画開始）
	dxCommon_->PreDraw();

	// オフスクリーンの画面の実態描画
	offscreenRenderer_->DrawOffscreenTexture();
}


void Engine::EndDrawBackBuffer() {
	// ImGuiの画面への描画
	imguiManager_->Draw(dxCommon_->GetCommandList());

	// 通常描画の終わり
	dxCommon_->PostDraw();

	// 描画そのもののEndFrame
	dxCommon_->EndFrame();


}

void Engine::Finalize() {
	// DebugDrawLineSystemの終了処理
	if (debugDrawManager_) {
		debugDrawManager_->Finalize();
	}

	// ResourceLoaderの終了処理
	if (resourceLoader_) {
		resourceLoader_->Finalize();
	}

	// ImGui終了処理
	if (imguiManager_) {
		imguiManager_->Finalize();
	}

	// オフスクリーンレンダラー終了処理
	if (offscreenRenderer_) {
		offscreenRenderer_->Finalize();
		offscreenRenderer_.reset();
	}

	// カメラコントローラー終了処理
	if (cameraController_) {
		cameraController_->Finalize();
	}

	// オーディオ終了処理
	if (audioManager_) {
		audioManager_->Finalize();
	}

	// 入力終了処理
	if (inputManager_) {
		inputManager_->Finalize();
	}

	// モデル終了処理
	if (modelManager_) {
		modelManager_->Finalize();
	}

	// テクスチャ終了処理
	if (textureManager_) {
		textureManager_->Finalize();
	}

	// ウィンドウ終了処理
	if (winApp_) {
		winApp_->Finalize();
		winApp_.reset();
	}

	// DirectX終了処理
	if (dxCommon_) {
		dxCommon_->Finalize();
		dxCommon_.reset();
	}

	// COM終了処理
	CoUninitialize();
}

void Engine::ImGui() {
#ifdef USEIMGUI

	//開発用UIの処理
	ImGui::Begin("Engine");

	//FPS関連
	frameTimer_->ImGui();

	// GameTimer関連
	gameTimer_->ImGui();

	/// ResourceLoaderのImGui（各Managerの詳細を含む）
	resourceLoader_->ImGui();

	/// オフスクリーンレンダラー（グリッチエフェクト含む）のImGui
	offscreenRenderer_->ImGui();

	///入力のImGui
	inputManager_->ImGui();

	/// デバッグ線のImGui
	debugDrawManager_->ImGui();


	ImGui::End();

	///
	/// カメラコントローラーのデバッグUI
	///

	///個別のウィンドウに表示
	if (cameraController_) {
		//カメラコントローラーのアップデートがないとエラー
		cameraController_->ImGui();
	}

#endif
}