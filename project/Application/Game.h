#pragma once
/// 				Scene							///
#include "SceneManager.h"
#include "LightManager.h"
/// 				Transitions							///
#include "TransitionManager.h"
#include "SceneTransitionHelper.h"
#include "TransitionEffect/SlideEffect.h"

/// <summary>
/// ゲーム全体を管理するクラス
/// </summary>
class Game
{
public:
	Game();
	~Game();

	void Initialize();
	void Update();

	/// <summary>
	/// 3D描画（オフスクリーン内）
	/// </summary>
	void DrawOffscreen();

	/// <summary>
	/// UI描画（オフスクリーン外）
	/// </summary>
	void DrawBackBuffer();

	void ImGui();

	void Finalize();

private:
	/// <summary>
	/// シーンの初期化
	/// </summary>
	void InitializeScenes();

	/// <summary>
	/// トランジションエフェクトの登録
	/// </summary>
	void RegisterTransitionEffects();

	// シーンマネージャー
	SceneManager* sceneManager_;

	// トランジションマネージャー
	TransitionManager* transitionManager_;

	//ライトマネージャー
	LightManager* lightManager_;

};