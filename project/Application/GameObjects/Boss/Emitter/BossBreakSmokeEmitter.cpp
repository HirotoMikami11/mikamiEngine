#include "BossBreakSmokeEmitter.h"
#include "Boss.h"
#include "Parts/BodyParts.h"
#include "ImGui/ImGuiManager.h"
#include "Random/Random.h"

BossBreakSmokeEmitter::BossBreakSmokeEmitter()
	: boss_(nullptr)
	, particleEditor_(nullptr)
	, showDebugInfo_(false)
{
	// エミッターデータの初期化
	for (int i = 0; i < kEmitterCount; ++i) {
		emitters_[i].instance = nullptr;
		emitters_[i].timer = 0.0f;
		emitters_[i].targetPart = nullptr;
		emitters_[i].isWaiting = false;
	}
}

BossBreakSmokeEmitter::~BossBreakSmokeEmitter()
{
	Finalize();
}

void BossBreakSmokeEmitter::Initialize(Boss* boss)
{
	boss_ = boss;
	particleEditor_ = ParticleEditor::GetInstance();

	// パーティクルインスタンスを作成
	CreateParticleInstances();
}

void BossBreakSmokeEmitter::CreateParticleInstances()
{
	if (!particleEditor_) {
		return;
	}

	// 2個のエミッターインスタンスを作成
	for (int i = 0; i < kEmitterCount; ++i) {
		std::string instanceName = std::format("BreakSmoke[{}]", i);

		// 既存インスタンスがあれば削除（重複回避）
		auto* existing = particleEditor_->GetInstance(instanceName);
		if (existing) {
			particleEditor_->DestroyInstance(instanceName);
		}

		// 新しいインスタンスを作成
		particleEditor_->CreateInstance("BreakSmokeEffect", instanceName);
		emitters_[i].instance = particleEditor_->GetInstance(instanceName);

		// 初期状態は待機中
		emitters_[i].isWaiting = true;
		emitters_[i].timer = 0.0f;
		emitters_[i].targetPart = nullptr;
	}
}

std::vector<BaseParts*> BossBreakSmokeEmitter::GetInactiveBodyParts()
{
	std::vector<BaseParts*> inactiveParts;

	if (!boss_) {
		return inactiveParts;
	}

	// 全パーツを取得
	const std::vector<BaseParts*>& allParts = boss_->GetAllBodyParts();

	// 非アクティブなBodyパーツのみを抽出
	// allPartsは [Head, Body0, Body1, ..., Tail] の順
	// Bodyパーツはインデックス1から(allParts.size() - 1)まで
	for (size_t i = 1; i < allParts.size() - 1; ++i) {
		BaseParts* part = allParts[i];
		if (part && !part->IsActive()) {
			inactiveParts.push_back(part);
		}
	}

	return inactiveParts;
}

void BossBreakSmokeEmitter::Update()
{
	if (!boss_) {
		return;
	}

	// 非アクティブなBodyパーツを取得
	std::vector<BaseParts*> inactiveParts = GetInactiveBodyParts();

	// 非アクティブなパーツがない場合、全エミッターを待機状態に
	if (inactiveParts.empty()) {
		for (int i = 0; i < kEmitterCount; ++i) {
			emitters_[i].isWaiting = true;
			emitters_[i].targetPart = nullptr;
			emitters_[i].timer = 0.0f;
		}
		return;
	}

	// 各エミッターを更新
	for (int i = 0; i < kEmitterCount; ++i) {
		// 使用可能なエミッター数を制限（非アクティブパーツ数まで）
		if (i >= static_cast<int>(inactiveParts.size())) {
			emitters_[i].isWaiting = true;
			emitters_[i].targetPart = nullptr;
			continue;
		}

		UpdateEmitter(emitters_[i], inactiveParts);
	}
}

