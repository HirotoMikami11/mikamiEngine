#include "Boss.h"
#include <format>

#include "Logger.h"
#include "State/IdleState.h"
#include "State/DebugMoveState.h"
#include "State/SplineMoveState.h"
#include "State/SplineMove8WayShootState.h"
#include "State/SplineMoveRotateShootState.h"
#include "ImGui/ImGuiManager.h"
#include "Bullet/BossBullet.h"

Boss::Boss()
	: dxCommon_(nullptr)
	, previousHeadPosition_({ 0.0f, 0.0f, 0.0f })
{
}

Boss::~Boss() = default;

void Boss::Initialize(DirectXCommon* dxCommon, const Vector3& position) {
	dxCommon_ = dxCommon;

	// パーツの初期化
	InitializeParts();

	// 初期位置を設定
	if (head_) {
		head_->SetPosition(position);
		previousHeadPosition_ = position;

		// 初期位置履歴を作成（全パーツが正しい位置に配置されるように）
		Vector3 historyPosition = position;
		positionHistory_.push_back(historyPosition);

		// 各パーツの初期位置を計算（allPartsCache_を使用）
		for (size_t i = 1; i < allPartsCache_.size(); ++i) {
			BaseParts* prevPart = allPartsCache_[i - 1];
			BaseParts* currentPart = allPartsCache_[i];

			// 前のパーツと現在のパーツの間の距離を計算
			float distance = CalculateDistanceBetweenParts(prevPart, currentPart);

			// Z軸方向に移動
			historyPosition.z -= distance;
			positionHistory_.push_back(historyPosition);
		}

		// 履歴から各パーツの位置を設定
		UpdatePartsPositionsFromHistory();
	}

	// パーツのHPを設定
	SetPartsHP();

	// UIクラスの初期化
	bossUI_ = std::make_unique<BossUI>();
	bossUI_->Initialize(dxCommon_);

	// スプラインシステムの初期化
	DebugDrawLineSystem* debugDrawSystem = Engine::GetInstance()->GetDebugDrawManager();

	splineTrack_ = std::make_unique<BossSplineTrack>();
	splineMovement_ = std::make_unique<BossSplineMovement>();
	splineDebugger_ = std::make_unique<BossSplineDebugger>();
	moveEditor_ = std::make_unique<BossMoveEditor>();

	splineMovement_->Initialize(splineTrack_.get());
	splineDebugger_->Initialize(debugDrawSystem, splineTrack_.get());
	moveEditor_->Initialize(splineTrack_.get(), splineMovement_.get(), splineDebugger_.get());

	// 爆発エミッターの初期化
	explosionEmitter_ = std::make_unique<BossExplosionEmitter>();
	explosionEmitter_->Initialize(this);

	// PhaseManagerの初期化（ExplosionEmitterの後）
	phaseManager_ = std::make_unique<BossPhaseManager>();
	phaseManager_->Initialize(this, explosionEmitter_.get());

	// StateManagerの初期化（PhaseManager から Phase1の設定を受け取る）
	stateManager_ = std::make_unique<BossStateManager>();
	const PhaseConfig& phase1Config = phaseManager_->GetCurrentPhaseConfig();
	stateManager_->Initialize(phase1Config);

	// Phase1のパーツ状態を設定（衝突属性とアクティブ状態）
	UpdatePartsState();

	// 初期Stateを設定
	currentState_ = std::make_unique<SplineMoveRotateShootState>("resources/CSV/BossMove/RotateShoot_1.csv");
	currentState_->Initialize();

	// 弾のプールを作成
	bulletPool_ = std::make_unique<BossBulletPool>();
	bulletPool_->Initialize(dxCommon_, kBulletPoolSize);

	// 砂煙エミッターの初期化
	smokeEmitter_ = std::make_unique<BossSmokeEmitter>();
	smokeEmitter_->Initialize(this);

	// 破壊煙エミッターの初期化
	smokeBreakEmitter_ = std::make_unique<BossBreakSmokeEmitter>();
	smokeBreakEmitter_->Initialize(this);
}

float Boss::CalculateDistanceBetweenParts(BaseParts* part1, BaseParts* part2) {
	if (!part1 || !part2) {
		return kBasePartSize + partsOffset_;
	}

	// 各パーツのZ方向のサイズを取得（スケールを考慮）
	float part1Size = part1->GetScale().z * kBasePartSize;
	float part2Size = part2->GetScale().z * kBasePartSize;

	// 2つのパーツの半径の合計 + オフセット
	float distance = (part1Size / 2.0f) + (part2Size / 2.0f) + partsOffset_;

	return distance;
}

