#define NOMINMAX
#include "Particle.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "Object3DCommon.h"
#include "Random/Random.h"

void Particle::Initialize(DirectXCommon* dxCommon, const std::string& modelTag,
	uint32_t numParticles, const std::string& textureName) {
	directXCommon_ = dxCommon;
	modelTag_ = modelTag;
	textureName_ = textureName;
	numParticles_ = numParticles;


	// パーティクル配列の初期化
	particles_.resize(numParticles_);

	// 各パーティクルの初期状態を設定
	for (uint32_t i = 0; i < numParticles_; ++i) {
		// 位置を少しずつずらして配置
		particles_[i].transform.scale = { 1.0f, 1.0f, 1.0f };
		particles_[i].transform.rotate = { 0.0f, 0.0f, 0.0f };
		//原点から-1~1の範囲
		particles_[i].transform.translate = Random::GetInstance().GenerateVector3OriginOffset(1.0f);
		// 全て上方向の速度を設定
		particles_[i].velocity = Random::GetInstance().GenerateVector3OriginOffset(1.0f);
	}

	// GPU転送用バッファの作成
	CreateTransformBuffer();

	// モデルとマテリアル、テクスチャを設定
	SetModel(modelTag, textureName);
}

void Particle::CreateTransformBuffer() {
	// トランスフォーム用のリソースを作成
	transformResource_ = CreateBufferResource(
		directXCommon_->GetDevice(),
		sizeof(TransformationMatrix) * numParticles_
	);

	// トランスフォームデータにマップ
	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformData_));

	// 初期化
	for (uint32_t i = 0; i < numParticles_; ++i) {
		transformData_[i].World = MakeIdentity4x4();
		transformData_[i].WVP = MakeIdentity4x4();
	}

	// SRVの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = numParticles_;
	srvDesc.Buffer.StructureByteStride = sizeof(TransformationMatrix);

	srvHandle_ = directXCommon_->GetDescriptorManager()->AllocateSRV();
	directXCommon_->GetDevice()->CreateShaderResourceView(
		transformResource_.Get(),
		&srvDesc,
		srvHandle_.cpuHandle
	);
}

void Particle::Update(const Matrix4x4& viewProjectionMatrix, float deltaTime) {
	// 更新が有効な場合のみ物理更新を実行
	if (enableUpdate_) {
		UpdateParticles(deltaTime);
	}

	// GPU転送用のトランスフォーム行列を更新
	UpdateTransformBuffer(viewProjectionMatrix);

	// マテリアル更新
	materials_.UpdateAllUVTransforms();
}

void Particle::UpdateParticles(float deltaTime) {
	// 各パーティクルの物理更新
	for (auto& particle : particles_) {
		// 速度を位置に加算
		particle.transform.translate.x += particle.velocity.x * deltaTime;
		particle.transform.translate.y += particle.velocity.y * deltaTime;
		particle.transform.translate.z += particle.velocity.z * deltaTime;
	}
}

void Particle::UpdateTransformBuffer(const Matrix4x4& viewProjectionMatrix) {
	// 各パーティクルのワールド行列とWVP行列を計算
	for (uint32_t i = 0; i < numParticles_; ++i) {
		const auto& particle = particles_[i];

		// ワールド行列の計算
		transformData_[i].World = MakeAffineMatrix(
			particle.transform.scale,
			particle.transform.rotate,
			particle.transform.translate
		);

		// WVP行列の計算
		transformData_[i].WVP = Matrix4x4Multiply(
			transformData_[i].World,
			viewProjectionMatrix
		);
	}
}

void Particle::Draw(const Light& directionalLight) {
	if (!sharedModel_ || !sharedModel_->IsValid()) {
		return;
	}

	ID3D12GraphicsCommandList* commandList = directXCommon_->GetCommandList();
	particleCommon_->setCommonRenderSettings(commandList);

	// 全メッシュを描画
	const auto& meshes = sharedModel_->GetMeshes();
	for (size_t i = 0; i < meshes.size(); ++i) {
		const Mesh& mesh = meshes[i];
		size_t materialIndex = sharedModel_->GetMeshMaterialIndex(i);

		// 範囲チェック
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

		// ライトの設定
		commandList->SetGraphicsRootConstantBufferView(3,
			directionalLight.GetResource()->GetGPUVirtualAddress());

		// メッシュをバインドして描画（インスタンス数を指定）
		const_cast<Mesh&>(mesh).Bind(commandList);
		const_cast<Mesh&>(mesh).Draw(commandList, numParticles_);
	}
}

void Particle::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode(name_.c_str())) {
		// パーティクルシステム全体の設定
		if (ImGui::CollapsingHeader("Particle System", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Particle Count: %u", numParticles_);

			// 更新の有効/無効切り替え
			ImGui::Checkbox("Enable Update", &enableUpdate_);

			ImGui::Separator();
		}

		// 個別パーティクルの表示（最初の5つまで）
		if (ImGui::CollapsingHeader("Individual Particles")) {
			uint32_t displayCount = std::min(numParticles_, 5u);
			ImGui::Text("Showing first %u particles", displayCount);

			for (uint32_t i = 0; i < displayCount; ++i) {
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

					// 速度の編集
					if (ImGui::DragFloat3("Edit Velocity", &particle.velocity.x, 0.01f)) {
						// 速度が変更された
					}

					ImGui::TreePop();
				}
			}
		}

		// マテリアル設定
		if (ImGui::CollapsingHeader("Materials")) {
			size_t materialCount = GetMaterialCount();
			ImGui::Text("Material Count: %zu", materialCount);

			// 全マテリアル設定
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

				if (ImGui::Combo("Custom Texture", &currentIndex, textureNames.data(),
					static_cast<int>(textureNames.size()))) {
					if (currentIndex == 0) {
						SetTexture("");
					} else {
						SetTexture(textureList[currentIndex - 1]);
					}
				}
			}
		}

		ImGui::TreePop();
	}
#endif
}

void Particle::SetModel(const std::string& modelTag, const std::string& textureName) {
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
	materials_.Initialize(directXCommon_, materialCount);

	// モデルのマテリアル設定をコピー
	for (size_t i = 0; i < materialCount; ++i) {
		materials_.GetMaterial(i).CopyFrom(sharedModel_->GetMaterial(i));
	}

	SetTexture(textureName);
}