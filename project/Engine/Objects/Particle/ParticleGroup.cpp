#define NOMINMAX
#include "ParticleGroup.h"
#include "BaseField.h"
#include "ImGui/ImGuiManager.h"
#include "LightManager.h"
#include "Logger.h"
#include <algorithm>

void ParticleGroup::Initialize(DirectXCommon* dxCommon, const std::string& modelTag,
	uint32_t maxParticles, const std::string& textureName, bool useBillboard)
{
	dxCommon_ = dxCommon;
	modelTag_ = modelTag;
	textureName_ = textureName;
	maxParticles_ = maxParticles;
	useBillboard_ = useBillboard;

	activeParticleCount_ = 0;

	// パーティクル配列の予約
	particles_.reserve(maxParticles_);

	// GPU転送用バッファの作成
	CreateTransformBuffer();

	// モデルとマテリアルを設定
	SetModel(modelTag, textureName);
}

void ParticleGroup::CreateTransformBuffer() {
	// トランスフォーム用のリソースを作成
	transformResource_ = CreateBufferResource(
		dxCommon_->GetDevice(),
		sizeof(ParticleForGPU) * maxParticles_
	);

	// トランスフォームデータにマップ
	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));

	// 初期化
	for (uint32_t i = 0; i < maxParticles_; ++i) {
		instancingData_[i].World = MakeIdentity4x4();
		instancingData_[i].WVP = MakeIdentity4x4();
		instancingData_[i].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	auto descriptorManager = dxCommon_->GetDescriptorManager();

	// 構造化バッファ用のSRVを作成
	srvHandle_ = descriptorManager->CreateSRVForStructuredBuffer(
		transformResource_.Get(),
		maxParticles_,
		sizeof(ParticleForGPU)
	);

	// エラーチェック
	if (!srvHandle_.isValid) {
		Logger::Log(Logger::GetStream(),
			std::format("Failed to create SRV for ParticleGroup (maxParticles: {})\n", maxParticles_));
		assert(false && "SRV creation failed");
	}
}

bool ParticleGroup::AddParticle(const ParticleState& particle)
{
	// 最大数に達していないかチェック
	if (particles_.size() >= maxParticles_) {
		return false;
	}

	// パーティクルを追加
	particles_.push_back(particle);
	return true;
}

void ParticleGroup::ClearAllParticles()
{
	particles_.clear();
	activeParticleCount_ = 0;
}

void ParticleGroup::Update(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& billboardMatrix, float deltaTime, const std::vector<BaseField*>& fields)
{
	// パーティクルの更新
	UpdateParticles(deltaTime, fields);

	// GPU転送用のデータを更新（ビルボード行列はManagerから受け取る）
	UpdateParticleForGPUBuffer(viewProjectionMatrix, billboardMatrix);

	// マテリアル更新
	materials_.UpdateAllUVTransforms();
}

void ParticleGroup::UpdateParticles(float deltaTime, const std::vector<BaseField*>& fields)
{
	// アクティブなパーティクル数をリセット
	activeParticleCount_ = 0;

	// 死んだパーティクルを削除するためのイテレータ
	auto it = particles_.begin();
	while (it != particles_.end()) {
		auto& particle = *it;

		// 寿命チェック
		if (particle.lifeTime <= particle.currentTime) {
			// 寿命が尽きたパーティクルを削除
			it = particles_.erase(it);
			continue;
		}

		// フィールドの効果を適用
		bool shouldDelete = false;
		for (auto* field : fields) {
			if (field && field->IsEnabled() && field->IsInField(particle.transform.translate)) {
				// フィールドの効果を適用
				shouldDelete = field->ApplyEffect(particle, deltaTime);
				if (shouldDelete) {
					break;	// 削除フラグが立ったらループを抜ける
				}
			}
		}

		// フィールドによって削除フラグが立った場合
		if (shouldDelete) {
			it = particles_.erase(it);
			continue;
		}

		// 速度を位置に加算
		particle.transform.translate.x += particle.velocity.x * deltaTime;
		particle.transform.translate.y += particle.velocity.y * deltaTime;
		particle.transform.translate.z += particle.velocity.z * deltaTime;

		/// 寿命に関連した更新処理

		// 寿命の進行率を計算（0.0 ~ 1.0）
		float t = particle.currentTime / particle.lifeTime;

		// Color Over Lifetime
		if (particle.useColorOverLifetime) {
			// 開始色から終了色へ線形補間
			particle.color = Lerp(particle.startColor, particle.endColor, t);
		} else {
			//色を透明にしていく
			float alpha = 1.0f - t;
			particle.color.w = alpha;
		}

		// Size Over Lifetime
		if (particle.useSizeOverLifetime) {
			// 開始スケールから終了スケールへ線形補間
			particle.transform.scale = Lerp(particle.startScale, particle.endScale, t);
		}

		// Rotation
		if (particle.useRotation) {
			// 回転速度を適用（度/秒 → ラジアン/秒に変換）
			Vector3 rotationDelta = {
				DegToRad(particle.rotationSpeed.x) * deltaTime,
				DegToRad(particle.rotationSpeed.y) * deltaTime,
				DegToRad(particle.rotationSpeed.z) * deltaTime
			};

			particle.transform.rotate.x += rotationDelta.x;
			particle.transform.rotate.y += rotationDelta.y;
			particle.transform.rotate.z += rotationDelta.z;
		}

		// 寿命を進める
		particle.currentTime += deltaTime;

		// アクティブなパーティクルとしてカウント
		++activeParticleCount_;
		++it;
	}
}

