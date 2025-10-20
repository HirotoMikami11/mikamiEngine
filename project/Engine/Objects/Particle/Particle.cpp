#include "Particle.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "Object3DCommon.h"

void Particle::Initialize(DirectXCommon* dxCommon, const std::string& modelTag, const std::string& textureName) {
	directXCommon_ = dxCommon;
	modelTag_ = modelTag;
	textureName_ = textureName;

	// 個別のトランスフォームを初期化
	transform_.Initialize(dxCommon);
	Vector3Transform defaultTransform{
		{1.0f, 1.0f, 1.0f},  // scale
		{0.0f, 0.0f, 0.0f},  // rotate
		{0.0f, 0.0f, 0.0f}   // translate
	};
	transform_.SetTransform(defaultTransform);




	// モデルとマテリアル、テクスチャを設定
	SetModel(modelTag, textureName);
}

void Particle::Update(const Matrix4x4& viewProjectionMatrix) {
	// トランスフォーム行列の更新
	transform_.UpdateMatrix(viewProjectionMatrix);

	// マテリアル更新（常に自分のマテリアルを更新）
	materials_.UpdateAllUVTransforms();
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

		// 常に自分のマテリアルを使う
		commandList->SetGraphicsRootConstantBufferView(0,
			materials_.GetMaterial(materialIndex).GetResource()->GetGPUVirtualAddress());

		//トランスフォーム
		commandList->SetGraphicsRootDescriptorTable(1, transform_.GetSRV().gpuHandle);

		// テクスチャの設定
		if (!textureName_.empty()) {
			commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureHandle(textureName_));
		} else if (sharedModel_->HasTexture(materialIndex)) {
			commandList->SetGraphicsRootDescriptorTable(2,
				textureManager_->GetTextureHandle(sharedModel_->GetTextureTagName(materialIndex)));
		}
		//ライトの設定
		commandList->SetGraphicsRootConstantBufferView(3, directionalLight.GetResource()->GetGPUVirtualAddress());

		// メッシュをバインドして描画
		const_cast<Mesh&>(mesh).Bind(commandList);
		const_cast<Mesh&>(mesh).Draw(commandList,10);
	}
}

void Particle::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode(name_.c_str())) {
		// Transform
		if (ImGui::CollapsingHeader("Transform")) {
			Vector3 imguiPosition_ = transform_.GetPosition();
			Vector3 imguiRotation_ = transform_.GetRotation();
			Vector3 imguiScale_ = transform_.GetScale();

			if (ImGui::DragFloat3("Position", &imguiPosition_.x, 0.01f)) {
				transform_.SetPosition(imguiPosition_);
			}
			if (ImGui::DragFloat3("Rotation", &imguiRotation_.x, 0.01f)) {
				transform_.SetRotation(imguiRotation_);
			}
			if (ImGui::DragFloat3("Scale", &imguiScale_.x, 0.01f)) {
				transform_.SetScale(imguiScale_);
			}
		}

		// マテリアル設定
		if (ImGui::CollapsingHeader("Materials")) {
			size_t materialCount = GetMaterialCount();
			ImGui::Text("Material Count: %zu", materialCount);
			ImGui::Separator();

			// 複数マテリアルがある場合のみ全マテリアル設定を表示
			if (materialCount > 1) {
				if (ImGui::TreeNode("All Materials")) {
					Material& baseMaterial = GetMaterial(0);
					Vector4 color = baseMaterial.GetColor();
					LightingMode lightingMode = baseMaterial.GetLightingMode();

					if (ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&color.x))) {
						SetAllMaterialsColor(color, lightingMode);
					}

					const char* lightingModeNames[] = { "None", "Lambert", "Half-Lambert" };
					int currentModeIndex = static_cast<int>(lightingMode);
					if (ImGui::Combo("Lighting", &currentModeIndex, lightingModeNames, IM_ARRAYSIZE(lightingModeNames))) {
						LightingMode newMode = static_cast<LightingMode>(currentModeIndex);
						SetAllMaterialsColor(color, newMode);
					}

					ImGui::TreePop();
				}
				ImGui::Separator();
			}

			// 個別マテリアル設定
			for (size_t i = 0; i < materialCount; ++i) {
				std::string materialName = std::format("Material {}", i);

				if (ImGui::TreeNode(materialName.c_str())) {
					Material& material = GetMaterial(i);

					// 色設定（シンプル！）
					Vector4 color = material.GetColor();
					if (ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&color.x))) {
						material.SetColor(color);
					}

					// ライティング設定
					const char* lightingModeNames[] = { "None", "Lambert", "Half-Lambert" };
					LightingMode currentMode = material.GetLightingMode();
					int currentModeIndex = static_cast<int>(currentMode);
					if (ImGui::Combo("Lighting", &currentModeIndex, lightingModeNames, IM_ARRAYSIZE(lightingModeNames))) {
						material.SetLightingMode(static_cast<LightingMode>(currentModeIndex));
					}

					// UV設定
					Vector2 uvPosition = material.GetUVTransformTranslate();
					Vector2 uvScale = material.GetUVTransformScale();
					float uvRotateZ = material.GetUVTransformRotateZ();

					if (ImGui::DragFloat2("UV Translate", &uvPosition.x, 0.01f, -10.0f, 10.0f)) {
						material.SetUVTransformTranslate(uvPosition);
					}
					if (ImGui::DragFloat2("UV Scale", &uvScale.x, 0.01f, -10.0f, 10.0f)) {
						material.SetUVTransformScale(uvScale);
					}
					if (ImGui::SliderAngle("UV Rotate", &uvRotateZ)) {
						material.SetUVTransformRotateZ(uvRotateZ);
					}

					ImGui::TreePop();
				}
			}
		}

		// メッシュ情報
		if (ImGui::CollapsingHeader("Mesh Info") && sharedModel_) {
			ImGui::Text("Total Meshes: %zu", sharedModel_->GetMeshCount());

			const auto& meshes = sharedModel_->GetMeshes();
			const auto& objectNames = sharedModel_->GetObjectNames();

			for (size_t i = 0; i < meshes.size(); ++i) {
				const Mesh& mesh = meshes[i];
				std::string meshName = std::format("Mesh {} ({})", i,
					i < objectNames.size() ? objectNames[i] : "Unknown");

				if (ImGui::TreeNode(meshName.c_str())) {
					ImGui::Text("Mesh Type: %s", Mesh::MeshTypeToString(mesh.GetMeshType()).c_str());
					ImGui::Text("Vertex Count: %d", mesh.GetVertexCount());
					ImGui::Text("Index Count: %d", mesh.GetIndexCount());

					size_t materialIndex = sharedModel_->GetMeshMaterialIndex(i);
					ImGui::Text("Material Index: %zu", materialIndex);
					if (sharedModel_->HasTexture(materialIndex)) {
						ImGui::Text("Texture: %s", sharedModel_->GetTextureTagName(materialIndex).c_str());
					} else {
						ImGui::Text("Texture: none");
					}

					ImGui::TreePop();
				}
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

		ImGui::TreePop();
	}
#endif
}

void Particle::SetModel(const std::string& modelTag, const std::string& textureName)
{
	// 共有モデルを取得
	sharedModel_ = modelManager_->GetModel(modelTag);
	if (sharedModel_ == nullptr) {
		Logger::Log(Logger::GetStream(), std::format("Model '{}' not found! Call ModelManager::LoadModel first.\n", modelTag));
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

	SetTexture(textureName.c_str());
}
