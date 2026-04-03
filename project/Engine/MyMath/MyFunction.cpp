#define NOMINMAX
#include "MyFunction.h"
#include<cassert>
#include <numbers>
#include<algorithm>
#pragma comment(lib,"d3d12.lib")
/*-----------------------------------------------------------------------*/
//
//								計算関数
//
/*-----------------------------------------------------------------------*/


Microsoft::WRL::ComPtr <ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr <ID3D12Device> device, size_t sizeInBytes)
{
	//リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//UploadHeapを使う
	//リソースの設定
	D3D12_RESOURCE_DESC ResourceDesc{};
	//バッファリソース。テクスチャの場合はまた別の設定をする
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResourceDesc.Width = sizeInBytes;//リソースサイズ
	//バッファの場合はこれらは1にする決まり
	ResourceDesc.Height = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//実際にリソースを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&Resource)
	);
	assert(SUCCEEDED(hr));


	return Resource;
}

//	正射影ベクトルを求める関数
Vector3 Project(const Vector3& v1, const Vector3& v2) {
	Vector3 project = Multiply(Normalize(v2), Dot(v1, Normalize(v2)));
	return project;
}

//　最近接点を求める関数
Vector3 ClosestPoint(const Vector3& point, const Segment& segment) {
	Vector3 project = Project(Subtract(point, segment.origin), segment.diff);
	Vector3 closestPoint = Add(segment.origin, project);

	return closestPoint;
}

void UpdateMatrix4x4(const Vector3Transform transform, const Matrix4x4 viewProjectionMatrix, TransformationMatrix* matrixData) {
	matrixData->World = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	Matrix4x4 worldViewProjectionMatrix = Matrix4x4Multiply(matrixData->World, viewProjectionMatrix);

	matrixData->WVP = worldViewProjectionMatrix;

}



Vector3 CatmullRomPosition(const std::vector< Vector3>& points, float t) {

	// Catmull-Romスプラインには少なくとも4つの点が必要
	assert(points.size() >= 4 && "制禦点は4点以上必要です");

	// 区間数は制御店の飾宇-1
	size_t division = points.size() - 1;
	// 1区間の長さ(全体を1.0賭した時の割合)
	float areaWidth = 1.0f / division;

	// 区間番号
	size_t index = static_cast<size_t>(t / areaWidth);
	// 区間番号が上限を超えないようにする
	index = std::clamp(index, static_cast<size_t>(0), division - 1);

	// 区間内の支店を0.0f、終点を1.0fとする時の現在位置
	float t_2 = (t - areaWidth * index) / areaWidth;
	t_2 = std::clamp(t_2, 0.0f, 1.0f);


	// ４店分のインデックス
	size_t index0 = index - 1;
	size_t index1 = index;
	size_t index2 = index + 1;
	size_t index3 = index + 2;

	// 最初と最後
	if (index == 0) {
		index0 = index1;
	}

	// 最後
	if (index3 >= points.size()) {
		index3 = index2;
	}

	// 4点の座標
	const Vector3 p0 = points[index0];
	const Vector3 p1 = points[index1];
	const Vector3 p2 = points[index2];
	const Vector3 p3 = points[index3];

	// Catmull-Romスプライン曲線の補間を行う
	return CatmullRomInterpolation(p0, p1, p2, p3, t_2);
}


