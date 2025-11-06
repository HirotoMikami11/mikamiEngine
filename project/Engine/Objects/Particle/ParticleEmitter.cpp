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

	// デバッグ用LineRendererを初期化
	debugLineRenderer_ = std::make_unique<LineRenderer>();
	debugLineRenderer_->Initialize(dxCommon);

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

void ParticleEmitter::DrawDebug(const Matrix4x4& viewProjectionMatrix)
{
	if (!showDebugAABB_ || !debugLineRenderer_) {
		return;
	}
#ifdef USEIMGUI
	// AABBのデバッグ線を作成
	CreateDebugAABBLines();

	// LineRendererで描画
	debugLineRenderer_->Draw(viewProjectionMatrix);
#endif 


}

void ParticleEmitter::CreateDebugAABBLines()
{
	// 前回の線をクリア
	debugLineRenderer_->Reset();

	// エミッターの位置を取得
	Vector3 emitterPos = emitterTransform_.GetPosition();

	// AABBの8頂点を計算（エミッター位置を基準に）
	Vector3 vertices[8] = {
		// 底面（min.y）
		{ emitterPos.x + spawnArea_.min.x, emitterPos.y + spawnArea_.min.y, emitterPos.z + spawnArea_.min.z },	// 0: 左下手前
		{ emitterPos.x + spawnArea_.max.x, emitterPos.y + spawnArea_.min.y, emitterPos.z + spawnArea_.min.z },	// 1: 右下手前
		{ emitterPos.x + spawnArea_.max.x, emitterPos.y + spawnArea_.min.y, emitterPos.z + spawnArea_.max.z },	// 2: 右下奥
		{ emitterPos.x + spawnArea_.min.x, emitterPos.y + spawnArea_.min.y, emitterPos.z + spawnArea_.max.z },	// 3: 左下奥

		// 上面（max.y）
		{ emitterPos.x + spawnArea_.min.x, emitterPos.y + spawnArea_.max.y, emitterPos.z + spawnArea_.min.z },	// 4: 左上手前
		{ emitterPos.x + spawnArea_.max.x, emitterPos.y + spawnArea_.max.y, emitterPos.z + spawnArea_.min.z },	// 5: 右上手前
		{ emitterPos.x + spawnArea_.max.x, emitterPos.y + spawnArea_.max.y, emitterPos.z + spawnArea_.max.z },	// 6: 右上奥
		{ emitterPos.x + spawnArea_.min.x, emitterPos.y + spawnArea_.max.y, emitterPos.z + spawnArea_.max.z }	// 7: 左上奥
	};

	// 底面の4本の線
	debugLineRenderer_->AddLine(vertices[0], vertices[1], debugAABBColor_);	// 左下手前 → 右下手前
	debugLineRenderer_->AddLine(vertices[1], vertices[2], debugAABBColor_);	// 右下手前 → 右下奥
	debugLineRenderer_->AddLine(vertices[2], vertices[3], debugAABBColor_);	// 右下奥 → 左下奥
	debugLineRenderer_->AddLine(vertices[3], vertices[0], debugAABBColor_);	// 左下奥 → 左下手前

	// 上面の4本の線
	debugLineRenderer_->AddLine(vertices[4], vertices[5], debugAABBColor_);	// 左上手前 → 右上手前
	debugLineRenderer_->AddLine(vertices[5], vertices[6], debugAABBColor_);	// 右上手前 → 右上奥
	debugLineRenderer_->AddLine(vertices[6], vertices[7], debugAABBColor_);	// 右上奥 → 左上奥
	debugLineRenderer_->AddLine(vertices[7], vertices[4], debugAABBColor_);	// 左上奥 → 左上手前

	// 縦の4本の線
	debugLineRenderer_->AddLine(vertices[0], vertices[4], debugAABBColor_);	// 左下手前 → 左上手前
	debugLineRenderer_->AddLine(vertices[1], vertices[5], debugAABBColor_);	// 右下手前 → 右上手前
	debugLineRenderer_->AddLine(vertices[2], vertices[6], debugAABBColor_);	// 右下奥 → 右上奥
	debugLineRenderer_->AddLine(vertices[3], vertices[7], debugAABBColor_);	// 左下奥 → 左上奥
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