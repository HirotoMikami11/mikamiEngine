#include "State/SplineMoveRotateShootState.h"
#include "State/IdleState.h"
#include "Boss.h"
#include "CSVUtility.h"
#include "ImGui/ImGuiManager.h"
#include "BossSplineMovement.h"
#include "BossSplineTrack.h"
#include "Parts/HeadParts.h"
#include <format>
#include <algorithm>

SplineMoveRotateShootState::SplineMoveRotateShootState(
	const std::string& csvFilePath,
	int stopControlPointIndex,
	float startAngle,
	float endAngle,
	float rotationSpeed,
	int shootInterval,
	float bulletSpeed,
	int angleIntervalDuration,
	int maxRepeatCount
)
	: csvFilePath_(csvFilePath)
	, stopControlPointIndex_(stopControlPointIndex)
	, startAngle_(startAngle)
	, endAngle_(endAngle)
	, rotationSpeed_(rotationSpeed)
	, shootInterval_(shootInterval)
	, bulletSpeed_(bulletSpeed)
	, angleIntervalDuration_(angleIntervalDuration)
	, maxRepeatCount_(maxRepeatCount)
{
}

void SplineMoveRotateShootState::Initialize() {
	isInitialized_ = false;
	boss_ = nullptr;
	currentPhase_ = Phase::Initializing;
	rotatingSubPhase_ = RotatingSubPhase::RotatingToEnd;
	currentAngle_ = startAngle_;
	currentProgress_ = 0.0f;
	stopProgress_ = 0.0f;
	shootTimer_ = 0;
	intervalTimer_ = 0;
	currentRepeatCount_ = 0;
	baseRotationY_ = 0.0f;
}

void SplineMoveRotateShootState::Update(Boss* boss) {
	if (!boss) return;

	// フェーズごとの処理
	switch (currentPhase_) {
	case Phase::Initializing:
		UpdateInitializingPhase(boss);
		break;
	case Phase::MovingToStop:
		UpdateMovingToStopPhase(boss);
		break;
	case Phase::Stopping:
		UpdateStoppingPhase(boss);
		break;
	case Phase::Rotating:
		UpdateRotatingPhase(boss);
		break;
	case Phase::MovingToEnd:
		UpdateMovingToEndPhase(boss);
		break;
	case Phase::Completed:
		boss->ChangeState(std::make_unique<IdleState>());
		break;
	}
}

void SplineMoveRotateShootState::ImGui() {
#ifdef USEIMGUI
	ImGui::Text("State: Spline Move Rotate Shoot (Improved)");
	ImGui::Separator();

	// 現在のフェーズ表示
	const char* phaseNames[] = { "Initializing", "MovingToStop", "Stopping", "Rotating", "MovingToEnd", "Completed" };
	int phaseIndex = static_cast<int>(currentPhase_);
	ImGui::Text("Current Phase: %s", phaseNames[phaseIndex]);

	// 回転フェーズの場合、サブフェーズも表示
	if (currentPhase_ == Phase::Rotating) {
		const char* subPhaseNames[] = { "RotatingToEnd", "IntervalAtEnd", "RotatingToStart", "IntervalAtStart" };
		int subPhaseIndex = static_cast<int>(rotatingSubPhase_);
		ImGui::Text("  Sub Phase: %s", subPhaseNames[subPhaseIndex]);
		ImGui::Text("  Current Angle: %.1f deg", currentAngle_);
		ImGui::Text("  Repeat Count: %d / %d", currentRepeatCount_, maxRepeatCount_);

		// インターバル中はタイマー表示
		if (rotatingSubPhase_ == RotatingSubPhase::IntervalAtEnd ||
			rotatingSubPhase_ == RotatingSubPhase::IntervalAtStart) {
			ImGui::Text("  Interval Timer: %d / %d", intervalTimer_, angleIntervalDuration_);
		}
	}

	ImGui::Separator();

	// パラメータ表示
	ImGui::Text("Parameters:");
	ImGui::Text("  CSV File: %s", csvFilePath_.c_str());
	ImGui::DragInt("  Stop Control Point Index", &stopControlPointIndex_, 0.1f, 0, 10);
	ImGui::DragFloat("  Start Angle (deg)", &startAngle_, 1.0f, -180.0f, 180.0f);
	ImGui::DragFloat("  End Angle (deg)", &endAngle_, 1.0f, -180.0f, 180.0f);
	ImGui::DragFloat("  Rotation Speed (deg/frame)", &rotationSpeed_, 0.1f, 0.1f, 10.0f);
	ImGui::DragInt("  Shoot Interval (frames)", &shootInterval_, 1, 1, 60);
	ImGui::DragFloat("  Bullet Speed", &bulletSpeed_, 0.01f, 0.05f, 1.0f);
	ImGui::DragInt("  Angle Interval Duration (frames)", &angleIntervalDuration_, 1, 0, 120);
	ImGui::DragInt("  Max Repeat Count", &maxRepeatCount_, 1, 1, 10);

	ImGui::Separator();

	// 進行状況
	if (isInitialized_ && boss_) {
		BossSplineMovement* movement = boss_->GetSplineMovement();
		if (movement) {
			float progress = movement->GetProgress();
			ImGui::Text("Progress:");
			ImGui::Text("  Current: %.3f", progress);
			ImGui::Text("  Stop Position: %.3f", stopProgress_);
			ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f),
				std::format("{:.1f}%%", progress * 100.0f).c_str());
		}
	}
