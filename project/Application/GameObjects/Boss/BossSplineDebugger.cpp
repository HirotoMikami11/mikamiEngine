#include "BossSplineDebugger.h"
#include <format>

void BossSplineDebugger::Initialize(DebugDrawLineSystem* debugDrawSystem, BossSplineTrack* track) {
	debugDrawSystem_ = debugDrawSystem;
	track_ = track;
}

void BossSplineDebugger::Draw() {
	if (!debugDrawSystem_ || !track_ || !track_->IsValid()) {
		return;
	}

	// スプライン曲線を描画
	if (showSpline_) {
		GenerateSplineLines();
	}

	// 制御点を描画
	if (showControlPoints_) {
		DrawControlPoints();
	}
}

void BossSplineDebugger::GenerateSplineLines() {
	if (!debugDrawSystem_ || !track_ || !track_->IsValid()) {
		return;
	}

	// スプライン曲線を分割して線分を生成
	std::vector<Vector3> points;
	points.reserve(splineSegments_ + 1);

	for (int i = 0; i <= splineSegments_; ++i) {
		float t = static_cast<float>(i) / static_cast<float>(splineSegments_);
		Vector3 point = track_->CalculatePosition(t);
		points.push_back(point);
	}

	// 隣接する2点を結ぶ線分を追加
	for (size_t i = 0; i < points.size() - 1; ++i) {
		debugDrawSystem_->AddLine(points[i], points[i + 1], splineColor_);
	}
}

void BossSplineDebugger::DrawControlPoints() {
	if (!debugDrawSystem_ || !track_) {
		return;
	}

	const auto& controlPoints = track_->GetControlPoints();

	// 各制御点に十字線を描画
	for (int i = 0; i < static_cast<int>(controlPoints.size()); ++i) {
		const Vector3& point = controlPoints[i];

		// 選択中の制御点かどうかで色を変える
		Vector4 pointColor = (i == selectedPointIndex_) ? selectedPointColor_ : controlPointColor_;

		// 十字線を描画
		debugDrawSystem_->DrawCross(point, controlPointSize_, pointColor);
	}
}