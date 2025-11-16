#include "State/SplineMove8WayShootState.h"
#include "State/IdleState.h"
#include "Boss.h"
#include "CSVUtility.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "BossSplineMovement.h"
#include "BossSplineTrack.h"
#include <format>

SplineMove8WayShootState::SplineMove8WayShootState(const std::string& csvFilePath)
	: csvFilePath_(csvFilePath)
	, isInitialized_(false)
	, boss_(nullptr)
	, shootTimer_(0)
{
}

void SplineMove8WayShootState::Initialize() {
	isInitialized_ = false;
	boss_ = nullptr;
	shootTimer_ = 0;
}

void SplineMove8WayShootState::Update(Boss* boss) {
	if (!boss) {
		return;
	}

	// 初回のみ初期化処理
	if (!isInitialized_) {
		if (!LoadAndSetup(boss)) {
			// 初期化失敗時はIdleStateに遷移
			Logger::Log("SplineMove8WayShootState: Initialization failed, transitioning to Idle\\n");
			boss->ChangeState(std::make_unique<IdleState>());
			return;
		}
		isInitialized_ = true;
		boss_ = boss;
	}

	// スプライン移動システムの取得
	BossSplineMovement* movement = boss->GetSplineMovement();
	BossSplineTrack* track = boss->GetSplineTrack();

	if (!movement || !track || !track->IsValid()) {
		boss->ChangeState(std::make_unique<IdleState>());
		return;
	}

	// 移動の更新
	float deltaTime = 1.0f / 60.0f;  // 60FPS想定
	float speed = boss->GetMoveSpeed();
	movement->Update(deltaTime, speed);

	// 現在位置を取得してBossの頭に設定
	Vector3 currentPosition = movement->GetCurrentPosition();
	boss->SetHeadPosition(currentPosition);

	// 進行方向を取得してBossの頭の向きに設定
	Vector3 forwardDirection = movement->GetForwardDirection(0.01f);

	// Y軸回転角度を計算（-Z方向が前方）
	float rotationY = std::atan2(-forwardDirection.x, -forwardDirection.z);
	boss->SetHeadRotationY(rotationY);

	// 弾発射タイマーを更新
	shootTimer_++;
	if (shootTimer_ >= shootInterval_) {
		ShootBulletsFrom8Directions(boss);
		shootTimer_ = 0;
	}

	// 終点に到達したかチェック
	if (movement->IsAtEnd()) {
		boss->ChangeState(std::make_unique<IdleState>());
	}
}

void SplineMove8WayShootState::ImGui() {
#ifdef USEIMGUI
	ImGui::Text("State: Spline Move 8-Way Shoot");
	ImGui::Separator();

	// CSVファイルパス
	ImGui::Text("CSV File: %s", csvFilePath_.c_str());

	// パラメータ調整
	ImGui::DragInt("Shoot Interval (frames)", &shootInterval_, 1, 10, 300);
	ImGui::DragFloat("Bullet Speed", &bulletSpeed_, 0.01f, 0.05f, 1.0f);

	// タイマー表示
	ImGui::Text("Shoot Timer: %d / %d", shootTimer_, shootInterval_);

	// 初期化状態
	if (isInitialized_) {
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Initialized");

		if (boss_) {
			BossSplineMovement* movement = boss_->GetSplineMovement();
			if (movement) {
				// 進行度表示
				float progress = movement->GetProgress();
				ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f),
					std::format("{:.1f}%%", progress * 100.0f).c_str());

				// 現在位置
				Vector3 currentPos = movement->GetCurrentPosition();
				ImGui::Text("Current Position: (%.2f, %.2f, %.2f)",
					currentPos.x, currentPos.y, currentPos.z);

				// 移動状態
				if (movement->IsMoving()) {
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Moving...");
				}

				if (movement->IsAtEnd()) {
					ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "At End");
				}
			}
		}
	} else {
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Status: Not Initialized");
	}
#endif
}

bool SplineMove8WayShootState::LoadAndSetup(Boss* boss) {
	if (!boss) {
		return false;
	}

	// スプライン系の取得
	BossSplineTrack* track = boss->GetSplineTrack();
	BossSplineMovement* movement = boss->GetSplineMovement();

	if (!track || !movement) {
		return false;
	}

	// CSVから制御点を読み込み
	std::vector<Vector3> controlPoints;
	if (!CSVUtility::LoadVector3List(csvFilePath_, controlPoints)) {
		return false;
	}

	// 制御点が4点未満の場合はエラー
	if (controlPoints.size() < 4) {
		return false;
	}

	// Trackに制御点を設定
	track->SetControlPoints(controlPoints);

	// 長さテーブルを構築
	track->BuildLengthTable();

	// Movementをリセット
	movement->ResetPosition();

	// 最初の座標にワープ
	Vector3 startPosition = movement->GetCurrentPosition();
	boss->SetHeadPosition(startPosition);
	boss->ClearPositionHistory();

	// 次のポイントの方向を向く
	Vector3 forwardDirection = movement->GetForwardDirection(0.05f);
	float rotationY = std::atan2(-forwardDirection.x, -forwardDirection.z);
	boss->SetHeadRotationY(rotationY);

	// 移動を開始
	movement->StartMovement();

	return true;
}

void SplineMove8WayShootState::ShootBulletsFrom8Directions(Boss* boss) {
	if (!boss) {
		return;
	}

	// Activeな全パーツを取得
	std::vector<BaseParts*> activeParts = boss->GetActiveBodyParts();

	// 8方向のベクトル（XZ平面、Y=0）
	float angleStep = DegToRad(180.0f);
	std::vector<Vector3> directions;
	for (int i = 0; i < 8; ++i) {
		float angle = angleStep * i;
		Vector3 dir = {
			std::sin(angle) * bulletSpeed_,
			0.0f,
			std::cos(angle) * bulletSpeed_
		};
		directions.push_back(dir);
	}

	// 各パーツから8方向に弾を発射
	for (auto* part : activeParts) {
		Vector3 partPosition = part->GetPosition();

		for (const auto& direction : directions) {
			boss->FireBullet(partPosition, direction);
		}
	}
}
