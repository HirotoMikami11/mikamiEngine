#define NOMINMAX
#include "DebugDrawLineSystem.h"
#include "ImGui/ImGuiManager.h"
#include "Logger.h"
#include <numbers>
#include <cmath>

DebugDrawLineSystem* DebugDrawLineSystem::GetInstance()
{
	static DebugDrawLineSystem instance;
	return &instance;
}

void DebugDrawLineSystem::Initialize(DirectXCommon* dxCommon)
{
	if (isInitialized_) {
		Logger::Log(Logger::GetStream(), "DebugDrawLineSystem: Already initialized!\n");
		return;
	}

	dxCommon_ = dxCommon;

	// LineRendererを取得
	lineRenderer_ = std::make_unique<LineRenderer>();
	lineRenderer_->Initialize(dxCommon);

	isInitialized_ = true;
	isUse_ = false;
#ifdef USEIMGUI
	isUse_ = true;
#endif

	Logger::Log(Logger::GetStream(), "DebugDrawLineSystem: Initialized successfully!!\n");
}

void DebugDrawLineSystem::Reset()
{
	if (!isInitialized_ || !lineRenderer_) {
		return;
	}

	// LineRendererの線分をクリア
	lineRenderer_->Reset();

}

void DebugDrawLineSystem::Draw(const Matrix4x4& viewProjectionMatrix)
{
	if (!isInitialized_ || !lineRenderer_ || !isUse_) {
		return;
	}

	// LineRendererで一括描画
	lineRenderer_->Draw(viewProjectionMatrix);
}

void DebugDrawLineSystem::Finalize()
{
	// LineRendererはシングルトンなので解放しない
	lineRenderer_ = nullptr;
	dxCommon_ = nullptr;
	isInitialized_ = false;

	Logger::Log(Logger::GetStream(), "DebugDrawLineSystem: Finalized.\n");
}

void DebugDrawLineSystem::AddLine(const Vector3& start, const Vector3& end, const Vector4& color)
{
	if (!isInitialized_ || !isUse_ || !lineRenderer_) {
		return;
	}

	lineRenderer_->AddLine(start, end, color);
}

void DebugDrawLineSystem::AddLine(const Vector3& start, const Vector3& end, const uint32_t& color)
{
	if (!isInitialized_ || !isUse_ || !lineRenderer_) {
		return;
	}

	lineRenderer_->AddLine(start, end, Uint32ToColorVector(color));
}

void DebugDrawLineSystem::DrawAABB(const AABB& aabb, const Vector4& color)
{
	if (!isInitialized_ || !isUse_) {
		return;
	}

	// AABBの8頂点を計算
	Vector3 vertices[8];
	CalculateAABBVertices(aabb, vertices);

	// 底面の4本の線
	AddLine(vertices[0], vertices[1], color);
	AddLine(vertices[1], vertices[2], color);
	AddLine(vertices[2], vertices[3], color);
	AddLine(vertices[3], vertices[0], color);

	// 上面の4本の線
	AddLine(vertices[4], vertices[5], color);
	AddLine(vertices[5], vertices[6], color);
	AddLine(vertices[6], vertices[7], color);
	AddLine(vertices[7], vertices[4], color);

	// 縦の4本の線
	AddLine(vertices[0], vertices[4], color);
	AddLine(vertices[1], vertices[5], color);
	AddLine(vertices[2], vertices[6], color);
	AddLine(vertices[3], vertices[7], color);
}

void DebugDrawLineSystem::DrawAABB(const AABB& aabb, const uint32_t& color)
{
	DrawAABB(aabb, Uint32ToColorVector(color));
}

