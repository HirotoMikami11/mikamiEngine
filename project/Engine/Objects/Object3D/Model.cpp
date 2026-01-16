#define NOMINMAX
#include "Model.h"
#include "MaterialGroup.h"

// Assimpのインクルード
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <fstream>
#include <sstream>
#include <algorithm>

void Model::Initialize(DirectXCommon* dxCommon, const MeshType meshType, const std::string& directoryPath, const std::string& filename)
{
	dxCommon_ = dxCommon;

	//																			//
	//								メッシュの作成									//
	//																			//

	///モデルの場合は、ファイルパスなどを入れる
	if (meshType == MeshType::MODEL_OBJ) {
		// Assimpを使用して複数オブジェクト対応でデータを読み込む
		modelDataList_ = LoadModelWithAssimp(directoryPath, filename);

		// 各ModelDataからMeshを作成
		meshes_.clear();
		meshMaterialIndices_.clear();

		for (const auto& modelData : modelDataList_) {
			Mesh mesh;
			mesh.InitializeFromData(dxCommon_, modelData);
			meshes_.push_back(std::move(mesh));
			meshMaterialIndices_.push_back(modelData.materialIndex);
		}

		// 全マテリアル情報を収集してマテリアルとテクスチャを作成
		std::set<std::string> uniqueMaterials;
		std::map<std::string, MaterialDataModel> materialMap;

		for (const auto& modelData : modelDataList_) {
			if (!modelData.materialName.empty()) {
				uniqueMaterials.insert(modelData.materialName);
				materialMap[modelData.materialName] = modelData.material;
			}
		}

		// MaterialGroupを初期化
		size_t materialCount = std::max(uniqueMaterials.size(), size_t(1));
		materialGroup_.Initialize(dxCommon_, materialCount);

		textureTagNames_.clear();
		textureTagNames_.resize(materialCount);

		TextureManager* textureManager = TextureManager::GetInstance();

		// マテリアルを設定
		if (uniqueMaterials.empty()) {
			// マテリアルが1つもない場合は、デフォルトマテリアルを使用
			Logger::Log(Logger::GetStream(),
				std::format("  No materials found, using default material\n"));
		} else {
			// マテリアルが存在する場合は、各マテリアルを設定
			size_t materialIndex = 0;
			for (const auto& materialName : uniqueMaterials) {
				Material& material = materialGroup_.GetMaterial(materialIndex);
				material.SetLitObjectSettings(); // デフォルトでライティング有効

				// テクスチャを読み込み
				const auto& materialData = materialMap[materialName];
				if (!materialData.textureFilePath.empty()) {
					std::string textureTag = GetTextureFileNameFromPath(materialData.textureFilePath);
					if (textureManager->LoadTexture(materialData.textureFilePath, textureTag)) {
						textureTagNames_[materialIndex] = textureTag;
						Logger::Log(Logger::GetStream(), std::format("Loaded texture: {} as {}\n", materialData.textureFilePath, textureTag));
					} else {
						Logger::Log(Logger::GetStream(), std::format("Failed to load texture: {}\n", materialData.textureFilePath));
					}
				}
				materialIndex++;
			}
		}

		filePath_ = directoryPath + "/" + filename;
	} else {
		///モデル以外の場合は、パス入れないで生成
		Mesh mesh;
		mesh.Initialize(dxCommon_, meshType);
		meshes_.clear();
		meshes_.push_back(std::move(mesh));

		objectNames_.clear();
		objectNames_.push_back("primitive_" + Mesh::MeshTypeToString(meshType));

		// プリミティブ用のシングルマテリアル
		materialGroup_.Initialize(dxCommon_, 1);

		textureTagNames_.clear();
		textureTagNames_.push_back("");

		meshMaterialIndices_.clear();
		meshMaterialIndices_.push_back(0); // 最初のマテリアルを使用

		filePath_ = "primitive_" + Mesh::MeshTypeToString(meshType);
	}
}

