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
	/// 描画Submit（Renderer へのSubmitのみ）
	/// </summary>
	void Draw() override;

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