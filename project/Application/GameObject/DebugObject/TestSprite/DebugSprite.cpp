#include "DebugSprite.h"
#include <algorithm>
#include "CameraController.h"
#include "Engine.h"
#include "ImGui/ImGuiManager.h"

void DebugSprite::Initialize()
{
	SetTag(ObjectTag::UI);

	dxCommon_ = Engine::GetInstance()->GetDirectXCommon();
	needsRegenerate_ = true;
	GenerateSprites();
}

void DebugSprite::Update()
{
	if (needsRegenerate_) {
		GenerateSprites();
	}

	spriteViewProjectionMatrix_ = CameraController::GetInstance()->GetViewProjectionMatrixSprite();
	for (auto& sprite : sprites_) {
		sprite->Update(spriteViewProjectionMatrix_);
	}
}

void DebugSprite::Draw()
{
	for (auto& sprite : sprites_) {
		sprite->Draw();
	}
}

void DebugSprite::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode("DebugSprite")) {
		bool changed = false;
		changed |= ImGui::DragInt("Columns", &columns_, 1.0f, 1, 1000);
		changed |= ImGui::DragInt("Rows", &rows_, 1.0f, 1, 1000);
		changed |= ImGui::DragFloat2("LeftTop Position", &leftTopPosition_.x, 1.0f, 0.0f, 10000.0f);
		changed |= ImGui::DragFloat2("Sprite Size", &spriteSize_.x, 0.1f, 1.0f, 512.0f);
		changed |= ImGui::DragFloat2("Offset", &spacing_.x, 0.1f, 0.0f, 512.0f);

		if (changed) {
			needsRegenerate_ = true;
		}

		if (ImGui::Button("Generate")) {
			needsRegenerate_ = true;
		}

		ImGui::Text("Sprite Count: %zu", sprites_.size());
		ImGui::TreePop();
	}
#endif
}

void DebugSprite::GenerateSprites()
{
	sprites_.clear();

	if (!dxCommon_) {
		return;
	}

	columns_ = (columns_ < 1) ? 1 : columns_;
	rows_ = (rows_ < 1) ? 1 : rows_;

	const float stepX = (spriteSize_.x + spacing_.x < 1.0f) ? 1.0f : (spriteSize_.x + spacing_.x);
	const float stepY = (spriteSize_.y + spacing_.y < 1.0f) ? 1.0f : (spriteSize_.y + spacing_.y);

	const float startX = leftTopPosition_.x;
	const float startY = leftTopPosition_.y;

	const int total = columns_ * rows_;
	sprites_.reserve(static_cast<size_t>(total));

	for (int y = 0; y < rows_; ++y) {
		for (int x = 0; x < columns_; ++x) {
			auto sprite = std::make_unique<Sprite>();
			const Vector2 center = {
				startX + static_cast<float>(x) * stepX,
				startY + static_cast<float>(y) * stepY
			};

			sprite->Initialize(dxCommon_, "white", center, spriteSize_);
			sprite->SetLayerOrder(LayerOrder::UI);
			sprite->SetRenderGroup(RenderGroup::AlphaBlend);

			sprites_.push_back(std::move(sprite));
		}
	}

	needsRegenerate_ = false;
}
