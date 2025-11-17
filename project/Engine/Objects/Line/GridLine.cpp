#define NOMINMAX // C++標準のstd::maxを使えるようにするため(windows.hが上書きしてしまっている)
#include "GridLine.h"
#include "ImGui/ImGuiManager.h"
#include <cmath>

void GridLine::Initialize(
	DirectXCommon* dxCommon,
	const GridLineType& gridType,
	float size,
	float interval,
	float majorInterval,
	const Vector4& normalColor,
	const Vector4& majorColor)
{
	directXCommon_ = dxCommon;

	// DebugDrawLineSystemを取得
	debugDrawLineSystem_ = DebugDrawLineSystem::GetInstance();
	

	// グリッド設定を保存
	SetGridSettings(gridType, size, interval, majorInterval, normalColor, majorColor);
}

void GridLine::SetGridSettings(
	const GridLineType& gridType,
	float size,
	float interval,
	float majorInterval,
	const Vector4& normalColor,
	const Vector4& majorColor)
{
	gridType_ = gridType;
	gridSize_ = size;
	gridInterval_ = interval;
	gridMajorInterval_ = majorInterval;
	gridNormalColor_ = normalColor;
	gridMajorColor_ = majorColor;
}

void GridLine::Draw()
{
	if (!isVisible_ || !debugDrawLineSystem_) {
		return;
	}

	// グリッドタイプに応じて描画
	switch (gridType_) {
	case GridLineType::XZ:
		DrawXZGrid();
		break;
	case GridLineType::XY:
		DrawXYGrid();
		break;
	case GridLineType::YZ:
		DrawYZGrid();
		break;
	default:
		Logger::Log(Logger::GetStream(), "GridLine: Unknown GridLineType specified.\n");
		return;
	}
}

void GridLine::DrawXZGrid()
{
	float halfSize = gridSize_ * 0.5f;

	// X方向の線（Z軸に沿って）
	for (float x = -halfSize; x <= halfSize; x += gridInterval_) {
		Vector4 color;
		if (std::abs(x) < 0.001f) {
			// 原点のZ軸は青色
			color = { 0.0f, 0.0f, 1.0f, 1.0f };
		} else if (std::fmod(std::abs(x), gridMajorInterval_) < 0.001f) {
			color = gridMajorColor_;
		} else {
			color = gridNormalColor_;
		}

		Vector3 start = { center_.x + x, center_.y, center_.z - halfSize };
		Vector3 end = { center_.x + x, center_.y, center_.z + halfSize };
		debugDrawLineSystem_->AddLine(start, end, color);
	}

	// Z方向の線（X軸に沿って）
	for (float z = -halfSize; z <= halfSize; z += gridInterval_) {
		Vector4 color;
		if (std::abs(z) < 0.001f) {
			// 原点のX軸は赤色
			color = { 1.0f, 0.0f, 0.0f, 1.0f };
		} else if (std::fmod(std::abs(z), gridMajorInterval_) < 0.001f) {
			color = gridMajorColor_;
		} else {
			color = gridNormalColor_;
		}

		Vector3 start = { center_.x - halfSize, center_.y, center_.z + z };
		Vector3 end = { center_.x + halfSize, center_.y, center_.z + z };
		debugDrawLineSystem_->AddLine(start, end, color);
	}
}

