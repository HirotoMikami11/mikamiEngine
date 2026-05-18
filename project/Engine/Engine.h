#pragma once
#include<memory>
#include<string>

///BaseSystem
#include "Logger.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Dump.h"

///Managers
#include "Input.h"
#include "Audio/AudioManager.h"
#include "Texture/TextureManager.h"
#include "Model/ModelManager.h"
#include "ResourceLoader/ResourceLoader.h"
#include "ImGui/ImGuiManager.h" 
#include "JsonSettings.h" 
#include "FrameTimer.h"
#include "GameTimer.h"
#include "OffscreenRenderer.h"
#include "DebugDrawLineSystem.h"
#include "LightManager.h"

///Objects
#include "CameraController.h"
#include "Object3D.h"
#include "Sprite.h"
#include "ParticleCommon.h"
#include "Object3DRenderer.h"
#include "SpriteRenderer.h"
#include "SkyboxRenderer.h"
#include "Skybox.h"

class Engine {
public:
	//シングルトン
	static Engine* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="title">タイトルバーの文字</param>
	void Initialize(const std::wstring& title);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <returns></returns>
	void Update();

	/// <summary>
	/// フレーム開始処理（コマンドリストオープン・Rendererリセット）
	/// Draw() より前に呼ぶ
	/// </summary>
	void BeginFrame();

	/// <summary>
	/// オフスクリーン描画開始（RT = オフスクリーンテクスチャ）
	/// Draw() より後、EndOffscreen() より前に呼ぶ
	/// </summary>
	void BeginOffscreen();

	/// <summary>
	/// オフスクリーン描画終了（FlushOffscreen + ポストエフェクト）
	/// </summary>
	void EndOffscreen();

	/// <summary>
	/// バックバッファ描画開始（オフスクリーン → バックバッファ合成）
	/// </summary>
	void BeginBackBuffer();

	/// <summary>
	/// バックバッファ描画終了（FlushUI + ImGui + EndFrame）
	/// </summary>
	void EndBackBuffer();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// Imgui
	/// </summary>
	void ImGui();

	/// <summary>
	/// ウィンドウメッセージ処理
	/// </summary>
	/// <returns></returns>
	bool ProcessMessage();

	// ゲッター
	WinApp* GetWinApp() const { return winApp_.get(); }
	DirectXCommon* GetDirectXCommon() const { return dxCommon_.get(); }
	OffscreenRenderer* GetOffscreenRenderer() const { return offscreenRenderer_.get(); }
	DebugDrawLineSystem* GetDebugDrawManager() const { return debugDrawManager_; }
	GameTimer* GetGameTimer() const { return gameTimer_; }  // ← 追加
	bool IsClosedWindow() const { return closedWindow_; }

private:
	/// <summary>
	/// コピー禁止
	/// </summary>
	Engine() = default;
	~Engine() = default;
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	/// <summary>
	/// WinApp,DirectXCommonの初期化
	/// </summary>
	/// <param name="title"></param>
	void InitializeBase(const std::wstring& title);

	/// <summary>
	/// それぞれのマネージャーの初期化
	/// </summary>
	void InitializeManagers();

	/// <summary>
	/// デバッグ時のゲーム画面imGui表示
	/// </summary>
	void GameViewportImGui();

	// 基盤システム
	std::unique_ptr<WinApp> winApp_;
	std::unique_ptr<DirectXCommon> dxCommon_;


	// オフスクリーン
	std::unique_ptr<OffscreenRenderer> offscreenRenderer_;

#ifdef USEIMGUI
	// FinalPass バッファ（オフスクリーンを描画した後、ImGui::Image に渡すテクスチャ）
	Microsoft::WRL::ComPtr<ID3D12Resource> finalPassTexture_;
	DescriptorHeapManager::DescriptorHandle finalPassRtvHandle_;
	DescriptorHeapManager::DescriptorHandle finalPassSrvHandle_;
	static constexpr float kFinalPassClearColor_[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	/// <summary>
	/// FinalPassバッファを作成する
	/// </summary>
	void CreateFinalPassBuffer();
#endif

	// マネージャー
	Input* inputManager_;
	TextureManager* textureManager_;
	AudioManager* audioManager_;
	ImGuiManager* imguiManager_;
	ModelManager* modelManager_;
	ResourceLoader* resourceLoader_;
	FrameTimer* frameTimer_;
	GameTimer* gameTimer_;
	DebugDrawLineSystem* debugDrawManager_;
	JsonSettings* jsonSettings_;

	LightManager* lightManager_;

	// カメラコントローラー
	CameraController* cameraController_;

	//ウィンドウを閉じるか否か
	bool closedWindow_ = false;
};