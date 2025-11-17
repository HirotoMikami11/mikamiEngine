#pragma once
/// 				Scene							///
#include "Scene/SceneManager.h"


/// 				Transitions							///
#include "Transition/TransitionManager.h"
#include "Transition/SceneTransitionHelper.h"
#include "Transition/TransitionEffect/SlideEffect.h"

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

};