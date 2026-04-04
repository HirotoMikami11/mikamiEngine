#pragma once
#include <memory>
#include "BaseScene.h"
#include "DebugObject/TestPlayer/TestPlayer.h"
#include "DebugObject/TestWall/TestWall.h"
#include "DebugObject/TestObject/TestObject.h"
#include "DebugObject/TestShooter/TestShooter.h"


/// <summary>
/// デバッグ用シーン
/// このシーンで様々な機能を試す
/// </summary>
class DebugScene : public BaseScene {
public:
	DebugScene();
	~DebugScene() override;

	void ConfigureOffscreenEffects() override;

protected:

	void OnInitialize()		override;
	void OnUpdate()			override;
	void OnDrawOffscreen()	override;
	void OnImGui()			override;
	void OnFinalize()		override;

private:


	// カメラ
	CameraController* cameraController_ = nullptr;
	Matrix4x4 viewProjectionMatrix_{};

	//ライティング
	PointLight* pointLight_;
	SpotLight* spotLight_;
	RectLight* rectLight_;

	// ゲームオブジェクト
	TestPlayer* testPlayer_ = nullptr;
	TestWall* testWall_ = nullptr;
	TestObject* testObject_ = nullptr;
	TestShooter* testShooter_ = nullptr;

	// Manager外オブジェクト
	std::unique_ptr<Model3D> terrain_;
	std::unique_ptr<Model3D> gltfPlane_;
	std::unique_ptr<Model3D> objPlane_;

	// システム参照
	DirectXCommon* dxCommon_;
	OffscreenRenderer* offscreenRenderer_;
};
