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
	if (head_) {
		head_->SetPosition(position);
		previousHeadPosition_ = position;

		// 初期位置履歴を作成（全パーツが正しい位置に配置されるように）
		size_t totalParts = 1 + kBodyCount + 1;  // 頭 + 体 + 尻尾
		for (size_t i = 0; i < totalParts; ++i) {
			Vector3 historyPosition = position;
			historyPosition.z -= static_cast<float>(i) * partsDistance_;
			positionHistory_.push_back(historyPosition);
		}

		// 各パーツの初期位置を設定
		UpdatePartsPositions();
	}

	// パーツのHPを設定
	SetPartsHP();

	// Phase1のパーツ状態を設定（衝突属性とアクティブ状態）
	UpdatePartsState();

	// 初期Stateを設定（Idle）
	currentState_ = std::make_unique<IdleState>();
	currentState_->Initialize();
}

void Boss::InitializeParts() {
	// 頭パーツ（黄色）
	head_ = std::make_unique<HeadParts>();
	head_->Initialize(directXCommon_, Vector3{ 0.0f, 0.0f, 0.0f });

	// 体パーツ（白） × 5
	bodies_.clear();
	for (size_t i = 0; i < kBodyCount; ++i) {
		auto body = std::make_unique<BodyParts>();
		Vector3 bodyPosition = { 0.0f, 0.0f, -static_cast<float>(i + 1) * partsDistance_ };
		body->Initialize(directXCommon_, bodyPosition);
		body->SetBoss(this);  // Bossへの参照を設定
		bodies_.push_back(std::move(body));
	}

	// 尻尾パーツ（緑）
	tail_ = std::make_unique<TailParts>();
	Vector3 tailPosition = { 0.0f, 0.0f, -static_cast<float>(kBodyCount + 1) * partsDistance_ };
	tail_->Initialize(directXCommon_, tailPosition);
	tail_->SetBoss(this);  // Bossへの参照を設定
}

void Boss::SetPartsHP() {
	// 体パーツのHP: Boss HP / 5
	float bodyHP = maxBossHP_ / static_cast<float>(kBodyCount);
	for (auto& body : bodies_) {
		body->SetHP(bodyHP);
	}

	// 尻尾のHP: Boss HPと同じ
	if (tail_) {
		tail_->SetHP(maxBossHP_);
	}
}

void Boss::SetHP(float hp) {
	maxBossHP_ = hp;
	bossHP_ = hp;
	SetPartsHP();
}

void Boss::Update(const Matrix4x4& viewProjectionMatrix) {
	viewProjectionMatrix_ = viewProjectionMatrix;

	// Stateの更新
	if (currentState_) {
		currentState_->Update(this);
	}

	// Phase遷移チェック
	CheckPhaseTransition();

	// 位置履歴の更新
	UpdatePositionHistory();

	// 各パーツの位置を更新
	UpdatePartsPositions();

	// 各パーツの向きを更新
	UpdatePartsRotations();

	// 各パーツの行列更新
	if (head_) {
		head_->Update(viewProjectionMatrix);
	}
	for (auto& body : bodies_) {
		body->Update(viewProjectionMatrix);
	}
	if (tail_) {
		tail_->Update(viewProjectionMatrix);
	}
}

void Boss::UpdatePositionHistory() {
	if (!head_) return;

	// 現在の頭の位置を取得
	Vector3 currentHeadPosition = head_->GetPosition();

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
	if (!head_ || positionHistory_.empty()) return;

	// 全パーツのリストを作成
	std::vector<BaseParts*> allParts;
	for (auto& body : bodies_) {
		allParts.push_back(body.get());
	}
	if (tail_) {
		allParts.push_back(tail_.get());
	}

	// 各パーツに対して、履歴から適切な位置を取得
	for (size_t i = 0; i < allParts.size(); ++i) {
		// このパーツが参照すべき履歴インデックスを計算
		// パーツ間距離 × (インデックス + 1) 分だけ離れた位置を参照（頭の次から）
		float targetDistance = static_cast<float>(i + 1) * partsDistance_;

		// 履歴を走査して、目標距離に最も近い位置を探す
		float accumulatedDistance = 0.0f;
		Vector3 targetPosition = head_->GetPosition();

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
		allParts[i]->SetPosition(targetPosition);
	}
}

