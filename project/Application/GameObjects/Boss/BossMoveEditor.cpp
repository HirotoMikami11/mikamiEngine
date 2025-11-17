#include "BossMoveEditor.h"
#include "Logger.h"
#include "ImGui/ImGuiManager.h"
#include <format>
#include <cstring>

void BossMoveEditor::Initialize(BossSplineTrack* track, BossSplineMovement* movement, BossSplineDebugger* debugger) {
	track_ = track;
	movement_ = movement;
	debugger_ = debugger;

	// デフォルトの制御点を作成
	CreateDefaultPoints();

	Logger::Log("BossMoveEditor: Initialized\n");
}

void BossMoveEditor::Update() {
	// プレビュー再生中の場合、進行度を更新
	if (isPreviewPlaying_ && movement_) {
		previewProgress_ += 0.001f;  // ゆっくり進める
		if (previewProgress_ >= 1.0f) {
			previewProgress_ = 0.0f;  // ループ
		}
		movement_->SetProgress(previewProgress_);
	}
}

void BossMoveEditor::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode("Boss")) {

		// メイン情報
		ImGui::Text("Boss Spline Control Point Editor");
		ImGui::Separator();

		// サブセクション
		ShowMainControls();
		ImGui::Spacing();

		ShowControlPointsList();
		ImGui::Spacing();

		ShowFileOperations();
		ImGui::Spacing();

		ShowMovementControls();
		ImGui::Spacing();

		ShowDebugControls();

		ImGui::TreePop();
	}
#endif
}

bool BossMoveEditor::LoadFromCSV(const std::string& filename) {
	std::vector<Vector3> loadedPoints;

	if (!CSVUtility::LoadVector3List(filename, loadedPoints)) {
		return false;
	}

	// 制御点が4点未満の場合はエラー
	if (loadedPoints.size() < 4) {
		Logger::Log(std::format("BossMoveEditor: Invalid point count (need at least 4, got {})\n",
			loadedPoints.size()));
		return false;
	}

	// 制御点を更新
	controlPoints_ = loadedPoints;
	ApplyControlPoints();

	Logger::Log(std::format("BossMoveEditor: Successfully loaded {} points from {}\n",
		controlPoints_.size(), filename));

	isDirty_ = false;
	return true;
}

bool BossMoveEditor::SaveToCSV(const std::string& filename) {
	if (controlPoints_.size() < 4) {
		Logger::Log(std::format("BossMoveEditor: Cannot save - need at least 4 points (got {})\n",
			controlPoints_.size()));
		return false;
	}

	if (!CSVUtility::SaveVector3List(filename, controlPoints_)) {
		Logger::Log(std::format("BossMoveEditor: Failed to save to {}\n", filename));
		return false;
	}

	Logger::Log(std::format("BossMoveEditor: Successfully saved {} points to {}\n",
		controlPoints_.size(), filename));

	isDirty_ = false;
	return true;
}

void BossMoveEditor::AddControlPoint(const Vector3& position) {
	controlPoints_.push_back(position);
	ApplyControlPoints();
	MarkDirty();

	Logger::Log(std::format("BossMoveEditor: Added control point at ({:.2f}, {:.2f}, {:.2f})\n",
		position.x, position.y, position.z));
}

void BossMoveEditor::RemoveControlPoint(int index) {
	if (!IsValidIndex(index)) {
		return;
	}

	controlPoints_.erase(controlPoints_.begin() + index);

	// 選択中の制御点を調整
	if (selectedPointIndex_ == index) {
		selectedPointIndex_ = -1;
	} else if (selectedPointIndex_ > index) {
		selectedPointIndex_--;
	}

	ApplyControlPoints();
	MarkDirty();

	Logger::Log(std::format("BossMoveEditor: Removed control point at index {}\n", index));
}

void BossMoveEditor::UpdateControlPoint(int index, const Vector3& newPosition) {
	if (!IsValidIndex(index)) {
		return;
	}

	controlPoints_[index] = newPosition;
	ApplyControlPoints();
	MarkDirty();
}

