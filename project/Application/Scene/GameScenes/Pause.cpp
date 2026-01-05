#include "Pause.h"
#include "ImGui/ImGuiManager.h"
#include "Transition/SceneTransitionHelper.h"

void Pause::Initialize(DirectXCommon* dxCommon) {
	// 背景Sprite初期化（黒色、半透明）
	backgroundSprite_ = std::make_unique<Sprite>();
	backgroundSprite_->Initialize(dxCommon, backgroundPosition_, backgroundSize_);
	backgroundSprite_->SetColor({ 0.0f, 0.0f, 0.0f, 0.0f }); // 初期は透明

	// "ゲーム再開"テキストSprite初期化（白色、仮画像white2x2使用）
	resumeTextSprite_ = std::make_unique<Sprite>();
	resumeTextSprite_->Initialize(dxCommon, "pause1", resumeTextPosition_, resumeTextSize_);
	resumeTextSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f }); // 初期は透明

	// "タイトルへ"テキストSprite初期化（白色、仮画像white2x2使用）
	titleTextSprite_ = std::make_unique<Sprite>();
	titleTextSprite_->Initialize(dxCommon, "pause2", titleTextPosition_, titleTextSize_);
	titleTextSprite_->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f }); // 初期は透明

	// 初期状態設定
	isPaused_ = false;
	shouldReturnToTitle_ = false;
	selectMode_ = PauseSelectMode::Resume;
	easingTimer_ = 0.0f;
	isEasingIn_ = false;
	isEasingOut_ = false;
	currentBackgroundAlpha_ = 0.0f;
	currentTextAlpha_ = 0.0f;
	selectColorTimer_ = 0.0f;
	selectSizeTimer_ = 0.0f;

	// サイズの初期化
	currentResumeSize_ = resumeTextSize_;
	currentTitleSize_ = titleTextSize_;

	// 初期色の設定（Resume が選択中なので黄色、Title は白）
	resumeColor_ = selectedColor_;
	titleColor_ = unselectedColor_;
}

void Pause::Update(const Matrix4x4& viewProjectionMatrix) {
	// ポーズ切り替え入力チェック
	CheckPauseToggleInput();

	// イージング更新
	UpdateAlphaEasing();

	// ポーズ中の場合のみセレクト操作を受け付ける
	if (isPaused_ && !isEasingIn_ && !isEasingOut_) {
		CheckSelectInput();
		CheckDecideInput();
	} else if (!isPaused_) {
		// ポーズ中でない場合は前フレーム状態をリセット
		prevStickUp_ = false;
		prevStickDown_ = false;
		// 色とサイズもリセット
		selectColorTimer_ = 0.0f;
		selectSizeTimer_ = 0.0f;
	}

	// セレクト色のイージング更新
	UpdateSelectColorEasing();

	// セレクトサイズのイージング更新
	UpdateSelectSizeEasing();

	// Sprite更新
	backgroundSprite_->SetPosition(backgroundPosition_);
	backgroundSprite_->SetScale(backgroundSize_);
	backgroundSprite_->Update(viewProjectionMatrix);

	resumeTextSprite_->SetPosition(resumeTextPosition_);
	resumeTextSprite_->SetScale(currentResumeSize_); // 現在のサイズを使用
	resumeTextSprite_->Update(viewProjectionMatrix);

	titleTextSprite_->SetPosition(titleTextPosition_);
	titleTextSprite_->SetScale(currentTitleSize_); // 現在のサイズを使用
	titleTextSprite_->Update(viewProjectionMatrix);
}

void Pause::Draw() {
	// 背景描画
	backgroundSprite_->Draw();

	// テキスト描画
	resumeTextSprite_->Draw();
	titleTextSprite_->Draw();
}

