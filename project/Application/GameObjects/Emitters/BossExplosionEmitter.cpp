#include "BossExplosionEmitter.h"
#include "Boss.h"
#include "ImGui/ImGuiManager.h"
#include <format>

BossExplosionEmitter::BossExplosionEmitter()
	: boss_(nullptr)
	, particleEditor_(nullptr)
	, currentExplosionIndex_(-1)
	, isSequenceActive_(false)
{
}

BossExplosionEmitter::~BossExplosionEmitter()
{
	Finalize();
}

void BossExplosionEmitter::Initialize(Boss* boss)
{
	boss_ = boss;
	particleEditor_ = ParticleEditor::GetInstance();

	// パーティクルインスタンスを作成
	CreateParticleInstances();
}

void BossExplosionEmitter::CreateParticleInstances()
{
	if (!boss_ || !particleEditor_) {
		return;
	}

	// 全パーツを取得（Head, Body0~5, Tail）
	const std::vector<BaseParts*>& allParts = boss_->GetAllBodyParts();

	// 尻尾→頭の順にするため、逆順でキューに追加
	explosionQueue_.clear();
	for (int i = static_cast<int>(allParts.size()) - 1; i >= 0; --i) {
		PartExplosionData data;
		data.targetPart = allParts[i];

		// インスタンス名を生成
		std::string instanceName = std::format("BossExplosion[{}]", i);

		// 既存インスタンスがあれば削除
		auto* existing = particleEditor_->GetInstance(instanceName);
		if (existing) {
			particleEditor_->DestroyInstance(instanceName);
		}

		// 新しいインスタンスを作成
		particleEditor_->CreateInstance(kExplosionPresetName_, instanceName);
		data.instance = particleEditor_->GetInstance(instanceName);

		explosionQueue_.push_back(data);
	}
}

void BossExplosionEmitter::StartExplosionSequence()
{
	if (explosionQueue_.empty()) {
		return;
	}

	isSequenceActive_ = true;
	currentExplosionIndex_ = 0;

	// 全データをリセット
	for (auto& data : explosionQueue_) {
		data.explosionTimer = 0.0f;
		data.hideTimer = 0.0f;
		data.hasExploded = false;
		data.hasHidden = false;
	}

	// 最初の爆発を即座に実行
	ExecuteNextExplosion();
}

void BossExplosionEmitter::Update()
{
	if (!isSequenceActive_ || currentExplosionIndex_ < 0) {
		return;
	}

	// 現在のパーツの処理
	if (currentExplosionIndex_ < static_cast<int>(explosionQueue_.size())) {
		PartExplosionData& current = explosionQueue_[currentExplosionIndex_];

		// 爆発済みでまだ非表示にしていない場合
		if (current.hasExploded && !current.hasHidden) {
			current.hideTimer += 1.0f / 60.0f;

			// 非表示時間に達したらパーツを消す
			if (current.hideTimer >= hideDelay_) {
				if (current.targetPart) {
					current.targetPart->SetVisible(false);
				}
				current.hasHidden = true;
			}
		}

		// 次の爆発までの待機時間を更新
		if (current.hasExploded && current.hasHidden) {
			current.explosionTimer += 1.0f / 60.0f;

			// インターバル経過で次のパーツへ
			if (current.explosionTimer >= explosionInterval_) {
				currentExplosionIndex_++;

				// まだパーツが残っている場合は次の爆発を実行
				if (currentExplosionIndex_ < static_cast<int>(explosionQueue_.size())) {
					ExecuteNextExplosion();
				} else {
					// 全パーツ完了
					isSequenceActive_ = false;
				}
			}
		}
	}
}

void BossExplosionEmitter::ExecuteNextExplosion()
{
	if (currentExplosionIndex_ < 0 ||
		currentExplosionIndex_ >= static_cast<int>(explosionQueue_.size())) {
		return;
	}

	PartExplosionData& data = explosionQueue_[currentExplosionIndex_];

	if (!data.instance || !data.targetPart) {
		return;
	}

	auto* emitter = data.instance->GetEmitter(kEmitterName_);
	if (!emitter) {
		return;
	}

	// パーツの位置にエミッターを配置
	Vector3 partPosition = data.targetPart->GetPosition();
	emitter->GetTransform().SetPosition(partPosition);

	// 放出開始
	emitter->SetEmitEnabled(true);
	// カメラシェイク
	CameraController::GetInstance()->StartCameraShake(0.8f, 0.6f);

	// 爆発済みフラグを立てる
	data.hasExploded = true;
	data.explosionTimer = 0.0f;
	data.hideTimer = 0.0f;
}

bool BossExplosionEmitter::IsExplosionComplete() const
{
	if (!isSequenceActive_) {
		// シーケンスが非アクティブで、全パーツ処理済みなら完了
		return currentExplosionIndex_ >= static_cast<int>(explosionQueue_.size());
	}
	return false;
}

void BossExplosionEmitter::Finalize()
{
	if (!particleEditor_) {
		return;
	}

	// 全インスタンスを削除
	for (size_t i = 0; i < explosionQueue_.size(); ++i) {
		std::string instanceName = std::format("BossExplosion[{}]",
			explosionQueue_.size() - 1 - i);
		particleEditor_->DestroyInstance(instanceName);
	}

	explosionQueue_.clear();
}

void BossExplosionEmitter::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode("Boss Explosion Emitter")) {
		ImGui::Checkbox("Sequence Active", &isSequenceActive_);
		ImGui::Text("Current Index: %d / %zu",
			currentExplosionIndex_, explosionQueue_.size());

		ImGui::Separator();

		// パラメータ調整
		ImGui::DragFloat("Explosion Interval (sec)", &explosionInterval_, 0.01f, 0.1f, 3.0f);
		ImGui::DragFloat("Hide Delay (sec)", &hideDelay_, 0.01f, 0.1f, 2.0f);

		ImGui::Separator();

		// 各パーツの状態表示
		if (ImGui::TreeNode("Parts Status")) {
			for (size_t i = 0; i < explosionQueue_.size(); ++i) {
				const auto& data = explosionQueue_[i];

				std::string label = std::format("Part[{}]", i);
				if (ImGui::TreeNode(label.c_str())) {
					ImGui::Text("Exploded: %s", data.hasExploded ? "Yes" : "No");
					ImGui::Text("Hidden: %s", data.hasHidden ? "Yes" : "No");

					if (data.hasExploded) {
						ImGui::Text("Explosion Timer: %.2f", data.explosionTimer);
					}
					if (data.hasExploded && !data.hasHidden) {
						ImGui::Text("Hide Timer: %.2f / %.2f",
							data.hideTimer, hideDelay_);
						ImGui::ProgressBar(data.hideTimer / hideDelay_);
					}

					if (data.targetPart) {
						Vector3 pos = data.targetPart->GetPosition();
						ImGui::Text("Position: (%.2f, %.2f, %.2f)",
							pos.x, pos.y, pos.z);
					}

					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}

		ImGui::TreePop();
	}
#endif
}