void BossMoveEditor::SelectControlPoint(int index) {
	if (index < -1 || index >= static_cast<int>(controlPoints_.size())) {
		return;
	}

	selectedPointIndex_ = index;

	// デバッガーにも選択状態を反映
	if (debugger_) {
		debugger_->SetSelectedPointIndex(index);
	}
}

void BossMoveEditor::ApplyControlPoints() {
	if (!track_) {
		return;
	}

	// Trackに制御点を設定
	track_->SetControlPoints(controlPoints_);

	// 等間隔移動が有効な場合、長さテーブルを構築
	if (movement_ && movement_->IsUniformSpeedEnabled() && track_->IsValid()) {
		track_->BuildLengthTable();
	}

	Logger::Log("BossMoveEditor: Applied control points to track\n");
}

void BossMoveEditor::CreateDefaultPoints() {
	controlPoints_.clear();

	// デフォルトの4点を作成（直線的な配置）
	controlPoints_.push_back({ 0.0f, 1.5f, 0.0f });
	controlPoints_.push_back({ 5.0f, 1.5f, 0.0f });
	controlPoints_.push_back({ 10.0f, 1.5f, 0.0f });
	controlPoints_.push_back({ 15.0f, 1.5f, 0.0f });

	ApplyControlPoints();

	Logger::Log("BossMoveEditor: Created default control points\n");
}

void BossMoveEditor::MarkDirty() {
	isDirty_ = true;
}

bool BossMoveEditor::IsValidIndex(int index) const {
	return index >= 0 && index < static_cast<int>(controlPoints_.size());
}

void BossMoveEditor::ShowMainControls() {
#ifdef USEIMGUI
	if (ImGui::CollapsingHeader("Main Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
		// 制御点の数を表示
		ImGui::Text("Control Points: %d", static_cast<int>(controlPoints_.size()));

		// 有効性チェック
		bool isValid = controlPoints_.size() >= 4;
		if (isValid) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Valid");
		} else {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Status: Invalid (need at least 4 points)");
		}

		// 変更フラグ
		if (isDirty_) {
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Unsaved changes");
		}

		ImGui::Separator();

		// 新規制御点の追加
		ImGui::Text("Add New Control Point");
		ImGui::DragFloat3("Position##New", &newPointPosition_.x, 0.1f);

		if (ImGui::Button("Add Point")) {
			AddControlPoint(newPointPosition_);
		}

		ImGui::Separator();

		// デフォルトポイントの作成
		if (ImGui::Button("Create Default Points")) {
			CreateDefaultPoints();
		}

		ImGui::SameLine();

		// 全削除
		if (ImGui::Button("Clear All")) {
			controlPoints_.clear();
			selectedPointIndex_ = -1;
			ApplyControlPoints();
			MarkDirty();
		}
	}
#endif
}

void BossMoveEditor::ShowControlPointsList() {
#ifdef USEIMGUI
	if (ImGui::CollapsingHeader("Control Points List", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (controlPoints_.empty()) {
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No control points");
			return;
		}

		// 各制御点を表示
		for (int i = 0; i < static_cast<int>(controlPoints_.size()); ++i) {
			ImGui::PushID(i);

			// 選択状態の表示
			bool isSelected = (i == selectedPointIndex_);
			if (isSelected) {
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(1.0f, 1.0f, 0.0f, 0.3f));
			}

			// ツリーノードで表示
			std::string label = std::format("Point {} ", i);

			if (ImGui::TreeNode(label.c_str())) {
				// 座標編集
				Vector3 pos = controlPoints_[i];
				if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
					UpdateControlPoint(i, pos);
				}

				// 選択ボタン
				if (ImGui::Button("Select")) {
					SelectControlPoint(i);
				}

				ImGui::SameLine();

				// 削除ボタン
				if (ImGui::Button("Delete")) {
					RemoveControlPoint(i);
					ImGui::TreePop();
					if (isSelected) {
						ImGui::PopStyleColor();
					}
					ImGui::PopID();
					break;
				}

				ImGui::TreePop();
			}

			if (isSelected) {
				ImGui::PopStyleColor();
			}

			ImGui::PopID();
		}
	}
#endif
}