void ParticleGroup::UpdateParticleForGPUBuffer(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& billboardMatrix)
{
	// アクティブなパーティクルのみGPUバッファに書き込む
	for (uint32_t i = 0; i < activeParticleCount_ && i < particles_.size(); ++i) {
		const auto& particle = particles_[i];

		if (useBillboard_) {
			// ビルボードが有効な場合（ビルボード行列はManagerから受け取る）
			Matrix4x4 scaleMatrix = MakeScaleMatrix(particle.transform.scale);
			Matrix4x4 translateMatrix = MakeTranslateMatrix(particle.transform.translate);

			// World = Scale * Billboard * Translate
			instancingData_[i].World = Matrix4x4Multiply(
				Matrix4x4Multiply(scaleMatrix, billboardMatrix),
				translateMatrix
			);
		} else {
			// ビルボードが無効な場合
			instancingData_[i].World = MakeAffineMatrix(
				particle.transform.scale,
				particle.transform.rotate,
				particle.transform.translate
			);
		}

		// WVP行列の計算
		instancingData_[i].WVP = Matrix4x4Multiply(
			instancingData_[i].World,
			viewProjectionMatrix
		);

		// 色の設定
		instancingData_[i].color = particle.color;
	}
}

void ParticleGroup::Draw()
{
	if (!sharedModel_ || !sharedModel_->IsValid()) {
		return;
	}

	if (activeParticleCount_ == 0) {
		return;
	}

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	particleCommon_->setCommonRenderSettings(commandList);

	// 全メッシュを描画
	const auto& meshes = sharedModel_->GetMeshes();
	for (size_t i = 0; i < meshes.size(); ++i) {
		const Mesh& mesh = meshes[i];
		size_t materialIndex = sharedModel_->GetMeshMaterialIndex(i);

		if (materialIndex >= materials_.GetMaterialCount()) {
			materialIndex = 0;
		}

		// マテリアル設定
		commandList->SetGraphicsRootConstantBufferView(0,
			materials_.GetMaterial(materialIndex).GetResource()->GetGPUVirtualAddress());

		// トランスフォーム（構造化バッファ）
		commandList->SetGraphicsRootDescriptorTable(1, srvHandle_.gpuHandle);

		// テクスチャの設定
		if (!textureName_.empty()) {
			commandList->SetGraphicsRootDescriptorTable(2,
				textureManager_->GetTextureHandle(textureName_));
		} else if (sharedModel_->HasTexture(materialIndex)) {
			commandList->SetGraphicsRootDescriptorTable(2,
				textureManager_->GetTextureHandle(sharedModel_->GetTextureTagName(materialIndex)));
		}

		// メッシュをバインドして描画（アクティブなパーティクル数を指定）
		const_cast<Mesh&>(mesh).Bind(commandList);
		const_cast<Mesh&>(mesh).Draw(commandList, activeParticleCount_);
	}
}