bool Model::LoadFromOBJ(const std::string& directoryPath, const std::string& filename, DirectXCommon* dxCommon) {
	// 既に読み込み済みの場合はスキップ
	if (IsValid() && filePath_ == directoryPath + "/" + filename) {
		return true;
	}

	dxCommon_ = dxCommon;
	filePath_ = directoryPath + "/" + filename;

	// Assimpを使用して複数オブジェクト対応でファイルを読み込み
	modelDataList_ = LoadModelWithAssimp(directoryPath, filename);

	if (modelDataList_.empty()) {
		Logger::Log(Logger::GetStream(), std::format("Failed to load model data from: {}\n", filename));
		return false;
	}

	// 各ModelDataからMeshを作成
	meshes_.clear();
	meshMaterialIndices_.clear();

	for (size_t i = 0; i < modelDataList_.size(); ++i) {
		Mesh mesh;
		mesh.InitializeFromData(dxCommon, modelDataList_[i]);
		meshes_.push_back(std::move(mesh));
		meshMaterialIndices_.push_back(modelDataList_[i].materialIndex);
	}

	// 全マテリアル情報を収集してマテリアルとテクスチャを作成
	std::set<std::string> uniqueMaterials;
	std::map<std::string, MaterialDataModel> materialMap;

	for (const auto& modelData : modelDataList_) {
		if (!modelData.materialName.empty()) {
			uniqueMaterials.insert(modelData.materialName);
			materialMap[modelData.materialName] = modelData.material;
		}
	}

	// MaterialGroupを初期化
	size_t materialCount = std::max(uniqueMaterials.size(), size_t(1));
	materialGroup_.Initialize(dxCommon, materialCount);

	textureTagNames_.clear();
	textureTagNames_.resize(materialCount);

	TextureManager* textureManager = TextureManager::GetInstance();

	// マテリアルを設定
	if (uniqueMaterials.empty()) {
		// マテリアルが1つもない場合は、デフォルトマテリアルを使用
		Logger::Log(Logger::GetStream(),
			std::format("  No materials found, using default material\n"));
	} else {
		// マテリアルが存在する場合は、各マテリアルを設定
		size_t materialIndex = 0;
		for (const auto& materialName : uniqueMaterials) {
			Material& material = materialGroup_.GetMaterial(materialIndex);
			material.SetLitObjectSettings(); // デフォルトでライティング有効

			// テクスチャを読み込み
			const auto& materialData = materialMap[materialName];
			if (!materialData.textureFilePath.empty()) {
				std::string textureTag = GetTextureFileNameFromPath(materialData.textureFilePath);
				if (textureManager->LoadTexture(materialData.textureFilePath, textureTag)) {
					textureTagNames_[materialIndex] = textureTag;
					Logger::Log(Logger::GetStream(), std::format("Loaded texture: {} as {}\n", materialData.textureFilePath, textureTag));
				} else {
					Logger::Log(Logger::GetStream(), std::format("Failed to load texture: {}\n", materialData.textureFilePath));
				}
			}
			materialIndex++;
		}
	}

	Logger::Log(Logger::GetStream(), std::format("Model loaded from file with Assimp: {} ({} meshes, {} materials)\n",
		filename, meshes_.size(), materialGroup_.GetMaterialCount()));
	return true;
}

bool Model::LoadFromPrimitive(MeshType meshType, DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// プリミティブメッシュを作成
	Mesh mesh;
	mesh.Initialize(dxCommon, meshType);

	meshes_.clear();
	meshes_.push_back(std::move(mesh));

	objectNames_.clear();
	objectNames_.push_back("primitive_" + Mesh::MeshTypeToString(meshType));

	// プリミティブ用のシングルマテリアル
	materialGroup_.Initialize(dxCommon, 1);

	textureTagNames_.clear();
	textureTagNames_.push_back("");

	meshMaterialIndices_.clear();
	meshMaterialIndices_.push_back(0); // 最初のマテリアルを使用

	// プリミティブにはテクスチャは無い
	filePath_ = "primitive_" + Mesh::MeshTypeToString(meshType);

	Logger::Log(Logger::GetStream(), std::format("Model loaded from primitive: {}\n", Mesh::MeshTypeToString(meshType)));
	return true;
}

void Model::UpdateMaterials() {
	materialGroup_.UpdateAllUVTransforms();
}

void Model::Unload() {
	meshes_.clear(); // 全メッシュをクリア
	objectNames_.clear();
	materialGroup_ = MaterialGroup(); // MaterialGroupをリセット
	textureTagNames_.clear();
	meshMaterialIndices_.clear();
	filePath_.clear();
	modelDataList_.clear(); // モデルデータリストをクリア
}

