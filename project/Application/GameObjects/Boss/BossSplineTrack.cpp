#include "BossSplineTrack.h"
#include "Logger.h"
#include <algorithm>
#include <format>

void BossSplineTrack::SetControlPoints(const std::vector<Vector3>& controlPoints) {
	controlPoints_ = controlPoints;

	// 制御点が変更されたら長さテーブルをクリア
	lengthTable_.clear();
	totalLength_ = 0.0f;

	// 制御点の数をログ出力
	if (!IsValid()) {
		Logger::Log(std::format("BossSplineTrack: Control points set, but invalid (need at least 4 points, got {})\n",
			controlPoints_.size()));
	} else {
		Logger::Log(std::format("BossSplineTrack: Control points set ({} points)\n", controlPoints_.size()));
	}
}

Vector3 BossSplineTrack::CalculatePosition(float t) const {
	if (!IsValid()) {
		Logger::Log("BossSplineTrack: Cannot calculate position - invalid control points\n");
		return Vector3{ 0.0f, 0.0f, 0.0f };
	}

	// tを0.0～1.0の範囲にクランプ
	t = std::clamp(t, 0.0f, 1.0f);

	// CatmullRomPosition関数を使用（MyFunctionにある）
	return CatmullRomPosition(controlPoints_, t);
}

void BossSplineTrack::BuildLengthTable(int resolution) {
	lengthTable_.clear();

	if (!IsValid()) {
		totalLength_ = 0.0f;
		Logger::Log("BossSplineTrack: Cannot build length table - invalid control points\n");
		return;
	}

	// テーブルのメモリを事前確保
	lengthTable_.reserve(resolution + 1);

	float totalLength = 0.0f;
	Vector3 previousPos = CalculatePosition(0.0f);

	// 最初のエントリ（t=0, length=0）
	lengthTable_.push_back({ 0.0f, 0.0f, 0.0f });

	// 曲線を細かく分割して長さを計算
	for (int i = 1; i <= resolution; ++i) {
		float t = static_cast<float>(i) / static_cast<float>(resolution);
		Vector3 currentPos = CalculatePosition(t);

		// 前の点からの距離を計算
		float segmentLength = Length(currentPos - previousPos);
		totalLength += segmentLength;

		// テーブルに追加
		lengthTable_.push_back({ t, totalLength, segmentLength });

		previousPos = currentPos;
	}

	totalLength_ = totalLength;

	Logger::Log(std::format("BossSplineTrack: Length table built (resolution={}, total length={:.2f})\n",
		resolution, totalLength_));
}

float BossSplineTrack::GetTFromLength(float targetLength) const {
	// 長さテーブルが空の場合
	if (lengthTable_.empty()) {
		Logger::Log("BossSplineTrack: Length table not built\n");
		return 0.0f;
	}

	// 範囲外チェック
	if (targetLength <= 0.0f) {
		return 0.0f;
	}

	if (targetLength >= totalLength_) {
		return 1.0f;
	}

	// バイナリサーチで対応するエントリを見つける
	auto it = std::lower_bound(
		lengthTable_.begin(),
		lengthTable_.end(),
		targetLength,
		[](const LengthTableEntry& entry, float length) {
			return entry.length < length;
		}
	);

	// 見つからない場合（終端）
	if (it == lengthTable_.end()) {
		return 1.0f;
	}

	// 最初の場合
	if (it == lengthTable_.begin()) {
		return 0.0f;
	}

	// 線形補間でより正確なtを計算
	auto prevIt = it - 1;
	float ratio = (targetLength - prevIt->length) / (it->length - prevIt->length);
	return prevIt->t + ratio * (it->t - prevIt->t);
}