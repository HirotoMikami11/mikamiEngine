#define NOMINMAX
#include "BossSplineMovement.h"
#include "Logger.h"
#include <format>

void BossSplineMovement::Initialize(BossSplineTrack* track) {
	track_ = track;
	t_ = 0.0f;
	isAtEnd_ = false;
	isMoving_ = false;

	Logger::Log("BossSplineMovement: Initialized\n");
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

	// 等間隔移動が有効で、長さテーブルが構築されている場合
	if (uniformSpeedEnabled_ && track_->HasLengthTable() && track_->GetTotalLength() > 0.0f) {
		// 距離ベースでの移動
		float currentLength = t_ * track_->GetTotalLength();
		float moveDistance = speed * deltaTime;
		currentLength += moveDistance;

		// 終点チェック
		if (currentLength >= track_->GetTotalLength()) {
			currentLength = track_->GetTotalLength();
			isMoving_ = false;
			isAtEnd_ = true;
		}

		// 進行度を更新
		t_ = currentLength / track_->GetTotalLength();
	} else {
		// 従来の移動：tパラメータベース
		// 速度を曲線全体の長さで正規化
		float totalLength = track_->GetTotalLength();
		float normalizedSpeed = (totalLength > 0.0f) ? (speed / totalLength) : 0.01f;

		t_ += normalizedSpeed * deltaTime;

		// 終点チェック
		if (t_ >= 1.0f) {
			t_ = 1.0f;
			isMoving_ = false;
			isAtEnd_ = true;
		}
	}
}

void BossSplineMovement::ResetPosition() {
	t_ = 0.0f;
	isMoving_ = false;
	isAtEnd_ = false;

	Logger::Log("BossSplineMovement: Position reset\n");
}

void BossSplineMovement::SetProgress(float progress) {
	t_ = std::clamp(progress, 0.0f, 1.0f);
	isAtEnd_ = (t_ >= 1.0f);

	Logger::Log(std::format("BossSplineMovement: Progress set to {:.2f}\n", t_));
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
		float currentLength = t_ * track_->GetTotalLength();
		float lookAheadLength = currentLength + (lookAheadDistance * track_->GetTotalLength());

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

void BossSplineMovement::SetUniformSpeedEnabled(bool enabled) {
	uniformSpeedEnabled_ = enabled;

	// 等間隔移動を有効にする場合、長さテーブルが必要
	if (enabled && track_ && track_->IsValid() && !track_->HasLengthTable()) {
		Logger::Log("BossSplineMovement: Uniform speed enabled, but length table not built. Building now...\n");
		track_->BuildLengthTable();
	}

	Logger::Log(std::format("BossSplineMovement: Uniform speed {}\n", enabled ? "enabled" : "disabled"));
}