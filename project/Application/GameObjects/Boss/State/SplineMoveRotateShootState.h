#pragma once
#include "State/BossState.h"
#include <string>

/// <summary>
/// スプライン移動 → 停止 → 頭部回転 → 弾発射 のState
/// 往復回転にも対応
/// </summary>
class SplineMoveRotateShootState : public BossState {
public:
	enum class Phase {
		Moving,
		Stopping,
		Rotating,
		Shooting,
		Completed
	};

	SplineMoveRotateShootState() = default;
	~SplineMoveRotateShootState() override = default;

	explicit SplineMoveRotateShootState(
		const std::string& csvFilePath,
		float startAngle = -60.0f,
		float endAngle = 60.0f,
		float rotationSpeed = 2.0f,
		float bulletSpeed = 0.3f,
		int maxRepeatCount = 1
	);

	void Initialize() override;
	void Update(Boss* boss) override;
	void ImGui() override;
	const char* GetStateName() const override { return "SplineMoveRotateShoot"; }

private:
	bool LoadAndSetup(Boss* boss);
	void UpdateMovingPhase(Boss* boss);
	void UpdateStoppingPhase(Boss* boss);
	void UpdateRotatingPhase(Boss* boss);
	void UpdateShootingPhase(Boss* boss);
	void ShootBulletFromHead(Boss* boss);

	std::string csvFilePath_;
	Phase currentPhase_ = Phase::Moving;
	bool isInitialized_ = false;
	Boss* boss_ = nullptr;

	float startAngle_;
	float endAngle_;
	float rotationSpeed_;
	float currentAngle_;
	float baseRotationY_;
	float bulletSpeed_;
	int shootInterval_;
	int shootTimer_;
	float moveProgress_;
	float stopProgress_;
	bool hasReachedStop_;

	// 往復回転用
	int currentRepeatCount_ = 0;
	int maxRepeatCount_ = 1;
	bool rotatingForward_ = true;
};
