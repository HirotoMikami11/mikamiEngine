#pragma once
#include <memory>
#include <array>

#include "Engine.h"
#include "DirectXCommon.h"
#include "Managers/Scene/BaseScene.h"

#include "Sprite.h"
#include "Light.h"
#include "Object3D.h"
#include "GridLine.h"
#include "CameraController.h"
#include "ParticleSystem.h"

/// <summary>
/// ゲームシーン
/// </summary>
class GameClearScene : public BaseScene {
public:
	GameClearScene();
	~GameClearScene() override;


	/// <summary>
	/// シーンに入った時のオフスクリーン設定
	/// </summary>
	void ConfigureOffscreenEffects() override;

	/// <summary>
	/// オブジェクト初期化（シーン切り替え毎に実行）
	/// </summary>
	void Initialize() override;

	void Update() override;

	/// <summary>
	/// 3D描画処理（オフスクリーン内）
	/// </summary>
	void DrawOffscreen() override;

	/// <summary>
	/// UI描画処理（オフスクリーン外）
	/// </summary>
	void DrawBackBuffer() override;

	void Finalize() override;

	// ImGui描画
	void ImGui() override;

private:
	void InitializeGameObjects();
	void UpdateGameObjects();

	// ゲームオブジェクト
	std::unique_ptr<Sphere> sphere_;
	std::unique_ptr<GridLine> gridLine_;

	// ライティング
	Light directionalLight_;

	// カメラ
	CameraController* cameraController_;
	Matrix4x4 viewProjectionMatrix;

	// システム参照
	DirectXCommon* directXCommon_;
	OffscreenRenderer* offscreenRenderer_;
};