void BossMoveEditor::ShowFileOperations() {
#ifdef USEIMGUI
	if (ImGui::CollapsingHeader("File Operations")) {
		// ファイルパス入力
		ImGui::Text("CSV File Path:");
		if (ImGui::InputText("##CSVPath", csvFilePathBuffer_, sizeof(csvFilePathBuffer_))) {
			csvFilePath_ = csvFilePathBuffer_;
		}

		// 保存ボタン
		if (ImGui::Button("Save to CSV")) {
			if (SaveToCSV(csvFilePath_)) {
				Logger::Log("BossMoveEditor: Save successful\n");
			} else {
				Logger::Log("BossMoveEditor: Save failed\n");
			}
		}

		ImGui::SameLine();

		// 読み込みボタン
		if (ImGui::Button("Load from CSV")) {
			if (LoadFromCSV(csvFilePath_)) {
				Logger::Log("BossMoveEditor: Load successful\n");
			} else {
				Logger::Log("BossMoveEditor: Load failed\n");
			}
		}

	}
#endif
}

void BossMoveEditor::ShowMovementControls() {
#ifdef USEIMGUI
	if (ImGui::CollapsingHeader("Movement Preview")) {
		if (!track_ || !track_->IsValid()) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Invalid track - need at least 4 points");
			return;
		}

		// プレビュー再生
		ImGui::Checkbox("Auto Preview", &isPreviewPlaying_);

		// 進行度スライダー
		if (ImGui::SliderFloat("Progress", &previewProgress_, 0.0f, 1.0f)) {
			if (movement_) {
				movement_->SetProgress(previewProgress_);
			}
		}

		// リセットボタン
		if (ImGui::Button("Reset Progress")) {
			previewProgress_ = 0.0f;
			if (movement_) {
				movement_->SetProgress(0.0f);
			}
		}

		// 現在位置の表示
		if (movement_) {
			Vector3 currentPos = movement_->GetCurrentPosition();
			ImGui::Text("Current Position: (%.2f, %.2f, %.2f)",
				currentPos.x, currentPos.y, currentPos.z);

			Vector3 forward = movement_->GetForwardDirection();
			ImGui::Text("Forward Direction: (%.2f, %.2f, %.2f)",
				forward.x, forward.y, forward.z);
		}
	}
#endif
}

void BossMoveEditor::ShowDebugControls() {
#ifdef USEIMGUI
	if (ImGui::CollapsingHeader("Debug Display")) {
		if (!debugger_) {
			ImGui::Text("Debugger not available");
			return;
		}

		// 表示切り替え
		bool showSpline = debugger_->IsSplineVisible();
		if (ImGui::Checkbox("Show Spline Curve", &showSpline)) {
			debugger_->SetSplineVisible(showSpline);
		}

		bool showControlPoints = debugger_->IsControlPointsVisible();
		if (ImGui::Checkbox("Show Control Points", &showControlPoints)) {
			debugger_->SetControlPointsVisible(showControlPoints);
		}

		ImGui::Separator();

		// スプライン曲線の設定
		ImGui::Text("Spline Curve Settings:");

		Vector4 splineColor = debugger_->GetSplineColor();
		if (ImGui::ColorEdit4("Spline Color", &splineColor.x)) {
			debugger_->SetSplineColor(splineColor);
		}

		int segments = debugger_->GetSplineSegments();
		if (ImGui::SliderInt("Segments", &segments, 10, 200)) {
			debugger_->SetSplineSegments(segments);
		}

		ImGui::Separator();

		// 制御点の設定
		ImGui::Text("Control Point Settings:");

		Vector4 pointColor = debugger_->GetControlPointColor();
		if (ImGui::ColorEdit4("Point Color", &pointColor.x)) {
			debugger_->SetControlPointColor(pointColor);
		}

		Vector4 selectedColor = debugger_->GetSelectedPointColor();
		if (ImGui::ColorEdit4("Selected Color", &selectedColor.x)) {
			debugger_->SetSelectedPointColor(selectedColor);
		}

		float pointSize = debugger_->GetControlPointSize();
		if (ImGui::DragFloat("Point Size", &pointSize, 0.1f, 0.1f, 5.0f)) {
			debugger_->SetControlPointSize(pointSize);
		}
	}
#endif
}