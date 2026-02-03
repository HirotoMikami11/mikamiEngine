#include "EnemyWorm.h"
#include "ImGui/ImGuiManager.h"
#include <numbers>
#include <format>

EnemyWorm::EnemyWorm()
	: dxCommon_(nullptr)
{
}

EnemyWorm::~EnemyWorm() = default;

void EnemyWorm::Initialize(DirectXCommon* dxCommon, const Vector3& position) {
	dxCommon_ = dxCommon;
	position_ = position;

	// 初期回転角度を設定（奥向き）
	currentHeadRotationY_ = std::numbers::pi_v<float>;

	// パーツの初期化
	InitializeParts();

	// コライダーキャッシュを構築
	collidersCache_.clear();
	if (head_) collidersCache_.push_back(head_.get());
	for (auto& body : bodies_) {
		collidersCache_.push_back(body.get());
	}

	// パーツキャッシュを構築（頭→体1→...→体7の順）
	allPartsCache_.clear();
	if (head_) allPartsCache_.push_back(head_.get());
	for (auto& body : bodies_) {
		allPartsCache_.push_back(body.get());
	}
}

void EnemyWorm::InitializeParts() {
	// 体パーツ（白）× 7個 - 一番下が position_、上に向かって配置
	bodies_.clear();
	for (size_t i = 0; i < kBodyCount; ++i) {
		auto body = std::make_unique<WormBodyParts>();
		Vector3 bodyPosition = position_;
		// 一番下（i=6）が position_、上に向かって配置
		bodyPosition.y += partsDistance_ * (kBodyCount - 1 - i);
		body->Initialize(dxCommon_, bodyPosition, "Boss_Body", "white2x2");
		body->SetEnemy(this);
		bodies_.push_back(std::move(body));
	}

	// 頭パーツ（黄色）- 一番上に配置
	head_ = std::make_unique<WormHeadParts>();
	Vector3 headPosition = position_;
	headPosition.y += partsDistance_ * kBodyCount;  // 体7個分上
	head_->Initialize(dxCommon_, headPosition, "Boss_Head", "white2x2");
	head_->SetEnemy(this);
}

void EnemyWorm::Update(const Matrix4x4& viewProjectionMatrix) {
	if (!isActive_) {
		return;
	}

	// パーツの位置と回転を更新
	UpdatePartsTransform();

	// 各パーツの更新
	if (head_) head_->Update(viewProjectionMatrix);
	for (auto& body : bodies_) {
		body->Update(viewProjectionMatrix);
	}
}

void EnemyWorm::UpdatePartsTransform() {
	if (!head_ || bodies_.empty()) {
		return;
	}

	// フラグに応じて姿勢を更新
	if (lookAtPlayer_) {
		UpdateLookAtPlayerPose();
	} else {
		UpdateStraightPose();
	}
}

void EnemyWorm::UpdateStraightPose() {
	// 直立状態：すべてのパーツを上方向に一直線に並べる
	// モデルは回転0で手前を向くので、Y軸180度回転で奥を向かせる
	// 円柱モデルを縦向きにするため、X軸90度回転を追加
	const float straightRotationY = std::numbers::pi_v<float>;  // 180度
	const float modelRotationX = std::numbers::pi_v<float> / 2.0f;  // 90度（縦向き）

	// 頭の回転を直立状態に補間
	currentHeadRotationY_ = straightRotationY;

	// 一番下の体パーツ（体6）を position_ に固定
	for (size_t i = 0; i < bodies_.size(); ++i) {
		Vector3 bodyPos = position_;
		bodyPos.y += partsDistance_ * (kBodyCount - 1 - i);  // 下から順に上へ
		bodies_[i]->SetPosition(bodyPos);
		bodies_[i]->SetRotation({ modelRotationX, straightRotationY, 0.0f });
	}

	// 頭の位置と回転を設定（一番上）
	Vector3 headPos = position_;
	headPos.y += partsDistance_ * kBodyCount;
	head_->SetPosition(headPos);
	head_->SetRotation({ modelRotationX, straightRotationY, 0.0f });
}