void GridLine::DrawXYGrid()
{
	float halfSize = gridSize_ * 0.5f;

	// X方向の線（Y軸に沿って）
	for (float x = -halfSize; x <= halfSize; x += gridInterval_) {
		Vector4 color;
		if (std::abs(x) < 0.001f) {
			// 原点のY軸は緑色
			color = { 0.0f, 1.0f, 0.0f, 1.0f };
		} else if (std::fmod(std::abs(x), gridMajorInterval_) < 0.001f) {
			color = gridMajorColor_;
		} else {
			color = gridNormalColor_;
		}

		Vector3 start = { center_.x + x, center_.y - halfSize, center_.z };
		Vector3 end = { center_.x + x, center_.y + halfSize, center_.z };
		debugDrawLineSystem_->AddLine(start, end, color);
	}

	// Y方向の線（X軸に沿って）
	for (float y = -halfSize; y <= halfSize; y += gridInterval_) {
		Vector4 color;
		if (std::abs(y) < 0.001f) {
			// 原点のX軸は赤色
			color = { 1.0f, 0.0f, 0.0f, 1.0f };
		} else if (std::fmod(std::abs(y), gridMajorInterval_) < 0.001f) {
			color = gridMajorColor_;
		} else {
			color = gridNormalColor_;
		}

		Vector3 start = { center_.x - halfSize, center_.y + y, center_.z };
		Vector3 end = { center_.x + halfSize, center_.y + y, center_.z };
		debugDrawLineSystem_->AddLine(start, end, color);
	}
}

void GridLine::DrawYZGrid()
{
	float halfSize = gridSize_ * 0.5f;

	// Y方向の線（Z軸に沿って）X=0
	for (float y = -halfSize; y <= halfSize; y += gridInterval_) {
		Vector4 color;
		if (std::abs(y) < 0.001f) {
			// 原点のZ軸は青色
			color = { 0.0f, 0.0f, 1.0f, 1.0f };
		} else if (std::fmod(std::abs(y), gridMajorInterval_) < 0.001f) {
			color = gridMajorColor_;
		} else {
			color = gridNormalColor_;
		}

		Vector3 start = { center_.x, center_.y + y, center_.z - halfSize };
		Vector3 end = { center_.x, center_.y + y, center_.z + halfSize };
		debugDrawLineSystem_->AddLine(start, end, color);
	}

	// Z方向の線（Y軸に沿って）X=0
	for (float z = -halfSize; z <= halfSize; z += gridInterval_) {
		Vector4 color;
		if (std::abs(z) < 0.001f) {
			// 原点のY軸は緑色
			color = { 0.0f, 1.0f, 0.0f, 1.0f };
		} else if (std::fmod(std::abs(z), gridMajorInterval_) < 0.001f) {
			color = gridMajorColor_;
		} else {
			color = gridNormalColor_;
		}

		Vector3 start = { center_.x, center_.y - halfSize, center_.z + z };
		Vector3 end = { center_.x, center_.y + halfSize, center_.z + z };
		debugDrawLineSystem_->AddLine(start, end, color);
	}
}

void GridLine::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode(name_.c_str())) {
		// 基本設定
		ImGui::Checkbox("Visible", &isVisible_);

		// 中心位置
		if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::DragFloat3("Center", &center_.x, 0.1f);
		}

		// グリッド設定
		if (ImGui::CollapsingHeader("Grid Settings")) {
			bool gridChanged = false;

			// グリッドタイプ選択
			const char* gridTypes[] = { "XZ Plane", "XY Plane", "YZ Plane" };
			int currentType = static_cast<int>(gridType_);
			if (ImGui::Combo("Grid Type", &currentType, gridTypes, 3)) {
				gridType_ = static_cast<GridLineType>(currentType);
				gridChanged = true;
			}

			if (ImGui::DragFloat("Grid Size", &gridSize_, 1.0f, 10.0f, 200.0f)) {
				gridChanged = true;
			}

			if (ImGui::DragFloat("Grid Interval", &gridInterval_, 0.1f, 0.1f, 5.0f)) {
				gridChanged = true;
			}

			if (ImGui::DragFloat("Major Interval", &gridMajorInterval_, 1.0f, 2.0f, 50.0f)) {
				gridChanged = true;
			}

			if (ImGui::ColorEdit4("Normal Color", &gridNormalColor_.x)) {
				gridChanged = true;
			}

			if (ImGui::ColorEdit4("Major Color", &gridMajorColor_.x)) {
				gridChanged = true;
			}

			// 変更があった場合は設定を再適用（次フレームで反映）
			if (gridChanged) {
				ImGui::Text("Settings will be applied on next frame");
			}
		}

		ImGui::TreePop();
	}
#endif
}