void BossBreakSmokeEmitter::UpdateEmitter(EmitterData& emitterData, const std::vector<BaseParts*>& inactiveParts)
{
	if (!emitterData.instance) {
		return;
	}

	auto* emitter = emitterData.instance->GetEmitter(kEmitterName_);
	if (!emitter) {
		return;
	}

	// 待機中の処理
	if (emitterData.isWaiting) {
		// タイマーを進める
		emitterData.timer += 1.0f/60.0f;

		// 待機時間が経過したら起動
		if (emitterData.timer >= kRestartWaitTime_) {
			ActivateEmitter(emitterData, inactiveParts);
			emitterData.isWaiting = false;
			emitterData.timer = 0.0f;
		}
	}
	// 放出中の処理
	else {
		// 対象パーツがまだ非アクティブで存在する場合、位置を更新
		if (emitterData.targetPart && !emitterData.targetPart->IsActive()) {
			// 保存されている面の位置を再計算して更新
			Vector3 newPosition = CalculateFacePosition(emitterData.targetPart, emitterData.targetFace);
			emitter->GetTransform().SetPosition(newPosition);
		}

		// エミッターが放出を停止したら待機状態へ
		if (!emitter->IsEmitting()) {
			emitterData.isWaiting = true;
			emitterData.timer = 0.0f;
			emitterData.targetPart = nullptr;
		}
	}
}

void BossBreakSmokeEmitter::ActivateEmitter(EmitterData& emitterData, const std::vector<BaseParts*>& inactiveParts)
{
	if (inactiveParts.empty() || !emitterData.instance) {
		return;
	}

	auto* emitter = emitterData.instance->GetEmitter(kEmitterName_);
	if (!emitter) {
		return;
	}

	// 使用可能なパーツを探す（既に他のエミッターが使用していないパーツ）
	BaseParts* selectedPart = nullptr;
	std::vector<BaseParts*> availableParts;

	// 自分以外のエミッターが使用していないパーツを収集
	int currentEmitterIndex = -1;
	for (int i = 0; i < kEmitterCount; ++i) {
		if (&emitters_[i] == &emitterData) {
			currentEmitterIndex = i;
			break;
		}
	}

	for (BaseParts* part : inactiveParts) {
		if (!IsPartAlreadyUsed(part, currentEmitterIndex)) {
			availableParts.push_back(part);
		}
	}

	// 使用可能なパーツがない場合は何もしない
	if (availableParts.empty()) {
		return;
	}

	// ランダムにパーツを選択
	int randomIndex = Random::GetInstance().GenerateInt(0, static_cast<int>(availableParts.size()) - 1);
	selectedPart = availableParts[randomIndex];

	// ランダムに面を選択
	PartsFace face = GetRandomFace();

	// 面の位置と法線を計算
	Vector3 position = CalculateFacePosition(selectedPart, face);
	Vector3 direction = GetFaceNormal(face);

	// エミッターの位置と方向を設定
	emitter->GetTransform().SetPosition(position);
	emitter->SetEmitDirection(direction);

	// 放出開始
	emitter->SetEmitEnabled(true);

	// エミッターデータを更新（選択した面も保存）
	emitterData.targetPart = selectedPart;
	emitterData.targetFace = face;
}

Vector3 BossBreakSmokeEmitter::CalculateFacePosition(BaseParts* part, PartsFace face)
{
	if (!part) {
		return Vector3{ 0.0f, 0.0f, 0.0f };
	}

	// パーツの中心座標を取得
	Vector3 centerPosition = part->GetPosition();

	// パーツのスケールを取得
	Vector3 scale = part->GetScale();

	// 半サイズ
	Vector3 halfSize = {
		scale.x / 2.0f,
		scale.y / 2.0f,
		scale.z / 2.0f
	};

	// 面の位置を計算
	Vector3 facePosition = centerPosition;

	switch (face) {
	case PartsFace::Top:
		// 上面の中心
		facePosition.y += halfSize.y;
		break;
	case PartsFace::Front:
		// 前面の中心 (+Z方向)
		facePosition.z += halfSize.z;
		break;
	case PartsFace::Back:
		// 後面の中心 (-Z方向)
		facePosition.z -= halfSize.z;
		break;
	case PartsFace::Left:
		// 左面の中心 (-X方向)
		facePosition.x -= halfSize.x;
		break;
	case PartsFace::Right:
		// 右面の中心 (+X方向)
		facePosition.x += halfSize.x;
		break;
	default:
		break;
	}

	return facePosition;
}

Vector3 BossBreakSmokeEmitter::GetFaceNormal(PartsFace face)
{
	Vector3 normal = { 0.0f, 0.0f, 0.0f };

	switch (face) {
	case PartsFace::Top:
		normal = { 0.0f, 1.0f, 0.0f };	// 上
		break;
	case PartsFace::Front:
		normal = { 0.0f, 0.0f, 1.0f };	// 前
		break;
	case PartsFace::Back:
		normal = { 0.0f, 0.0f, -1.0f };	// 後
		break;
	case PartsFace::Left:
		normal = { -1.0f, 0.0f, 0.0f };	// 左
		break;
	case PartsFace::Right:
		normal = { 1.0f, 0.0f, 0.0f };	// 右
		break;
	default:
		break;
	}

	return normal;
}