void Pause::ImGui() {
#ifdef USEIMGUI
	if (ImGui::CollapsingHeader("Pause")) {
		ImGui::Text("Paused: %s", isPaused_ ? "Yes" : "No");
		ImGui::Text("Select Mode: %s",
			selectMode_ == PauseSelectMode::Resume ? "Resume" : "BackToTitle");

		ImGui::Separator();
		ImGui::Text("Background");
		ImGui::DragFloat2("BG Position", &backgroundPosition_.x, 1.0f);
		ImGui::DragFloat2("BG Size", &backgroundSize_.x, 1.0f);

		ImGui::Separator();
		ImGui::Text("Resume Text");
		ImGui::DragFloat2("Resume Position", &resumeTextPosition_.x, 1.0f);
		ImGui::DragFloat2("Resume Size", &resumeTextSize_.x, 1.0f);

		ImGui::Separator();
		ImGui::Text("Title Text");
		ImGui::DragFloat2("Title Position", &titleTextPosition_.x, 1.0f);
		ImGui::DragFloat2("Title Size", &titleTextSize_.x, 1.0f);

		ImGui::Separator();
		ImGui::Text("Colors");
		ImGui::ColorEdit4("Selected Color", &selectedColor_.x);
		ImGui::ColorEdit4("Unselected Color", &unselectedColor_.x);

		ImGui::Separator();
		ImGui::Text("Size Scale");
		ImGui::SliderFloat("Selected Scale", &selectedScale_, 1.0f, 2.0f);
		ImGui::Text("Unselected Scale: %.2f", unselectedScale_);
	}
#endif
}

void Pause::CheckPauseToggleInput() {
	// ゲームパッドのSTARTボタン または キーボードのESCキーでポーズ切り替え
	bool toggleInput = input_->IsGamePadButtonTrigger(Input::GamePadButton::START) ||
		input_->IsKeyTrigger(DIK_ESCAPE);

	if (toggleInput && !isEasingIn_ && !isEasingOut_) {
		isPaused_ = !isPaused_;

		if (isPaused_) {
			// ポーズ開始：GameTimerを停止してイージング開始
			gameTimer_->Pause();
			isEasingIn_ = true;
			easingTimer_ = 0.0f;
			targetBackgroundAlpha_ = 0.8f;
			targetTextAlpha_ = 1.0f;
			selectMode_ = PauseSelectMode::Resume; // セレクトをリセット
			selectColorTimer_ = 0.0f;
			selectSizeTimer_ = 0.0f;

			// 初期色とサイズを設定（Resume が選択中）
			resumeColor_ = selectedColor_;
			titleColor_ = unselectedColor_;
			currentResumeSize_ = resumeTextSize_ * selectedScale_;
			currentTitleSize_ = titleTextSize_ * unselectedScale_;
		} else {
			// ポーズ解除：イージングアウト開始
			isEasingOut_ = true;
			easingTimer_ = 0.0f;
			targetBackgroundAlpha_ = 0.0f;
			targetTextAlpha_ = 0.0f;
		}
	}
}

void Pause::CheckSelectInput() {
	// アナログスティックの現在の状態を取得
	float stickY = input_->GetAnalogStick(Input::AnalogStick::LEFT_Y);
	bool currentStickUp = stickY > 0.5f;
	bool currentStickDown = stickY < -0.5f;

	// アナログスティックのトリガー判定（前フレームfalse、今フレームtrue）
	bool stickUpTrigger = currentStickUp && !prevStickUp_;
	bool stickDownTrigger = currentStickDown && !prevStickDown_;

	// 十字キー上下 / W,S / 矢印上下 / アナログスティック上下（トリガー）で選択切り替え
	bool upInput = input_->IsGamePadButtonTrigger(Input::GamePadButton::UP) ||
		input_->IsKeyTrigger(DIK_W) ||
		input_->IsKeyTrigger(DIK_UP) ||
		stickUpTrigger;

	bool downInput = input_->IsGamePadButtonTrigger(Input::GamePadButton::DOWN) ||
		input_->IsKeyTrigger(DIK_S) ||
		input_->IsKeyTrigger(DIK_DOWN) ||
		stickDownTrigger;

	if (upInput || downInput) {
		// セレクトモードを切り替え
		if (selectMode_ == PauseSelectMode::Resume) {
			selectMode_ = PauseSelectMode::BackToTitle;
		} else {
			selectMode_ = PauseSelectMode::Resume;
		}
		selectColorTimer_ = 0.0f; // 色変化のタイマーをリセット
		selectSizeTimer_ = 0.0f;  // サイズ変化のタイマーをリセット
	}

	// 前フレームの状態を保存
	prevStickUp_ = currentStickUp;
	prevStickDown_ = currentStickDown;
}

