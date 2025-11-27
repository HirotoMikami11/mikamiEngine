#include "BossSmokeEmitter.h"
#include "Boss.h"
#include "ImGui/ImGuiManager.h"

BossSmokeEmitter::BossSmokeEmitter()
	: boss_(nullptr)
	, particleEditor_(nullptr)
	, headSmokeInstance_(nullptr)
	, tailSmokeInstance_(nullptr)
	, bodyPartsCount_(0)
	, showDebugPositions_(false)
{
}

BossSmokeEmitter::~BossSmokeEmitter()
{
	Finalize();
}

void BossSmokeEmitter::Initialize(Boss* boss)
{
	boss_ = boss;
	particleEditor_ = ParticleEditor::GetInstance();

	// パーティクルインスタンスを作成
	CreateParticleInstances();
}

void BossSmokeEmitter::CreateParticleInstances()
{
	if (!boss_ || !particleEditor_) {
		return;
	}

	// ボスのアクティブなボディパーツを取得して数をカウント
	auto activeBodyParts = boss_->GetActiveBodyParts();
	bodyPartsCount_ = activeBodyParts.size();

	// 頭用のパーティクルインスタンス作成
	std::string headInstanceName = "Smoke[Head]";

	// 既存インスタンスがあれば削除（重複回避）
	auto* existingHead = particleEditor_->GetInstance(headInstanceName);
	if (existingHead) {
		particleEditor_->DestroyInstance(headInstanceName);
	}

	particleEditor_->CreateInstance("WalkSmokeEffect", headInstanceName);
	headSmokeInstance_ = particleEditor_->GetInstance(headInstanceName);

	// 体用のパーティクルインスタンス作成
	bodySmokeInstances_.clear();
	bodySmokeInstances_.reserve(bodyPartsCount_);

	for (size_t i = 0; i < bodyPartsCount_; ++i) {
		std::string bodyInstanceName = std::format("Smoke[Body{}]", i);

		// 既存インスタンスがあれば削除
		auto* existingBody = particleEditor_->GetInstance(bodyInstanceName);
		if (existingBody) {
			particleEditor_->DestroyInstance(bodyInstanceName);
		}

		particleEditor_->CreateInstance("WalkSmokeEffect", bodyInstanceName);
		ParticlePresetInstance* bodyInstance = particleEditor_->GetInstance(bodyInstanceName);
		bodySmokeInstances_.push_back(bodyInstance);
	}

	// 尻尾用のパーティクルインスタンス作成
	std::string tailInstanceName = "Smoke[Tail]";

	// 既存インスタンスがあれば削除
	auto* existingTail = particleEditor_->GetInstance(tailInstanceName);
	if (existingTail) {
		particleEditor_->DestroyInstance(tailInstanceName);
	}

	particleEditor_->CreateInstance("WalkSmokeEffect", tailInstanceName);
	tailSmokeInstance_ = particleEditor_->GetInstance(tailInstanceName);
}

Vector3 BossSmokeEmitter::CalculateBottomPosition(BaseParts* part)
{
	if (!part) {
		return Vector3{ 0.0f, 0.0f, 0.0f };
	}

	// パーツの中心座標を取得
	Vector3 centerPosition = part->GetPosition();

	// パーツのスケールを取得
	Vector3 scale = part->GetScale();

	// パーツの下面座標 = 中心座標 - (スケールY / 2)
	// キューブの基本サイズは1.0なので、スケールがそのままサイズになる
	Vector3 bottomPosition = centerPosition;
	bottomPosition.y -= scale.y / 2.0f;

	return bottomPosition;
}

void BossSmokeEmitter::UpdateEmitterPosition(ParticlePresetInstance* instance, const Vector3& position)
{
	if (!instance) {
		return;
	}

	// エミッタを取得して位置を更新
	auto* emitter = instance->GetEmitter(kEmitterName_);
	if (emitter) {
		emitter->GetTransform().SetPosition(position);
	}
}

