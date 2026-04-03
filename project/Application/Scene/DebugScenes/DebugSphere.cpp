#include "DebugSphere.h"
#include "ImGui/ImGuiManager.h"

DebugSphere::DebugSphere()
	: directXCommon_(nullptr)
{
}

void DebugSphere::Initialize(DirectXCommon* dxCommon)
{
	directXCommon_ = dxCommon;
	GenerateSpheres();
}

void DebugSphere::GenerateSpheres()
{
	spheres_.clear();

	int rows = (sphereCount_ + sphereCountPerRow_ - 1) / sphereCountPerRow_;

	for (int i = 0; i < sphereCount_; ++i)
	{
		int row = i / sphereCountPerRow_;
		int col = i % sphereCountPerRow_;

		float offsetX = (sphereCountPerRow_ - 1) * sphereSpacing_ * 0.5f;
		float offsetZ = (rows - 1) * sphereSpacing_ * 0.5f;

		Vector3Transform transform{
			{1.0f, 1.0f, 1.0f},
			{0.0f, 0.0f, 0.0f},
			{col * sphereSpacing_ - offsetX, 0.0f, row * sphereSpacing_ - offsetZ}
		};

		auto sphere = std::make_unique<Sphere>();
		sphere->Initialize(directXCommon_, "sphere", "monsterBall");
		sphere->SetTransform(transform);

		spheres_.push_back(std::move(sphere));
	}
}

void DebugSphere::Regenerate()
{
	GenerateSpheres();
}

void DebugSphere::Update(const Matrix4x4& viewProj)
{
	// 自動回転
	if (autoRotate_) {
		for (auto& s : spheres_) {
			s->AddRotation({ 0.0f, 0.5f, 0.0f });
			s->AddPosition({0.001f,0.0f,0.0f});
		}
	}

	for (auto& s : spheres_) {
		s->Update(viewProj);
	}
}

void DebugSphere::Draw()
{
	for (auto& s : spheres_) {
		s->Draw();
	}
}

void DebugSphere::ImGui()
{
#ifdef USEIMGUI
	ImGui::Text("デバッグ表示最大数確認");
	ImGui::Text("現在の球体数: %d", (int)spheres_.size());

	bool regen = false;

	if (ImGui::DragInt("球体数", &sphereCount_, 1, 1, 10000)) regen = true;
	if (ImGui::DragInt("列", &sphereCountPerRow_, 1, 1, 200)) regen = true;
	if (ImGui::DragFloat("空白", &sphereSpacing_, 0.1f, 0.2f, 50.0f)) regen = true;

	ImGui::Checkbox("回転モード", &autoRotate_);

	if (ImGui::Button("再生成") || regen) {
		Regenerate();
	}
#endif
}
