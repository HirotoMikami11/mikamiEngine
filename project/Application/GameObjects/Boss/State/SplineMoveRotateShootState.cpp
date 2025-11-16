#include "State/SplineMoveRotateShootState.h"
#include "State/IdleState.h"
#include "Boss.h"
#include "CSVUtility.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "BossSplineMovement.h"
#include "BossSplineTrack.h"
#include "Parts/HeadParts.h"
#include <format>

SplineMoveRotateShootState::SplineMoveRotateShootState(
	const std::string& csvFilePath,
	float startAngle,
	float endAngle,
	float rotationSpeed,
	float bulletSpeed,
	int maxRepeatCount
)
	: csvFilePath_(csvFilePath)
	, startAngle_(startAngle)
	, endAngle_(endAngle)
	, rotationSpeed_(rotationSpeed)
	, bulletSpeed_(bulletSpeed)
	, currentAngle_(startAngle)
	, baseRotationY_(0.0f)
	, shootInterval_(4)
	, shootTimer_(0)
	, moveProgress_(0.0f)
	, stopProgress_(0.99f)
	, hasReachedStop_(false)
	, isInitialized_(false)
	, boss_(nullptr)
	, currentPhase_(Phase::Moving)
	, currentRepeatCount_(0)
	, maxRepeatCount_(maxRepeatCount)
	, rotatingForward_(true)
{
}

void SplineMoveRotateShootState::Initialize() {
	isInitialized_ = false;
	boss_ = nullptr;
	currentPhase_ = Phase::Moving;
	currentAngle_ = startAngle_;
	shootTimer_ = 0;
	moveProgress_ = 0.0f;
	hasReachedStop_ = false;
	currentRepeatCount_ = 0;
	rotatingForward_ = true;
}

void SplineMoveRotateShootState::Update(Boss* boss) {
	if (!boss) return;

	if (!isInitialized_) {
		if (!LoadAndSetup(boss)) {
			Logger::Log("SplineMoveRotateShootState: Initialization failed, transitioning to Idle\n");
			boss->ChangeState(std::make_unique<IdleState>());
			return;
		}
		isInitialized_ = true;
		boss_ = boss;
	}

	switch (currentPhase_) {
	case Phase::Moving:    UpdateMovingPhase(boss); break;
	case Phase::Stopping:  UpdateStoppingPhase(boss); break;
	case Phase::Rotating:  UpdateRotatingPhase(boss); break;
	case Phase::Shooting:  UpdateShootingPhase(boss); break;
	case Phase::Completed: boss->ChangeState(std::make_unique<IdleState>()); break;
	}
}