void Pause::CheckDecideInput() {
	// Aボタン または Spaceキーで決定
	bool decideInput = input_->IsGamePadButtonTrigger(Input::GamePadButton::A) ||
		input_->IsKeyTrigger(DIK_SPACE);

	if (decideInput) {
		if (selectMode_ == PauseSelectMode::Resume) {
			// ゲーム再開
			isPaused_ = false;
			isEasingOut_ = true;
			easingTimer_ = 0.0f;
			targetBackgroundAlpha_ = 0.0f;
			targetTextAlpha_ = 0.0f;
		} else if (selectMode_ == PauseSelectMode::BackToTitle) {
			// タイトルへ戻る
			shouldReturnToTitle_ = true;
			// GameTimerを再開してから遷移
			gameTimer_->Resume();
			SceneTransitionHelper::FadeToScene("TitleScene", 1.0f);
		}
	}
}

void Pause::UpdateAlphaEasing() {
	// イージング中でない場合は何もしない
	if (!isEasingIn_ && !isEasingOut_) {
		return;
	}

	// 実時間のデルタタイムを使用（ゲーム時間に影響されない）
	float deltaTime = gameTimer_->GetRealDeltaTime();
	easingTimer_ += deltaTime;

	// イージング進行率を計算（0.0～1.0）
	float t = easingTimer_ / easingDuration_;
	if (t > 1.0f) {
		t = 1.0f;
	}

	// EaseInCubicを適用
	float easedT = EaseInCubic(t);

	if (isEasingIn_) {
		// フェードイン
		currentBackgroundAlpha_ = easedT * targetBackgroundAlpha_;
		currentTextAlpha_ = easedT * targetTextAlpha_;

		// イージング完了チェック
		if (t >= 1.0f) {
			isEasingIn_ = false;
			easingTimer_ = 0.0f;
		}
	} else if (isEasingOut_) {
		// フェードアウト
		currentBackgroundAlpha_ = (1.0f - easedT) * 1.0f;
		//背景色の最大値は0.8
		if (currentBackgroundAlpha_ > 0.8f) {
			currentBackgroundAlpha_ = 0.8f;
		};

		currentTextAlpha_ = (1.0f - easedT) * 1.0f;

		// イージング完了チェック
		if (t >= 1.0f) {
			isEasingOut_ = false;
			easingTimer_ = 0.0f;
			// ポーズ解除が完了したらGameTimerを再開
			gameTimer_->Resume();
		}
	}

	// Spriteの色に反映
	backgroundSprite_->SetColor({ 0.0f, 0.0f, 0.0f, currentBackgroundAlpha_ });
	resumeTextSprite_->SetColor({ resumeColor_.x, resumeColor_.y, resumeColor_.z, currentTextAlpha_ });
	titleTextSprite_->SetColor({ titleColor_.x, titleColor_.y, titleColor_.z, currentTextAlpha_ });
}

