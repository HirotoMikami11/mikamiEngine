#include "Boss.h"
#include "State/IdleState.h"
#include "State/DebugMoveState.h"
#include "Managers/ImGui/ImGuiManager.h"
#include <format>

Boss::Boss()
	: directXCommon_(nullptr)
	, viewProjectionMatrix_(MakeIdentity4x4())
	, previousHeadPosition_({ 0.0f, 0.0f, 0.0f })
{
}

Boss::~Boss() = default;

void Boss::Initialize(DirectXCommon* dxCommon, const Vector3& position) {
	directXCommon_ = dxCommon;

	// パーツの初期化
	InitializeParts();

	// 初期位置を設定
	if (!parts_.empty()) {
		parts_[0]->SetPosition(position);
		previousHeadPosition_ = position;

		// 初期位置履歴を作成（全パーツが正しい位置に配置されるように）
		for (size_t i = 0; i < parts_.size(); ++i) {
			Vector3 historyPosition = position;
			historyPosition.z -= static_cast<float>(i) * partsDistance_;
			positionHistory_.push_back(historyPosition);
		}

		// 各パーツの初期位置を設定
		UpdatePartsPositions();
	}

	// 初期Stateを設定（Idle）
	currentState_ = std::make_unique<IdleState>();
	currentState_->Initialize();
}

void Boss::InitializeParts() {
	// 頭パーツ（黄色）
	auto head = std::make_unique<HeadParts>();
	head->Initialize(directXCommon_, Vector3{ 0.0f, 0.0f, 0.0f });
	parts_.push_back(std::move(head));

	// 体パーツ（白） × 5
	for (size_t i = 0; i < kBodyCount; ++i) {
		auto body = std::make_unique<BodyParts>();
		Vector3 bodyPosition = { 0.0f, 0.0f, -static_cast<float>(i + 1) * partsDistance_ };
		body->Initialize(directXCommon_, bodyPosition);
		parts_.push_back(std::move(body));
	}

	// 尻尾パーツ（緑）
	auto tail = std::make_unique<TailParts>();
	Vector3 tailPosition = { 0.0f, 0.0f, -static_cast<float>(kBodyCount + 1) * partsDistance_ };
	tail->Initialize(directXCommon_, tailPosition);
	parts_.push_back(std::move(tail));
}

void Boss::Update(const Matrix4x4& viewProjectionMatrix) {
	viewProjectionMatrix_ = viewProjectionMatrix;

	// Stateの更新
	if (currentState_) {
		currentState_->Update(this);
	}

	// 位置履歴の更新
	UpdatePositionHistory();

	// 各パーツの位置を更新
	UpdatePartsPositions();

	// 各パーツの向きを更新
	UpdatePartsRotations();

	// 各パーツの行列更新
	for (auto& part : parts_) {
		part->Update(viewProjectionMatrix);
	}
}

void Boss::UpdatePositionHistory() {
	if (parts_.empty()) return;

	// 現在の頭の位置を取得
	Vector3 currentHeadPosition = parts_[0]->GetPosition();

	// 前回の位置からの移動距離を計算
	Vector3 movement = Subtract(currentHeadPosition, previousHeadPosition_);
	float movementDistance = Length(movement);

	// 移動量が閾値以上の場合のみ履歴を更新（ガタガタ防止）
	if (movementDistance > kHistoryUpdateThreshold) {
		positionHistory_.push_front(currentHeadPosition);
		previousHeadPosition_ = currentHeadPosition;

		// 履歴サイズの制限
		while (positionHistory_.size() > kMaxHistorySize) {
			positionHistory_.pop_back();
		}
	}
}

void Boss::UpdatePartsPositions() {
	if (parts_.empty() || positionHistory_.empty()) return;

	// 各パーツに対して、履歴から適切な位置を取得
	for (size_t i = 1; i < parts_.size(); ++i) {
		// このパーツが参照すべき履歴インデックスを計算
		// パーツ間距離 × インデックス分だけ離れた位置を参照
		float targetDistance = static_cast<float>(i) * partsDistance_;

		// 履歴を走査して、目標距離に最も近い位置を探す
		float accumulatedDistance = 0.0f;
		Vector3 targetPosition = parts_[0]->GetPosition();

		for (size_t historyIndex = 0; historyIndex < positionHistory_.size() - 1; ++historyIndex) {
			Vector3 currentPos = positionHistory_[historyIndex];
			Vector3 nextPos = positionHistory_[historyIndex + 1];

			float segmentDistance = Distance(currentPos, nextPos);

			if (accumulatedDistance + segmentDistance >= targetDistance) {
				// 目標距離に達した
				float remainingDistance = targetDistance - accumulatedDistance;
				float t = remainingDistance / segmentDistance;
				targetPosition = Lerp(currentPos, nextPos, t);
				break;
			}

			accumulatedDistance += segmentDistance;
		}

		// パーツの位置を設定
		parts_[i]->SetPosition(targetPosition);
	}
}

