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
	/// 3D描画前処理（オフスクリーン描画開始）
	/// </summary>
	void StartDrawOffscreen();

	/// <summary>
	/// 3D描画後処理（オフスクリーン描画終了)
	/// </summary>
	void EndDrawOffscreen();

	/// <summary>
	/// UI描画前処理（バックバッファ描画開始）
	/// </summary>
	void StartDrawBackBuffer();

	/// <summary>
	/// UI描画後処理（バックバッファ描画終了）
	/// </summary>
	void EndDrawBackBuffer();

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

	// 基盤システム
	std::unique_ptr<WinApp> winApp_;
	std::unique_ptr<DirectXCommon> dxCommon_;

	// オフスクリーン
	std::unique_ptr<OffscreenRenderer> offscreenRenderer_;

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