void DebugDrawLineSystem::DrawSphere(const Vector3& center, float radius, const Vector4& color, uint32_t subdivision)
{
	if (!isInitialized_ || !isUse_) {
		return;
	}

	const float pi = std::numbers::pi_v<float>;

	// 経度分割1つ分の角度
	const float lonEvery = (2.0f * pi) / static_cast<float>(subdivision);
	// 緯度分割1つ分の角度
	const float latEvery = pi / static_cast<float>(subdivision);

	// 緯度の方向に分割 -π/2 ~ π/2
	for (uint32_t latIndex = 0; latIndex < subdivision; ++latIndex) {
		float lat = (-pi / 2.0f) + latEvery * latIndex;

		// 経度の方向に分割 0 ~ 2π
		for (uint32_t lonIndex = 0; lonIndex < subdivision; ++lonIndex) {
			float lon = lonIndex * lonEvery;

			// world座標系でのa,b,cを求める
			Vector3 a = {
				std::cos(lat) * std::cos(lon),
				std::sin(lat),
				std::cos(lat) * std::sin(lon)
			};
			Vector3 b = {
				std::cos(lat + latEvery) * std::cos(lon),
				std::sin(lat + latEvery),
				std::cos(lat + latEvery) * std::sin(lon)
			};
			Vector3 c = {
				std::cos(lat) * std::cos(lon + lonEvery),
				std::sin(lat),
				std::cos(lat) * std::sin(lon + lonEvery)
			};

			// 球体の中心座標と半径を加味して、a,b,cをワールド座標に変換
			a = Add(center, Multiply(a, radius));
			b = Add(center, Multiply(b, radius));
			c = Add(center, Multiply(c, radius));

			// ab, acで線を引く
			AddLine(a, b, color);
			AddLine(a, c, color);
		}
	}
}

void DebugDrawLineSystem::DrawSphere(const Vector3& center, float radius, const uint32_t& color, uint32_t subdivision)
{
	DrawSphere(center, radius, Uint32ToColorVector(color), subdivision);
}

void DebugDrawLineSystem::DrawCross(const Vector3& position, float size, const Vector4& color)
{
	if (!isInitialized_ || !isUse_) {
		return;
	}

	float halfSize = size * 0.5f;

	// X軸方向
	AddLine(
		{ position.x - halfSize, position.y, position.z },
		{ position.x + halfSize, position.y, position.z },
		color
	);

	// Y軸方向
	AddLine(
		{ position.x, position.y - halfSize, position.z },
		{ position.x, position.y + halfSize, position.z },
		color
	);

	// Z軸方向
	AddLine(
		{ position.x, position.y, position.z - halfSize },
		{ position.x, position.y, position.z + halfSize },
		color
	);
}

void DebugDrawLineSystem::DrawCone(
	const Vector3& apex,
	const Vector3& direction,
	float height,
	float angle,
	const Vector4& color,
	uint32_t subdivision)
{
	if (!isInitialized_ || !isUse_) {
		return;
	}

	// 円錐の底面の半径を計算
	float radius = height * std::tan(angle);

	// 方向ベクトルに垂直な2つのベクトルを計算（円錐の底面を形成）
	Vector3 tangent, bitangent;
	if (std::abs(direction.y) < 0.999f) {
		// Y軸との外積で垂直ベクトルを生成
		tangent = Normalize(Cross({ 0.0f, 1.0f, 0.0f }, direction));
	} else {
		// directionがY軸に近い場合はX軸を使用
		tangent = Normalize(Cross({ 1.0f, 0.0f, 0.0f }, direction));
	}
	bitangent = Normalize(Cross(direction, tangent));

	// 底面の中心位置を計算
	Vector3 baseCenter = Add(apex, Multiply(direction, height));

	const float pi = std::numbers::pi_v<float>;
	const float angleStep = (2.0f * pi) / static_cast<float>(subdivision);

	// 底面の円周上の点を計算
	std::vector<Vector3> circlePoints;
	circlePoints.reserve(subdivision);

	for (uint32_t i = 0; i < subdivision; ++i) {
		float theta = angleStep * i;
		float cosTheta = std::cos(theta);
		float sinTheta = std::sin(theta);

		// 円周上の点を計算
		Vector3 offset = Add(
			Multiply(tangent, radius * cosTheta),
			Multiply(bitangent, radius * sinTheta)
		);
		Vector3 point = Add(baseCenter, offset);
		circlePoints.push_back(point);

		// 底面の円周を描画
		if (i > 0) {
			AddLine(circlePoints[i - 1], point, color);
		}
	}

	// 最後の点と最初の点を接続して円周を閉じる
	if (!circlePoints.empty()) {
		AddLine(circlePoints.back(), circlePoints.front(), color);
	}

	// 頂点から底面の4点（上下左右）への線を描画
	if (subdivision >= 4) {
		// 上下左右のインデックスを計算（円周を4分割）
		uint32_t quarterStep = subdivision / 4;

		// 右側の点（0度方向）
		AddLine(apex, circlePoints[0], color);

		// 上側の点（90度方向）
		AddLine(apex, circlePoints[quarterStep], color);

		// 左側の点（180度方向）
		AddLine(apex, circlePoints[quarterStep * 2], color);

		// 下側の点（270度方向）
		AddLine(apex, circlePoints[quarterStep * 3], color);
	}
}

