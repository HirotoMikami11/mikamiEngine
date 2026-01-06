#pragma once
#include <memory>
#include "Sprite.h"
#include "Input.h"
#include "GameTimer.h"

/// <summary>
/// セレクトモード列挙型
/// </summary>
enum class PauseSelectMode {
	Resume,			// ゲーム再開
	BackToTitle		// タイトルへ
};

/// <summary>
/// ポーズ画面管理クラス
/// </summary>
class Pause {
public:
	Pause() = default;
	~Pause() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

	/// <summary>
	/// ポーズ状態を取得
	/// </summary>
	bool IsPaused() const { return isPaused_; }

	/// <summary>
	/// タイトルへ戻るフラグを取得
	/// </summary>
	bool ShouldReturnToTitle() const { return shouldReturnToTitle_; }

private:
	/// <summary>
	/// ポーズ切り替え入力チェック
	/// </summary>
	void CheckPauseToggleInput();

	/// <summary>
	/// セレクト操作の入力チェック
	/// </summary>
	void CheckSelectInput();

	/// <summary>
	/// 決定ボタン入力チェック
	/// </summary>
	void CheckDecideInput();

	/// <summary>
	/// 透明度のイージング更新
	/// </summary>
	void UpdateAlphaEasing();

	/// <summary>
	/// セレクト色のイージング更新
	/// </summary>
	void UpdateSelectColorEasing();

	/// <summary>
	/// セレクトサイズのイージング更新
	/// </summary>
	void UpdateSelectSizeEasing();

	/// <summary>
	/// イージング関数（EaseInCubic）
	/// </summary>
	float EaseInCubic(float x);

	/// <summary>
	/// イージング関数（EaseOutBounce）
	/// </summary>
	float EaseOutBounce(float x);

	// Input参照
	Input* input_ = Input::GetInstance();

	// GameTimer参照
	GameTimer* gameTimer_ = &GameTimer::GetInstance();

	// Sprite
	std::unique_ptr<Sprite> backgroundSprite_;		// 半透明黒背景
	std::unique_ptr<Sprite> resumeTextSprite_;		// "ゲーム再開"
	std::unique_ptr<Sprite> titleTextSprite_;		// "タイトルへ"

	// ポーズ状態
	bool isPaused_ = false;

	// タイトルへ戻るフラグ
	bool shouldReturnToTitle_ = false;

	// セレクトモード
	PauseSelectMode selectMode_ = PauseSelectMode::Resume;

	// イージング用
	float easingTimer_ = 0.0f;				// イージング経過時間
	const float easingDuration_ = 0.5f;		// イージング時間
	bool isEasingIn_ = false;				// フェードイン中か
	bool isEasingOut_ = false;				// フェードアウト中か

	// 透明度（ターゲット値）
	float targetBackgroundAlpha_ = 0.0f;	// 背景の目標透明度
	float targetTextAlpha_ = 0.0f;			// テキストの目標透明度
	float currentBackgroundAlpha_ = 0.0f;	// 背景の現在透明度
	float currentTextAlpha_ = 0.0f;			// テキストの現在透明度

	// セレクト色のイージング
	float selectColorTimer_ = 0.0f;			// 色変化のタイマー
	const float selectColorDuration_ = 0.3f;// 色変化の時間
	Vector4 resumeColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };		// ゲーム再開の色
	Vector4 titleColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };		// タイトルへの色
	Vector4 selectedColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };	// 選択中の色
	Vector4 unselectedColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };	// 非選択の色（白）

	// セレクトサイズのイージング
	float selectSizeTimer_ = 0.0f;			// サイズ変化のタイマー
	const float selectSizeDuration_ = 0.5f;	// サイズ変化の時間
	float selectedScale_ = 1.2f;			// 選択中のスケール倍率（ImGuiで調整可能）
	float unselectedScale_ = 1.0f;			// 非選択のスケール倍率
	Vector2 currentResumeSize_;				// ゲーム再開の現在サイズ
	Vector2 currentTitleSize_;				// タイトルへの現在サイズ

	// レイアウト設定（ImGuiで調整可能）
	Vector2 backgroundPosition_ = { 640.0f, 360.0f };		// 背景の位置
	Vector2 backgroundSize_ = { 1280.0f, 720.0f };			// 背景のサイズ

	Vector2 resumeTextPosition_ = { 640.0f, 300.0f };		// "ゲーム再開"の位置
	Vector2 resumeTextSize_ = { 300.0f, 80.0f };			// "ゲーム再開"のサイズ

	Vector2 titleTextPosition_ = { 640.0f, 420.0f };		// "タイトルへ"の位置
	Vector2 titleTextSize_ = { 300.0f, 80.0f };				// "タイトルへ"のサイズ

	// アナログスティックの前フレーム状態
	bool prevStickUp_ = false;
	bool prevStickDown_ = false;
};