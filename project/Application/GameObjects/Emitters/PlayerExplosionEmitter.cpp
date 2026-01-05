#include "PlayerExplosionEmitter.h"
#include "Player.h"
#include "ImGui/ImGuiManager.h"

PlayerExplosionEmitter::PlayerExplosionEmitter()
	: player_(nullptr)
	, particleEditor_(nullptr)
	, explosionInstance_(nullptr)
	, isSequenceActive_(false)
	, hasExploded_(false)
	, isComplete_(false)
	, waitTimer_(0.0f)
{
}

PlayerExplosionEmitter::~PlayerExplosionEmitter()
{
	Finalize();
}

void PlayerExplosionEmitter::Initialize(Player* player)
{
	player_ = player;
	particleEditor_ = ParticleEditor::GetInstance();

	// パーティクルインスタンスを作成
	CreateParticleInstance();
}

void PlayerExplosionEmitter::CreateParticleInstance()
{
	if (!particleEditor_) {
		return;
	}

	// 既存インスタンスがあれば削除（重複回避）
	auto* existing = particleEditor_->GetInstance(kInstanceName_);
	if (existing) {
		particleEditor_->DestroyInstance(kInstanceName_);
	}

	// 新しいインスタンスを作成
	particleEditor_->CreateInstance(kExplosionPresetName_, kInstanceName_);
	explosionInstance_ = particleEditor_->GetInstance(kInstanceName_);

	// 初期状態でエミッターを無効化
	if (explosionInstance_) {
		auto* emitter = explosionInstance_->GetEmitter(kEmitterName_);
		if (emitter) {
			emitter->SetEmitEnabled(false);
		}
	}
}

void PlayerExplosionEmitter::StartExplosionSequence()
{
	if (!explosionInstance_) {
		return;
	}

	// シーケンス開始
	isSequenceActive_ = true;
	hasExploded_ = false;
	isComplete_ = false;
	waitTimer_ = 0.0f;

	// 爆発を実行
	ExecuteExplosion();
}

void PlayerExplosionEmitter::ExecuteExplosion()
{
	if (!explosionInstance_ || !player_) {
		return;
	}

	auto* emitter = explosionInstance_->GetEmitter(kEmitterName_);
	if (!emitter) {
		return;
	}

	// プレイヤーの位置にエミッターを配置
	Vector3 playerPosition = player_->GetWorldPosition();
	emitter->GetTransform().SetPosition(playerPosition);

	// 放出開始
	emitter->SetEmitEnabled(true);

	// カメラシェイク
	CameraController::GetInstance()->StartCameraShake(1.0f, 0.8f);

	// 爆発済みフラグを立てる
	hasExploded_ = true;
}

bool PlayerExplosionEmitter::IsEffectFinished() const
{
	if (!explosionInstance_) {
		return true;
	}

	auto* emitter = explosionInstance_->GetEmitter(kEmitterName_);
	if (!emitter) {
		return true;
	}

	// エミッターが放出を停止したかチェック
	return !emitter->IsEmitting();
}

void PlayerExplosionEmitter::Update()
{
	if (!isSequenceActive_) {
		return;
	}

	// 爆発済みで、エフェクトが終了している場合
	if (hasExploded_ && IsEffectFinished()) {
		// 待機タイマーを進める
		waitTimer_ += 1.0f / 60.0f;

		// 待機時間が経過したら演出完了
		if (waitTimer_ >= waitTime_) {
			isComplete_ = true;
			isSequenceActive_ = false;
		}
	}
}

void PlayerExplosionEmitter::Finalize()
{
	if (!particleEditor_) {
		return;
	}

	// インスタンスを削除
	if (explosionInstance_) {
		particleEditor_->DestroyInstance(kInstanceName_);
		explosionInstance_ = nullptr;
	}
}

void PlayerExplosionEmitter::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode("Player Explosion Emitter")) {
		ImGui::Checkbox("Sequence Active", &isSequenceActive_);
		ImGui::Checkbox("Has Exploded", &hasExploded_);
		ImGui::Checkbox("Is Complete", &isComplete_);

		ImGui::Separator();

		// パラメータ調整
		ImGui::DragFloat("Wait Time (sec)", &waitTime_, 0.1f, 0.0f, 5.0f);

		if (hasExploded_ && !isComplete_) {
			ImGui::Text("Wait Timer: %.2f / %.2f", waitTimer_, waitTime_);
			ImGui::ProgressBar(waitTimer_ / waitTime_);
		}

		ImGui::Separator();

		// エフェクト状態
		if (explosionInstance_) {
			auto* emitter = explosionInstance_->GetEmitter(kEmitterName_);
			if (emitter) {
				bool isEmitting = emitter->IsEmitting();
				ImGui::Text("Effect Emitting: %s", isEmitting ? "Yes" : "No");

				Vector3 pos = emitter->GetTransform().GetPosition();
				ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
			}
		}

		ImGui::Separator();

		// テストボタン
		if (ImGui::Button("Start Explosion Test")) {
			StartExplosionSequence();
		}

		ImGui::TreePop();
	}
#endif
}