void EnemyWorm::UpdateLookAtPlayerPose() {
	// プレイヤー追尾状態：下部は直立、上部のみがカーブしてプレイヤー方向を向く

	const float straightRotationY = std::numbers::pi_v<float>;  // 直立時の角度（180度）
	const float modelRotationX = std::numbers::pi_v<float> / 2.0f;  // 90度（縦向き）

	// === 下部：固定パーツ（直立状態） ===
	for (size_t i = kBodyCount - fixedPartsCount_; i < kBodyCount; ++i) {
		Vector3 bodyPos = position_;
		bodyPos.y += partsDistance_ * (kBodyCount - 1 - i);  // 下から順に上へ
		bodies_[i]->SetPosition(bodyPos);
		bodies_[i]->SetRotation({ modelRotationX, straightRotationY, 0.0f });
	}

	// === 上部：自由パーツ（カーブ） ===
	// 最初の自由パーツのインデックス
	size_t firstFreePartIndex = kBodyCount - fixedPartsCount_ - 1;

	// 頭の目標回転角度を計算
	float targetHeadRotationY = CalculateHeadTargetRotation();

	// 頭の回転を補間（ゆっくり動く）
	float angleDiff = targetHeadRotationY - currentHeadRotationY_;
	// 角度を-π ~ πの範囲に正規化
	while (angleDiff > std::numbers::pi_v<float>) angleDiff -= 2.0f * std::numbers::pi_v<float>;
	while (angleDiff < -std::numbers::pi_v<float>) angleDiff += 2.0f * std::numbers::pi_v<float>;
	// 補間
	currentHeadRotationY_ += angleDiff * headRotationSpeed_;

	// === ステップ1: 位置の計算（下から上へ） ===
	Vector3 prevPos;

	// 最初の自由パーツは固定部分の一番上から開始
	if (fixedPartsCount_ > 0) {
		size_t lastFixedIndex = kBodyCount - fixedPartsCount_;
		prevPos = bodies_[lastFixedIndex]->GetPosition();
	} else {
		prevPos = position_;
	}

	// 自由パーツの位置を下から上へ計算
	for (int i = static_cast<int>(firstFreePartIndex); i >= 0; --i) {
		// 上方向に伸びる
		Vector3 direction = { 0.0f, 1.0f, 0.0f };
		Vector3 currentPos = Add(prevPos, Multiply(direction, partsDistance_));
		bodies_[i]->SetPosition(currentPos);
		prevPos = currentPos;
	}

	// 頭の位置を計算
	{
		Vector3 body1Pos = bodies_[0]->GetPosition();
		Vector3 headDirection = {
			std::sin(currentHeadRotationY_),
			1.0f,  // 上方向
			std::cos(currentHeadRotationY_)
		};
		headDirection = Normalize(headDirection);
		Vector3 headPos = Add(body1Pos, Multiply(headDirection, partsDistance_));
		head_->SetPosition(headPos);
	}

	// === ステップ2: 回転の計算（位置確定後、上から下へ） ===
	
	// 頭の回転：体1の方向を向く（下を向く）
	{
		Vector3 headPos = head_->GetPosition();
		Vector3 body1Pos = bodies_[0]->GetPosition();
		Vector3 direction = Subtract(body1Pos, headPos);  // 体1 - 頭（下を向く）
		if (Length(direction) > 0.001f) {
			float rotationY = std::atan2(-direction.x, -direction.z);
			head_->SetRotation({ modelRotationX, rotationY, 0.0f });
		} else {
			head_->SetRotation({ modelRotationX, currentHeadRotationY_, 0.0f });
		}
	}

	// 自由パーツの回転：次のパーツ（下のパーツ）の方向を向く
	for (int i = 0; i <= static_cast<int>(firstFreePartIndex); ++i) {
		Vector3 currentPartPos = bodies_[i]->GetPosition();
		Vector3 nextPartPos;

		// 次のパーツ（下のパーツ）の位置を取得
		if (i == static_cast<int>(firstFreePartIndex)) {
			// 最下部の自由パーツ：固定部分の一番上を向く
			if (fixedPartsCount_ > 0) {
				size_t lastFixedIndex = kBodyCount - fixedPartsCount_;
				nextPartPos = bodies_[lastFixedIndex]->GetPosition();
			} else {
				nextPartPos = position_;
			}
		} else {
			// それ以外：次の体パーツ（下）を向く
			nextPartPos = bodies_[i + 1]->GetPosition();
		}

		// このパーツから次のパーツ（下）への方向ベクトル
		Vector3 direction = Subtract(nextPartPos, currentPartPos);  // 下 - 現在
		if (Length(direction) > 0.001f) {
			float rotationY = std::atan2(-direction.x, -direction.z);
			bodies_[i]->SetRotation({ modelRotationX, rotationY, 0.0f });
		} else {
			bodies_[i]->SetRotation({ modelRotationX, straightRotationY, 0.0f });
		}
	}
}

float EnemyWorm::CalculateHeadTargetRotation() const {
	// 頭の現在位置を取得（更新前）
	Vector3 headPos = head_->GetPosition();

	// 頭からプレイヤーへの方向ベクトル
	Vector3 headToPlayer = Subtract(playerPosition_, headPos);

	// Y軸回転角度を計算（XZ平面での角度）
	// モデルの顔（-Z方向）がプレイヤーを向くように計算
	float targetRotationY = std::atan2(headToPlayer.x, headToPlayer.z);

	return targetRotationY;
}

void EnemyWorm::Draw() {
	if (!isActive_) {
		return;
	}

	if (head_) head_->Draw();
	for (auto& body : bodies_) {
		body->Draw();
	}
}

