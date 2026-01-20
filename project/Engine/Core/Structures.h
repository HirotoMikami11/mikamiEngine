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
/// Node構造体（glTFの階層構造をサポート）
/// </summary>
struct Node {
	Matrix4x4 localMatrix;			// このNodeのローカル変換行列（Transform）
	std::string name;				// Nodeの名前
	std::vector<Node> children;		// 子供のNode
};

/// <summary>
/// モデルデータ
/// </summary>
struct ModelData {
	std::vector<VertexData> vertices;
	MaterialDataModel material;
	std::string materialName = "";	// マテリアル名
	size_t materialIndex = 0;		// マテリアルインデックス
	Node rootNode;					// ルートNode（glTF対応）
};

/// <summary>
/// カメラ情報（GPU用）
/// </summary>
struct CameraForGPU final {
	Vector3 worldPosition;	// カメラのワールド座標
	float padding;			// 隙間
};

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

/// <summary>
/// 平行光源（GPU用）
/// </summary>
struct DirectionalLightData final {
	Vector4 color;		// 色
	Vector3 direction;	// 方向
	float intensity;	// 強度
};

/// <summary>
/// ポイントライト（GPU用）
/// </summary>
struct PointLightData final {
	Vector4 color;		// 色
	Vector3 position;	// 位置
	float intensity;	// 強度
	float radius;		// 影響範囲
	float decay;		// 減衰率
	float padding[2];	// アライメント調整
};

/// <summary>
/// スポットライト（GPU用）
/// </summary>
struct SpotLightData final {
	Vector4 color;			// 色
	Vector3 position;		// ライトの位置
	float intensity;		// 輝度
	Vector3 direction;		// 方向（正規化済み）
	float distance;			// ライトの届く最大距離
	float decay;			// 減衰率
	float cosAngle;			// スポットライトの余弦（外側の角度）
	float cosFalloffStart;	// フォールオフ開始の余弦（内側の角度）
	float padding;			// アライメント調整
};

/// <summary>
/// エリアライト（矩形ライト）（GPU用）
/// </summary>
struct RectLightData final {
	Vector4 color;			// 色
	Vector3 position;		// 矩形の中心位置
	float intensity;		// 輝度
	Vector3 normal;			// 矩形の法線（面の向き）
	float width;			// 矩形の幅
	Vector3 tangent;		// 矩形のローカルX軸（横方向）
	float height;			// 矩形の高さ
	Vector3 bitangent;		// 矩形のローカルY軸（縦方向）
	float decay;			// 減衰率
};

/// <summary>
/// 統合ライティングデータ（GPU用）
/// 全てのライト情報を1つのConstantBufferにまとめる
/// </summary>
struct LightingData final {
	DirectionalLightData directionalLight;	// 平行光源
	PointLightData pointLights[32];			// ポイントライト（最大32個）
	int32_t numPointLights;					// 有効なポイントライト数
	float padding1[3];						// アライメント調整
	SpotLightData spotLights[16];			// スポットライト（最大16個）
	int32_t numSpotLights;					// 有効なスポットライト数
	float padding2[3];						// アライメント調整
	RectLightData rectLights[8];			// エリアライト（最大8個）
	int32_t numRectLights;					// 有効なエリアライト数
	float padding3[3];						// アライメント調整
};