void Boss::Update(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewProjectionMatirxSprite)
{
	// PhaseManager更新（Phase遷移チェック、Death演出制御）
	if (phaseManager_) {
		phaseManager_->Update();

		// Phase変更があった場合、StateManagerに新しい設定を適用
		if (phaseManager_->HasPhaseChanged()) {
			const PhaseConfig& config = phaseManager_->GetCurrentPhaseConfig();
			stateManager_->UpdateConfig(config);
			UpdatePartsState();  // パーツの状態更新
			InvalidateActivePartsCache();
			phaseManager_->ResetPhaseChangeFlag();
		}
	}

	// State更新
	if (currentState_) {
		currentState_->Update(this);
	}

	// AI動作（Death中は動作しない）
	BossPhase currentPhase = GetCurrentPhase();
	if (currentPhase != BossPhase::Death &&
		stateManager_ && stateManager_->IsAIEnabled()) {
		if (currentState_ && strcmp(currentState_->GetStateName(), "Idle") == 0) {
			stateManager_->TransitionToRandomState(this);
		}
	}

	// Death演出中の自動進行処理
	// WaitingStateFinish状態でIdleになったら、次のフェーズへ自動進行
	if (currentPhase == BossPhase::Death && phaseManager_) {
		DeathSubPhase deathSubPhase = phaseManager_->GetDeathSubPhase();
		if (deathSubPhase == DeathSubPhase::WaitingStateFinish) {
			if (currentState_ && strcmp(currentState_->GetStateName(), "Idle") == 0) {
				// Idleになったので、PhaseManagerに終了を通知
				phaseManager_->NotifyStateFinished();
			}
		}
	}

	// Phase遷移チェック
	CheckPhaseTransition();

	// 位置履歴の更新
	UpdatePositionHistory();

	// 全パーツの位置・回転・行列を更新
	UpdateAllParts(viewProjectionMatrix);

	// 弾の更新
	UpdateBullets(viewProjectionMatrix);

	// 砂煙エミッターの更新
	if (smokeEmitter_) {
		smokeEmitter_->Update();
	}
	// 破壊煙エミッターの更新
	if (smokeBreakEmitter_) {
		smokeBreakEmitter_->Update();
	}
	// 爆発エミッターの更新
	if (explosionEmitter_) {
		explosionEmitter_->Update();
	}

	// UIの更新
	bossUI_->Update(bossHP_, maxBossHP_, viewProjectionMatirxSprite);

	// スプラインエディタの更新
	if (moveEditor_) {
		moveEditor_->Update();
	}
}

void Boss::Draw() {
	// 全パーツを描画
	if (head_) {
		head_->Draw();
	}
	for (auto& body : bodies_) {
		body->Draw();
	}
	if (tail_) {
		tail_->Draw();
	}

	DrawBullets();

	if (splineDebugger_) {
		splineDebugger_->Draw();
	}
}

void Boss::DrawUI()
{
	bossUI_->Draw();
}