void Boss::UpdatePartsRotations() {
	if (!head_) return;

	// 全パーツのリストを作成（頭を含む）
	std::vector<BaseParts*> allParts;
	allParts.push_back(head_.get());
	for (auto& body : bodies_) {
		allParts.push_back(body.get());
	}
	if (tail_) {
		allParts.push_back(tail_.get());
	}

	// 各パーツの向きを次のパーツ方向に設定
	for (size_t i = 0; i < allParts.size() - 1; ++i) {
		Vector3 currentPosition = allParts[i]->GetPosition();
		Vector3 nextPosition = allParts[i + 1]->GetPosition();

		// 次のパーツへの方向ベクトルを計算
		Vector3 direction = Subtract(nextPosition, currentPosition);
		float distance = Length(direction);

		// 距離が十分にある場合のみ回転を適用（ゼロ除算防止）
		if (distance > 0.001f) {
			// Y軸回転角度を計算（-Z方向が前方）
			float rotationY = std::atan2(-direction.x, -direction.z);
			allParts[i]->SetRotationY(rotationY);
		}
	}

	// 最後のパーツ（尻尾）は一つ前のパーツと同じ向き
	if (allParts.size() >= 2) {
		Vector3 lastRotation = allParts[allParts.size() - 2]->GetRotation();
		allParts[allParts.size() - 1]->SetRotation(lastRotation);
	}
}

void Boss::CheckPhaseTransition() {
	// 現在のPhaseと前回のPhaseが変わった場合のみ処理
	if (currentPhase_ != previousPhase_) {
		UpdatePartsState();
		previousPhase_ = currentPhase_;
	}

	// Phase遷移チェック
	if (currentPhase_ == BossPhase::Phase1 && bossHP_ <= 0.0f) {
		TransitionToPhase2();
	} else if (currentPhase_ == BossPhase::Phase2 && bossHP_ <= 0.0f) {
		TransitionToDeathPhase();
	}
}

void Boss::TransitionToPhase2() {
	currentPhase_ = BossPhase::Phase2;

	// BossのHPを全回復
	bossHP_ = maxBossHP_;

	// パーツのHPをリセット
	SetPartsHP();

	// パーツ状態を更新（アクティブ状態と衝突属性）
	UpdatePartsState();
}

void Boss::TransitionToDeathPhase() {
	currentPhase_ = BossPhase::Death;

	// パーツ状態を更新（全パーツ非アクティブ）
	UpdatePartsState();
}

void Boss::UpdatePartsState() {
	if (currentPhase_ == BossPhase::Phase1) {
		// Phase1: 頭と体がアクティブ、尻尾が非アクティブ

		// 頭は常にアクティブ（ダメージは受けない）
		if (head_) {
			head_->SetActive(true);
		}

		// 体をアクティブにしてEnemy属性に
		for (auto& body : bodies_) {
			body->SetActive(true);
			body->SetPhase1Attribute();
		}

		// 尻尾を非アクティブに
		if (tail_) {
			tail_->SetActive(false);
			tail_->SetPhase1Attribute();  // 属性は設定するが、非アクティブなのでダメージは受けない
		}

	} else if (currentPhase_ == BossPhase::Phase2) {
		// Phase2: 頭と尻尾がアクティブ、体が非アクティブ

		// 頭は常にアクティブ（ダメージは受けない）
		if (head_) {
			head_->SetActive(true);
		}

		// 体を非アクティブに
		for (auto& body : bodies_) {
			body->SetActive(false);
			body->SetPhase2Attribute();  // 属性は設定するが、非アクティブなのでダメージは受けない
		}

		// 尻尾をアクティブにしてEnemy属性に
		if (tail_) {
			tail_->SetActive(true);
			tail_->SetPhase2Attribute();
		}

	} else if (currentPhase_ == BossPhase::Death) {
		// Death: すべて非アクティブ

		if (head_) {
			head_->SetActive(false);
		}

		for (auto& body : bodies_) {
			body->SetActive(false);
		}

		if (tail_) {
			tail_->SetActive(false);
		}
	}
}

