#include "PlayerBulletHitEffectPool.h"
#include "ImGui/ImGuiManager.h"
#include "GameTimer.h"

PlayerBulletHitEffectPool::PlayerBulletHitEffectPool()
	: particleEditor_(nullptr)
	, dxCommon_(nullptr)
	, nextIndex_(0)
	, showDebugInfo_(false)
{
	// エフェクトデータの初期化
	for (size_t i = 0; i < kPoolSize_; ++i) {
		effects_[i].instance = nullptr;
		effects_[i].isActive = false;
		effects_[i].activeTimer = 0.0f;
	}
}

PlayerBulletHitEffectPool::~PlayerBulletHitEffectPool()
{
	Finalize();
}

void PlayerBulletHitEffectPool::Initialize(DirectXCommon* dxCommon, const std::string& presetName, const std::string& emitterName)
{
	dxCommon_ = dxCommon;
	presetName_ = presetName;
	emitterName_ = emitterName;

	// ParticleEditorシングルトンを取得
	particleEditor_ = ParticleEditor::GetInstance();

	// パーティクルインスタンスを作成
	CreateParticleInstances();
}

void PlayerBulletHitEffectPool::CreateParticleInstances()
{
	if (!particleEditor_) {
		return;
	}

	// プールサイズ分のエミッターインスタンスを事前生成
	for (size_t i = 0; i < kPoolSize_; ++i) {
		// インスタンス名を生成（例: "PlayerBulletHit[0]"）
		std::string instanceName = std::format("{}[{}]", presetName_, i);

		// 既存インスタンスがあれば削除（重複回避）
		auto* existing = particleEditor_->GetInstance(instanceName);
		if (existing) {
			particleEditor_->DestroyInstance(instanceName);
		}

		// 新しいインスタンスを作成
		particleEditor_->CreateInstance(presetName_, instanceName);
		effects_[i].instance = particleEditor_->GetInstance(instanceName);

		// 初期状態は非アクティブ
		effects_[i].isActive = false;
		effects_[i].activeTimer = 0.0f;

		// エミッターが存在する場合、初期状態で放出を無効化
		if (effects_[i].instance) {
			auto* emitter = effects_[i].instance->GetEmitter(emitterName_);
			if (emitter) {
				emitter->SetEmitEnabled(false);
			}
		}
	}
}

int PlayerBulletHitEffectPool::GetNextAvailableIndex()
{
	// ラウンドロビン方式で空きスロットを検索
	for (size_t i = 0; i < kPoolSize_; ++i) {
		size_t checkIndex = (nextIndex_ + i) % kPoolSize_;

		// 非アクティブなスロットが見つかったら返す
		if (!effects_[checkIndex].isActive) {
			nextIndex_ = (checkIndex + 1) % kPoolSize_;
			return static_cast<int>(checkIndex);
		}
	}

	// 空きスロットがない場合
	return -1;
}

bool PlayerBulletHitEffectPool::TriggerEffect(const Vector3& position, const Vector3& direction)
{
	// 次に使用可能なインデックスを取得
	int index = GetNextAvailableIndex();

	// プールが満杯の場合
	if (index < 0) {
		return false;
	}

	// エフェクトを発動
	ActivateEffect(static_cast<size_t>(index), position, direction);

	return true;
}

void PlayerBulletHitEffectPool::ActivateEffect(size_t index, const Vector3& position, const Vector3& direction)
{
	if (index >= kPoolSize_ || !effects_[index].instance) {
		return;
	}

	// エミッターを取得
	auto* emitter = effects_[index].instance->GetEmitter(emitterName_);
	if (!emitter) {
		return;
	}

	// エミッターの位置を設定
	emitter->GetTransform().SetPosition(position);

	// エミッターの方向を設定（弾丸の進行方向の逆向き）
	emitter->SetEmitDirection(direction);

	// 放出を開始
	emitter->SetEmitEnabled(true);

	// アクティブ状態にする
	effects_[index].isActive = true;
	effects_[index].activeTimer = 0.0f;
}

bool PlayerBulletHitEffectPool::IsEffectFinished(size_t index) const
{
	if (index >= kPoolSize_ || !effects_[index].instance) {
		return true;
	}

	// エミッターを取得
	auto* emitter = effects_[index].instance->GetEmitter(emitterName_);
	if (!emitter) {
		return true;
	}

	// エミッターが放出を停止したかチェック
	// IsEmitting()がfalseになったら放出完了
	return !emitter->IsEmitting();
}