void DebugDrawLineSystem::DrawCone(const Vector3& apex, const Vector3& direction, float height, float angle, const uint32_t& color, uint32_t subdivision)
{
	DrawCone(apex, direction, height, angle, Uint32ToColorVector(color), subdivision);
}

void DebugDrawLineSystem::DrawRectangle(
	const Vector3& center,
	const Vector3& normal,
	const Vector3& tangent,
	const Vector3& bitangent,
	float width,
	float height,
	const Vector4& color)
{
	if (!isInitialized_ || !isUse_) {
		return;
	}

	float halfWidth = width * 0.5f;
	float halfHeight = height * 0.5f;

	// 矩形の4つの頂点を計算
	Vector3 vertices[4];

	// 右上
	vertices[0] = Add(center, Add(
		Multiply(tangent, halfWidth),
		Multiply(bitangent, halfHeight)
	));

	// 右下
	vertices[1] = Add(center, Add(
		Multiply(tangent, halfWidth),
		Multiply(bitangent, -halfHeight)
	));

	// 左下
	vertices[2] = Add(center, Add(
		Multiply(tangent, -halfWidth),
		Multiply(bitangent, -halfHeight)
	));

	// 左上
	vertices[3] = Add(center, Add(
		Multiply(tangent, -halfWidth),
		Multiply(bitangent, halfHeight)
	));

	// 矩形の4辺を描画
	AddLine(vertices[0], vertices[1], color);  // 右辺
	AddLine(vertices[1], vertices[2], color);  // 下辺
	AddLine(vertices[2], vertices[3], color);  // 左辺
	AddLine(vertices[3], vertices[0], color);  // 上辺

	// 中心から法線方向への線を描画（方向を表示）
	Vector3 directionEnd = Add(center, normal);
	AddLine(center, directionEnd, color);
}

void DebugDrawLineSystem::DrawRectangle(const Vector3& center,const Vector3& normal,const Vector3& tangent,const Vector3& bitangent,float width,	float height,const uint32_t& color){
	DrawRectangle(center, normal, tangent, bitangent, width, height, Uint32ToColorVector(color));
}


void DebugDrawLineSystem::ConfigureGrid(
	const GridLineType& gridType,
	float size,
	float interval,
	float majorInterval)
{
	gridType_ = gridType;
	gridSize_ = size;
	gridInterval_ = interval;
	gridMajorInterval_ = majorInterval;
}

void DebugDrawLineSystem::SetGridColors(const Vector4& normalColor, const Vector4& majorColor)
{
	gridNormalColor_ = normalColor;
	gridMajorColor_ = majorColor;
}

uint32_t DebugDrawLineSystem::GetLineCount() const
{
	if (!lineRenderer_) {
		return 0;
	}
	return lineRenderer_->GetLineCount();
}

bool DebugDrawLineSystem::IsFull() const
{
	if (!lineRenderer_) {
		return true;
	}
	return lineRenderer_->IsFull();
}

