#pragma once
#include <memory>

#include "BaseScene.h"
#include "ModelFont/ModelFont.h"
#include "Ground.h"
/// <summary>
/// ゲームオーバーシーン
/// </summary>
class GameOverScene : public BaseScene {
public:
	GameOverScene();
	~GameOverScene() override;

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
	std::unique_ptr<ModelFont> overFont_;
	std::unique_ptr<ModelFont> pressA_;
	//地面
	std::unique_ptr<Ground> ground_;
	// カメラ
	CameraController* cameraController_;
	Matrix4x4 viewProjectionMatrix;

	// システム参照
	DirectXCommon* dxCommon_;
	OffscreenRenderer* offscreenRenderer_;
};