void PlayerBulletHitEffectPool::Update()
{
	// GameTimerからデルタタイムを取得
	GameTimer& gameTimer = GameTimer::GetInstance();
	float deltaTime = gameTimer.GetDeltaTime();

	// 全エフェクトの状態を更新
	for (size_t i = 0; i < kPoolSize_; ++i) {
		// 非アクティブなエフェクトはスキップ
		if (!effects_[i].isActive) {
			continue;
		}

		// アクティブ時間を更新（デバッグ用）
		effects_[i].activeTimer += deltaTime;

		// エフェクトが放出完了したかチェック
		if (IsEffectFinished(i)) {
			// 非アクティブ状態に戻す
			effects_[i].isActive = false;
			effects_[i].activeTimer = 0.0f;

			// エミッターの放出を確実に無効化
			if (effects_[i].instance) {
				auto* emitter = effects_[i].instance->GetEmitter(emitterName_);
				if (emitter) {
					emitter->SetEmitEnabled(false);
				}
			}
		}
	}
}

size_t PlayerBulletHitEffectPool::GetActiveEffectCount() const
{
	size_t count = 0;
	for (size_t i = 0; i < kPoolSize_; ++i) {
		if (effects_[i].isActive) {
			count++;
		}
	}
	return count;
}

void PlayerBulletHitEffectPool::Finalize()
{
	if (!particleEditor_) {
		return;
	}

	// 全エミッターインスタンスを削除
	for (size_t i = 0; i < kPoolSize_; ++i) {
		if (effects_[i].instance) {
			std::string instanceName = std::format("{}[{}]", presetName_, i);
			particleEditor_->DestroyInstance(instanceName);
			effects_[i].instance = nullptr;
		}
	}
}

void PlayerBulletHitEffectPool::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode("Player Bullet Hit Effect Pool")) {
		ImGui::Checkbox("Show Debug Info", &showDebugInfo_);

		ImGui::Separator();

		// プール情報
		ImGui::Text("Preset Name: %s", presetName_.c_str());
		ImGui::Text("Emitter Name: %s", emitterName_.c_str());
		ImGui::Text("Pool Size: %zu", kPoolSize_);
		ImGui::Text("Active Effects: %zu / %zu", GetActiveEffectCount(), kPoolSize_);

		// プログレスバー
		float activeRatio = static_cast<float>(GetActiveEffectCount()) / static_cast<float>(kPoolSize_);
		ImGui::ProgressBar(activeRatio, ImVec2(0.0f, 0.0f),
			std::format("{}/{}", GetActiveEffectCount(), kPoolSize_).c_str());

		ImGui::Text("Next Index: %zu", nextIndex_);

		ImGui::Separator();

		// デバッグ情報表示
		if (showDebugInfo_) {
			if (ImGui::TreeNode("Effect Details")) {
				for (size_t i = 0; i < kPoolSize_; ++i) {
					ImGui::PushID(static_cast<int>(i));

					// エフェクト番号
					if (ImGui::TreeNode(std::format("Effect [{}]", i).c_str())) {
						const EffectData& effect = effects_[i];

						// 状態表示
						if (effect.isActive) {
							ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Active");
							ImGui::Text("Active Timer: %.2f sec", effect.activeTimer);
						} else {
							ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Status: Inactive");
						}

						// インスタンス情報
						ImGui::Text("Instance: %s", effect.instance ? "Valid" : "Invalid");

						// エミッター情報
						if (effect.instance) {
							auto* emitter = effect.instance->GetEmitter(emitterName_);
							if (emitter) {
								bool isEmitting = emitter->IsEmitting();
								ImGui::Text("Is Emitting: %s", isEmitting ? "Yes" : "No");

								Vector3 pos = emitter->GetTransform().GetPosition();
								ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);

								Vector3 dir = emitter->GetEmitDirection();
								ImGui::Text("Direction: (%.2f, %.2f, %.2f)", dir.x, dir.y, dir.z);
							} else {
								ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Emitter: Not Found");
							}
						}

						ImGui::TreePop();
					}

					ImGui::PopID();
				}
				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}
#endif
}