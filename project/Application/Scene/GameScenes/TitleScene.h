#pragma once
#include <memory>

#include "BaseScene.h"
#include "ModelFont/ModelFont.h"

#include "TitleField.h"


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
	std::unique_ptr<ModelFont> pressA_;
	//地面
	std::unique_ptr<TitleField> titleField_;

	//カメラ移動
	float cameraMoveSpeed_;

	// パーティクルシステム
	ParticleSystem* particleSystem_;
	ParticleEditor* particleEditor_;

	// ライティング

	// カメラ
	CameraController* cameraController_;
	Matrix4x4 viewProjectionMatrix;
	int BGMHandle_;
	// システム参照
	DirectXCommon* dxCommon_;
	OffscreenRenderer* offscreenRenderer_;
};