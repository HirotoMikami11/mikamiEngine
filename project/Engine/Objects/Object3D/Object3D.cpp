#include "Object3D.h"
#include "ImGui/ImGuiManager.h"
#include "CameraController.h"

void Object3D::Initialize(DirectXCommon* dxCommon, const std::string& modelTag, const std::string& textureName) {
	dxCommon_ = dxCommon;
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

void Object3D::Update(const Matrix4x4& viewProjectionMatrix) {
	// トランスフォーム行列の更新
	// UpdateMatrix内でモデルオフセットが自動的に適用される
	transform_.UpdateMatrix(viewProjectionMatrix);

	// マテリアル更新（常に自分のマテリアルを更新）
	materials_.UpdateAllUVTransforms();
}

void Object3D::Draw() {
	if (!sharedModel_ || !sharedModel_->IsValid()) {
		return;
	}

	Object3DRenderer* renderer = Object3DRenderer::GetInstance();

	// -----------------------------------------------------------------------
	// ソート用の深度値を計算
	// ワールド行列の平行移動成分（行3）からオブジェクトのワールド座標を取得し、
	// カメラとの距離の2乗を計算する（単調増加なのでソートの大小関係が保持される）
	// -----------------------------------------------------------------------
	const Matrix4x4& worldMat = transform_.GetWorldMatrix();
	Vector3 worldPos = { worldMat.m[3][0], worldMat.m[3][1], worldMat.m[3][2] };
	Vector3 camPos   = CameraController::GetInstance()->GetPosition();
	float dx = worldPos.x - camPos.x;
	float dy = worldPos.y - camPos.y;
	float dz = worldPos.z - camPos.z;
	float sortDepth = dx * dx + dy * dy + dz * dz;

	// -----------------------------------------------------------------------
	// 全メッシュ分の描画リクエストをキューに積む
	// GPU コマンド発行は Object3DRenderer::Draw3D() が一括で行う
	// -----------------------------------------------------------------------
	const auto& meshes = sharedModel_->GetMeshes();
	for (size_t i = 0; i < meshes.size(); ++i) {
		const Mesh& mesh = meshes[i];

		// メッシュに対応するマテリアルインデックスを取得（範囲外は 0 にクランプ）
		size_t materialIndex = sharedModel_->GetMeshMaterialIndex(i);
		if (materialIndex >= materials_.GetMaterialCount()) {
			materialIndex = 0;
		}
		const Material& mat = materials_.GetMaterial(materialIndex);

		// テクスチャハンドルを取得
		// カスタムテクスチャ > モデルの埋め込みテクスチャ の優先順位
		D3D12_GPU_DESCRIPTOR_HANDLE textureHandle = {};
		if (!textureName_.empty()) {
			textureHandle = textureManager_->GetTextureHandle(textureName_);
		} else if (sharedModel_->HasTexture(materialIndex)) {
			textureHandle = textureManager_->GetTextureHandle(
				sharedModel_->GetTextureTagName(materialIndex));
		}

		// マテリアルのアルファ値でレンダーグループを決定
		// alpha < 1 → AlphaBlend（奥から手前へソート）、それ以外 → Opaque
		RenderGroup group = (mat.GetColor().w < 1.0f)
			? RenderGroup::AlphaBlend
			: RenderGroup::Opaque;

		// ModelSubmission を構築して Object3DRenderer のキューに積む
		// transformGpuAddr / materialGpuAddr は現時点では各オブジェクト固有の
		// GPU バッファアドレスを使用する（将来的に UploadRingBuffer に移行予定）
		ModelSubmission submission{};
		submission.mesh             = &mesh;
		submission.transformGpuAddr = transform_.GetResource()->GetGPUVirtualAddress();
		submission.materialGpuAddr  = mat.GetResource()->GetGPUVirtualAddress();
		submission.textureHandle    = textureHandle;
		submission.group            = group;
		submission.sortDepth        = sortDepth;
		renderer->Submit(submission);
	}
}

void Object3D::ImGui() {
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

					const char* lightingModeNames[] = { "None", "Lambert", "Half-Lambert","PhongSpecular" };
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

					// 色設定
					Vector4 color = material.GetColor();
					if (ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&color.x))) {
						material.SetColor(color);
					}

					// ライティング設定
					const char* lightingModeNames[] = { "None", "Lambert", "Half-Lambert","PhongSpecular" };
					LightingMode currentMode = material.GetLightingMode();
					int currentModeIndex = static_cast<int>(currentMode);
					if (ImGui::Combo("Lighting", &currentModeIndex, lightingModeNames, IM_ARRAYSIZE(lightingModeNames))) {
						material.SetLightingMode(static_cast<LightingMode>(currentModeIndex));
					}


					if (currentMode == LightingMode::PhongSpecular) {
						float shininess = material.GetShininess();
						if (ImGui::DragFloat("Specular Shininess", &shininess, 0.01f, 0.0f, 100.0f)) {
							material.SetShininess(shininess);
						}
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

void Object3D::SetModel(const std::string& modelTag, const std::string& textureName)
{
	// 共有モデルを取得
	sharedModel_ = modelManager_->GetModel(modelTag);
	if (sharedModel_ == nullptr) {
		Logger::Log(Logger::GetStream(), std::format("Model '{}' not found! Call ModelManager::LoadModel first.\n", modelTag));
		assert(false && "Model not preloaded! Call ModelManager::LoadModel first.");
		return;
	}

	//																			//
	//				モデルのrootNode.localMatrixをオフセットとして設定				//
	//																			//
	
	// glTFモデルの場合: rootNode.localMatrixが存在し、モデル空間での初期姿勢を表す
	// OBJモデルの場合: rootNode.localMatrixは単位行列なので影響なし
	const auto& modelDataList = sharedModel_->GetModelData();
	if (!modelDataList.empty()) {
		// モデルのrootNode.localMatrixをTransformのモデルオフセットとして設定
		// これにより、Update()時に自動的にオフセットが適用される
		transform_.SetModelOffset(modelDataList[0].rootNode.localMatrix);
	} else {
		// モデルデータがない場合は単位行列を設定（変換なし）
		transform_.SetModelOffset(MakeIdentity4x4());
	}

	//																			//
	//						マテリアルの初期化とコピー							//
	//																			//

	// 個別マテリアルを初期化（モデルからコピー）
	size_t materialCount = sharedModel_->GetMaterialCount();
	materials_.Initialize(dxCommon_, materialCount);

	// モデルのマテリアル設定をコピー
	for (size_t i = 0; i < materialCount; ++i) {
		materials_.GetMaterial(i).CopyFrom(sharedModel_->GetMaterial(i));
	}

	// テクスチャ設定
	SetTexture(textureName.c_str());
}