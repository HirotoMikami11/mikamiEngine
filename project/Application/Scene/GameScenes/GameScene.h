#pragma once
#include <memory>

#include "BaseScene.h"
#include "CollisionManager.h"

#include "Ground.h"
#include "Player.h"
#include "Boss.h"
#include "Wall.h"

/// <summary>
/// ゲームシーン
/// </summary>
class GameScene : public BaseScene {
public:
	GameScene();
	~GameScene() override;


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
	void UpdateCollison();

	// ゲームオブジェクト
	std::unique_ptr<Ground> ground_;
	std::unique_ptr<Player> player_;
	std::unique_ptr<Boss> boss_;
	std::unique_ptr<Wall> wall_;

	// カメラ
	CameraController* cameraController_;
	Matrix4x4 viewProjectionMatrix;
	Matrix4x4 viewProjectionMatrixSprite;

	// 衝突マネージャー
	std::unique_ptr<CollisionManager> collisionManager_;

	// システム参照
	DirectXCommon* dxCommon_;
	OffscreenRenderer* offscreenRenderer_;
};