void Boss::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode("Boss")) {
		// HP情報
		if (ImGui::CollapsingHeader("HP")) {
			ImGui::Text("Boss HP: %.1f / %.1f", bossHP_, maxBossHP_);
			float hpPercentage = (maxBossHP_ > 0.0f) ? (bossHP_ / maxBossHP_) * 100.0f : 0.0f;
			ImGui::ProgressBar(bossHP_ / maxBossHP_, ImVec2(0.0f, 0.0f), std::format("{:.1f}%%", hpPercentage).c_str());

			// HPのリセットボタン
			if (ImGui::Button("Reset HP")) {
				bossHP_ = maxBossHP_;
				SetPartsHP();
			}
			// HPのリセットボタン
			if (ImGui::Button("Take Damege 100")) {
				bossHP_ -= 100;
				SetPartsHP();
			}

			// UI情報
			bossUI_->ImGui();
		}

		// PhaseManager情報
		if (phaseManager_) {
			phaseManager_->ImGui();
		}

		// State情報
		if (ImGui::CollapsingHeader("State", ImGuiTreeNodeFlags_DefaultOpen)) {
			std::vector<std::string> stateNames = {
				"Idle",
				"DebugMove",
				"SplineMove",
				"8WayShot",
				"RotateShot"
			};

			static int stateIndex = 0;

			if (MyImGui::Combo("State Select", stateIndex, stateNames)) {
				switch (stateIndex) {
				case 0: ChangeState(std::make_unique<IdleState>()); break;
				case 1: ChangeState(std::make_unique<DebugMoveState>()); break;
				case 2: ChangeState(std::make_unique<SplineMoveState>("resources/CSV/BossMove/Move_1.csv")); break;
				case 3: ChangeState(std::make_unique<SplineMove8WayShootState>("resources/CSV/BossMove/Move_1.csv")); break;
				case 4: ChangeState(std::make_unique<SplineMoveRotateShootState>("resources/CSV/BossMove/RotateShoot_1.csv")); break;
				}
			}
			currentState_->ImGui();

			// StateManager表示
			if (stateManager_) {
				stateManager_->ImGui();
			}
		}

		// パラメータ設定
		if (ImGui::CollapsingHeader("Parameters")) {
			// パーツ間のオフセット（隙間）の調整
			ImGui::DragFloat("Parts Offset", &partsOffset_, 0.01f, 0.0f, 5.0f);
			ImGui::Text("(Distance between parts = Part1 Size/2 + Part2 Size/2 + Offset)");

			// 移動速度
			ImGui::DragFloat("Base Move Speed", &baseMoveSpeed_, 0.1f, 0.1f, 10.0f);
			ImGui::Text("Current Move Speed: %.2f (Base %.2f x Multiplier %.2f)",
				GetMoveSpeed(), baseMoveSpeed_,
				phaseManager_ ? phaseManager_->GetCurrentPhaseConfig().moveSpeedMultiplier : 1.0f);

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

		if (ImGui::CollapsingHeader("Bullet Pool Info")) {
			ImGui::Text("Pool Size: %zu", bulletPool_->GetPoolSize());
			ImGui::Text("Active Bullets: %zu", bulletPool_->GetActiveBulletCount());

			float usage = static_cast<float>(bulletPool_->GetActiveBulletCount()) /
				static_cast<float>(bulletPool_->GetPoolSize()) * 100.0f;
			ImGui::ProgressBar(usage / 100.0f, ImVec2(0.0f, 0.0f),
				std::format("Usage: {:.1f}%%", usage).c_str());

			if (usage > 90.0f) {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
					"Warning: Bullet pool almost full!");
			}

			if (ImGui::Button("Reset All Bullets")) {
				bulletPool_->ResetAll();
			}
		}

		// 爆発エミッター
		if (explosionEmitter_) {
			explosionEmitter_->ImGui();
		}

		// スプラインシステム
		if (ImGui::CollapsingHeader("Spline System")) {
			if (moveEditor_) {
				moveEditor_->ImGui();
			} else {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Move Editor not initialized");
			}
		}

		ImGui::TreePop();
	}
#endif
}

void Boss::InitializeParts() {
	// 頭パーツ（黄色）
	head_ = std::make_unique<HeadParts>();
	Vector3 headPosition = { 0.0f, 0.0f, 0.0f };
	head_->Initialize(dxCommon_, headPosition, "Boss_Head", "white2x2");

	// 体パーツ（白） × 6
	bodies_.clear();
	for (size_t i = 0; i < kBodyCount; ++i) {
		auto body = std::make_unique<BodyParts>();
		Vector3 bodyPosition = { 0.0f, 0.0f, -static_cast<float>(i + 1) * (kBasePartSize + partsOffset_) };
		body->Initialize(dxCommon_, bodyPosition, "Boss_Body", "white2x2");
		body->SetBoss(this);
		bodies_.push_back(std::move(body));
	}

	// 尻尾パーツ（緑）
	tail_ = std::make_unique<TailParts>();
	Vector3 tailPosition = { 0.0f, 0.0f, -static_cast<float>(kBodyCount + 1) * (kBasePartSize + partsOffset_) };
	tail_->Initialize(dxCommon_, tailPosition, "Boss_Tail", "white2x2");
	tail_->SetBoss(this);

	//キャッシュを作成
	RebuildPartsCache();
}

