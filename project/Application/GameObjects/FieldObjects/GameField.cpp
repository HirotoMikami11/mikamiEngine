#include "GameField.h"
#include "ImGui/ImGuiManager.h"

GameField::GameField() = default;
GameField::~GameField() = default;

void GameField::Initialize(DirectXCommon* dxCommon) {
	ground_ = std::make_unique<Ground>();
	ground_->Initialize(dxCommon, { 0.0f, -0.51f, 0.0f });

	wall_ = std::make_unique<Wall>();
	wall_->Initialize(dxCommon);

	torch_ = std::make_unique<Torch>();
	torch_->Initialize(dxCommon);

	groundLight_ = std::make_unique<GroundLight>();
	groundLight_->Initialize(LightManager::GetInstance());
}

void GameField::Update(const Matrix4x4& viewProjectionMatrix) {
	ground_->Update(viewProjectionMatrix);
	wall_->Update(viewProjectionMatrix);
	torch_->Update(viewProjectionMatrix);
	groundLight_->Update();
}

void GameField::Draw() {
	ground_->Draw();
	wall_->Draw();
	torch_->Draw();
}

void GameField::ImGui() {
#ifdef USEIMGUI

	ImGui::Spacing();
	ImGui::Text("Ground");
	ground_->ImGui();

	ImGui::Spacing();
	ImGui::Text("Wall");
	wall_->ImGui();

	ImGui::Spacing();
	ImGui::Text("Torch");
	torch_->ImGui();

	ImGui::Spacing();
	groundLight_->ImGui();

	ImGui::Spacing();
#endif
}