#endif
}

bool SplineMoveRotateShootState::LoadAndSetup(Boss* boss) {
	if (!boss) return false;

	BossSplineTrack* track = boss->GetSplineTrack();
	BossSplineMovement* movement = boss->GetSplineMovement();
	if (!track || !movement) { return false; }

	// CSVから制御点を読み込み
	std::vector<Vector3> controlPoints;
	if (!CSVUtility::LoadVector3List(csvFilePath_, controlPoints)) {
		return false;
	}

	if (controlPoints.size() < 4) {
		return false;
	}

	// 停止位置の制御点インデックスが有効か確認
	if (stopControlPointIndex_ < 0 || stopControlPointIndex_ >= static_cast<int>(controlPoints.size())) {
		return false;
	}

	// Trackに設定
	track->SetControlPoints(controlPoints);
	track->BuildLengthTable();

	// 停止位置のprogressを計算
	stopProgress_ = CalculateTFromControlPointIndex(stopControlPointIndex_, controlPoints.size());
	// Movementを初期化
	movement->ResetPosition();

	// 開始位置（t=0）にワープ
	Vector3 startPosition = movement->GetCurrentPosition();
	boss->SetHeadPosition(startPosition);

	// 履歴をクリア
	boss->ClearPositionHistory();

	// 初期方向を設定
	Vector3 forwardDirection = movement->GetForwardDirection(0.05f);
	float rotationY = std::atan2(-forwardDirection.x, -forwardDirection.z);
	boss->SetHeadRotationY(rotationY);

	// 全パーツを頭の向きに合わせて一直線に整列
	boss->AlignAllPartsInLine();
	return true;
}

float SplineMoveRotateShootState::CalculateTFromControlPointIndex(
	int controlPointIndex,
	size_t totalControlPoints) const
{
	if (totalControlPoints <= 1) return 0.0f;

	// Catmull-Romスプラインでは、制御点index=nに対応するtは:
	// t = n / (totalControlPoints - 1)
	float t = static_cast<float>(controlPointIndex) / static_cast<float>(totalControlPoints - 1);

	// 0.0～1.0の範囲にクランプ
	return std::clamp(t, 0.0f, 1.0f);
}

void SplineMoveRotateShootState::UpdateInitializingPhase(Boss* boss) {
	// 初回のみセットアップ実行
	if (!isInitialized_) {
		if (!LoadAndSetup(boss)) {
			boss->ChangeState(std::make_unique<IdleState>());
			return;
		}
		isInitialized_ = true;
		boss_ = boss;
	}

	// 次のフェーズへ移行
	BossSplineMovement* movement = boss->GetSplineMovement();
	if (movement) {
		movement->StartMovement();
		currentPhase_ = Phase::MovingToStop;
	}
}

