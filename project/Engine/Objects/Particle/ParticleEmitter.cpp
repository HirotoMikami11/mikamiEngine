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

	// DebugDrawLineSystemを取得
	debugDrawLineSystem_ = DebugDrawLineSystem::GetInstance();

	// 初期状態
	frequencyTimer_ = 0.0f;

	// AABBを正しい状態に
	FixAABBMinMax(spawnArea_);
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

	// エミッターの位置を取得
	Vector3 emitterPos = emitterTransform_.GetPosition();

	// AABBの範囲内でランダムな位置を生成
	Vector3 randomPosInAABB = {
		Random::GetInstance().GenerateFloat(spawnArea_.min.x, spawnArea_.max.x),
		Random::GetInstance().GenerateFloat(spawnArea_.min.y, spawnArea_.max.y),
		Random::GetInstance().GenerateFloat(spawnArea_.min.z, spawnArea_.max.z)
	};

	state.transform.scale = { 1.0f, 1.0f, 1.0f };
	state.transform.rotate = { 0.0f, 0.0f, 0.0f };
	state.transform.translate = {
		emitterPos.x + randomPosInAABB.x,
		emitterPos.y + randomPosInAABB.y,
		emitterPos.z + randomPosInAABB.z
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

void ParticleEmitter::SetSpawnAreaSize(const Vector3& size)
{
	// 中心からの±sizeで設定
	spawnArea_.min = { -size.x, -size.y, -size.z };
	spawnArea_.max = { size.x, size.y, size.z };
	FixAABBMinMax(spawnArea_);
}

void ParticleEmitter::AddLineDebug()
{
	if (!showDebugAABB_) {
		return;
	}
#ifdef USEIMGUI
	// ワールド座標でのAABBを取得
	AABB worldAABB = GetWorldAABB();

	// DebugDrawLineSystemでAABBを描画
	debugDrawLineSystem_->DrawAABB(worldAABB, debugAABBColor_);
#endif 
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
			ImGui::DragFloat("Velocity Range", &velocityRange_, 0.1f, 0.0f, 10.0f);
			ImGui::DragFloat("Life Time Min", &particleLifeTimeMin_, 0.1f, 0.1f, 10.0f);
			ImGui::DragFloat("Life Time Max", &particleLifeTimeMax_, 0.1f, 0.1f, 10.0f);

			// 最小値が最大値を超えないようにする
			if (particleLifeTimeMin_ > particleLifeTimeMax_) {
				particleLifeTimeMax_ = particleLifeTimeMin_;
			}
		}

		// AABB発生範囲設定
		if (ImGui::CollapsingHeader("Spawn Area (AABB)")) {
			bool aabbChanged = false;

			ImGui::Text("AABB Min");
			if (ImGui::DragFloat3("Min", &spawnArea_.min.x, 0.1f)) {
				aabbChanged = true;
			}

			ImGui::Text("AABB Max");
			if (ImGui::DragFloat3("Max", &spawnArea_.max.x, 0.1f)) {
				aabbChanged = true;
			}

			if (aabbChanged) {
				FixAABBMinMax(spawnArea_);
			}

			// サイズ表示
			Vector3 size = {
				spawnArea_.max.x - spawnArea_.min.x,
				spawnArea_.max.y - spawnArea_.min.y,
				spawnArea_.max.z - spawnArea_.min.z
			};
			ImGui::Text("Size: (%.2f, %.2f, %.2f)", size.x, size.y, size.z);

			ImGui::Separator();

			// クイック設定ボタン
			if (ImGui::Button("Set Small (1x1x1)")) {
				SetSpawnAreaSize({ 0.5f, 0.5f, 0.5f });
			}
			ImGui::SameLine();
			if (ImGui::Button("Set Medium (2x2x2)")) {
				SetSpawnAreaSize({ 1.0f, 1.0f, 1.0f });
			}
		}

		// デバッグ描画設定
		if (ImGui::CollapsingHeader("Debug Visualization")) {
			ImGui::Checkbox("Show AABB", &showDebugAABB_);
			ImGui::ColorEdit4("AABB Color", &debugAABBColor_.x);
		}

		ImGui::TreePop();
	}
#endif
}

AABB ParticleEmitter::GetWorldAABB() const
{
	// エミッターの位置を取得
	Vector3 emitterPos = emitterTransform_.GetPosition();

	// ローカル座標のAABBをワールド座標に変換
	AABB worldAABB;
	worldAABB.min = {
		emitterPos.x + spawnArea_.min.x,
		emitterPos.y + spawnArea_.min.y,
		emitterPos.z + spawnArea_.min.z
	};
	worldAABB.max = {
		emitterPos.x + spawnArea_.max.x,
		emitterPos.y + spawnArea_.max.y,
		emitterPos.z + spawnArea_.max.z
	};

	return worldAABB;
}