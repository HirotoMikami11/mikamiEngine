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

	//JsonSettings初期化
	jsonSettings_ = JsonSettings::GetInstance();

	// カメラコントローラー取得
	cameraController_ = CameraController::GetInstance();

	// ライトマネージャーの初期化
	lightManager_ = LightManager::GetInstance();
	lightManager_->Initialize(dxCommon_.get());

	// スプライトの共通部分を初期化
	SpriteCommon::GetInstance()->Initialize(dxCommon_.get());

	// オブジェクト3Dの共通部分を初期化
	Object3DCommon::GetInstance()->Initialize(dxCommon_.get());

	// Object3DRenderer を初期化（PSO 自己生成 + UploadRingBuffer 確保）
	Object3DRenderer::GetInstance()->Initialize(dxCommon_.get());

	// パーティクルの共通部分を初期化
	ParticleCommon::GetInstance()->Initialize(dxCommon_.get());

	// ImGui初期化
	imguiManager_ = ImGuiManager::GetInstance();
	imguiManager_->Initialize(winApp_.get(), dxCommon_.get());

	// オフスクリーンレンダラー初期化
	offscreenRenderer_ = std::make_unique<OffscreenRenderer>();
	offscreenRenderer_->Initialize(dxCommon_.get());

	// DebugDrawLineSystem初期化
	debugDrawManager_ = DebugDrawLineSystem::GetInstance();
	debugDrawManager_->Initialize(dxCommon_.get());

#ifdef USEIMGUI
	// FinalPassバッファ作成（Game View 表示用）
	CreateFinalPassBuffer();
#endif
}

void Engine::Update() {

	// FPS開始
	frameTimer_->BeginFrame();

	// GameTimerの更新(deltaTimeから計算)
	gameTimer_->Update(frameTimer_->GetDeltaTime());

	// 入力更新
	inputManager_->Update();

	// AudioManagerの更新(再生し終わったインスタンスの削除)
	audioManager_->Update();

	// ライトマネージャーの更新
	if (lightManager_) {
		lightManager_->Update();
	}


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

	// デバッグ描画
	if (debugDrawManager_) {
		debugDrawManager_->Reset();					// フレーム開始時にリセット
		debugDrawManager_->GenerateGridLines();		// グリッド線自動生成
		lightManager_->DebugDrawLight();			// ライトの描画
	}

	// Object3DRenderer のフレームリセット（UploadRingBuffer インデックスを0に戻す）
	Object3DRenderer::GetInstance()->BeginFrame();

	/// オフスクリーンの描画準備（3D描画用）
	offscreenRenderer_->PreDraw();

}

void Engine::EndDrawOffscreen() {

	// デバッグ線の一括描画（カメラコントローラーから行列取得）アプリ―ケーションに持っていく
	if (debugDrawManager_ && cameraController_) {
		debugDrawManager_->Draw(cameraController_->GetViewProjectionMatrix());
	}
	// Submit された 3D オブジェクトを一括 GPU 描画（ソート後に GPU コマンド発行）
	// PostDraw() より前に呼ぶことでオフスクリーン RT に描画される
	Object3DRenderer::GetInstance()->Draw3D();

	/// オフスクリーンの描画終了
	offscreenRenderer_->PostDraw();
}


void Engine::StartDrawBackBuffer() {

#ifdef USEIMGUI
	auto* cmdList = dxCommon_->GetCommandList();

	// FinalPass: SRV → RenderTarget
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = finalPassTexture_.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	cmdList->ResourceBarrier(1, &barrier);

	// FinalPass と DSV をクリア（DrawOffscreenTexture の前に行う）
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxCommon_->GetDescriptorManager()->GetCPUHandle(
		DescriptorHeapManager::HeapType::DSV, GraphicsConfig::kMainDSVIndex);
	cmdList->ClearRenderTargetView(finalPassRtvHandle_.cpuHandle, kFinalPassClearColor_, 0, nullptr);
	cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// ビューポート・シザー矩形（フル解像度）
	D3D12_VIEWPORT vp = { 0.0f, 0.0f,
		static_cast<float>(GraphicsConfig::kClientWidth),
		static_cast<float>(GraphicsConfig::kClientHeight),
		0.0f, 1.0f };
	D3D12_RECT sr = { 0, 0,
		static_cast<LONG>(GraphicsConfig::kClientWidth),
		static_cast<LONG>(GraphicsConfig::kClientHeight) };
	cmdList->RSSetViewports(1, &vp);
	cmdList->RSSetScissorRects(1, &sr);

	// オフスクリーン（3D + ポストエフェクト）を FinalPass に合成
	// DrawOffscreenTexture 内部の OMSetRenderTargets を FinalPass に向ける
	offscreenRenderer_->DrawOffscreenTexture(finalPassRtvHandle_.cpuHandle, dsvHandle);

#else
	// 通常パス：スワップチェーンに直接描画
	dxCommon_->PreDraw();
	offscreenRenderer_->DrawOffscreenTexture();
#endif
}