void ParticleGroup::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode(name_.c_str())) {
		// パーティクルグループ全体の設定
		if (ImGui::CollapsingHeader("Particle Group", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Active Particles: %u / %u", activeParticleCount_, maxParticles_);
			ImGui::Checkbox("Use Billboard", &useBillboard_);

			ImGui::Separator();
		}


		// テクスチャ設定
		if (ImGui::CollapsingHeader("Texture")) {
			std::vector<std::string> textureList = textureManager_->GetTextureTagList();
			if (!textureList.empty()) {
				std::vector<const char*> textureNames;
				textureNames.push_back("Default");

				for (const auto& texture : textureList) {
					textureNames.push_back(texture.c_str());
				}

				int currentIndex = 0;
				if (!textureName_.empty()) {
					for (size_t i = 0; i < textureList.size(); ++i) {
						if (textureList[i] == textureName_) {
							currentIndex = static_cast<int>(i + 1);
							break;
						}
					}
				}

				if (ImGui::Combo("Custom Texture", &currentIndex, textureNames.data(), static_cast<int>(textureNames.size()))) {
					if (currentIndex == 0) {
						SetTexture("");
					} else {
						SetTexture(textureList[currentIndex - 1]);
					}
				}
			} else {
				ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No textures loaded");
			}
		}



		// 個別パーティクルの表示（最初の5つまで）
		if (ImGui::CollapsingHeader("Individual Particles")) {
			uint32_t displayCount = std::min(activeParticleCount_, 5u);
			ImGui::Text("Showing first %u particles", displayCount);

			for (uint32_t i = 0; i < displayCount && i < particles_.size(); ++i) {
				std::string particleLabel = std::format("Particle {}", i);
				if (ImGui::TreeNode(particleLabel.c_str())) {
					auto& particle = particles_[i];

					ImGui::Text("Position: (%.2f, %.2f, %.2f)",
						particle.transform.translate.x,
						particle.transform.translate.y,
						particle.transform.translate.z);

					ImGui::Text("Velocity: (%.2f, %.2f, %.2f)",
						particle.velocity.x,
						particle.velocity.y,
						particle.velocity.z);

					ImGui::Text("Life: %.2f / %.2f",
						particle.currentTime,
						particle.lifeTime);

					ImGui::ColorEdit4("Color", &particle.color.x);

					ImGui::TreePop();
				}
			}
		}

		// マテリアル設定
		if (ImGui::CollapsingHeader("Materials")) {
			size_t materialCount = GetMaterialCount();
			ImGui::Text("Material Count: %zu", materialCount);

			if (ImGui::TreeNode("All Materials")) {
				Material& baseMaterial = GetMaterial(0);
				Vector4 color = baseMaterial.GetColor();
				LightingMode lightingMode = baseMaterial.GetLightingMode();

				if (ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&color.x))) {
					SetAllMaterialsColor(color, lightingMode);
				}

				const char* lightingModeNames[] = { "None", "Lambert", "Half-Lambert" };
				int currentModeIndex = static_cast<int>(lightingMode);
				if (ImGui::Combo("Lighting", &currentModeIndex, lightingModeNames,
					IM_ARRAYSIZE(lightingModeNames))) {
					SetAllMaterialsColor(color, static_cast<LightingMode>(currentModeIndex));
				}

				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}
#endif
}

void ParticleGroup::SetModel(const std::string& modelTag, const std::string& textureName)
{
	// 共有モデルを取得
	sharedModel_ = modelManager_->GetModel(modelTag);
	if (sharedModel_ == nullptr) {
		Logger::Log(Logger::GetStream(),
			std::format("Model '{}' not found! Call ModelManager::LoadModel first.\n", modelTag));
		assert(false && "Model not preloaded! Call ModelManager::LoadModel first.");
		return;
	}

	// 個別マテリアルを初期化（モデルからコピー）
	size_t materialCount = sharedModel_->GetMaterialCount();
	materials_.Initialize(dxCommon_, materialCount);

	// モデルのマテリアル設定をコピー
	for (size_t i = 0; i < materialCount; ++i) {
		materials_.GetMaterial(i).CopyFrom(sharedModel_->GetMaterial(i));
	}

	textureName_ = textureName;
}