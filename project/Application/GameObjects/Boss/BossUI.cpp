#include "BossUI.h"
#include "Managers/ImGui/ImGuiManager.h"

void BossUI::Initialize(DirectXCommon* dxCommon) {
	directXCommon_ = dxCommon;
	InitializeGauges();

}

void BossUI::Update(float currentHP, float maxHP, const Matrix4x4& viewProjectionMatrixSprite) {
	UpdateGauges(currentHP, maxHP);

	// 全スプライトの更新
	if (hpGaugeBar_) hpGaugeBar_->Update(viewProjectionMatrixSprite);
	if (hpGaugeFill_) hpGaugeFill_->Update(viewProjectionMatrixSprite);
	if (hpGaugeLight_) hpGaugeLight_->Update(viewProjectionMatrixSprite);

}

void BossUI::Draw() {
	// HP/ENゲージの描画
	if (hpGaugeBar_) hpGaugeBar_->Draw();
	if (hpGaugeFill_) hpGaugeFill_->Draw();
	if (hpGaugeLight_) hpGaugeLight_->Draw();
}

void BossUI::InitializeGauges() {

	//パラメータ
	frameOffset_ = { 5.0f,5.0f };
	gaugeSize_ = { 900.0f,50.0f };
	hpGaugePosition_ = { GraphicsConfig::kClientWidth / 2 - gaugeSize_.x / 2, 50.0f };
	gaugeFrameSize_ = gaugeSize_ + frameOffset_;
	LightScrollSpeed_ = 0.005f;

	// HPゲージの枠
	hpGaugeBar_ = std::make_unique<Sprite>();
	hpGaugeBar_->Initialize(
		directXCommon_,
		"white",
		hpGaugePosition_,
		gaugeFrameSize_,
		{ 0.0f, 0.5f }
	);
	hpGaugeBar_->SetColor(backgroundColor_);

	// HPゲージの中身
	hpGaugeFill_ = std::make_unique<Sprite>();
	hpGaugeFill_->Initialize(
		directXCommon_,
		"white",
		{ hpGaugePosition_.x + 2.0f, hpGaugePosition_.y },
		gaugeSize_,
		{ 0.0f, 0.5f }
	);
	hpGaugeFill_->SetColor(hpNormalColor_);

	hpGaugeLight_ = std::make_unique<Sprite>();
	hpGaugeLight_->Initialize(
		directXCommon_,
		"gaugeLight",
		{ hpGaugePosition_.x + 2.0f, hpGaugePosition_.y },
		gaugeSize_,
		{ 0.0f, 0.5f }
	);

}

void BossUI::UpdateGauges(float currentHP, float maxHP)
{
	float hpRatio = (currentHP > 0.0f) ? currentHP / maxHP : 0.0f;
	hpRatio = std::clamp(hpRatio, 0.0f, 1.0f); // 0～1に制限

	if (hpGaugeFill_) {
		Vector2 currentSize = hpGaugeFill_->GetScale();
		currentSize.x = gaugeSize_.x * hpRatio;
		hpGaugeFill_->SetScale(currentSize);
	}

	if (hpGaugeLight_) {

		Vector2 currentSize = hpGaugeLight_->GetScale();
		currentSize.x = gaugeSize_.x * hpRatio;
		hpGaugeLight_->SetScale(currentSize);

		Vector2 uvTransform = hpGaugeLight_->GetUVTransformTranslate();
		uvTransform.x += LightScrollSpeed_;
		hpGaugeLight_->SetUVTransformTranslate(uvTransform);

	}


}

void BossUI::SetGaugePosition(const Vector2& hpPosition) {
	hpGaugePosition_ = hpPosition;
	// 枠の左側をoffset/2だけ左にずらす
	Vector2 barPosition = { hpGaugePosition_.x - frameOffset_.x / 2.0f, hpGaugePosition_.y };
	if (hpGaugeBar_) hpGaugeBar_->SetPosition(barPosition);
	if (hpGaugeFill_) hpGaugeFill_->SetPosition(hpGaugePosition_);
}

void BossUI::SetGaugeSize(const Vector2& size, const Vector2 frameOffset) {
	gaugeSize_ = size;
	frameOffset_ = frameOffset;
	gaugeFrameSize_ = size + frameOffset;
	if (hpGaugeBar_) {
		hpGaugeBar_->SetScale(gaugeFrameSize_);
		// 枠の左側をoffset/2だけ左にずらす
		Vector2 barPosition = { hpGaugePosition_.x - frameOffset_.x / 2.0f, hpGaugePosition_.y };
		hpGaugeBar_->SetPosition(barPosition);
	}
	if (hpGaugeFill_) hpGaugeFill_->SetScale(gaugeSize_);
}


void BossUI::SetGaugeColors(const Vector4& hpColor, const Vector4& backgroundColor) {
	hpNormalColor_ = hpColor;
	backgroundColor_ = backgroundColor;

	// 色を更新
	if (hpGaugeBar_) hpGaugeBar_->SetColor(backgroundColor_);
	if (hpGaugeFill_) hpGaugeFill_->SetColor(hpNormalColor_);
}

void BossUI::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode("Boss UI")) {
		// ゲージ位置設定
		if (ImGui::DragFloat2("HP Gauge Position", &hpGaugePosition_.x, 1.0f, 0.0f, 1280.0f)) {
			SetGaugePosition(hpGaugePosition_);
		}

		// ゲージサイズ設定
		if (ImGui::DragFloat2("Gauge Size", &gaugeSize_.x, 1.0f, 10.0f, 4000.0f)) {
			SetGaugeSize(gaugeSize_, frameOffset_);
		}

		if (ImGui::DragFloat2("Frame Offset", &frameOffset_.x, 0.1f, 10.0f, 50.0f)) {
			SetGaugeSize(gaugeSize_, frameOffset_);
		}

		// 色設定
		ImGui::Separator();
		ImGui::ColorEdit4("HP Normal Color", &hpNormalColor_.x);
		ImGui::ColorEdit4("Background Color", &backgroundColor_.x);
		ImGui::TreePop();
	}
#endif
}