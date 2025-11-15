#define NOMINMAX
#include "BossSplineMovement.h"
#include <format>
#include <algorithm> // std::clamp

void BossSplineMovement::Initialize(BossSplineTrack* track) {
	track_ = track;
	t_ = 0.0f;
	distance_ = 0.0f;
	isAtEnd_ = false;
	isMoving_ = false;
}

void BossSplineMovement::Update(float deltaTime, float speed) {
	// 移動していない、またはトラックが無効な場合は更新しない
	if (!isMoving_ || !track_ || !track_->IsValid()) {
		return;
	}

	// 終端に到達している場合は更新しない
	if (isAtEnd_) {
		return;
	}

	if (uniformSpeedEnabled_ && track_->HasLengthTable() && track_->GetTotalLength() > 0.0f) {
		// 距離ベースでの移動
		distance_ += speed * deltaTime;

		// 終点チェック
		if (distance_ >= track_->GetTotalLength()) {
			distance_ = track_->GetTotalLength();
			isMoving_ = false;
			isAtEnd_ = true;
		}

		// 距離からtを取得
		t_ = track_->GetTFromLength(distance_);
	} else {
		// tパラメータベース（従来通り）
		float totalLength = track_->GetTotalLength();
		float normalizedSpeed = (totalLength > 0.0f) ? (speed / totalLength) : 0.01f;
		t_ += normalizedSpeed * deltaTime;
		if (t_ >= 1.0f) {
			t_ = 1.0f;
			isMoving_ = false;
			isAtEnd_ = true;
		}
		// t_から距離も更新
		distance_ = t_ * totalLength;
	}
}

void BossSplineMovement::ResetPosition() {
	t_ = 0.0f;
	distance_ = 0.0f;
	isMoving_ = false;
	isAtEnd_ = false;
}

void BossSplineMovement::SetProgress(float progress) {
	t_ = std::clamp(progress, 0.0f, 1.0f);
	if (track_ && track_->GetTotalLength() > 0.0f) {
		distance_ = t_ * track_->GetTotalLength();
	} else {
		distance_ = 0.0f;
	}
	isAtEnd_ = (t_ >= 1.0f);
}

Vector3 BossSplineMovement::GetCurrentPosition() const {
	if (!track_ || !track_->IsValid()) {
		return Vector3{ 0.0f, 0.0f, 0.0f };
	}

	return track_->CalculatePosition(t_);
}

Vector3 BossSplineMovement::GetLookAheadPosition(float lookAheadDistance) const {
	if (!track_ || !track_->IsValid()) {
		return Vector3{ 0.0f, 0.0f, 1.0f };
	}

	// 先読みtを計算
	float lookAheadT = t_;

	if (uniformSpeedEnabled_ && track_->HasLengthTable() && track_->GetTotalLength() > 0.0f) {
		// 等間隔移動の場合は距離ベースで先読み
		float lookAheadLength = distance_ + (lookAheadDistance * track_->GetTotalLength());

		// 終点を超えない
		if (lookAheadLength >= track_->GetTotalLength()) {
			lookAheadLength = track_->GetTotalLength();
		}

		lookAheadT = track_->GetTFromLength(lookAheadLength);
	} else {
		// 通常移動の場合
		lookAheadT = std::min(t_ + lookAheadDistance, 1.0f);
	}

	return track_->CalculatePosition(lookAheadT);
}

Vector3 BossSplineMovement::GetForwardDirection(float lookAheadDistance) const {
	if (!track_ || !track_->IsValid()) {
		return Vector3{ 0.0f, 0.0f, 1.0f };
	}

	Vector3 currentPos = GetCurrentPosition();
	Vector3 lookAheadPos = GetLookAheadPosition(lookAheadDistance);

	Vector3 direction = lookAheadPos - currentPos;

	// 方向ベクトルが極小の場合はデフォルト方向を返す
	if (Length(direction) < 0.001f) {
		return Vector3{ 0.0f, 0.0f, 1.0f };
	}

	return Normalize(direction);
}