void Engine::EndDrawBackBuffer() {

#ifdef USEIMGUI
	auto* cmdList = dxCommon_->GetCommandList();

	// FinalPass: RenderTarget → SRV（ImGui::Image で参照できる状態に）
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = finalPassTexture_.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	cmdList->ResourceBarrier(1, &barrier);

	// スワップチェーンを RT に設定（ImGui のみ描画）
	dxCommon_->PreDraw();

	// ImGui を スワップチェーンに描画（Game View 含む）
	imguiManager_->Draw(cmdList);

	dxCommon_->PostDraw();
	dxCommon_->EndFrame();

#else
	// 通常パス
	imguiManager_->Draw(dxCommon_->GetCommandList());
	dxCommon_->PostDraw();
	dxCommon_->EndFrame();
#endif
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

#ifdef USEIMGUI
	// FinalPass バッファ解放
	{
		auto* descriptorManager = dxCommon_->GetDescriptorManager();
		if (finalPassRtvHandle_.isValid) {
			descriptorManager->ReleaseRTV(finalPassRtvHandle_.index);
		}
		if (finalPassSrvHandle_.isValid) {
			descriptorManager->ReleaseSRV(finalPassSrvHandle_.index);
		}
		finalPassTexture_.Reset();
	}
#endif

	// オフスクリーンレンダラー終了処理
	if (offscreenRenderer_) {
		offscreenRenderer_->Finalize();
		offscreenRenderer_.reset();
	}

	// ライトマネージャー終了処理
	if (lightManager_) {
		lightManager_->Finalize();
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

	// Object3DRenderer の GPU リソース解放（dxCommon_ 解放より先に行う）
	Object3DRenderer::GetInstance()->Finalize();

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

	// デバッグ時のみゲーム画面をImGuiで表示
	GameViewportImGui();

	//開発用UIの処理
	ImGui::Begin("エンジン");

	//FPS関連
	frameTimer_->ImGui();

	// GameTimer関連
	gameTimer_->ImGui();

	/// ResourceLoaderのImGui（各Managerの詳細を含む）
	resourceLoader_->ImGui();

	///JsonSettingsのImGui
	jsonSettings_->ImGui();

	/// オフスクリーンレンダラー（グリッチエフェクト含む）のImGui
	offscreenRenderer_->ImGui();

	///入力のImGui
	inputManager_->ImGui();

	/// デバッグ線のImGui
	debugDrawManager_->ImGui();


	ImGui::End();

	///
	///個別のウィンドウに表示
	///

	// カメラコントローラーのImGui
	if (cameraController_) {
		cameraController_->ImGui();
	}

	//　ライトマネージャーのImGui
	if (lightManager_) {
		lightManager_->ImGui();
	}
#endif
}

bool Engine::ProcessMessage()
{
	if (winApp_ && !winApp_->ProsessMessege()) {
		closedWindow_ = true;
	}
	return !closedWindow_;
}

#ifdef USEIMGUI
void Engine::CreateFinalPassBuffer() {
	auto* device = dxCommon_->GetDevice();
	auto* descriptorManager = dxCommon_->GetDescriptorManager();

	// テクスチャリソース作成（RTV + SRV 兼用）
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	// TYPELESS にすることで RTV(SRGB) と SRV(UNORM) を同一リソースに作れる
	// RTV: SRGB → OffscreenRenderer の PSO フォーマット(R8G8B8A8_UNORM_SRGB) に一致させる
	// SRV: UNORM → ImGui::Image がガンマ変換なしでサンプリングできる
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Width = GraphicsConfig::kClientWidth;
	resourceDesc.Height = GraphicsConfig::kClientHeight;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	// ClearValue のフォーマットは RTV フォーマット(SRGB)に合わせる
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	clearValue.Color[0] = kFinalPassClearColor_[0];
	clearValue.Color[1] = kFinalPassClearColor_[1];
	clearValue.Color[2] = kFinalPassClearColor_[2];
	clearValue.Color[3] = kFinalPassClearColor_[3];

	HRESULT hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(&finalPassTexture_));
	assert(SUCCEEDED(hr));
	finalPassTexture_->SetName(L"FinalPassTexture");

	// RTV: SRGB（OffscreenRenderer の PSO が R8G8B8A8_UNORM_SRGB を要求するため）
	finalPassRtvHandle_ = descriptorManager->CreateRTVForTexture2D(
		finalPassTexture_.Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

	// SRV: UNORM（ImGui::Image でガンマ変換なしにサンプリングするため）
	finalPassSrvHandle_ = descriptorManager->CreateSRVForTexture2D(
		finalPassTexture_.Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1);

	Logger::Log(Logger::GetStream(), "FinalPassBuffer created.\n");
}
#endif


void Engine::GameViewportImGui()
{
#ifdef USEIMGUI

	// Game View ウィンドウ（FinalPass テクスチャを ImGui::Image で表示）
	ImGui::SetNextWindowSize(ImVec2(854.0f, 505.0f), ImGuiCond_FirstUseEver);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("ゲームビュー");
	ImGui::PopStyleVar();
	{
		ImVec2 region = ImGui::GetContentRegionAvail();
		if (region.x > 0.0f && region.y > 0.0f) {
			const float targetAspect =
				static_cast<float>(GraphicsConfig::kClientWidth) /
				static_cast<float>(GraphicsConfig::kClientHeight);
			ImVec2 displaySize = region;
			if (region.x / region.y > targetAspect) {
				displaySize.x = region.y * targetAspect;
			} else {
				displaySize.y = region.x / targetAspect;
			}
			// 中央寄せ
			ImGui::SetCursorPos(ImVec2(
				ImGui::GetCursorPosX() + (region.x - displaySize.x) * 0.5f,
				ImGui::GetCursorPosY() + (region.y - displaySize.y) * 0.5f));
			ImGui::Image(
				reinterpret_cast<ImTextureID>(
					reinterpret_cast<void*>(finalPassSrvHandle_.gpuHandle.ptr)),
				displaySize);
		}
	}
	ImGui::End();
#endif
}

