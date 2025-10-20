#pragma once
#include <memory>

#include "Object3D.h"
#include "Particle.h"
#include "Light.h"
#include "GridLine.h"
#include "CameraController.h"

#include "Engine.h"
#include "DirectXCommon.h"
#include "Managers/Scene/BaseScene.h"

/// <summary>
/// デバッグ用シーン
/// このシーンで様々な機能を試す
/// </summary>
class DebugScene : public BaseScene {
public:
	DebugScene();
	~DebugScene() override;

	/// <summary>
	/// リソース読み込み（1回のみ実行）
	/// </summary>
	void LoadResources() override;

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
	std::unique_ptr<Plane> plane_;
	std::unique_ptr<GridLine> gridLine_;
	std::unique_ptr<Particle> particle_;

	// ライティング
	Light directionalLight_;

	// カメラ
	CameraController* cameraController_;
	Matrix4x4 viewProjectionMatrix;

	// システム参照
	DirectXCommon* directXCommon_;
	OffscreenRenderer* offscreenRenderer_;

	// リソース管理
	TextureManager* textureManager_;
};