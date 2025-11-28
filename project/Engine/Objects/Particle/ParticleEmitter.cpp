#include "ParticleEmitter.h"
#include "ParticleGroup.h"
#include "ImGui/ImGuiManager.h"

void ParticleEmitter::Initialize(DirectXCommon* dxCommon, const std::string& targetGroupName)
{
	dxCommon_ = dxCommon;
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

	// エミッター寿命処理
	if (useEmitterLifeTime_) {
		emitterCurrentTime_ += deltaTime;

		// 寿命が尽きた場合
		if (emitterCurrentTime_ >= emitterLifeTime_) {
			if (emitterLifeTimeLoop_) {
				// ループする場合、時間をリセット
				emitterCurrentTime_ = 0.0f;
			} else {
				// ループしない場合、発生を停止
				isEmitting_ = false;
				return;
			}
		}
	}

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

	// Scale設定（ランダム範囲）
	state.transform.scale = {
		Random::GetInstance().GenerateFloat(particleScaleMin_.x, particleScaleMax_.x),
		Random::GetInstance().GenerateFloat(particleScaleMin_.y, particleScaleMax_.y),
		Random::GetInstance().GenerateFloat(particleScaleMin_.z, particleScaleMax_.z)
	};

	// Rotate設定（ランダム範囲）
	state.transform.rotate = {
		Random::GetInstance().GenerateFloat(particleRotateMin_.x, particleRotateMax_.x),
		Random::GetInstance().GenerateFloat(particleRotateMin_.y, particleRotateMax_.y),
		Random::GetInstance().GenerateFloat(particleRotateMin_.z, particleRotateMax_.z)
	};

	// 位置設定
	state.transform.translate = {
		emitterPos.x + randomPosInAABB.x,
		emitterPos.y + randomPosInAABB.y,
		emitterPos.z + randomPosInAABB.z
	};

	// 速度設定（新方式 or 旧方式）
	if (useDirectionalEmit_) {
		// 新方式：方向指定発射
		// 散らばり角度をラジアンに変換
		float spreadRad = DegToRad(spreadAngle_);

		// ランダムな角度オフセットを生成（-spreadRad/2 ~ +spreadRad/2）
		float randomTheta = Random::GetInstance().GenerateFloat(-spreadRad / 2.0f, spreadRad / 2.0f);
		float randomPhi = Random::GetInstance().GenerateFloat(0.0f, 2.0f * std::numbers::pi_v<float>);

		// 散らばりベクトルを生成
		Vector3 spread = {
			sinf(randomTheta) * cosf(randomPhi),
			sinf(randomTheta) * sinf(randomPhi),
			cosf(randomTheta) - 1.0f
		};

		// 発射方向に散らばりを加える
		Vector3 finalDirection = {
			emitDirection_.x + spread.x,
			emitDirection_.y + spread.y,
			emitDirection_.z + spread.z
		};

		// 正規化して初速度をかける
		finalDirection = Normalize(finalDirection);
		state.velocity = {
			finalDirection.x * initialSpeed_,
			finalDirection.y * initialSpeed_,
			finalDirection.z * initialSpeed_
		};
	} else {
		// 旧方式：ランダム速度（互換性のため）
		state.velocity = Random::GetInstance().GenerateVector3OriginOffset(velocityRange_);
	}

	// ランダムな色を設定
	state.color = Random::GetInstance().GenerateRandomVector4();

	// 寿命を設定
	state.lifeTime = Random::GetInstance().GenerateFloat(particleLifeTimeMin_, particleLifeTimeMax_);
	state.currentTime = 0.0f;

	// ========================================
	//	時間で色変化(Color Over Lifetime)
	// ========================================
	if (enableColorOverLifetime_) {
		state.useColorOverLifetime = true;
		state.startColor = particleStartColor_;
		state.endColor = particleEndColor_;
		state.color = particleStartColor_;  // 初期色を設定
	}

	// ========================================
	// 時間でサイズ変化(Size Over Lifetime)
	// ========================================
	if (enableSizeOverLifetime_) {
		state.useSizeOverLifetime = true;
		state.startScale = particleStartScale_;
		state.endScale = particleEndScale_;
		state.transform.scale = particleStartScale_;  // 初期スケールを設定
	}

	// ========================================
	// 回転(Rotation)
	// ========================================
	if (enableRotation_) {
		state.useRotation = true;
		state.rotationSpeed = rotationSpeed_;
	}

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

			if (ImGui::Checkbox("Is Emitting", &isEmitting_)) {
				// 発生状態が変わった場合、タイマーをリセット
				if (isEmitting_) {
					frequencyTimer_ = 0.0f;
				}
			}

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

		// エミッター寿命設定
		if (ImGui::CollapsingHeader("Emitter Lifetime")) {
			ImGui::Checkbox("Use Emitter Lifetime", &useEmitterLifeTime_);

			if (useEmitterLifeTime_) {
				ImGui::DragFloat("Lifetime (sec)", &emitterLifeTime_, 0.1f, 0.1f, 60.0f);
				ImGui::Checkbox("Loop", &emitterLifeTimeLoop_);

				// 現在の経過時間表示
				ImGui::ProgressBar(emitterCurrentTime_ / emitterLifeTime_,
					ImVec2(-1.0f, 0.0f),
					std::format("Time: {:.2f} / {:.2f}", emitterCurrentTime_, emitterLifeTime_).c_str());

				// リセットボタン
				if (ImGui::Button("Reset Timer")) {
					ResetEmitterTime();
					isEmitting_ = true;  // 発生も再開
				}

				// 生存状態表示
				if (IsEmitterAlive()) {
					ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Alive");
				} else {
					ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Status: Dead");
				}
			}

			ImGui::Separator();
		}

		// パーティクル初期設定
		if (ImGui::CollapsingHeader("Particle Settings")) {
			// 寿命設定
			ImGui::Text("Lifetime");
			ImGui::DragFloat("Life Time Min", &particleLifeTimeMin_, 0.1f, 0.1f, 10.0f);
			ImGui::DragFloat("Life Time Max", &particleLifeTimeMax_, 0.1f, 0.1f, 10.0f);

			// 最小値が最大値を超えないようにする
			if (particleLifeTimeMin_ > particleLifeTimeMax_) {
				particleLifeTimeMax_ = particleLifeTimeMin_;
			}

			ImGui::Separator();

			// スケール設定
			ImGui::Text("Scale Range");
			ImGui::DragFloat3("Scale Min", &particleScaleMin_.x, 0.1f, 0.1f, 10.0f);
			ImGui::DragFloat3("Scale Max", &particleScaleMax_.x, 0.1f, 0.1f, 10.0f);

			ImGui::Separator();

			// 回転設定
			ImGui::Text("Rotation Range (Radians)");
			ImGui::DragFloat3("Rotate Min", &particleRotateMin_.x, 0.1f, -3.14159f, 3.14159f);
			ImGui::DragFloat3("Rotate Max", &particleRotateMax_.x, 0.1f, -3.14159f, 3.14159f);

			ImGui::Separator();
		}

		// 速度設定
		if (ImGui::CollapsingHeader("Velocity Settings")) {
			ImGui::Checkbox("Use Directional Emit", &useDirectionalEmit_);

			if (useDirectionalEmit_) {
				// 新方式：方向指定発射
				ImGui::Text("Directional Emit Mode");
				ImGui::Separator();

				// 発射方向
				Vector3 dir = emitDirection_;
				if (ImGui::DragFloat3("Emit Direction", &dir.x, 0.01f, -1.0f, 1.0f)) {
					SetEmitDirection(dir);
				}

				// 正規化ボタン
				if (ImGui::Button("Normalize Direction")) {
					SetEmitDirection(emitDirection_);
				}

				// 初速度
				ImGui::DragFloat("Initial Speed", &initialSpeed_, 0.1f, 0.0f, 20.0f);

				// 散らばり角度
				ImGui::SliderFloat("Spread Angle (deg)", &spreadAngle_, 0.0f, 180.0f);

				ImGui::Separator();

				// プリセットボタン
				ImGui::Text("Direction Presets:");
				if (ImGui::Button("Up")) SetEmitDirection({ 0.0f, 1.0f, 0.0f });
				ImGui::SameLine();
				if (ImGui::Button("Down")) SetEmitDirection({ 0.0f, -1.0f, 0.0f });
				ImGui::SameLine();
				if (ImGui::Button("Forward")) SetEmitDirection({ 0.0f, 0.0f, 1.0f });
				ImGui::SameLine();
				if (ImGui::Button("Back")) SetEmitDirection({ 0.0f, 0.0f, -1.0f });

			} else {
				// 旧方式：ランダム速度
				ImGui::Text("Random Velocity Mode");
				ImGui::Separator();
				ImGui::DragFloat("Velocity Range", &velocityRange_, 0.1f, 0.0f, 10.0f);
			}

			ImGui::Separator();
		}

		// ========================================
		// Color Over Lifetime
		// ========================================
		if (ImGui::CollapsingHeader("Color Over Lifetime")) {
			ImGui::Checkbox("Enable Color Over Lifetime", &enableColorOverLifetime_);

			if (enableColorOverLifetime_) {
				ImGui::Spacing();
				ImGui::Text("Color will transition from Start to End over particle lifetime");
				ImGui::Separator();

				ImGui::ColorEdit4("Start Color", &particleStartColor_.x);
				ImGui::ColorEdit4("End Color", &particleEndColor_.x);

				ImGui::Spacing();
				ImGui::TextDisabled("Tip: Set End Color alpha to 0 for fade-out effect");
			}

			ImGui::Separator();
		}

		// ========================================
		// Size Over Lifetime
		// ========================================
		if (ImGui::CollapsingHeader("Size Over Lifetime")) {
			ImGui::Checkbox("Enable Size Over Lifetime", &enableSizeOverLifetime_);

			if (enableSizeOverLifetime_) {
				ImGui::Spacing();
				ImGui::Text("Size will transition from Start to End over particle lifetime");
				ImGui::Separator();

				ImGui::DragFloat3("Start Scale", &particleStartScale_.x, 0.1f, 0.1f, 10.0f);
				ImGui::DragFloat3("End Scale", &particleEndScale_.x, 0.1f, 0.0f, 10.0f);

				ImGui::Spacing();
				ImGui::TextDisabled("Tip: Set End Scale to (0, 0, 0) to shrink to nothing");
			}

			ImGui::Separator();
		}

		// ========================================
		// Rotation
		// ========================================
		if (ImGui::CollapsingHeader("Rotation")) {
			ImGui::Checkbox("Enable Rotation", &enableRotation_);

			if (enableRotation_) {
				ImGui::Spacing();
				ImGui::Text("Particles will rotate continuously at specified speed");
				ImGui::Separator();

				ImGui::DragFloat3("Rotation Speed (deg/s)", &rotationSpeed_.x, 1.0f, -360.0f, 360.0f);

				ImGui::Spacing();
				ImGui::TextDisabled("Rotation speed in degrees per second");
				ImGui::TextDisabled("X: Roll, Y: Pitch, Z: Yaw");
			}

			ImGui::Separator();
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