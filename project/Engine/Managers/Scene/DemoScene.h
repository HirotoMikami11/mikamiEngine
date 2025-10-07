#pragma once
#include <memory>
#include <array>

#include "Sprite.h"
#include "Light.h"
#include "GameObject.h"
#include "CameraController.h"
#include "GridLine.h"
#include "Engine.h"
#include "DirectXCommon.h"
#include "Managers/Scene/BaseScene.h"


/// <summary>
/// 絶対動くシーン(作り変えたりしない)
/// デバッグ用に使用するシーンはDebugSceneなのでこのシーンは基本的に動かさない
/// </summary>
class DemoScene : public BaseScene {
public:
	DemoScene();
	~DemoScene() override;

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
	std::unique_ptr<Sphere> sphere_;
	std::unique_ptr<Plane> plane_;

	std::unique_ptr<Model3D> modelMultiMesh_;
	std::unique_ptr<Model3D> modelMultiMaterial_;

	std::unique_ptr<Sprite> sprite_;
	std::unique_ptr<GridLine> gridLine_;

	// ライティング
	Light directionalLight_;

	// カメラ
	CameraController* cameraController_;
	Matrix4x4 viewProjectionMatrix;
	Matrix4x4 viewProjectionMatrixSprite;

	// システム参照
	DirectXCommon* directXCommon_;
	OffscreenRenderer* offscreenRenderer_;

	// リソース管理
	ModelManager* modelManager_;
	TextureManager* textureManager_;
};