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

	// パーツの初期化
	InitializeParts();

	// コライダーキャッシュを構築
	collidersCache_.clear();
	if (head_) collidersCache_.push_back(head_.get());
	if (body_) collidersCache_.push_back(body_.get());
	if (tail_) collidersCache_.push_back(tail_.get());

	// パーツキャッシュを構築
	allPartsCache_.clear();
	if (head_) allPartsCache_.push_back(head_.get());
	if (body_) allPartsCache_.push_back(body_.get());
	if (tail_) allPartsCache_.push_back(tail_.get());
}

void EnemyWorm::InitializeParts() {
	// 頭パーツ（黄色）
	head_ = std::make_unique<WormHeadParts>();
	head_->Initialize(dxCommon_, position_, "Boss_Head", "white2x2");
	head_->SetEnemy(this);

	// 体パーツ（白）
	body_ = std::make_unique<WormBodyParts>();
	Vector3 bodyPosition = position_;
	bodyPosition.z += partsDistance_;  // 頭の後ろに配置
	body_->Initialize(dxCommon_, bodyPosition, "Boss_Body", "white2x2");
	body_->SetEnemy(this);

	// 尻尾パーツ（緑）
	tail_ = std::make_unique<WormTailParts>();
	Vector3 tailPosition = position_;
	tailPosition.z += partsDistance_ * 2.0f;  // 体の後ろに配置
	tail_->Initialize(dxCommon_, tailPosition, "Boss_Tail", "white2x2");
	tail_->SetEnemy(this);
}

void EnemyWorm::Update(const Matrix4x4& viewProjectionMatrix) {
	if (!isActive_) {
		return;
	}

	// うねうね動作の更新
	UpdateWaveMotion();

	// パーツの位置と回転を更新
	UpdatePartsTransform();

	// 各パーツの更新
	if (head_) head_->Update(viewProjectionMatrix);
	if (body_) body_->Update(viewProjectionMatrix);
	if (tail_) tail_->Update(viewProjectionMatrix);
}

void EnemyWorm::UpdateWaveMotion() {
	// タイマーを進める
	waveTimer_ += waveSpeed_ * (1.0f / 60.0f);  // 60FPS想定

	// 2πを超えたらリセット
	if (waveTimer_ > 2.0f * std::numbers::pi_v<float>) {
		waveTimer_ -= 2.0f * std::numbers::pi_v<float>;
	}
}

void EnemyWorm::UpdatePartsTransform() {
	if (!head_ || !body_ || !tail_) {
		return;
	}

	// 頭の位置を設定（基準位置 + 上下動）
	Vector3 headPos = position_;
	headPos.y += std::sin(waveTimer_) * waveAmplitude_;
	head_->SetPosition(headPos);
	head_->SetRotation({ 0.0f, std::numbers::pi_v<float>, 0.0f });  // 180度回転（手前を向く）

	// 体の位置を設定（頭に追従、少し遅延）
	Vector3 bodyPos = position_;
	bodyPos.z += partsDistance_;
	bodyPos.y += std::sin(waveTimer_ - 1.0f) * waveAmplitude_;  // 位相をずらす
	body_->SetPosition(bodyPos);

	// 体の向きを頭の方向に設定
	Vector3 bodyToHead = Subtract(headPos, bodyPos);
	if (Length(bodyToHead) > 0.001f) {
		float rotationY = std::atan2(bodyToHead.x, bodyToHead.z);
		body_->SetRotationY(rotationY);
	}

	// 尻尾の位置を設定（体に追従、さらに遅延）
	Vector3 tailPos = position_;
	tailPos.z += partsDistance_ * 2.0f;
	tailPos.y += std::sin(waveTimer_ - 2.0f) * waveAmplitude_;  // さらに位相をずらす
	tail_->SetPosition(tailPos);

	// 尻尾の向きを体の方向に設定
	Vector3 tailToBody = Subtract(bodyPos, tailPos);
	if (Length(tailToBody) > 0.001f) {
		float rotationY = std::atan2(tailToBody.x, tailToBody.z);
		tail_->SetRotationY(rotationY);
	}
}

void EnemyWorm::Draw() {
	if (!isActive_) {
		return;
	}

	if (head_) head_->Draw();
	if (body_) body_->Draw();
	if (tail_) tail_->Draw();
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
			if (ImGui::DragFloat3("Position", &position_.x, 0.1f)) {
				// 位置変更時は即座に反映
				UpdatePartsTransform();
			}

			ImGui::Separator();
			ImGui::Text("Wave Motion Parameters");
			ImGui::DragFloat("Wave Speed", &waveSpeed_, 0.1f, 0.0f, 10.0f);
			ImGui::DragFloat("Wave Amplitude", &waveAmplitude_, 0.1f, 0.0f, 5.0f);
			ImGui::DragFloat("Parts Distance", &partsDistance_, 0.1f, 0.5f, 5.0f);
			
			ImGui::Text("Wave Timer: %.2f", waveTimer_);
		}

		// 状態
		if (ImGui::CollapsingHeader("Status")) {
			ImGui::Checkbox("Is Active", &isActive_);
			ImGui::Checkbox("Show Colliders", &showColliders_);
			
			if (head_) head_->SetColliderVisible(showColliders_);
			if (body_) body_->SetColliderVisible(showColliders_);
			if (tail_) tail_->SetColliderVisible(showColliders_);
		}

		// パーツ情報
		if (ImGui::CollapsingHeader("Parts")) {
			if (head_) head_->ImGui("Head (Yellow)");
			if (body_) body_->ImGui("Body (White)");
			if (tail_) tail_->ImGui("Tail (Green)");
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
	if (body_) body_->SetActive(active);
	if (tail_) tail_->SetActive(active);

	// 非アクティブ時は黒に変更
	if (!active) {
		if (head_) head_->SetColor(0x000000FF);
		if (body_) body_->SetColor(0x000000FF);
		if (tail_) tail_->SetColor(0x000000FF);
	}
}
