#pragma once
#include <memory>
#include "BaseScene.h"


/// <summary>
/// デバッグ用シーン
/// このシーンで様々な機能を試す
/// </summary>
class TetrisScene : public BaseScene {
public:
	TetrisScene();
	~TetrisScene() override;

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


	// カメラ
	CameraController* cameraController_;
	Matrix4x4 viewProjectionMatrix;

	// システム参照
	DirectXCommon* dxCommon_;
	OffscreenRenderer* offscreenRenderer_;
};