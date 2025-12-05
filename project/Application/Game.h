#pragma once
/// Framework 
#include "Framework.h"
/// Scene
#include "SceneManager.h"
/// Transitions
#include "TransitionManager.h"
#include "SceneTransitionHelper.h"


/// <summary>
/// ゲーム全体を管理するクラス
/// </summary>
class Game : public Framework
{
public:
	Game();
	~Game() override;

	void Initialize() override;
	void Update() override;

	/// <summary>
	/// 3D描画（オフスクリーン内）
	/// </summary>
	void DrawOffscreen() override;

	/// <summary>
	/// UI描画（オフスクリーン外）
	/// </summary>
	void DrawBackBuffer() override;

	void ImGui() override;

	void Finalize() override;

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