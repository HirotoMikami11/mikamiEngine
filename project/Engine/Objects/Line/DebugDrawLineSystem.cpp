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
	isUse_ = false;
#ifdef USEIMGUI
	isUse_ = true;
#endif

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
	if (!isInitialized_ || !lineRenderer_ || !isUse_) {
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
	if (!isInitialized_ || !isUse_) {
		return;
	}

	lineRenderer_->AddLine(start, end, color);
}

void DebugDrawLineSystem::AddLine(const Vector3& start, const Vector3& end, const uint32_t& color)
{
	if (!isInitialized_ || !isUse_) {
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

void DebugDrawLineSystem::DrawSphere(const Vector3& center, float radius, const uint32_t& color, uint32_t subdivision)
{
	DrawSphere(center, radius, Uint32ToColorVector(color));
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
		ImGui::Checkbox("USE", &isUse_);

		// LineRendererの詳細情報
		lineRenderer_->ImGui();
	}
#endif
}