///始点と終点がそれぞれ前後の点と繋がらない標準的な曲線
Vector3 DefaultCatmullRomPosition(const std::vector<Vector3>& points, float t) {
	if (points.size() < 4) {
		return Vector3{ 0.0f, 0.0f, 0.0f };
	}

	// 全体のセグメント数
	int numSegments = static_cast<int>(points.size()) - 3;

	// tからセグメントインデックスとローカルtを計算
	float scaledT = t * numSegments;
	int segmentIndex = static_cast<int>(scaledT);
	float localT = scaledT - segmentIndex;

	// 最後のセグメントの場合の調整
	if (segmentIndex >= numSegments) {
		segmentIndex = numSegments - 1;
		localT = 1.0f;
	}

	// 4つの制御点を取得
	const Vector3& p0 = points[segmentIndex];
	const Vector3& p1 = points[segmentIndex + 1];
	const Vector3& p2 = points[segmentIndex + 2];
	const Vector3& p3 = points[segmentIndex + 3];

	// CatmullRom補間の計算
	float t2 = localT * localT;
	float t3 = t2 * localT;

	Vector3 result;
	result.x = 0.5f * ((2.0f * p1.x) +
		(-p0.x + p2.x) * localT +
		(2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 +
		(-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3);

	result.y = 0.5f * ((2.0f * p1.y) +
		(-p0.y + p2.y) * localT +
		(2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t2 +
		(-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t3);

	result.z = 0.5f * ((2.0f * p1.z) +
		(-p0.z + p2.z) * localT +
		(2.0f * p0.z - 5.0f * p1.z + 4.0f * p2.z - p3.z) * t2 +
		(-p0.z + 3.0f * p1.z - 3.0f * p2.z + p3.z) * t3);

	return result;
}


/// CatmullRomスプライン曲線の補間
Vector3 CatmullRomInterpolation(const  Vector3& p0, const  Vector3& p1, const  Vector3& p2, const  Vector3& p3, float t) {

	const float s = 0.5f; // 1/2
	float t2 = t * t;     // tの二乗
	float t3 = t2 * t;    // tの三乗

	Vector3 e3 = -p0 + 3 * p1 - 3 * p2 + p3;
	Vector3 e2 = 2 * p0 - 5 * p1 + 4 * p2 - p3;
	Vector3 e1 = -p0 + p2;
	Vector3 e0 = 2 * p1;

	return s * (e3 * t3 + e2 * t2 + e1 * t + e0);
}

/*-----------------------------------------------------------------------*/
///								座標関数
/*-----------------------------------------------------------------------*/


Vector3 ConvertWorldToScreenPosition(const Vector3& worldPosition, const Matrix4x4& viewProjectionMatrix) {
	// ビューポート行列を作成
	Matrix4x4 matViewport = MakeViewportMatrix(0, 0, GraphicsConfig::kClientWidth, GraphicsConfig::kClientHeight, 0, 1);
	// ビュープロジェクション行列を作成
	Matrix4x4 matViewProjectionViewport = Matrix4x4Multiply(viewProjectionMatrix, matViewport);

	// ワールドからスクリーンに座標変換(ここから3Dから2Dになる)
	Vector3 screenPosition = Transform(worldPosition, matViewProjectionViewport);
	return { screenPosition.x, screenPosition.y, screenPosition.z }; // 2D座標に変換して返す
}

/*-----------------------------------------------------------------------*/
//
//  当たり判定の補助関数
//
/*-----------------------------------------------------------------------*/

// 線分と平面の衝突点の座標を求める
Vector3 MakeCollisionPoint(const Segment& segment, const PlaneMath& PlaneMath) {
	///衝突点
	Vector3 CollsionPoint;

	//平面と線の衝突判定と同様
	float dot = Dot(segment.diff, PlaneMath.normal);
	assert(dot != 0.0f);
	float t = (PlaneMath.distance - (Dot(segment.origin, PlaneMath.normal))) / dot;

	//衝突点を求める
	//p=origin+tb
	CollsionPoint = Add(segment.origin, Multiply(segment.diff, t));
	return CollsionPoint;
}

/// <summary>
/// 最大最小を正しくする関数
/// </summary>
/// <param name="aabb"></param>
void FixAABBMinMax(AABB& aabb) {

	aabb.min.x = (std::min)(aabb.min.x, aabb.max.x);
	aabb.max.x = (std::max)(aabb.min.x, aabb.max.x);

	aabb.min.y = (std::min)(aabb.min.y, aabb.max.y);
	aabb.max.y = (std::max)(aabb.min.y, aabb.max.y);

	aabb.min.z = (std::min)(aabb.min.z, aabb.max.z);
	aabb.max.z = (std::max)(aabb.min.z, aabb.max.z);
}