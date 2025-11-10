#pragma once
//ファイルに書いたり読んだりするライブラリ
#include<fstream>
#include"MyMath.h"


/// <summary>
/// 頂点データ
/// </summary>
struct VertexData final {
	Vector4 position;//座標
	Vector2 texcoord;//UV座標系(テクスチャ座標系)
	Vector3 normal;	//法線
};

/// <summary>
/// mtlファイルを読んで使えるようにする
/// </summary>
struct MaterialDataModel {
	std::string textureFilePath;	//テクスチャファイルのパス
};

/// <summary>
/// モデルデータ
/// </summary>
struct ModelData {
	std::vector<VertexData> vertices;
	MaterialDataModel material;
	std::string materialName = "";	// マテリアル名
	size_t materialIndex = 0;		// マテリアルインデックス
};

/// <summary>
/// カメラ情報（GPU用）
/// </summary>
struct CameraForGPU final {
	Vector3 worldPosition;	// カメラのワールド座標
	float padding;			// 隙間
};

/// <summary>
/// マテリアル
/// </summary>
struct  MaterialData final {
	Vector4 color;				//色
	int32_t enableLighting;		//ライティングするか
	int32_t lightingMode;		//ライティングモード（0:None, 1:Lambert, 2:HalfLambert, 3:PhongSpecular）
	float shininess;			//光沢度（鏡面反射の鋭さ）
	float padding;				//隙間埋める
	Matrix4x4 uvTransform;
};