void SplineMoveRotateShootState::ImGui() {
#ifdef USEIMGUI
	ImGui::Text("State: Spline Move Rotate Shoot");
	ImGui::Separator();

	const char* phaseNames[] = { "Moving", "Stopping", "Rotating", "Shooting", "Completed" };
	int phaseIndex = static_cast<int>(currentPhase_);
	ImGui::Text("Current Phase: %s", phaseNames[phaseIndex]);

	ImGui::Text("CSV File: %s", csvFilePath_.c_str());
	ImGui::DragFloat("Start Angle (deg)", &startAngle_, 1.0f, -180.0f, 180.0f);
	ImGui::DragFloat("End Angle (deg)", &endAngle_, 1.0f, -180.0f, 180.0f);
	ImGui::DragFloat("Rotation Speed (deg/frame)", &rotationSpeed_, 0.1f, 0.1f, 10.0f);
	ImGui::DragFloat("Bullet Speed", &bulletSpeed_, 0.01f, 0.05f, 1.0f);
	ImGui::DragFloat("Stop Progress", &stopProgress_, 0.01f, 0.0f, 1.0f);

	if (currentPhase_ == Phase::Rotating || currentPhase_ == Phase::Shooting) {
		ImGui::Text("Current Angle: %.1f deg", currentAngle_);
		ImGui::Text("Repeat: %d / %d", currentRepeatCount_ / 2, maxRepeatCount_);
	}

	if (isInitialized_ && boss_) {
		BossSplineMovement* movement = boss_->GetSplineMovement();
		if (movement) {
			float progress = movement->GetProgress();
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

	if (!track || !movement) return false;

	std::vector<Vector3> controlPoints;
	if (!CSVUtility::LoadVector3List(csvFilePath_, controlPoints)) return false;
	if (controlPoints.size() < 4) return false;

	track->SetControlPoints(controlPoints);
	track->BuildLengthTable();
	movement->ResetPosition();

	Vector3 startPosition = movement->GetCurrentPosition();
	boss->SetHeadPosition(startPosition);
	boss->ClearPositionHistory();

	Vector3 forwardDirection = movement->GetForwardDirection(0.05f);
	float rotationY = std::atan2(-forwardDirection.x, -forwardDirection.z);
	boss->SetHeadRotationY(rotationY);

	movement->StartMovement();
	return true;
}

void SplineMoveRotateShootState::UpdateMovingPhase(Boss* boss) {
	BossSplineMovement* movement = boss->GetSplineMovement();
	if (!movement) { currentPhase_ = Phase::Completed; return; }

	float deltaTime = 1.0f / 60.0f;
	float speed = boss->GetMoveSpeed();
	movement->Update(deltaTime, speed);

	Vector3 currentPosition = movement->GetCurrentPosition();
	boss->SetHeadPosition(currentPosition);

	Vector3 forwardDirection = movement->GetForwardDirection(0.01f);
	float rotationY = std::atan2(-forwardDirection.x, -forwardDirection.z);
	boss->SetHeadRotationY(rotationY);

	moveProgress_ = movement->GetProgress();
	if (moveProgress_ >= stopProgress_) {
		hasReachedStop_ = true;
		currentPhase_ = Phase::Stopping;
		baseRotationY_ = rotationY;
		Logger::Log("SplineMoveRotateShootState: Reached stop position, transitioning to Stopping\n");
	}
}

void SplineMoveRotateShootState::UpdateStoppingPhase(Boss* boss) {
	currentPhase_ = Phase::Rotating;
	currentAngle_ = startAngle_;
	currentRepeatCount_ = 0;
	rotatingForward_ = true;
	Logger::Log("SplineMoveRotateShootState: Transitioning to Rotating phase\n");
}

void SplineMoveRotateShootState::UpdateRotatingPhase(Boss* boss) {
	currentAngle_ += rotatingForward_ ? rotationSpeed_ : -rotationSpeed_;

	bool reachedTarget = false;
	if (rotatingForward_ && currentAngle_ >= endAngle_) {
		currentAngle_ = endAngle_;
		reachedTarget = true;
	} else if (!rotatingForward_ && currentAngle_ <= startAngle_) {
		currentAngle_ = startAngle_;
		reachedTarget = true;
	}

	float finalRotationY = baseRotationY_ + DegToRad(currentAngle_);
	boss->SetHeadRotationY(finalRotationY);

	shootTimer_++;
	if (shootTimer_ >= shootInterval_) {
		ShootBulletFromHead(boss);
		shootTimer_ = 0;
	}

	if (reachedTarget) {
		rotatingForward_ = !rotatingForward_;
		currentRepeatCount_++;
		if (currentRepeatCount_ >= maxRepeatCount_ * 2) {
			currentPhase_ = Phase::Completed;
			Logger::Log("SplineMoveRotateShootState: Rotation completed, transitioning to Idle\n");
		}
	}
}

void SplineMoveRotateShootState::UpdateShootingPhase(Boss* boss) {
	ShootBulletFromHead(boss);
	currentPhase_ = Phase::Completed;
	Logger::Log("SplineMoveRotateShootState: Shooting complete, transitioning to Idle\n");
}

void SplineMoveRotateShootState::ShootBulletFromHead(Boss* boss) {
	if (!boss) return;
	std::vector<BaseParts*> activeParts = boss->GetActiveBodyParts();
	if (activeParts.empty()) return;

	BaseParts* headPart = activeParts[0];
	Vector3 headPosition = headPart->GetPosition();
	Vector3 headRotation = headPart->GetRotation();

	float rotY = headRotation.y;
	Vector3 bulletVelocity = {
		-std::sin(rotY) * bulletSpeed_,
		0.0f,
		-std::cos(rotY) * bulletSpeed_
	};

	boss->FireBullet(headPosition, bulletVelocity);
}
