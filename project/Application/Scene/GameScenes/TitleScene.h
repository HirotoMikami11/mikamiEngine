#pragma once
#include <memory>

#include "BaseScene.h"
#include "ModelFont/ModelFont.h"


/// <summary>
/// タイトルシーン
/// </summary>
class TitleScene : public BaseScene {
public:
	TitleScene();
	~TitleScene() override;


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

	std::unique_ptr<ModelFont> titleRogo_;
	std::unique_ptr<GridLine> gridLine_;

	// ライティング
	Light directionalLight_;

	// カメラ
	CameraController* cameraController_;
	Matrix4x4 viewProjectionMatrix;

	// システム参照
	DirectXCommon* dxCommon_;
	OffscreenRenderer* offscreenRenderer_;
	DebugDrawLineSystem* debugDrawLineSystem_;
};