void SplineMoveRotateShootState::UpdateMovingToStopPhase(Boss* boss) {
	BossSplineMovement* movement = boss->GetSplineMovement();
	if (!movement) {
		currentPhase_ = Phase::Completed;
		return;
	}

	// 現在の進行度を取得
	currentProgress_ = movement->GetProgress();

	// 停止位置に到達したかチェック
	if (currentProgress_ >= stopProgress_) {
		// 停止位置に到達 → 移動を停止
		movement->StopMovement();

		// 現在の向きを基準角度として保存
		Vector3 forwardDir = movement->GetForwardDirection(0.01f);
		baseRotationY_ = std::atan2(-forwardDir.x, -forwardDir.z);

		currentPhase_ = Phase::Stopping;
		return;
	}

	// まだ到達していない → 移動を継続
	float deltaTime = 1.0f / 60.0f;
	float speed = boss->GetMoveSpeed();

	// オーバーシュート防止: 次のフレームで超えそうなら速度を調整
	BossSplineTrack* track = boss->GetSplineTrack();
	if (track && track->HasLengthTable()) {
		float currentDistance = currentProgress_ * track->GetTotalLength();
		float stopDistance = stopProgress_ * track->GetTotalLength();
		float remainingDistance = stopDistance - currentDistance;
		float nextFrameDistance = speed * deltaTime;

		// 次のフレームで超える場合は速度を制限
		if (nextFrameDistance > remainingDistance && remainingDistance > 0.0f) {
			speed = remainingDistance / deltaTime;
		}
	}

	movement->Update(deltaTime, speed);

	// 位置と向きを更新
	UpdatePositionAndRotation(boss);
}

void SplineMoveRotateShootState::UpdateStoppingPhase(Boss* boss) {
	// 回転フェーズの準備
	currentAngle_ = startAngle_;
	rotatingSubPhase_ = RotatingSubPhase::RotatingToEnd;
	currentRepeatCount_ = 0;
	shootTimer_ = 0;
	intervalTimer_ = 0;

	// 回転フェーズへ移行
	currentPhase_ = Phase::Rotating;
}

void SplineMoveRotateShootState::UpdateRotatingPhase(Boss* boss) {
	// サブフェーズごとの処理
	switch (rotatingSubPhase_) {
	case RotatingSubPhase::RotatingToEnd:
		UpdateRotatingToEnd(boss);
		break;
	case RotatingSubPhase::IntervalAtEnd:
		UpdateIntervalAtEnd(boss);
		break;
	case RotatingSubPhase::RotatingToStart:
		UpdateRotatingToStart(boss);
		break;
	case RotatingSubPhase::IntervalAtStart:
		UpdateIntervalAtStart(boss);
		break;
	}
}

void SplineMoveRotateShootState::UpdateMovingToEndPhase(Boss* boss) {
	BossSplineMovement* movement = boss->GetSplineMovement();
	if (!movement) {
		currentPhase_ = Phase::Completed;
		return;
	}

	// 移動更新
	float deltaTime = 1.0f / 60.0f;
	float speed = boss->GetMoveSpeed();
	movement->Update(deltaTime, speed);

	// 位置と向きを更新
	UpdatePositionAndRotation(boss);

	// 終点に到達したかチェック
	if (movement->IsAtEnd()) {
		movement->StopMovement();
		currentPhase_ = Phase::Completed;
	}
}

void SplineMoveRotateShootState::UpdateRotatingToEnd(Boss* boss) {
	// startAngle → endAngle へ回転
	currentAngle_ += rotationSpeed_;

	// endAngleに到達したかチェック
	if (currentAngle_ >= endAngle_) {
		currentAngle_ = endAngle_;

		// 次のサブフェーズへ（インターバル）
		rotatingSubPhase_ = RotatingSubPhase::IntervalAtEnd;
		intervalTimer_ = 0;
	}

	// 頭の回転を反映
	float finalRotationY = baseRotationY_ + DegToRad(currentAngle_);
	boss->SetHeadRotationY(finalRotationY);

	// 回転中は弾を発射
	shootTimer_++;
	if (shootTimer_ >= shootInterval_) {
		ShootBulletFromHead(boss);
		shootTimer_ = 0;
	}
}

