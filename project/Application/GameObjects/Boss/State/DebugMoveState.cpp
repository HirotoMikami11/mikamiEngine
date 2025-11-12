#include "State/DebugMoveState.h"
#include "Boss.h"
#include "GameTimer.h"
#include "Managers/ImGui/ImGuiManager.h"

void DebugMoveState::Initialize() {
	// 初期目標座標はコンストラクタで設定済み
}

void DebugMoveState::Update(Boss* boss) {
	// 現在の頭の位置を取得
	Vector3 currentPosition = boss->GetHeadPosition();

	// 目標座標への方向ベクトルを計算
	Vector3 direction = Subtract(targetPosition_, currentPosition);
	float distance = Length(direction);

	// 到着判定
	if (distance < arrivalThreshold_) {
		// 目標に到着したので停止
		return;
	}

	// 方向を正規化
	Vector3 normalizedDirection = Normalize(direction);

	// デルタタイムを取得
	GameTimer& gameTimer = GameTimer::GetInstance();
	float deltaTime = gameTimer.GetDeltaTime();

	// 移動速度を適用
	float moveSpeed = boss->GetMoveSpeed();
	Vector3 velocity = Multiply(normalizedDirection, moveSpeed);

	// 移動量を計算
	Vector3 movement = Multiply(velocity, deltaTime);

	// 移動量が目標距離を超えないようにクランプ
	float movementDistance = Length(movement);
	if (movementDistance > distance) {
		movement = Multiply(normalizedDirection, distance);
	}

	// Bossに移動を適用
	boss->MoveHead(movement);

	// 頭の向きを進行方向に設定（Y軸回転）
	float targetAngle = std::atan2(normalizedDirection.x, normalizedDirection.z);
	boss->SetHeadRotationY(targetAngle);
}

void DebugMoveState::ImGui() {
#ifdef USEIMGUI
	ImGui::Text("State: DebugMove");
	ImGui::Separator();

	// 目標座標の表示と編集
	ImGui::DragFloat3("Target Position", &targetPosition_.x, 0.1f);

	// 到着判定距離の表示
	ImGui::Text("Arrival Threshold: %.2f", arrivalThreshold_);
#endif
}