#include "GroundLight.h"
#include "ImGui/ImGuiManager.h"
#include "GameTimer.h"

GroundLight::GroundLight() = default;

GroundLight::~GroundLight() {
	// スポットライトを削除
	if (lightManager_ && spotLight_) {
		lightManager_->RemoveSpotLight(spotLight_);
		spotLight_ = nullptr;
	}
}

void GroundLight::Initialize(LightManager* lightManager) {
	lightManager_ = lightManager;

	// スポットライトを作成
	spotLight_ = lightManager_->AddSpotLight(
		{ 0.0f, -109.2f, 0.0f },		// 座標
		{ -90.0f, 0.0f, 0.0f },			// 真上を向く
		{ 1.0f, 0.3f, 0.0f, 1.0f },		// 色
		20.0f,							// 強度
		110.0f,							// 最大距離（初期値）
		0.1f,							// 減衰率
		25.0f,							// スポット角度
		0.0f							// フォールオフ開始角度
	);

	// イージングタイマーを初期化
	easingTimer_ = 0.0f;
	isReversing_ = false;
}

void GroundLight::Update() {
	if (!spotLight_) return;

	// タイマーを進める
	easingTimer_ += GameTimer::GetInstance().GetDeltaTime();

	// 正規化された時間（0.0～1.0）
	float normalizedTime = easingTimer_ / easingDuration_;

	// 時間が経過したら方向を反転
	if (normalizedTime >= 1.0f) {
		isReversing_ = !isReversing_;
		easingTimer_ = 0.0f;
		normalizedTime = 0.0f;
	}


	float easedTime = EaseInOutSine(normalizedTime);

	// 距離を計算
	float currentDistance;
	float currentColorG;
	if (isReversing_) {
		currentDistance = Lerp(maxDistance_, minDistance_, easedTime);
		currentColorG = Lerp(maxColorG_, minColorG_, easedTime);
	} else {
		currentDistance = Lerp(minDistance_, maxDistance_, easedTime);
		currentColorG = Lerp(minColorG_, maxColorG_, easedTime);
	}

	// スポットライトの距離を更新
	spotLight_->SetDistance(currentDistance);
	// スポットライトの色を更新
	spotLight_->SetColor({ 1.0f, currentColorG, 0.0f, 1.0f });
}

void GroundLight::SetDistanceRange(float minDistance, float maxDistance) {
	minDistance_ = minDistance;
	maxDistance_ = maxDistance;
}

void GroundLight::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode("Ground Light")) {
		// イージングパラメータ
		if (ImGui::DragFloat("Easing Duration (sec)", &easingDuration_, 0.1f, 0.1f, 10.0f)) {
			// 変更時は現在のサイクルをリセット
			easingTimer_ = 0.0f;
		}

		ImGui::DragFloat("Min Distance", &minDistance_, 0.1f, 50.0f, 200.0f);
		ImGui::DragFloat("Max Distance", &maxDistance_, 0.1f, 50.0f, 200.0f);

		// 現在の状態表示
		ImGui::Separator();
		ImGui::Text("Current Timer: %.2f / %.2f", easingTimer_, easingDuration_);
		ImGui::Text("Direction: %s", isReversing_ ? "Max -> Min" : "Min -> Max");

		if (spotLight_) {
			ImGui::Text("Current Distance: %.2f", spotLight_->GetDistance());
		}

		// スポットライトのUI（詳細設定用）
		if (spotLight_) {
			ImGui::Separator();
			spotLight_->ImGui("Spot Light Settings");
		}

		ImGui::TreePop();
	}
#endif
}

