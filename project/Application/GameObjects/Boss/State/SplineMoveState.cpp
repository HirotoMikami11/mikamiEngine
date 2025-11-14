#include "State/SplineMoveState.h"
#include "State/IdleState.h"
#include "Boss.h"
#include "Logger.h"
#include "CSVUtility.h"
#include "Managers/ImGui/ImGuiManager.h"
#include <format>

SplineMoveState::SplineMoveState(const std::string& csvFilePath)
	: csvFilePath_(csvFilePath)
	, isInitialized_(false)
	, boss_(nullptr)
{
}

void SplineMoveState::Initialize() {
	isInitialized_ = false;
	boss_ = nullptr;

	Logger::Log(std::format("SplineMoveState: Initialized with CSV: {}\n", csvFilePath_));
}

void SplineMoveState::Update(Boss* boss) {
	if (!boss) {
		return;
	}

	// 初回のみ初期化処理
	if (!isInitialized_) {
		if (!LoadAndSetup(boss)) {
			// 初期化失敗時はIdleStateに遷移
			Logger::Log("SplineMoveState: Initialization failed, transitioning to Idle\n");
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
		Logger::Log("SplineMoveState: Invalid spline system\n");
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

	// 終点に到達したかチェック
	if (movement->IsAtEnd()) {
		Logger::Log("SplineMoveState: Reached end, transitioning to Idle\n");
		boss->ChangeState(std::make_unique<IdleState>());
	}
}

void SplineMoveState::ImGui() {
#ifdef USEIMGUI
	ImGui::Text("State: Spline Move");
	ImGui::Separator();

	// CSVファイルパス
	ImGui::Text("CSV File: %s", csvFilePath_.c_str());

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

				// 進行方向
				Vector3 forward = movement->GetForwardDirection();
				ImGui::Text("Forward Direction: (%.2f, %.2f, %.2f)",
					forward.x, forward.y, forward.z);

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

bool SplineMoveState::LoadAndSetup(Boss* boss) {
	if (!boss) {
		Logger::Log("SplineMoveState: Boss is null\n");
		return false;
	}

	// スプライン系の取得
	BossSplineTrack* track = boss->GetSplineTrack();
	BossSplineMovement* movement = boss->GetSplineMovement();

	if (!track || !movement) {
		Logger::Log("SplineMoveState: Spline system not available\n");
		return false;
	}

	// CSVから制御点を読み込み
	std::vector<Vector3> controlPoints;
	if (!CSVUtility::LoadVector3List(csvFilePath_, controlPoints)) {
		Logger::Log(std::format("SplineMoveState: Failed to load CSV: {}\n", csvFilePath_));
		return false;
	}

	// 制御点が4点未満の場合はエラー
	if (controlPoints.size() < 4) {
		Logger::Log(std::format("SplineMoveState: Invalid point count (need at least 4, got {})\n",
			controlPoints.size()));
		return false;
	}

	// Trackに制御点を設定
	track->SetControlPoints(controlPoints);

	// 長さテーブルを構築
	track->BuildLengthTable();

	// Movementをリセット
	movement->ResetPosition();

	// 最初の座標にワープ
	WarpToStartPosition(boss);

	// 次のポイントの方向を向く
	LookAtNextPoint(boss);

	// 移動を開始
	movement->StartMovement();

	Logger::Log(std::format("SplineMoveState: Setup completed with {} control points\n",
		controlPoints.size()));

	return true;
}

void SplineMoveState::WarpToStartPosition(Boss* boss) {
	if (!boss) {
		return;
	}

	BossSplineMovement* movement = boss->GetSplineMovement();
	if (!movement) {
		return;
	}

	// 最初の位置を取得（t=0）
	Vector3 startPosition = movement->GetCurrentPosition();

	// ボスの頭を最初の位置に設定
	boss->SetHeadPosition(startPosition);

	// 位置履歴をクリア
	boss->ClearPositionHistory();

	Logger::Log(std::format("SplineMoveState: Warped to start position (%.2f, %.2f, %.2f)\n",
		startPosition.x, startPosition.y, startPosition.z));
}

void SplineMoveState::LookAtNextPoint(Boss* boss) {
	if (!boss) {
		return;
	}

	BossSplineMovement* movement = boss->GetSplineMovement();
	if (!movement) {
		return;
	}

	// 次のポイントの方向を取得
	Vector3 forwardDirection = movement->GetForwardDirection(0.05f);  // 少し先を見る

	// Y軸回転角度を計算（-Z方向が前方）
	float rotationY = std::atan2(-forwardDirection.x, -forwardDirection.z);

	// ボスの頭の向きを設定
	boss->SetHeadRotationY(rotationY);

	Logger::Log(std::format("SplineMoveState: Looking at next point (rotY: {:.2f} rad)\n", rotationY));
}