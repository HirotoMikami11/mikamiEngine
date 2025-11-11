#include "DebugDrawLineSystem.h"
#include "Managers/ImGui/ImGuiManager.h"
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
		Logger::Log(Logger::GetStream(), "DebugDrawLineSystem initialized!\n");
		return;
	}

	directXCommon_ = dxCommon;

	// LineRendererを作成・初期化
	lineRenderer_ = std::make_unique<LineRenderer>();
	lineRenderer_->Initialize(dxCommon);

	isInitialized_ = true;
	Logger::Log(Logger::GetStream(), "DebugDrawLineSystem successfully!!\n");
}

void DebugDrawLineSystem::Reset()
{
	if (!isInitialized_ || !lineRenderer_) {
		return;
	}

	// 全ての線分をクリア
	lineRenderer_->Reset();
}

void DebugDrawLineSystem::Draw(const Matrix4x4& viewProjectionMatrix)
{
	if (!isInitialized_ || !lineRenderer_) {
		return;
	}

	// LineRendererで一括描画
	lineRenderer_->Draw(viewProjectionMatrix);
}

void DebugDrawLineSystem::Finalize()
{
	if (lineRenderer_) {
		lineRenderer_.reset();
	}

	directXCommon_ = nullptr;
	isInitialized_ = false;

	Logger::Log(Logger::GetStream(), "DebugDrawLineSystem: Finalized.\n");
}

void DebugDrawLineSystem::AddLine(const Vector3& start, const Vector3& end, const Vector4& color)
{
	if (!isInitialized_ || !lineRenderer_) {
		return;
	}

	lineRenderer_->AddLine(start, end, color);
}

void DebugDrawLineSystem::DrawAABB(const AABB& aabb, const Vector4& color)
{
	if (!isInitialized_ ) {
		return;
	}

	// AABBの8頂点を計算
	Vector3 vertices[8];
	CalculateAABBVertices(aabb, vertices);

	// 底面の4本の線
	AddLine(vertices[0], vertices[1], color);	// 左下手前 → 右下手前
	AddLine(vertices[1], vertices[2], color);	// 右下手前 → 右下奥
	AddLine(vertices[2], vertices[3], color);	// 右下奥 → 左下奥
	AddLine(vertices[3], vertices[0], color);	// 左下奥 → 左下手前

	// 上面の4本の線
	AddLine(vertices[4], vertices[5], color);	// 左上手前 → 右上手前
	AddLine(vertices[5], vertices[6], color);	// 右上手前 → 右上奥
	AddLine(vertices[6], vertices[7], color);	// 右上奥 → 左上奥
	AddLine(vertices[7], vertices[4], color);	// 左上奥 → 左上手前

	// 縦の4本の線
	AddLine(vertices[0], vertices[4], color);	// 左下手前 → 左上手前
	AddLine(vertices[1], vertices[5], color);	// 右下手前 → 右上手前
	AddLine(vertices[2], vertices[6], color);	// 右下奥 → 右上奥
	AddLine(vertices[3], vertices[7], color);	// 左下奥 → 左上奥
}