void BossSmokeEmitter::Update()
{
	if (!boss_) {
		return;
	}

	// アクティブなボディパーツを取得
	auto activeBodyParts = boss_->GetActiveBodyParts();

	// 頭の位置を更新
	if (headSmokeInstance_) {
		// activeBodyPartsの最初の要素が頭（HeadParts）
		if (!activeBodyParts.empty()) {
			BaseParts* headPart = activeBodyParts[0];
			Vector3 headBottomPos = CalculateBottomPosition(headPart);
			UpdateEmitterPosition(headSmokeInstance_, headBottomPos);
		}
	}

	// 体の位置を更新
	// activeBodyPartsには [Head, Body0, Body1, ..., Tail] の順で格納されている
	size_t bodyStartIndex = 1; // Headの次から
	size_t bodyEndIndex = activeBodyParts.size() - 1; // Tailの前まで

	for (size_t i = 0; i < bodySmokeInstances_.size(); ++i) {
		size_t bodyIndex = bodyStartIndex + i;

		if (bodyIndex < bodyEndIndex && bodyIndex < activeBodyParts.size()) {
			BaseParts* bodyPart = activeBodyParts[bodyIndex];
			Vector3 bodyBottomPos = CalculateBottomPosition(bodyPart);
			UpdateEmitterPosition(bodySmokeInstances_[i], bodyBottomPos);
		}
	}

	// 尻尾の位置を更新
	if (tailSmokeInstance_) {
		// activeBodyPartsの最後の要素が尻尾（TailParts）
		if (!activeBodyParts.empty()) {
			BaseParts* tailPart = activeBodyParts.back();
			Vector3 tailBottomPos = CalculateBottomPosition(tailPart);
			UpdateEmitterPosition(tailSmokeInstance_, tailBottomPos);
		}
	}
}

void BossSmokeEmitter::Finalize()
{
	if (!particleEditor_) {
		return;
	}

	// 頭のパーティクルインスタンス削除
	if (headSmokeInstance_) {
		particleEditor_->DestroyInstance("Smoke[Head]");
		headSmokeInstance_ = nullptr;
	}

	// 体のパーティクルインスタンス削除
	for (size_t i = 0; i < bodySmokeInstances_.size(); ++i) {
		std::string bodyInstanceName = std::format("Smoke[Body{}]", i);
		particleEditor_->DestroyInstance(bodyInstanceName);
	}
	bodySmokeInstances_.clear();

	// 尻尾のパーティクルインスタンス削除
	if (tailSmokeInstance_) {
		particleEditor_->DestroyInstance("Smoke[Tail]");
		tailSmokeInstance_ = nullptr;
	}
}

void BossSmokeEmitter::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode("Boss Smoke Emitter")) {
		ImGui::Checkbox("Show Debug Positions", &showDebugPositions_);

		ImGui::Text("Body Parts Count: %zu", bodyPartsCount_);
		ImGui::Text("Active Smoke Instances:");
		ImGui::Text("  Head: %s", headSmokeInstance_ ? "Active" : "Inactive");
		ImGui::Text("  Body: %zu", bodySmokeInstances_.size());
		ImGui::Text("  Tail: %s", tailSmokeInstance_ ? "Active" : "Inactive");

		// デバッグ情報表示
		if (showDebugPositions_ && boss_) {
			auto activeBodyParts = boss_->GetActiveBodyParts();

			if (ImGui::TreeNode("Emitter Positions")) {
				// 頭
				if (!activeBodyParts.empty()) {
					Vector3 headPos = CalculateBottomPosition(activeBodyParts[0]);
					ImGui::Text("Head: (%.2f, %.2f, %.2f)", headPos.x, headPos.y, headPos.z);
				}

				// 体
				size_t bodyStartIndex = 1;
				size_t bodyEndIndex = activeBodyParts.size() - 1;
				for (size_t i = 0; i < bodySmokeInstances_.size(); ++i) {
					size_t bodyIndex = bodyStartIndex + i;
					if (bodyIndex < bodyEndIndex && bodyIndex < activeBodyParts.size()) {
						Vector3 bodyPos = CalculateBottomPosition(activeBodyParts[bodyIndex]);
						ImGui::Text("Body[%zu]: (%.2f, %.2f, %.2f)", i, bodyPos.x, bodyPos.y, bodyPos.z);
					}
				}

				// 尻尾
				if (!activeBodyParts.empty()) {
					Vector3 tailPos = CalculateBottomPosition(activeBodyParts.back());
					ImGui::Text("Tail: (%.2f, %.2f, %.2f)", tailPos.x, tailPos.y, tailPos.z);
				}

				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}
#endif
}