void DebugDrawLineSystem::CalculateAABBVertices(const AABB& aabb, Vector3 vertices[8]) const
{
	// AABBの8頂点を計算
	vertices[0] = { aabb.min.x, aabb.min.y, aabb.min.z };	// 0: 左下手前
	vertices[1] = { aabb.max.x, aabb.min.y, aabb.min.z };	// 1: 右下手前
	vertices[2] = { aabb.max.x, aabb.min.y, aabb.max.z };	// 2: 右下奥
	vertices[3] = { aabb.min.x, aabb.min.y, aabb.max.z };	// 3: 左下奥
	vertices[4] = { aabb.min.x, aabb.max.y, aabb.min.z };	// 4: 左上手前
	vertices[5] = { aabb.max.x, aabb.max.y, aabb.min.z };	// 5: 右上手前
	vertices[6] = { aabb.max.x, aabb.max.y, aabb.max.z };	// 6: 右上奥
	vertices[7] = { aabb.min.x, aabb.max.y, aabb.max.z };	// 7: 左上奥
}




void DebugDrawLineSystem::GenerateGridLines()
{
	// isUse_とisGridVisible_が有効でない場合は描画しない
	if (!isUse_ || !isGridVisible_ || !lineRenderer_) {
		return;
	}

	// グリッドタイプに応じて描画
	switch (gridType_) {
	case GridLineType::XZ:
		DrawXZGrid();
		break;
	case GridLineType::XY:
		DrawXYGrid();
		break;
	case GridLineType::YZ:
		DrawYZGrid();
		break;
	default:
		Logger::Log(Logger::GetStream(), "DebugDrawLineSystem: Unknown GridLineType specified.\n");
		return;
	}
}

void DebugDrawLineSystem::DrawXZGrid()
{
	float halfSize = gridSize_ * 0.5f;

	// X方向の線（Z軸に沿って）
	for (float x = -halfSize; x <= halfSize; x += gridInterval_) {
		Vector4 color;
		if (std::abs(x) < 0.001f) {
			// 原点のZ軸は青色
			color = { 0.0f, 0.0f, 1.0f, 1.0f };
		} else if (std::fmod(std::abs(x), gridMajorInterval_) < 0.001f) {
			color = gridMajorColor_;
		} else {
			color = gridNormalColor_;
		}

		Vector3 start = { gridCenter_.x + x, gridCenter_.y, gridCenter_.z - halfSize };
		Vector3 end = { gridCenter_.x + x, gridCenter_.y, gridCenter_.z + halfSize };
		lineRenderer_->AddLine(start, end, color);
	}

	// Z方向の線（X軸に沿って）
	for (float z = -halfSize; z <= halfSize; z += gridInterval_) {
		Vector4 color;
		if (std::abs(z) < 0.001f) {
			// 原点のX軸は赤色
			color = { 1.0f, 0.0f, 0.0f, 1.0f };
		} else if (std::fmod(std::abs(z), gridMajorInterval_) < 0.001f) {
			color = gridMajorColor_;
		} else {
			color = gridNormalColor_;
		}

		Vector3 start = { gridCenter_.x - halfSize, gridCenter_.y, gridCenter_.z + z };
		Vector3 end = { gridCenter_.x + halfSize, gridCenter_.y, gridCenter_.z + z };
		lineRenderer_->AddLine(start, end, color);
	}
}

void DebugDrawLineSystem::DrawXYGrid()
{
	float halfSize = gridSize_ * 0.5f;

	// X方向の線（Y軸に沿って）
	for (float x = -halfSize; x <= halfSize; x += gridInterval_) {
		Vector4 color;
		if (std::abs(x) < 0.001f) {
			// 原点のY軸は緑色
			color = { 0.0f, 1.0f, 0.0f, 1.0f };
		} else if (std::fmod(std::abs(x), gridMajorInterval_) < 0.001f) {
			color = gridMajorColor_;
		} else {
			color = gridNormalColor_;
		}

		Vector3 start = { gridCenter_.x + x, gridCenter_.y - halfSize, gridCenter_.z };
		Vector3 end = { gridCenter_.x + x, gridCenter_.y + halfSize, gridCenter_.z };
		lineRenderer_->AddLine(start, end, color);
	}

	// Y方向の線（X軸に沿って）
	for (float y = -halfSize; y <= halfSize; y += gridInterval_) {
		Vector4 color;
		if (std::abs(y) < 0.001f) {
			// 原点のX軸は赤色
			color = { 1.0f, 0.0f, 0.0f, 1.0f };
		} else if (std::fmod(std::abs(y), gridMajorInterval_) < 0.001f) {
			color = gridMajorColor_;
		} else {
			color = gridNormalColor_;
		}

		Vector3 start = { gridCenter_.x - halfSize, gridCenter_.y + y, gridCenter_.z };
		Vector3 end = { gridCenter_.x + halfSize, gridCenter_.y + y, gridCenter_.z };
		lineRenderer_->AddLine(start, end, color);
	}
}

