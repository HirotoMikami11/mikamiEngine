#include "DamageVignette.h"
#include "ImGui/ImGuiManager.h"
#include <algorithm>

void DamageVignette::Initialize() {
	vignetteEffect_ = Engine::GetInstance()->GetOffscreenRenderer()->GetDamageEffect();

	// 初期パラメータを設定
	SetupVignetteParameters();

	Logger::Log(Logger::GetStream(), "DamageVignette initialized successfully!\n");
}

void DamageVignette::Update(float deltaTime) {
	if (!isPlaying_ || !vignetteEffect_) {
		return;
	}

	// 時間を更新
	currentTime_ += deltaTime * easingSpeed_;

	// エフェクト時間の正規化 (0.0f～1.0f)
	float normalizedTime = currentTime_ / duration_;

	if (normalizedTime >= 1.0f) {
		// エフェクト終了
		vignetteEffect_->SetVignetteStrength(0.0f);
		vignetteEffect_->SetEnabled(false);
		isPlaying_ = false;
		currentTime_ = 0.0f;
		return;
	}

	// イージング計算
	float easedValue;
	if (normalizedTime <= 0.5f) {
		// 前半：0→1
		float t = normalizedTime * 2.0f;
		easedValue = EaseInOutQuad(t);
	} else {
		// 後半：1→0
		float t = (normalizedTime - 0.5f) * 2.0f;
		easedValue = 1.0f - EaseInOutQuad(t);
	}

	// 強度を設定（0.0f～targetStrength）
	float currentStrength = easedValue * params_.targetStrength;
	vignetteEffect_->SetVignetteStrength(currentStrength);
}

void DamageVignette::TriggerDamageEffect() {
	if (!vignetteEffect_) {
		return;
	}

	// エフェクトを再開始（既に実行中でも強制リセット）
	isPlaying_ = true;
	currentTime_ = 0.0f;

	// ビネットパラメータを設定
	SetupVignetteParameters();

	// エフェクトを有効化
	vignetteEffect_->SetEnabled(true);

	// 初期強度を0に設定
	vignetteEffect_->SetVignetteStrength(0.0f);
}

void DamageVignette::SetupVignetteParameters() {
	if (!vignetteEffect_) {
		return;
	}

	// ダメージエフェクト用のパラメータを設定
	vignetteEffect_->SetVignetteColor(params_.damageColor);
	vignetteEffect_->SetVignetteRadius(params_.radius);
	vignetteEffect_->SetVignetteSoftness(params_.softness);
}

void DamageVignette::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode("Damage Effect")) {
		// エフェクト状態
		ImGui::Text("Is Playing: %s", isPlaying_ ? "YES" : "NO");
		ImGui::Text("Current Time: %.3f", currentTime_);
		ImGui::Text("Progress: %.1f%%", isPlaying_ ? (currentTime_ / duration_) * 100.0f : 0.0f);

		// コントロール
		ImGui::Separator();
		if (ImGui::Button("Trigger Damage Effect")) {
			TriggerDamageEffect();
		}

		// パラメータ調整
		ImGui::Separator();
		ImGui::DragFloat("Duration", &duration_, 0.01f, 0.1f, 5.0f, "%.2f");
		ImGui::DragFloat("Easing Speed", &easingSpeed_, 0.1f, 0.1f, 10.0f, "%.1f");

		// ビネットパラメータ
		if (ImGui::CollapsingHeader("Vignette Parameters")) {
			bool changed = false;
			changed |= ImGui::ColorEdit4("Damage Color", &params_.damageColor.x);
			changed |= ImGui::DragFloat("Target Strength", &params_.targetStrength, 0.01f, 0.0f, 2.0f, "%.2f");
			changed |= ImGui::DragFloat("Radius", &params_.radius, 0.01f, 0.0f, 1.0f, "%.3f");
			changed |= ImGui::DragFloat("Softness", &params_.softness, 0.01f, 0.0f, 1.0f, "%.3f");

			if (changed) {
				SetupVignetteParameters();
			}
		}

		// 現在の強度表示
		if (vignetteEffect_) {
			ImGui::Separator();
			ImGui::Text("Current Strength: %.3f", vignetteEffect_->GetVignetteStrength());
			ImGui::Text("Vignette Enabled: %s", vignetteEffect_->IsEnabled() ? "YES" : "NO");
		}

		ImGui::TreePop();
	}
#endif
}