void Boss::SetPartsHP() {
	// 体パーツのHP: Boss HP / 6
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

void Boss::UpdatePartsPositionsFromHistory() {
	if (!head_ || positionHistory_.empty()) return;

	// 体と尻尾のパーツのみのリスト（頭以外）
	std::vector<BaseParts*> allParts;
	for (auto& body : bodies_) {
		allParts.push_back(body.get());
	}
	if (tail_) {
		allParts.push_back(tail_.get());
	}

	// 各パーツに対して、履歴から適切な位置を取得
	for (size_t i = 0; i < allParts.size(); ++i) {
		// 前のパーツ（0番目なら頭、それ以外は前の体パーツ）
		BaseParts* prevPart = allPartsCache_[i];
		BaseParts* currentPart = allParts[i];

		// 前のパーツまでの累積距離を計算
		float targetDistance = 0.0f;
		for (size_t j = 0; j <= i; ++j) {
			BaseParts* part1 = allPartsCache_[j];
			BaseParts* part2 = allPartsCache_[j + 1];
			targetDistance += CalculateDistanceBetweenParts(part1, part2);
		}

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

		// 位置を設定
		currentPart->SetPosition(targetPosition);
	}
}


void Boss::UpdateAllParts(const Matrix4x4& viewProjectionMatrix) {
	// 1. 位置設定（履歴から距離ベースで計算）
	UpdatePartsPositionsFromHistory();

	// 2. 各パーツの回転を設定
	for (size_t i = 1; i < allPartsCache_.size(); ++i) {
		auto* part = allPartsCache_[i];
		auto* prevPart = allPartsCache_[i - 1];

		Vector3 currentPos = part->GetPosition();
		Vector3 prevPos = prevPart->GetPosition();

		Vector3 direction = currentPos - prevPos;
		if (Length(direction) > 0.001f) {
			float rotationY = std::atan2(-direction.x, -direction.z);
			part->SetRotationY(rotationY);
		}
	}

	// 3. 全パーツの行列更新
	for (auto* part : allPartsCache_) {
		part->Update(viewProjectionMatrix);
	}
}

void Boss::CheckPhaseTransition() {
	if (!phaseManager_) {
		return;
	}

	BossPhase currentPhase = phaseManager_->GetCurrentPhase();

	// HP条件でPhase遷移をトリガー
	if (currentPhase == BossPhase::Phase1 && bossHP_ <= 0.0f) {
		phaseManager_->TriggerPhase2Transition();
		bossHP_ = maxBossHP_;  // HP回復
		SetPartsHP();
	} else if (currentPhase == BossPhase::Phase2 && bossHP_ <= 0.0f) {
		phaseManager_->TriggerDeathTransition();
	}
}

void Boss::UpdatePartsState() {
	BossPhase currentPhase = GetCurrentPhase();

	if (currentPhase == BossPhase::Phase1) {
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
			tail_->SetPhase1Attribute();
		}

	} else if (currentPhase == BossPhase::Phase2) {
		// Phase2: 頭と尻尾がアクティブ、体が非アクティブ

		// 頭は常にアクティブ（ダメージは受けない）
		if (head_) {
			head_->SetActive(true);
		}

		// 体を非アクティブに
		for (auto& body : bodies_) {
			body->SetActive(false);
			body->SetPhase2Attribute();
		}

		// 尻尾をアクティブにしてEnemy属性に
		if (tail_) {
			tail_->SetActive(true);
			tail_->SetPhase2Attribute();
		}

	} else if (currentPhase == BossPhase::Death) {
		// Death: 現在の状態を維持（爆発演出で順次非表示にする）
		// パーツの状態は爆発エミッターが制御
	}
}

void Boss::TakeDamageFromPart(float damage) {
	bossHP_ -= damage;
	if (bossHP_ < 0.0f) {
		bossHP_ = 0.0f;
	}
}