BossBreakSmokeEmitter::PartsFace BossBreakSmokeEmitter::GetRandomFace()
{
	// 0～4のランダムな整数を生成（5つの面）
	int randomValue = Random::GetInstance().GenerateInt(0, static_cast<int>(PartsFace::Count) - 1);
	return static_cast<PartsFace>(randomValue);
}

bool BossBreakSmokeEmitter::IsPartAlreadyUsed(BaseParts* part, int excludeIndex)
{
	if (!part) {
		return false;
	}

	// 全エミッターをチェック
	for (int i = 0; i < kEmitterCount; ++i) {
		// 除外するインデックスはスキップ
		if (i == excludeIndex) {
			continue;
		}

		// 待機中でない（放出中）かつ、同じパーツを使用している
		if (!emitters_[i].isWaiting && emitters_[i].targetPart == part) {
			return true;
		}
	}

	return false;
}

void BossBreakSmokeEmitter::Finalize()
{
	if (!particleEditor_) {
		return;
	}

	// 全エミッターインスタンスを削除
	for (int i = 0; i < kEmitterCount; ++i) {
		if (emitters_[i].instance) {
			std::string instanceName = std::format("BreakSmoke[{}]", i);
			particleEditor_->DestroyInstance(instanceName);
			emitters_[i].instance = nullptr;
		}
	}
}

void BossBreakSmokeEmitter::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode("Boss Break Smoke Emitter")) {
		ImGui::Checkbox("Show Debug Info", &showDebugInfo_);

		ImGui::Separator();

		// 非アクティブパーツの情報
		std::vector<BaseParts*> inactiveParts = GetInactiveBodyParts();
		ImGui::Text("Inactive Body Parts: %zu", inactiveParts.size());

		ImGui::Separator();

		// 各エミッターの状態表示
		for (int i = 0; i < kEmitterCount; ++i) {
			if (ImGui::TreeNode(std::format("Emitter [{}]", i).c_str())) {
				const EmitterData& emitter = emitters_[i];

				ImGui::Text("Instance: %s", emitter.instance ? "Active" : "Inactive");

				if (emitter.isWaiting) {
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Status: Waiting");
					ImGui::Text("Timer: %.2f / %.2f sec", emitter.timer, kRestartWaitTime_);
					ImGui::ProgressBar(emitter.timer / kRestartWaitTime_);
				} else {
					ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Emitting");

					if (emitter.instance) {
						auto* particleEmitter = emitter.instance->GetEmitter(kEmitterName_);
						if (particleEmitter) {
							bool isEmitting = particleEmitter->IsEmitting();
							ImGui::Text("Particle Emitting: %s", isEmitting ? "Yes" : "No");

							Vector3 pos = particleEmitter->GetTransform().GetPosition();
							ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);

							Vector3 dir = particleEmitter->GetEmitDirection();
							ImGui::Text("Direction: (%.2f, %.2f, %.2f)", dir.x, dir.y, dir.z);
						}
					}
				}

				if (emitter.targetPart) {
					ImGui::Text("Target Part: Active");
					Vector3 partPos = emitter.targetPart->GetPosition();
					ImGui::Text("Part Position: (%.2f, %.2f, %.2f)", partPos.x, partPos.y, partPos.z);

					// 面の種類を表示
					const char* faceNames[] = { "Top", "Front", "Back", "Left", "Right" };
					int faceIndex = static_cast<int>(emitter.targetFace);
					if (faceIndex >= 0 && faceIndex < 5) {
						ImGui::Text("Target Face: %s", faceNames[faceIndex]);
					}
				} else {
					ImGui::Text("Target Part: None");
				}

				ImGui::TreePop();
			}
		}

		// デバッグ情報
		if (showDebugInfo_ && !inactiveParts.empty()) {
			ImGui::Separator();
			if (ImGui::TreeNode("Inactive Parts Detail")) {
				for (size_t i = 0; i < inactiveParts.size(); ++i) {
					BaseParts* part = inactiveParts[i];
					Vector3 pos = part->GetPosition();
					Vector3 scale = part->GetScale();

					ImGui::Text("Part[%zu]: Pos(%.2f, %.2f, %.2f) Scale(%.2f, %.2f, %.2f)",
						i, pos.x, pos.y, pos.z, scale.x, scale.y, scale.z);
				}
				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}
#endif
}