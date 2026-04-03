#pragma once
#include <memory>
#include "BaseScene.h"
#include "TestPlayer.h"
#include "DebugObject/TestWall/TestWall.h"
#include "DebugObject/TestObject/TestObject.h"
#include "CollisionManager/CollisionManager.h"


/// <summary>
/// デバッグ用シーン
/// このシーンで様々な機能を試す
/// </summary>
class DebugScene : public BaseScene {
public:
	DebugScene();
	~DebugScene() override;

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

	std::unique_ptr<Model3D> terrain_;
	std::unique_ptr<Model3D> gltfPlane_;
	std::unique_ptr<Model3D> objPlane_;


	// カメラ
	CameraController* cameraController_;
	Matrix4x4 viewProjectionMatrix;

	//ライティング
	PointLight* pointLight_;
	SpotLight* spotLight_;
	RectLight* rectLight_;

	// ゲームオブジェクト（GameObjectManager 管理、所有権は Manager 側）
	TestPlayer*  testPlayer_  = nullptr;
	TestWall*    testWall_    = nullptr;
	TestObject*  testObject_  = nullptr;

	// 衝突判定マネージャー
	CollisionManager collisionManager_;

	// システム参照
	DirectXCommon* dxCommon_;
	OffscreenRenderer* offscreenRenderer_;
};