void DebugDrawLineSystem::DrawSphere(const Vector3& center, float radius, const Vector4& color, uint32_t subdivision)
{
	if (!isInitialized_ || subdivision < 3) {
		return;
	}

	const float pi = std::numbers::pi_v<float>;

	// 経度分割1つ分の角度
	const float lonEvery = (2.0f * pi) / static_cast<float>(subdivision);
	// 緯度分割1つ分の角度
	const float latEvery = pi / static_cast<float>(subdivision);

	// 緯度の方向に分割 -π/2 ~ π/2
	for (uint32_t latIndex = 0; latIndex < subdivision; ++latIndex) {
		float lat = (-pi / 2.0f) + latEvery * latIndex;  // 現在の緯度(θ)

		// 経度の方向に分割 0 ~ 2π
		for (uint32_t lonIndex = 0; lonIndex < subdivision; ++lonIndex) {
			float lon = lonIndex * lonEvery;  // 現在の経度(φ)

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

void DebugDrawLineSystem::DrawGrid(
	const Vector3& center,
	float size,
	float interval,
	float majorInterval,
	const Vector4& normalColor,
	const Vector4& majorColor)
{
	if (!isInitialized_ ) {
		return;
	}

	float halfSize = size * 0.5f;

	// X方向の線（Z軸に沿って）
	for (float x = -halfSize; x <= halfSize; x += interval) {
		Vector4 color;
		if (std::abs(x) < 0.001f) {
			// Z軸は青色
			color = { 0.0f, 0.0f, 1.0f, 1.0f };
		} else if (std::fmod(std::abs(x), majorInterval) < 0.001f) {
			color = majorColor;
		} else {
			color = normalColor;
		}

		Vector3 start = { center.x + x, center.y, center.z - halfSize };
		Vector3 end = { center.x + x, center.y, center.z + halfSize };
		AddLine(start, end, color);
	}

	// Z方向の線（X軸に沿って）
	for (float z = -halfSize; z <= halfSize; z += interval) {
		Vector4 color;
		if (std::abs(z) < 0.001f) {
			// X軸は赤色
			color = { 1.0f, 0.0f, 0.0f, 1.0f };
		} else if (std::fmod(std::abs(z), majorInterval) < 0.001f) {
			color = majorColor;
		} else {
			color = normalColor;
		}

		Vector3 start = { center.x - halfSize, center.y, center.z + z };
		Vector3 end = { center.x + halfSize, center.y, center.z + z };
		AddLine(start, end, color);
	}
}

void DebugDrawLineSystem::DrawCross(const Vector3& position, float size, const Vector4& color)
{
	if (!isInitialized_) {
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

void DebugDrawLineSystem::DrawCapsule(const Vector3& start, const Vector3& end, float radius, const Vector4& color, int segments)
{
	if (!isInitialized_ || segments < 3) {
		return;
	}

	const float pi = std::numbers::pi_v<float>;

	// カプセルの中心軸
	Vector3 axis = Subtract(end, start);
	float height = Length(axis);

	if (height < 0.0001f) {
		// 高さが0の場合は球体として描画
		DrawSphere(start, radius, color, segments);
		return;
	}

	axis = Normalize(axis);

	// 軸に垂直な2つのベクトルを作成
	Vector3 perpendicular1, perpendicular2;
	if (std::abs(axis.y) < 0.9f) {
		perpendicular1 = Normalize(Cross({ 0.0f, 1.0f, 0.0f }, axis));
	} else {
		perpendicular1 = Normalize(Cross({ 1.0f, 0.0f, 0.0f }, axis));
	}
	perpendicular2 = Normalize(Cross(axis, perpendicular1));

	const float angleStep = (2.0f * pi) / static_cast<float>(segments);

	// 円柱部分（側面の線）
	for (int i = 0; i < segments; ++i) {
		float angle = angleStep * i;

		Vector3 offset = {
			radius * (std::cos(angle) * perpendicular1.x + std::sin(angle) * perpendicular2.x),
			radius * (std::cos(angle) * perpendicular1.y + std::sin(angle) * perpendicular2.y),
			radius * (std::cos(angle) * perpendicular1.z + std::sin(angle) * perpendicular2.z)
		};

		Vector3 p1 = Add(start, offset);
		Vector3 p2 = Add(end, offset);
		AddLine(p1, p2, color);
	}

	// 両端の半球
	const int hemisphereSegments = segments / 2;
	const float latStep = (pi / 2.0f) / static_cast<float>(hemisphereSegments);

	// 開始点の半球
	for (int latIndex = 0; latIndex < hemisphereSegments; ++latIndex) {
		float lat1 = -pi / 2.0f + latStep * latIndex;
		float lat2 = -pi / 2.0f + latStep * (latIndex + 1);

		for (int lonIndex = 0; lonIndex < segments; ++lonIndex) {
			float lon1 = angleStep * lonIndex;
			float lon2 = angleStep * (lonIndex + 1);

			// 球面座標から直交座標へ
			float r1 = radius * std::cos(lat1);
			float y1 = radius * std::sin(lat1);
			float r2 = radius * std::cos(lat2);
			float y2 = radius * std::sin(lat2);

			Vector3 localPos1 = {
				r1 * std::cos(lon1) * perpendicular1.x + r1 * std::sin(lon1) * perpendicular2.x + y1 * axis.x,
				r1 * std::cos(lon1) * perpendicular1.y + r1 * std::sin(lon1) * perpendicular2.y + y1 * axis.y,
				r1 * std::cos(lon1) * perpendicular1.z + r1 * std::sin(lon1) * perpendicular2.z + y1 * axis.z
			};

			Vector3 localPos2 = {
				r2 * std::cos(lon1) * perpendicular1.x + r2 * std::sin(lon1) * perpendicular2.x + y2 * axis.x,
				r2 * std::cos(lon1) * perpendicular1.y + r2 * std::sin(lon1) * perpendicular2.y + y2 * axis.y,
				r2 * std::cos(lon1) * perpendicular1.z + r2 * std::sin(lon1) * perpendicular2.z + y2 * axis.z
			};

			Vector3 localPos3 = {
				r1 * std::cos(lon2) * perpendicular1.x + r1 * std::sin(lon2) * perpendicular2.x + y1 * axis.x,
				r1 * std::cos(lon2) * perpendicular1.y + r1 * std::sin(lon2) * perpendicular2.y + y1 * axis.y,
				r1 * std::cos(lon2) * perpendicular1.z + r1 * std::sin(lon2) * perpendicular2.z + y1 * axis.z
			};

			Vector3 p1 = Add(start, localPos1);
			Vector3 p2 = Add(start, localPos2);
			Vector3 p3 = Add(start, localPos3);

			AddLine(p1, p2, color);
			AddLine(p1, p3, color);
		}
	}

	// 終了点の半球
	for (int latIndex = 0; latIndex < hemisphereSegments; ++latIndex) {
		float lat1 = latStep * latIndex;
		float lat2 = latStep * (latIndex + 1);

		for (int lonIndex = 0; lonIndex < segments; ++lonIndex) {
			float lon1 = angleStep * lonIndex;
			float lon2 = angleStep * (lonIndex + 1);

			float r1 = radius * std::cos(lat1);
			float y1 = radius * std::sin(lat1);
			float r2 = radius * std::cos(lat2);
			float y2 = radius * std::sin(lat2);

			Vector3 localPos1 = {
				r1 * std::cos(lon1) * perpendicular1.x + r1 * std::sin(lon1) * perpendicular2.x + y1 * axis.x,
				r1 * std::cos(lon1) * perpendicular1.y + r1 * std::sin(lon1) * perpendicular2.y + y1 * axis.y,
				r1 * std::cos(lon1) * perpendicular1.z + r1 * std::sin(lon1) * perpendicular2.z + y1 * axis.z
			};

			Vector3 localPos2 = {
				r2 * std::cos(lon1) * perpendicular1.x + r2 * std::sin(lon1) * perpendicular2.x + y2 * axis.x,
				r2 * std::cos(lon1) * perpendicular1.y + r2 * std::sin(lon1) * perpendicular2.y + y2 * axis.y,
				r2 * std::cos(lon1) * perpendicular1.z + r2 * std::sin(lon1) * perpendicular2.z + y2 * axis.z
			};

			Vector3 localPos3 = {
				r1 * std::cos(lon2) * perpendicular1.x + r1 * std::sin(lon2) * perpendicular2.x + y1 * axis.x,
				r1 * std::cos(lon2) * perpendicular1.y + r1 * std::sin(lon2) * perpendicular2.y + y1 * axis.y,
				r1 * std::cos(lon2) * perpendicular1.z + r1 * std::sin(lon2) * perpendicular2.z + y1 * axis.z
			};

			Vector3 p1 = Add(end, localPos1);
			Vector3 p2 = Add(end, localPos2);
			Vector3 p3 = Add(end, localPos3);

			AddLine(p1, p2, color);
			AddLine(p1, p3, color);
		}
	}
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

void DebugDrawLineSystem::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::CollapsingHeader("DebugDrawLine System")) {

		ImGui::Separator();

		ImGui::Text("Status: %s", isInitialized_ ? "Initialized" : "Not Initialized");

		// LineRendererの詳細情報
		lineRenderer_->ImGui();
	}
#endif
}