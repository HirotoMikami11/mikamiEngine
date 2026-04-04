#pragma once
#include <memory>
#include "BaseScene.h"


/// <summary>
/// ゲームシーン
/// </summary>
class GameScene : public BaseScene {
public:
	GameScene();
	~GameScene() override;

	void ConfigureOffscreenEffects() override;

protected:
	// --- BaseScene virtual フック ---
	void OnInitialize()     override;
	void OnUpdate()         override;
	void OnDrawOffscreen()  override;
	void OnDrawBackBuffer() override;
	void OnImGui()          override;

private:
	// カメラ
	CameraController* cameraController_ = nullptr;
	Matrix4x4 viewProjectionMatrix_{};

	// システム参照
	DirectXCommon*     dxCommon_          = nullptr;
	OffscreenRenderer* offscreenRenderer_ = nullptr;
};
