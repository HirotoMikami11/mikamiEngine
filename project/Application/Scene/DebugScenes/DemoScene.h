#pragma once
#include <memory>
#include "BaseScene.h"

/// <summary>
/// 絶対動くシーン（作り変えたりしない）
/// デバッグ用に使用するシーンは DebugScene なのでこのシーンは基本的に動かさない
/// </summary>
class DemoScene : public BaseScene {
public:
	DemoScene();
	~DemoScene() override;

	void ConfigureOffscreenEffects() override;

protected:

	void OnInitialize()		override;
	void OnUpdate()			override;
	void OnDraw()			override;
	void OnDrawBackBuffer() override;
	void OnImGui()			override;
	void OnFinalize()		override;

private:
	// Manager外オブジェクト
	std::unique_ptr<Sphere> sphere_;
	std::unique_ptr<Plane> plane_;
	std::unique_ptr<Model3D> modelMultiMesh_;
	std::unique_ptr<Model3D> modelMultiMaterial_;
	std::unique_ptr<Sprite> sprite_;

	// パーティクルシステム
	ParticleSystem* particleSystem_ = nullptr;
	ParticleEditor* particleEditor_ = nullptr;

	// カメラ
	CameraController* cameraController_ = nullptr;
	Matrix4x4 viewProjectionMatrix_{};
	Matrix4x4 viewProjectionMatrixSprite_{};

	// システム参照
	DirectXCommon* dxCommon_ = nullptr;
	OffscreenRenderer* offscreenRenderer_ = nullptr;
};