void Boss::UpdatePartsRotations() {
	if (parts_.size() < 2) return;

	// 各パーツの向きを次のパーツ方向に設定
	for (size_t i = 0; i < parts_.size() - 1; ++i) {
		Vector3 currentPosition = parts_[i]->GetPosition();
		Vector3 nextPosition = parts_[i + 1]->GetPosition();

		// 次のパーツへの方向ベクトルを計算
		Vector3 direction = Subtract(nextPosition, currentPosition);
		float distance = Length(direction);

		// 距離が十分にある場合のみ回転を適用（ゼロ除算防止）
		if (distance > 0.001f) {
			// Y軸回転角度を計算（-Z方向が前方）
			float rotationY = std::atan2(-direction.x, -direction.z);
			parts_[i]->SetRotationY(rotationY);
		}
	}

	// 最後のパーツ（尻尾）は一つ前のパーツと同じ向き
	if (parts_.size() >= 2) {
		Vector3 lastRotation = parts_[parts_.size() - 2]->GetRotation();
		parts_[parts_.size() - 1]->SetRotation(lastRotation);
	}
}

void Boss::Draw(const Light& directionalLight) {
	// 全パーツを描画
	for (auto& part : parts_) {
		part->Draw(directionalLight);
	}
}

void Boss::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode("Boss")) {
		// State情報
		if (ImGui::CollapsingHeader("State", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (currentState_) {
				ImGui::Text("Current State: %s", currentState_->GetStateName());
				ImGui::Separator();

				// State切り替えボタン
				if (ImGui::Button("Change to Idle")) {
					ChangeState(std::make_unique<IdleState>());
				}
				ImGui::SameLine();
				if (ImGui::Button("Change to DebugMove")) {
					ChangeState(std::make_unique<DebugMoveState>());
				}

				ImGui::Separator();

				// 現在のStateのImGui
				currentState_->ImGui();
			}
		}

		// Phase情報（将来の拡張用）
		if (ImGui::CollapsingHeader("Phase")) {
			const char* phaseNames[] = { "Phase 1", "Phase 2", "Phase 3" };
			int currentPhaseIndex = static_cast<int>(currentPhase_);
			ImGui::Text("Current Phase: %s", phaseNames[currentPhaseIndex]);
		}

		// パラメータ設定
		if (ImGui::CollapsingHeader("Parameters")) {
			// パーツ間距離の調整（隙間の調整）
			float gap = partsDistance_ - 1.0f; // キューブサイズ1.0fを引いた隙間
			if (ImGui::DragFloat("Parts Gap", &gap, 0.01f, 0.0f, 2.0f)) {
				partsDistance_ = 1.0f + gap; // キューブサイズ + 隙間
			}
			ImGui::Text("Parts Distance: %.2f", partsDistance_);

			// 移動速度
			ImGui::DragFloat("Move Speed", &moveSpeed_, 0.1f, 0.1f, 10.0f);

			// 履歴情報
			ImGui::Text("Position History Size: %zu", positionHistory_.size());
		}

		// パーツ情報
		if (ImGui::CollapsingHeader("Parts")) {
			ImGui::Text("Total Parts: %zu", parts_.size());
			ImGui::Separator();

			// 各パーツの情報
			for (size_t i = 0; i < parts_.size(); ++i) {
				std::string label;
				if (i == 0) {
					label = "Head (Yellow)";
				} else if (i <= kBodyCount) {
					label = std::format("Body {} (White)", i);
				} else {
					label = "Tail (Green)";
				}

				parts_[i]->ImGui(label.c_str());
			}
		}

		ImGui::TreePop();
	}
#endif
}

void Boss::ChangeState(std::unique_ptr<BossState> newState) {
	if (newState) {
		currentState_ = std::move(newState);
		currentState_->Initialize();
	}
}

Vector3 Boss::GetHeadPosition() const {
	if (!parts_.empty()) {
		return parts_[0]->GetPosition();
	}
	return Vector3{ 0.0f, 0.0f, 0.0f };
}

void Boss::MoveHead(const Vector3& movement) {
	if (!parts_.empty()) {
		Vector3 currentPosition = parts_[0]->GetPosition();
		Vector3 newPosition = Add(currentPosition, movement);
		parts_[0]->SetPosition(newPosition);
	}
}

void Boss::SetHeadRotationY(float rotationY) {
	if (!parts_.empty()) {
		parts_[0]->SetRotationY(rotationY);
	}
}