#include "CameraShake.h"
#include "Random/Random.h"
#include "ImGui/ImGuiManager.h"
#include <numbers>

void CameraShake::StartShake(float duration, float amplitude) {
	shakeDuration_ = duration;
	shakeTimer_ = 0.0f;
	shakeAmplitude_ = amplitude;
	shakeFrequency_ = 10.0f;
	currentPattern_ = ShakePattern::Simple;
}

void CameraShake::StartMultiShake(float duration, float amplitude, float frequency) {
	shakeDuration_ = duration;
	shakeTimer_ = 0.0f;
	shakeAmplitude_ = amplitude;
	shakeFrequency_ = frequency;
	currentPattern_ = ShakePattern::Multi;
}

void CameraShake::StopShake() {
	shakeTimer_ = shakeDuration_;
	shakeOffset_ = { 0.0f, 0.0f, 0.0f };
}

void CameraShake::Update(float deltaTime) {
	if (!IsShaking()) {
		shakeOffset_ = { 0.0f, 0.0f, 0.0f };
		return;
	}

	switch (currentPattern_) {
	case ShakePattern::Simple:
		UpdateSimpleShake(deltaTime);
		break;
	case ShakePattern::Multi:
		UpdateMultiShake(deltaTime);
		break;
	}

	shakeTimer_ += deltaTime;
}

void CameraShake::UpdateSimpleShake(float deltaTime) {
	// 進行度（0.0f～1.0f）
	float progress = shakeTimer_ / shakeDuration_;

	// 減衰カーブ（指数関数的減衰）
	float decay = std::exp(-progress * 3.0f);
	float currentAmplitude = shakeAmplitude_ * decay;

	// Randomクラスでランダムなオフセット生成
	shakeOffset_.x = Random::GetInstance().GenerateFloat(-1.0f, 1.0f) * currentAmplitude;
	shakeOffset_.y = Random::GetInstance().GenerateFloat(-1.0f, 1.0f) * currentAmplitude;
	shakeOffset_.z = Random::GetInstance().GenerateFloat(-0.5f, 0.5f) * currentAmplitude; // Z軸は控えめ
}

void CameraShake::UpdateMultiShake(float deltaTime) {
	// 進行度（0.0f～1.0f）
	float progress = shakeTimer_ / shakeDuration_;

	// 周期的な強度変化 + 全体的な減衰
	float wave = std::sin(shakeTimer_ * shakeFrequency_ * 2.0f * std::numbers::pi_v<float>);
	float decay = 1.0f - (progress * progress); // 二次関数的減衰
	float currentAmplitude = shakeAmplitude_ * std::abs(wave) * decay;

	// ランダムなオフセット
	shakeOffset_.x = Random::GetInstance().GenerateFloat(-1.0f, 1.0f) * currentAmplitude;
	shakeOffset_.y = Random::GetInstance().GenerateFloat(-1.0f, 1.0f) * currentAmplitude;
	shakeOffset_.z = Random::GetInstance().GenerateFloat(-0.3f, 0.3f) * currentAmplitude;
}

Vector3 CameraShake::GetOffset() const {
	return shakeOffset_;
}

bool CameraShake::IsShaking() const {
	return shakeTimer_ < shakeDuration_;
}

void CameraShake::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode("カメラシェイク機能")) {
		ImGui::Text("ステータス: %s", IsShaking() ? "シェイク中" : "停止中");
		ImGui::Text("タイマー: %.2f / %.2f", shakeTimer_, shakeDuration_);
		ImGui::Text("振動幅: %.3f", shakeAmplitude_);
		ImGui::Text("オフセット: (%.3f, %.3f, %.3f)",
			shakeOffset_.x, shakeOffset_.y, shakeOffset_.z);
		// テスト用ボタン
		if (ImGui::Button("シェイク")) {
			StartShake(1.0f, 0.5f);
		}
		ImGui::SameLine();
		if (ImGui::Button("強めのシェイク")) {
			StartMultiShake(2.0f, 0.8f, 12.0f);
		}
		ImGui::SameLine();
		if (ImGui::Button("シェイク停止")) {
			StopShake();
		}
		ImGui::TreePop();
	}
#endif
}