void SplineMoveRotateShootState::UpdateIntervalAtEnd(Boss* boss) {
	// endAngleで待機（発射なし）
	intervalTimer_++;

	// インターバル終了チェック
	if (intervalTimer_ >= angleIntervalDuration_) {
		// 次のサブフェーズへ（startAngleへ回転）
		rotatingSubPhase_ = RotatingSubPhase::RotatingToStart;
		shootTimer_ = 0;
	}

	// 角度は固定（回転しない）
	float finalRotationY = baseRotationY_ + DegToRad(currentAngle_);
	boss->SetHeadRotationY(finalRotationY);
}

void SplineMoveRotateShootState::UpdateRotatingToStart(Boss* boss) {
	// endAngle → startAngle へ回転（逆方向）
	currentAngle_ -= rotationSpeed_;

	// startAngleに到達したかチェック
	if (currentAngle_ <= startAngle_) {
		currentAngle_ = startAngle_;

		// 次のサブフェーズへ（インターバル）
		rotatingSubPhase_ = RotatingSubPhase::IntervalAtStart;
		intervalTimer_ = 0;
	}

	// 頭の回転を反映
	float finalRotationY = baseRotationY_ + DegToRad(currentAngle_);
	boss->SetHeadRotationY(finalRotationY);

	// 回転中は弾を発射
	shootTimer_++;
	if (shootTimer_ >= shootInterval_) {
		ShootBulletFromHead(boss);
		shootTimer_ = 0;
	}
}

void SplineMoveRotateShootState::UpdateIntervalAtStart(Boss* boss) {
	// startAngleで待機（発射なし）
	intervalTimer_++;

	// インターバル終了チェック
	if (intervalTimer_ >= angleIntervalDuration_) {
		// 往復カウントを増やす
		currentRepeatCount_++;

		if (currentRepeatCount_ >= maxRepeatCount_) {
			// 往復完了 → 移動再開フェーズへ
			BossSplineMovement* movement = boss->GetSplineMovement();
			if (movement) {
				// 停止位置から移動を再開
				movement->StartMovement();
				currentPhase_ = Phase::MovingToEnd;
			} else {
				currentPhase_ = Phase::Completed;
			}
		} else {
			// まだ往復が残っている → endAngleへ回転
			rotatingSubPhase_ = RotatingSubPhase::RotatingToEnd;
			shootTimer_ = 0;
		}
	}

	// 角度は固定（回転しない）
	float finalRotationY = baseRotationY_ + DegToRad(currentAngle_);
	boss->SetHeadRotationY(finalRotationY);
}

void SplineMoveRotateShootState::ShootBulletFromHead(Boss* boss) {
	if (!boss) return;

	// 頭パーツを取得
	std::vector<BaseParts*> activeParts = boss->GetActiveBodyParts();
	if (activeParts.empty()) return;

	BaseParts* headPart = activeParts[0];
	Vector3 headPosition = headPart->GetPosition();
	Vector3 headRotation = headPart->GetRotation();

	// 頭の向きに基づいて弾の速度ベクトルを計算
	float rotY = headRotation.y;
	Vector3 bulletVelocity = {
		-std::sin(rotY) * bulletSpeed_,
		0.0f,
		-std::cos(rotY) * bulletSpeed_
	};

	boss->FireBullet(headPosition, bulletVelocity);
}

void SplineMoveRotateShootState::UpdatePositionAndRotation(Boss* boss) {
	BossSplineMovement* movement = boss->GetSplineMovement();
	if (!movement) return;

	// 現在位置を更新
	boss->SetHeadPosition(movement->GetCurrentPosition());

	// 進行方向を取得して向きを更新
	Vector3 forwardDirection = movement->GetForwardDirection(0.01f);
	float rotationY = std::atan2(-forwardDirection.x, -forwardDirection.z);
	boss->SetHeadRotationY(rotationY);
}