std::string Model::GetFileNameWithoutExtension(const std::string& filename) {
	// 最後のドット（拡張子の開始位置）を見つける
	size_t lastDotPos = filename.find_last_of('.');

	if (lastDotPos != std::string::npos && lastDotPos > 0) {
		// 拡張子がある場合は、それを除いた部分を返す
		return filename.substr(0, lastDotPos);
	}

	// 拡張子がない場合はそのまま返す
	return filename;
}

std::string Model::GetTextureFileNameFromPath(const std::string& texturePath) {
	// パス区切り文字を探す
	size_t lastSlashPos = texturePath.find_last_of("/\\");

	// ファイル名部分を抽出
	std::string fileName;
	if (lastSlashPos != std::string::npos) {
		fileName = texturePath.substr(lastSlashPos + 1);
	} else {
		fileName = texturePath; // パス区切りがない場合はそのまま
	}

	// 拡張子を除去
	return GetFileNameWithoutExtension(fileName);
}

///*---------------------------------------------------------------------------*///
///					Assimpを使用したモデルデータの読み込み					///
///*---------------------------------------------------------------------------*///

std::vector<ModelData> Model::LoadModelWithAssimp(const std::string& directoryPath, const std::string& filename) {
	std::vector<ModelData> modelDataList;

	//																			//
	//							Importerの作成									//
	//																			//

	Assimp::Importer importer;
	std::string filePath = directoryPath + "/" + filename;

	Logger::Log(Logger::GetStream(), std::format("Loading model with Assimp: {}\n", filePath));

	//																			//

	//																			//
	//						ファイルの読み込みとオプション設定						//
	//																			//

	// ファイル拡張子を取得して小文字に変換
	std::string fileExtension = filename.substr(filename.find_last_of(".") + 1);
	std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);

	// glTF/glbファイルかどうかを判定
	bool isGLTF = (fileExtension == "gltf" || fileExtension == "glb");

	// Assimpでファイルを読み込む
	// オプション:
	// - aiProcess_Triangulate: 四角形以上のポリゴンを三角形に分割
	// - aiProcess_FlipWindingOrder: 三角形の巻き順を反転（OBJ/FBX用、glTFでは不要）
	// - aiProcess_FlipUVs: UVのY座標を反転（DirectX用）

	// 基本フラグ
	// 全てのファイル形式に対してFlipWindingOrderを適用
	// glTFの場合は頂点のX軸反転を行わないことで正しい向きになる
	uint32_t flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_FlipWindingOrder;

	const aiScene* scene = importer.ReadFile(filePath.c_str(), flags);

	Logger::Log(Logger::GetStream(),
		std::format("  File type: {}, X-axis flip: {}\n",
			fileExtension, !isGLTF ? "enabled" : "disabled"));

	//																			//
	//							読み込み失敗のチェック								//
	//																			//

	// エラーチェック1: sceneがnullptr
	if (!scene) {
		Logger::Log(Logger::GetStream(),
			std::format("Assimp Error: Failed to load scene\n  Error: {}\n", importer.GetErrorString()));
		return modelDataList;
	}

	// エラーチェック2: シーンが不完全
	if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
		Logger::Log(Logger::GetStream(),
			std::format("Assimp Error: Scene is incomplete\n  Error: {}\n", importer.GetErrorString()));
		return modelDataList;
	}

	// エラーチェック3: ルートノードがない
	if (!scene->mRootNode) {
		Logger::Log(Logger::GetStream(),
			std::format("Assimp Error: No root node\n  Error: {}\n", importer.GetErrorString()));
		return modelDataList;
	}

	// エラーチェック4: メッシュがない
	if (!scene->HasMeshes()) {
		Logger::Log(Logger::GetStream(),
			std::format("Assimp Warning: Model has no meshes: {}\n", filePath));
		return modelDataList;
	}

	//																			//
	//						マテリアルデータの事前収集								//
	//																			//

	// マテリアル名とインデックスのマッピング
	std::map<uint32_t, std::string> materialNames;
	std::map<std::string, MaterialDataModel> materialDataMap;
	std::map<std::string, size_t> materialIndexMap;
	size_t materialIndexCounter = 0;

	// Assimpのマテリアルを処理
	for (uint32_t matIndex = 0; matIndex < scene->mNumMaterials; ++matIndex) {
		aiMaterial* material = scene->mMaterials[matIndex];

		// マテリアル名を取得
		aiString materialName;
		material->Get(AI_MATKEY_NAME, materialName);
		std::string matName = materialName.C_Str();

		materialNames[matIndex] = matName;

		// マテリアルインデックスを割り当て
		if (materialIndexMap.find(matName) == materialIndexMap.end()) {
			materialIndexMap[matName] = materialIndexCounter++;
		}

		// Diffuseテクスチャを取得
		MaterialDataModel matData;
		if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString texturePath;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
				// テクスチャのパスを設定
				matData.textureFilePath = directoryPath + "/" + std::string(texturePath.C_Str());
			}
		}

		materialDataMap[matName] = matData;

		Logger::Log(Logger::GetStream(),
			std::format("  Material {}: {} -> texture: {}\n",
				matIndex, matName, matData.textureFilePath.empty() ? "none" : matData.textureFilePath));
	}

	//																			//
	//							メッシュの解析										//
	//																			//

	// オブジェクト名をクリア
	objectNames_.clear();

	// 各メッシュを処理
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];

		Logger::Log(Logger::GetStream(),
			std::format("  Processing mesh {}/{}: {}\n",
				meshIndex + 1, scene->mNumMeshes, mesh->mName.C_Str()));


		//					必須データのチェック（法線のみ）					//
		// 法線のチェック
		if (!mesh->HasNormals()) {
			Logger::Log(Logger::GetStream(),
				std::format("    Warning: Mesh '{}' has no normals, skipping\n", mesh->mName.C_Str()));
			continue;
		}

		// テクスチャ座標のチェック（警告のみ、継続可能）
		bool hasTexCoords = mesh->HasTextureCoords(0);
		if (!hasTexCoords) {
			Logger::Log(Logger::GetStream(),
				std::format("    Info: Mesh '{}' has no texture coordinates, using default (0,0)\n",
					mesh->mName.C_Str()));
		}


		//						ModelDataの準備									//
		ModelData modelData;

		// オブジェクト名を保存
		std::string objectName = mesh->mName.C_Str();
		if (objectName.empty()) {
			objectName = "mesh_" + std::to_string(meshIndex);
		}
		objectNames_.push_back(objectName);

		//																		//
		//							Faceの解析									//
		//																		//

		// 面を処理
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			const aiFace& face = mesh->mFaces[faceIndex];

			// 三角形のみサポート（aiProcess_Triangulateで保証される）
			if (face.mNumIndices != 3) {
				Logger::Log(Logger::GetStream(),
					std::format("    Warning: Face {} is not a triangle (indices: {}), skipping\n",
						faceIndex, face.mNumIndices));
				continue;
			}

			//						各頂点の処理									//

			// 各頂点を処理
			for (uint32_t i = 0; i < face.mNumIndices; ++i) {
				uint32_t vertexIndex = face.mIndices[i];

				VertexData vertex;


				//						頂点位置の取得						//
				const aiVector3D& position = mesh->mVertices[vertexIndex];

				// glTFの場合は座標系変換済みなのでX軸反転不要
				// OBJ等の場合はOpenGL座標系なのでX軸を反転して左手座標系に変換
				if (isGLTF) {
					vertex.position = {
						position.x,
						position.y,
						position.z,
						1.0f
					};
				} else {
					vertex.position = {
						-position.x,  // 左手座標系への変換（X軸を反転）
						position.y,
						position.z,
						1.0f
					};
				}

				//						法線の取得							//

				const aiVector3D& normal = mesh->mNormals[vertexIndex];

				// glTFの場合は座標系変換済みなのでX軸反転不要
				// OBJ等の場合はOpenGL座標系なのでX軸を反転して左手座標系に変換
				if (isGLTF) {
					vertex.normal = {
						normal.x,
						normal.y,
						normal.z
					};
				} else {
					vertex.normal = {
						-normal.x,    // 左手座標系への変換（X軸を反転）
						normal.y,
						normal.z
					};
				}



				//					テクスチャ座標の取得						//
				// テクスチャ座標を取得（aiProcess_FlipUVsで既にY反転済み）
				// テクスチャ座標がない場合はデフォルト値(0, 0)を使用
				if (hasTexCoords) {
					const aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
					vertex.texcoord = {
						texcoord.x,
						texcoord.y
					};
				} else {
					// テクスチャ座標がない場合はデフォルト値
					vertex.texcoord = { 0.0f, 0.0f };
				}

				// 頂点を追加
				modelData.vertices.push_back(vertex);
			}
		}


		//						マテリアル情報の設定							//
		// マテリアル情報を設定
		if (mesh->mMaterialIndex < scene->mNumMaterials) {
			std::string matName = materialNames[mesh->mMaterialIndex];
			modelData.materialName = matName;
			modelData.materialIndex = materialIndexMap[matName];
			modelData.material = materialDataMap[matName];

			Logger::Log(Logger::GetStream(),
				std::format("    Mesh '{}': {} vertices, material: '{}' (index: {})\n",
					objectName, modelData.vertices.size(), matName, modelData.materialIndex));
		} else {
			Logger::Log(Logger::GetStream(),
				std::format("    Mesh '{}': {} vertices, no material\n",
					objectName, modelData.vertices.size()));
		}


		//					ModelDataをリストに追加							//
		// モデルデータをリストに追加
		if (!modelData.vertices.empty()) {
			modelDataList.push_back(modelData);
		}
	}

	//																			//
	//						RootNodeの読み込みと設定							//
	//																			//

	// scene->mRootNodeを読み込んでNode階層を作成
	Node rootNode = ReadNode(scene->mRootNode);

	// 全てのModelDataにrootNodeを設定
	for (auto& modelData : modelDataList) {
		modelData.rootNode = rootNode;
	}

	//読み込み結果のログ
	Logger::Log(Logger::GetStream(),
		std::format("Successfully loaded {} meshes with {} materials from {}\n\n",
			modelDataList.size(), materialIndexMap.size(), filename));

	return modelDataList;
}
///*---------------------------------------------------------------------------*///
///						assimpのaiNodeを読み込む関数						///
///*---------------------------------------------------------------------------*///

