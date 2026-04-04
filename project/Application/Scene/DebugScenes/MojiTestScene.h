#pragma once
#include "BaseScene.h"
#include "Dollar1Recognizer.h"
#include <vector>

/// <summary>
/// 文字書きテストシーン
/// </summary>
class MojiTestScene : public BaseScene
{
public:
	MojiTestScene();
	~MojiTestScene() override;

	void ConfigureOffscreenEffects() override;

protected:

	void OnInitialize()		override;
	void OnUpdate()			override;
	void OnDrawOffscreen()	override;
	void OnDrawBackBuffer()	override;
	void OnFinalize()		override;
	void OnImGui()			override;

private:
	void InitializeGameObjects();
	void UpdateGameObjects();

	std::unique_ptr<Sphere>sphere_;
	std::unique_ptr<Model3D>terrain_;
	CameraController* cameraController_;
	Matrix4x4 viewProjectionMatrix;
	DirectXCommon* dxCommon_;
	OffscreenRenderer* offscreenRenderer_;

	// 図形認識キャンバスの状態
	std::vector<ImVec2> strokePoints_;
	bool isDrawing_ = false;
	bool hasResult_ = false;
	DollarResult lastResult_;
};