void EnemyWorm::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode("EnemyWorm")) {
		// HP情報
		if (ImGui::CollapsingHeader("HP", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("HP: %.1f / %.1f", hp_, maxHP_);
			float hpPercentage = (maxHP_ > 0.0f) ? (hp_ / maxHP_) * 100.0f : 0.0f;
			ImGui::ProgressBar(hp_ / maxHP_, ImVec2(0.0f, 0.0f), std::format("{:.1f}%%", hpPercentage).c_str());

			ImGui::DragFloat("Max HP", &maxHP_, 1.0f, 1.0f, 1000.0f);
			if (ImGui::Button("Reset HP")) {
				hp_ = maxHP_;
			}
			ImGui::SameLine();
			if (ImGui::Button("Take Damage 10")) {
				TakeDamage(10.0f);
			}
		}

		// 位置とパラメータ
		if (ImGui::CollapsingHeader("Transform & Motion", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Position (Bottom Part)");
			if (ImGui::DragFloat3("Position", &position_.x, 0.1f)) {
				// 位置変更時は即座に反映
				UpdatePartsTransform();
			}

			ImGui::Separator();
			ImGui::Text("Motion Parameters");
			ImGui::DragFloat("Parts Distance", &partsDistance_, 0.1f, 0.5f, 5.0f);
			
			ImGui::Separator();
			ImGui::Text("Player Tracking");
			ImGui::Checkbox("Look At Player", &lookAtPlayer_);
			ImGui::DragFloat3("Player Position", &playerPosition_.x, 0.1f);
			
			ImGui::Separator();
			ImGui::Text("Curve Settings");
			ImGui::DragFloat("Curve Smoothness", &curveSmoothness_, 0.01f, 0.0f, 1.0f);
			ImGui::Text("(0.0 = Sharp, 1.0 = Smooth)");
			
			int fixedCount = static_cast<int>(fixedPartsCount_);
			if (ImGui::SliderInt("Fixed Parts Count", &fixedCount, 0, static_cast<int>(kBodyCount))) {
				fixedPartsCount_ = static_cast<size_t>(fixedCount);
			}
			ImGui::Text("(Bottom %zu parts are fixed)", fixedPartsCount_);
			
			ImGui::DragFloat("Head Rotation Speed", &headRotationSpeed_, 0.01f, 0.01f, 1.0f);
			ImGui::Text("(0.01 = Slow, 1.0 = Instant)");
			
			ImGui::Separator();
			ImGui::Text("Current Head Rotation: %.2f rad (%.1f deg)", 
				currentHeadRotationY_, 
				currentHeadRotationY_ * 180.0f / std::numbers::pi_v<float>);
		}

		// 状態
		if (ImGui::CollapsingHeader("Status")) {
			ImGui::Checkbox("Is Active", &isActive_);
			ImGui::Checkbox("Show Colliders", &showColliders_);
			
			if (head_) head_->SetColliderVisible(showColliders_);
			for (auto& body : bodies_) {
				body->SetColliderVisible(showColliders_);
			}
		}

		// パーツ情報
		if (ImGui::CollapsingHeader("Parts")) {
			ImGui::Text("Total Parts: %zu (Head 1 + Body %zu)", allPartsCache_.size(), kBodyCount);
			ImGui::Text("Fixed Parts: %zu (Bottom)", fixedPartsCount_);
			ImGui::Text("Free Parts: %zu (Top + Head)", kBodyCount - fixedPartsCount_ + 1);
			ImGui::Separator();
			
			if (head_) head_->ImGui("Head (Yellow)");
			for (size_t i = 0; i < bodies_.size(); ++i) {
				bool isFixed = (i >= kBodyCount - fixedPartsCount_);
				std::string label = std::format("Body {} (White) {}", 
					i + 1, 
					isFixed ? "[FIXED]" : "[FREE]");
				bodies_[i]->ImGui(label.c_str());
			}
		}

		ImGui::TreePop();
	}
#endif
}

void EnemyWorm::TakeDamage(float damage) {
	if (!isActive_) {
		return;
	}

	hp_ -= damage;
	if (hp_ <= 0.0f) {
		hp_ = 0.0f;
		SetActive(false);
	}
}

const std::vector<Collider*>& EnemyWorm::GetColliders() {
	return collidersCache_;
}

Vector3 EnemyWorm::GetPosition() const {
	return position_;
}

void EnemyWorm::SetActive(bool active) {
	isActive_ = active;

	// 全パーツのアクティブ状態を設定
	if (head_) head_->SetActive(active);
	for (auto& body : bodies_) {
		body->SetActive(active);
	}

	// 非アクティブ時は黒に変更
	if (!active) {
		if (head_) head_->SetColor(0x000000FF);
		for (auto& body : bodies_) {
			body->SetColor(0x000000FF);
		}
	}
}