Node Model::ReadNode(const aiNode* node) {
	// 結果のNode構造体を作成
	Node result;

	//																			//
	//							NodeのLocalMatrixを取得							//
	//																			//

	// aiNodeのTransformationから行列を取得
	aiMatrix4x4 aiLocalMatrix = node->mTransformation;

	// aiMatrix4x4をMyMath::Matrix4x4に変換
	// assimpの行列は列優先、DirectXは行優先なので転置が必要
	result.localMatrix.m[0][0] = aiLocalMatrix.a1; result.localMatrix.m[0][1] = aiLocalMatrix.b1;
	result.localMatrix.m[0][2] = aiLocalMatrix.c1; result.localMatrix.m[0][3] = aiLocalMatrix.d1;

	result.localMatrix.m[1][0] = aiLocalMatrix.a2; result.localMatrix.m[1][1] = aiLocalMatrix.b2;
	result.localMatrix.m[1][2] = aiLocalMatrix.c2; result.localMatrix.m[1][3] = aiLocalMatrix.d2;

	result.localMatrix.m[2][0] = aiLocalMatrix.a3; result.localMatrix.m[2][1] = aiLocalMatrix.b3;
	result.localMatrix.m[2][2] = aiLocalMatrix.c3; result.localMatrix.m[2][3] = aiLocalMatrix.d3;

	result.localMatrix.m[3][0] = aiLocalMatrix.a4; result.localMatrix.m[3][1] = aiLocalMatrix.b4;
	result.localMatrix.m[3][2] = aiLocalMatrix.c4; result.localMatrix.m[3][3] = aiLocalMatrix.d4;

	//																			//
	//								Nodeの名前を取得							//
	//																			//

	// Node名を設定
	result.name = node->mName.C_Str();

	//																			//
	//							子Nodeを再帰的に読み込む						//
	//																			//

	// 子供のNodeを再帰的に読み込む
	result.children.resize(node->mNumChildren);
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		// 再帰的にReadNodeを呼び出して子供のNodeを作成
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}

	return result;
}