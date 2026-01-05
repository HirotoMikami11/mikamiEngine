#include "TitleFieldSegment.h"
#include "ImGui/ImGuiManager.h"

TitleFieldSegment::TitleFieldSegment() = default;
TitleFieldSegment::~TitleFieldSegment() = default;

void TitleFieldSegment::Initialize(DirectXCommon* dxCommon) {
	ground_ = std::make_unique<TitleGround>();
	ground_->Initialize(dxCommon, { 0.0f, -0.51f, 0.0f });

	skyGround_ = std::make_unique<TitleGround>();
	skyGround_->Initialize(dxCommon, { 0.0f, 17.0f, 0.0f });

	titleWall_ = std::make_unique<TitleWall>();
	titleWall_->Initialize(dxCommon);

}

void TitleFieldSegment::Update(const Matrix4x4& viewProjectionMatrix) {
	ground_->Update(viewProjectionMatrix);
	skyGround_->Update(viewProjectionMatrix);
	titleWall_->Update(viewProjectionMatrix);

}

void TitleFieldSegment::Draw() {
	ground_->Draw();
	skyGround_->Draw();
	titleWall_->Draw();
}

void TitleFieldSegment::ImGui() {
#ifdef USEIMGUI

	ImGui::Spacing();
	ImGui::Text("Ground");
	ground_->ImGui();

	ImGui::Spacing();
	ImGui::Text("TitleWall");
	titleWall_->ImGui();

	ImGui::Spacing();
#endif
}

void TitleFieldSegment::SetZOffset(float zOffset) {
	zOffset_ = zOffset;
	
	// Ground のオフセットを設定
	if (ground_) {
		ground_->SetZOffset(zOffset);
	}
	// SkyGround のオフセットを設定
	if (skyGround_) {
		skyGround_->SetZOffset(zOffset);
	}

	// Wall のオフセットを設定
	if (titleWall_) {
		titleWall_->SetZOffset(zOffset);
	}
}