void Pause::UpdateSelectColorEasing() {
	// ポーズ中でない場合は何もしない
	if (!isPaused_) {
		return;
	}

	// 実時間のデルタタイムを使用
	float deltaTime = gameTimer_->GetRealDeltaTime();
	selectColorTimer_ += deltaTime;

	// イージング進行率を計算
	float t = selectColorTimer_ / selectColorDuration_;
	if (t > 1.0f) {
		t = 1.0f;
	}

	// EaseInCubicを適用
	float easedT = EaseInCubic(t);

	// セレクトモードに応じて色を補間
	if (selectMode_ == PauseSelectMode::Resume) {
		// "ゲーム再開"が選択中
		resumeColor_.x = unselectedColor_.x + (selectedColor_.x - unselectedColor_.x) * easedT;
		resumeColor_.y = unselectedColor_.y + (selectedColor_.y - unselectedColor_.y) * easedT;
		resumeColor_.z = unselectedColor_.z + (selectedColor_.z - unselectedColor_.z) * easedT;

		// "タイトルへ"は非選択
		titleColor_.x = selectedColor_.x + (unselectedColor_.x - selectedColor_.x) * easedT;
		titleColor_.y = selectedColor_.y + (unselectedColor_.y - selectedColor_.y) * easedT;
		titleColor_.z = selectedColor_.z + (unselectedColor_.z - selectedColor_.z) * easedT;
	} else {
		// "タイトルへ"が選択中
		titleColor_.x = unselectedColor_.x + (selectedColor_.x - unselectedColor_.x) * easedT;
		titleColor_.y = unselectedColor_.y + (selectedColor_.y - unselectedColor_.y) * easedT;
		titleColor_.z = unselectedColor_.z + (selectedColor_.z - unselectedColor_.z) * easedT;

		// "ゲーム再開"は非選択
		resumeColor_.x = selectedColor_.x + (unselectedColor_.x - selectedColor_.x) * easedT;
		resumeColor_.y = selectedColor_.y + (unselectedColor_.y - selectedColor_.y) * easedT;
		resumeColor_.z = selectedColor_.z + (unselectedColor_.z - selectedColor_.z) * easedT;
	}
}

float Pause::EaseInCubic(float x) {
	return x * x * x;
}

float Pause::EaseOutBounce(float x) {
	const float n1 = 7.5625f;
	const float d1 = 2.75f;

	if (x < 1.0f / d1) {
		return n1 * x * x;
	} else if (x < 2.0f / d1) {
		x -= 1.5f / d1;
		return n1 * x * x + 0.75f;
	} else if (x < 2.5f / d1) {
		x -= 2.25f / d1;
		return n1 * x * x + 0.9375f;
	} else {
		x -= 2.625f / d1;
		return n1 * x * x + 0.984375f;
	}
}

void Pause::UpdateSelectSizeEasing() {
	// ポーズ中でない場合は何もしない
	if (!isPaused_) {
		return;
	}

	// 実時間のデルタタイムを使用
	float deltaTime = gameTimer_->GetRealDeltaTime();
	selectSizeTimer_ += deltaTime;

	// イージング進行率を計算
	float t = selectSizeTimer_ / selectSizeDuration_;
	if (t > 1.0f) {
		t = 1.0f;
	}

	// EaseOutBounceを適用
	float easedT = EaseOutBounce(t);

	// セレクトモードに応じてサイズを補間
	if (selectMode_ == PauseSelectMode::Resume) {
		// "ゲーム再開"が選択中 → 大きくなる
		float resumeScale = unselectedScale_ + (selectedScale_ - unselectedScale_) * easedT;
		currentResumeSize_ = resumeTextSize_ * resumeScale;

		// "タイトルへ"は非選択 → 通常サイズに戻る
		float titleScale = selectedScale_ + (unselectedScale_ - selectedScale_) * easedT;
		currentTitleSize_ = titleTextSize_ * titleScale;
	} else {
		// "タイトルへ"が選択中 → 大きくなる
		float titleScale = unselectedScale_ + (selectedScale_ - unselectedScale_) * easedT;
		currentTitleSize_ = titleTextSize_ * titleScale;

		// "ゲーム再開"は非選択 → 通常サイズに戻る
		float resumeScale = selectedScale_ + (unselectedScale_ - selectedScale_) * easedT;
		currentResumeSize_ = resumeTextSize_ * resumeScale;
	}
}