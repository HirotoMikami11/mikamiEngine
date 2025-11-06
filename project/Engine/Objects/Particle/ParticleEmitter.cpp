#include "ParticleEmitter.h"
#include "ParticleGroup.h"
#include "Managers/ImGui/ImGuiManager.h"

void ParticleEmitter::Initialize(DirectXCommon* dxCommon, const std::string& targetGroupName)
{
	directXCommon_ = dxCommon;
	targetGroupName_ = targetGroupName;

	// エミッタートランスフォームを初期化
	emitterTransform_.Initialize(dxCommon);
	emitterTransform_.SetDefaultTransform();

	// 初期状態
	frequencyTimer_ = 0.0f;
}

void ParticleEmitter::Update(float deltaTime, ParticleGroup* targetGroup)
{
	// ターゲットグループが設定されていない場合は何もしない
	if (!targetGroup) {
		return;
	}

	// エミッタートランスフォームの更新（ビュープロジェクション行列は不要）
	Matrix4x4 dummyMatrix = MakeIdentity4x4();
	emitterTransform_.UpdateMatrix(dummyMatrix);

	// パーティクルの発生
	if (isEmitting_) {
		EmitParticles(deltaTime, targetGroup);
	}
}

void ParticleEmitter::EmitParticles(float deltaTime, ParticleGroup* targetGroup)
{
	// 頻度タイマーを進める
	frequencyTimer_ += deltaTime;

	// 発生頻度に達したかチェック
	while (frequencyTimer_ >= emitFrequency_) {
		frequencyTimer_ -= emitFrequency_;

		// 指定された数のパーティクルを生成
		for (uint32_t i = 0; i < emitCount_; ++i) {
			// ターゲットグループに追加を試みる
			ParticleState newParticle = CreateNewParticle();

			if (!targetGroup->AddParticle(newParticle)) {
				// 追加に失敗した（満杯）場合はこれ以上追加しない
				break;
			}
		}
	}
}

ParticleState ParticleEmitter::CreateNewParticle()
{
	ParticleState state;

	// エミッターの位置を基準にランダムな位置を設定
	Vector3 emitterPos = emitterTransform_.GetPosition();
	Vector3 randomOffset = Random::GetInstance().GenerateVector3OriginOffset(spawnRange_);

	state.transform.scale = { 1.0f, 1.0f, 1.0f };
	state.transform.rotate = { 0.0f, 0.0f, 0.0f };
	state.transform.translate = {
		emitterPos.x + randomOffset.x,
		emitterPos.y + randomOffset.y,
		emitterPos.z + randomOffset.z
	};

	// ランダム速度を設定
	state.velocity = Random::GetInstance().GenerateVector3OriginOffset(velocityRange_);

	// ランダムな色を設定
	state.color = Random::GetInstance().GenerateRandomVector4();

	// 寿命を設定
	state.lifeTime = Random::GetInstance().GenerateFloat(particleLifeTimeMin_, particleLifeTimeMax_);
	state.currentTime = 0.0f;

	return state;
}

void ParticleEmitter::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode(name_.c_str())) {
		// エミッター設定
		if (ImGui::CollapsingHeader("Emitter Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Target Group: %s", targetGroupName_.c_str());

			ImGui::Separator();

			ImGui::Checkbox("Is Emitting", &isEmitting_);

			// エミッター位置
			Vector3 pos = emitterTransform_.GetPosition();
			if (ImGui::DragFloat3("Emitter Position", &pos.x, 0.1f)) {
				emitterTransform_.SetPosition(pos);
			}

			// 発生数
			int count = static_cast<int>(emitCount_);
			if (ImGui::SliderInt("Emit Count", &count, 1, 50)) {
				emitCount_ = static_cast<uint32_t>(count);
			}

			// 発生頻度
			ImGui::DragFloat("Emit Frequency (sec)", &emitFrequency_, 0.01f, 0.01f, 10.0f);
			ImGui::Text("Next emit in: %.2f sec", emitFrequency_ - frequencyTimer_);

			ImGui::Separator();
		}

		// パーティクル初期設定
		if (ImGui::CollapsingHeader("Particle Settings")) {
			ImGui::DragFloat("Spawn Range", &spawnRange_, 0.1f, 0.0f, 10.0f);
			ImGui::DragFloat("Velocity Range", &velocityRange_, 0.1f, 0.0f, 10.0f);
			ImGui::DragFloat("Life Time Min", &particleLifeTimeMin_, 0.1f, 0.1f, 10.0f);
			ImGui::DragFloat("Life Time Max", &particleLifeTimeMax_, 0.1f, 0.1f, 10.0f);

			// 最小値が最大値を超えないようにする
			if (particleLifeTimeMin_ > particleLifeTimeMax_) {
				particleLifeTimeMax_ = particleLifeTimeMin_;
			}
		}

		ImGui::TreePop();
	}
#endif
}