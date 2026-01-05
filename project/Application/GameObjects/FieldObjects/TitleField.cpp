#define NOMINMAX
#include "TitleField.h"
#include "ImGui/ImGuiManager.h"

TitleField::TitleField()
	: dxCommon_(nullptr)
	, nextSegmentIndex_(0)
	, previousCameraZ_(0.0f)
	, nextTransitionZ_(60.0f)
{
	// セグメント位置の初期化（3つ）
	segmentPositions_[0] = 0.0f;
	segmentPositions_[1] = segmentLength_;
	segmentPositions_[2] = segmentLength_ * 2.0f;
}

TitleField::~TitleField() = default;

void TitleField::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// 3つのセグメントを初期化
	for (int i = 0; i < 3; ++i) {
		segments_[i] = std::make_unique<TitleFieldSegment>();
		segments_[i]->Initialize(dxCommon_);
	}

	// 初期位置を設定
	// segment[0]: z = 0
	// segment[1]: z = 60
	// segment[2]: z = 120
	segmentPositions_[0] = 0.0f;
	segmentPositions_[1] = segmentLength_;
	segmentPositions_[2] = segmentLength_ * 2.0f;

	// 初期状態では segment[0] が次に移動するセグメント
	nextSegmentIndex_ = 0;
	nextTransitionZ_ = segmentLength_;

	// セグメントの位置を適用
	UpdateSegmentPositions();
}

void TitleField::Update(const Matrix4x4& viewProjectionMatrix, float cameraZ) {
	// カメラの位置に応じてセグメントの遷移をチェック
	CheckSegmentTransition(cameraZ);

	// 各セグメントを更新
	for (auto& segment : segments_) {
		if (segment) {
			segment->Update(viewProjectionMatrix);
		}
	}

	// 前回のカメラZ座標を保存
	previousCameraZ_ = cameraZ;
}

void TitleField::Draw() {
	// 両方のセグメントを描画
	for (auto& segment : segments_) {
		if (segment) {
			segment->Draw();
		}
	}
}

void TitleField::ImGui() {
#ifdef USEIMGUI
	if (ImGui::CollapsingHeader("TitleField")) {
		ImGui::Text("Segment Length: %.2f", segmentLength_);
		ImGui::Text("Next Transition Z: %.2f", nextTransitionZ_);
		ImGui::Text("Next Segment Index: %d", nextSegmentIndex_);

		ImGui::Separator();
		ImGui::Text("Segment Positions:");
		for (int i = 0; i < 3; ++i) {
			ImGui::Text("  Segment[%d] Z: %.2f", i, segmentPositions_[i]);
		}

		ImGui::Separator();
		// 各セグメントのImGui表示
		for (int i = 0; i < 3; ++i) {
			if (segments_[i]) {
				char label[32];
				snprintf(label, sizeof(label), "Segment[%d]", i);
				if (ImGui::TreeNode(label)) {
					segments_[i]->ImGui();
					ImGui::TreePop();
				}
			}
		}
	}
#endif
}

void TitleField::UpdateSegmentPositions() {
	// 各セグメントのZ座標オフセットを設定（3つ）
	for (int i = 0; i < 3; ++i) {
		if (segments_[i]) {
			segments_[i]->SetZOffset(segmentPositions_[i]);
		}
	}
}

void TitleField::CheckSegmentTransition(float cameraZ) {
	// カメラが次の遷移ポイントを超えたかチェック
	if (cameraZ >= nextTransitionZ_) {
		// 現在の最奥セグメントのZ座標を取得
		float maxZ = std::max({ segmentPositions_[0], segmentPositions_[1], segmentPositions_[2] });

		// 次に移動するセグメントを、最奥 + segmentLength_ の位置に配置
		segmentPositions_[nextSegmentIndex_] = maxZ + segmentLength_;

		// セグメントのオフセットを更新（再初期化不要）
		if (segments_[nextSegmentIndex_]) {
			segments_[nextSegmentIndex_]->SetZOffset(segmentPositions_[nextSegmentIndex_]);
		}

		// 次のセグメントインデックスを切り替え（0 → 1 → 2 → 0...）
		nextSegmentIndex_ = (nextSegmentIndex_ + 1) % 3;

		// 次の遷移ポイントを更新
		nextTransitionZ_ += segmentLength_;
	}
}