void Boss::TakeDamageFromPart(float damage) {
	bossHP_ -= damage;
	if (bossHP_ < 0.0f) {
		bossHP_ = 0.0f;
	}
}

void Boss::Draw(const Light& directionalLight) {
	// 全パーツを描画
	if (head_) {
		head_->Draw(directionalLight);
	}
	for (auto& body : bodies_) {
		body->Draw(directionalLight);
	}
	if (tail_) {
		tail_->Draw(directionalLight);
	}
}

std::vector<Collider*> Boss::GetColliders() {
	std::vector<Collider*> colliders;


	if (head_) {
		colliders.push_back(head_.get());
	}

	// 体パーツ
	for (auto& body : bodies_) {
		colliders.push_back(body.get());
	}

	// 尻尾パーツ
	if (tail_) {
		colliders.push_back(tail_.get());
	}

	return colliders;
}

void Boss::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode("Boss")) {
		// HP情報
		if (ImGui::CollapsingHeader("HP", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Boss HP: %.1f / %.1f", bossHP_, maxBossHP_);
			float hpPercentage = (maxBossHP_ > 0.0f) ? (bossHP_ / maxBossHP_) * 100.0f : 0.0f;
			ImGui::ProgressBar(bossHP_ / maxBossHP_, ImVec2(0.0f, 0.0f), std::format("{:.1f}%%", hpPercentage).c_str());

			// HPのリセットボタン
			if (ImGui::Button("Reset HP")) {
				bossHP_ = maxBossHP_;
				SetPartsHP();
			}
		}

		// Phase情報
		if (ImGui::CollapsingHeader("Phase", ImGuiTreeNodeFlags_DefaultOpen)) {
			const char* phaseNames[] = { "Phase 1", "Phase 2 ", "Death" };
			int currentPhaseIndex = static_cast<int>(currentPhase_);
			ImGui::Text("Current Phase: %s", phaseNames[currentPhaseIndex]);

			// Phase強制変更ボタン（デバッグ用）
			if (currentPhase_ == BossPhase::Phase1) {
				if (ImGui::Button("Force Transition to Phase2")) {
					TransitionToPhase2();
				}
			} else if (currentPhase_ == BossPhase::Phase2) {
				if (ImGui::Button("Force Transition to Death")) {
					TransitionToDeathPhase();
				}
			}
		}

		// State情報
		if (ImGui::CollapsingHeader("State")) {
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

			// コライダー表示フラグ
			ImGui::Checkbox("Show Colliders", &showColliders_);
			if (head_) head_->SetColliderVisible(showColliders_);
			for (auto& body : bodies_) body->SetColliderVisible(showColliders_);
			if (tail_) tail_->SetColliderVisible(showColliders_);
		}

		// パーツ情報
		if (ImGui::CollapsingHeader("Parts")) {
			size_t totalParts = 1 + kBodyCount + 1;
			ImGui::Text("Total Parts: %zu", totalParts);
			ImGui::Separator();

			// 頭
			if (head_) {
				head_->ImGui("Head (Yellow)");
			}

			// 体
			for (size_t i = 0; i < bodies_.size(); ++i) {
				std::string label = std::format("Body {} (White)", i + 1);
				bodies_[i]->ImGui(label.c_str());
			}

			// 尻尾
			if (tail_) {
				tail_->ImGui("Tail (Green)");
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
	if (head_) {
		return head_->GetPosition();
	}
	return Vector3{ 0.0f, 0.0f, 0.0f };
}

void Boss::MoveHead(const Vector3& movement) {
	if (head_) {
		Vector3 currentPosition = head_->GetPosition();
		Vector3 newPosition = Add(currentPosition, movement);
		head_->SetPosition(newPosition);
	}
}

void Boss::SetHeadRotationY(float rotationY) {
	if (head_) {
		head_->SetRotationY(rotationY);
	}
}