void DebugDrawLineSystem::DrawYZGrid()
{
	float halfSize = gridSize_ * 0.5f;

	// Y方向の線（Z軸に沿って）X=0
	for (float y = -halfSize; y <= halfSize; y += gridInterval_) {
		Vector4 color;
		if (std::abs(y) < 0.001f) {
			// 原点のZ軸は青色
			color = { 0.0f, 0.0f, 1.0f, 1.0f };
		} else if (std::fmod(std::abs(y), gridMajorInterval_) < 0.001f) {
			color = gridMajorColor_;
		} else {
			color = gridNormalColor_;
		}

		Vector3 start = { gridCenter_.x, gridCenter_.y + y, gridCenter_.z - halfSize };
		Vector3 end = { gridCenter_.x, gridCenter_.y + y, gridCenter_.z + halfSize };
		lineRenderer_->AddLine(start, end, color);
	}

	// Z方向の線（Y軸に沿って）X=0
	for (float z = -halfSize; z <= halfSize; z += gridInterval_) {
		Vector4 color;
		if (std::abs(z) < 0.001f) {
			// 原点のY軸は緑色
			color = { 0.0f, 1.0f, 0.0f, 1.0f };
		} else if (std::fmod(std::abs(z), gridMajorInterval_) < 0.001f) {
			color = gridMajorColor_;
		} else {
			color = gridNormalColor_;
		}

		Vector3 start = { gridCenter_.x, gridCenter_.y - halfSize, gridCenter_.z + z };
		Vector3 end = { gridCenter_.x, gridCenter_.y + halfSize, gridCenter_.z + z };
		lineRenderer_->AddLine(start, end, color);
	}
}

void DebugDrawLineSystem::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::CollapsingHeader("DebugDrawLine System")) {
		ImGui::Separator();

		ImGui::Text("Status: %s", isInitialized_ ? "Initialized" : "Not Initialized");
		ImGui::Checkbox("USE", &isUse_);

		// グリッド設定
		if (ImGui::TreeNode("Grid Settings")) {
			ImGui::Checkbox("Grid Visible", &isGridVisible_);

			// グリッドタイプ選択
			const char* gridTypes[] = { "XZ Plane", "XY Plane", "YZ Plane" };
			int currentType = static_cast<int>(gridType_);
			if (ImGui::Combo("Grid Type", &currentType, gridTypes, 3)) {
				gridType_ = static_cast<GridLineType>(currentType);
			}

			ImGui::DragFloat("Grid Size", &gridSize_, 1.0f, 10.0f, 500.0f);
			ImGui::DragFloat("Grid Interval", &gridInterval_, 0.1f, 0.1f, 10.0f);
			ImGui::DragFloat("Major Interval", &gridMajorInterval_, 1.0f, 2.0f, 100.0f);
			ImGui::DragFloat3("Grid Center", &gridCenter_.x, 0.1f);

			ImGui::ColorEdit4("Normal Color", &gridNormalColor_.x);
			ImGui::ColorEdit4("Major Color", &gridMajorColor_.x);

			ImGui::TreePop();
		}

		// LineRendererの詳細情報
		if (lineRenderer_) {
			lineRenderer_->ImGui();
		}
	}
#endif
}