const std::vector<Collider*>& Boss::GetColliders() {
	return collidersCache_;
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

void Boss::SetHeadPosition(const Vector3& position) {
	if (head_) {
		head_->SetPosition(position);
	}
}

float Boss::GetMoveSpeed() const {
	if (phaseManager_) {
		const PhaseConfig& config = phaseManager_->GetCurrentPhaseConfig();
		return baseMoveSpeed_ * config.moveSpeedMultiplier;
	}
	return baseMoveSpeed_;
}

BossPhase Boss::GetCurrentPhase() const {
	if (phaseManager_) {
		return phaseManager_->GetCurrentPhase();
	}
	return BossPhase::Phase1;
}

DeathSubPhase Boss::GetDeathSubPhase() const {
	if (phaseManager_) {
		return phaseManager_->GetDeathSubPhase();
	}
	return DeathSubPhase::None;
}

void Boss::ClearPositionHistory() {
	positionHistory_.clear();

	// 現在の頭の位置を履歴の開始点として追加
	if (head_) {
		Vector3 headPosition = head_->GetPosition();
		previousHeadPosition_ = headPosition;

		// 各パーツのスケールに基づいて距離を計算し、初期履歴を作成
		Vector3 historyPosition = headPosition;
		positionHistory_.push_back(historyPosition);

		// 各パーツの初期位置を計算（allPartsCache_を使用）
		for (size_t i = 1; i < allPartsCache_.size(); ++i) {
			BaseParts* prevPart = allPartsCache_[i - 1];
			BaseParts* currentPart = allPartsCache_[i];

			// 前のパーツと現在のパーツの間の距離を計算
			float distance = CalculateDistanceBetweenParts(prevPart, currentPart);

			// 現在の向きの逆方向に移動
			Vector3 direction = { 0.0f, 0.0f, -1.0f };
			if (i == 1) {
				// 頭の向きを取得
				Vector3 headRotation = head_->GetRotation();
				float rotY = headRotation.y;
				direction.x = -std::sin(rotY);
				direction.z = -std::cos(rotY);
			}

			historyPosition = Add(historyPosition, Multiply(direction, distance));
			positionHistory_.push_back(historyPosition);
		}
	}
}

void Boss::AlignAllPartsInLine() {
	if (!head_) {
		return;
	}

	// 頭の位置と向きを取得
	Vector3 headPosition = head_->GetPosition();
	Vector3 headRotation = head_->GetRotation();
	float headRotationY = headRotation.y;

	// 頭の後ろ方向ベクトルを計算
	Vector3 backDirection = {
		std::sin(headRotationY),
		0.0f,
		std::cos(headRotationY)
	};

	// 位置履歴をクリア
	positionHistory_.clear();

	// 現在の位置から後ろ方向に配置
	Vector3 currentPosition = headPosition;

	for (size_t i = 0; i < allPartsCache_.size(); ++i) {
		BaseParts* currentPart = allPartsCache_[i];

		// 位置を設定
		currentPart->SetPosition(currentPosition);

		// 向きを設定（全て頭と同じ向き）
		currentPart->SetRotationY(headRotationY);

		// 位置履歴に追加
		positionHistory_.push_back(currentPosition);

		// 次のパーツの位置を計算（後ろ方向に移動）
		if (i < allPartsCache_.size() - 1) {
			BaseParts* nextPart = allPartsCache_[i + 1];

			// パーツ間の距離を計算
			float distance = CalculateDistanceBetweenParts(currentPart, nextPart);

			// 後ろ方向に距離分移動
			currentPosition.x += backDirection.x * distance;
			currentPosition.y += backDirection.y * distance;
			currentPosition.z += backDirection.z * distance;
		}
	}

	// 前回の頭の位置を更新
	previousHeadPosition_ = headPosition;
}

bool Boss::FireBullet(const Vector3& position, const Vector3& velocity) {
	return bulletPool_->FireBullet(position, velocity);
}

void Boss::UpdateBullets(const Matrix4x4& viewProjectionMatrix) {
	bulletPool_->Update(viewProjectionMatrix);
}

void Boss::DrawBullets() {
	bulletPool_->Draw();
}

std::vector<BossBullet*> Boss::GetActiveBullets() const {
	return bulletPool_->GetActiveBullets();
}

const std::vector<BaseParts*>& Boss::GetActiveBodyParts() const {
	if (activePartsCacheDirty_) {
		activePartsCache_.clear();

		BossPhase currentPhase = GetCurrentPhase();

		if (head_ && head_->IsActive()) {
			activePartsCache_.push_back(head_.get());
		}

		if (currentPhase == BossPhase::Phase1) {
			for (auto& body : bodies_) {
				if (body && body->IsActive()) {
					activePartsCache_.push_back(body.get());
				}
			}
		}

		if (currentPhase == BossPhase::Phase2) {
			if (tail_ && tail_->IsActive()) {
				activePartsCache_.push_back(tail_.get());
			}
		}

		activePartsCacheDirty_ = false;
	}

	return activePartsCache_;
}


const std::vector<BaseParts*>& Boss::GetAllBodyParts() const {
	return allPartsCache_;
}

void Boss::RebuildPartsCache() {
	// 全パーツキャッシュ
	allPartsCache_.clear();
	if (head_) allPartsCache_.push_back(head_.get());
	for (auto& body : bodies_) {
		allPartsCache_.push_back(body.get());
	}
	if (tail_) allPartsCache_.push_back(tail_.get());

	// 全コライダーキャッシュ
	collidersCache_.clear();
	if (head_) collidersCache_.push_back(head_.get());
	for (auto& body : bodies_) {
		collidersCache_.push_back(body.get());
	}
	if (tail_) collidersCache_.push_back(tail_.get());

	// アクティブパーツキャッシュを無効化
	activePartsCacheDirty_ = true;
}

void Boss::InvalidateActivePartsCache() {
	activePartsCacheDirty_ = true;
}