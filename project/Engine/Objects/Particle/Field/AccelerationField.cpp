#include "AccelerationField.h"
#include "Managers/ImGui/ImGuiManager.h"

void AccelerationField::Initialize(DirectXCommon* dxCommon)
{
	// 基底クラスの初期化を呼ぶ
	BaseField::Initialize(dxCommon);

	// AABBを正しい状態に
	FixAABBMinMax(area_);

	// デフォルトのデバッグカラー（緑色）
	debugColor_ = { 0.0f, 1.0f, 0.0f, 1.0f };
}

bool AccelerationField::ApplyEffect(ParticleState& particle, float deltaTime)
{
	// パーティクルに加速度を適用
	particle.velocity.x += acceleration_.x * deltaTime;
	particle.velocity.y += acceleration_.y * deltaTime;
	particle.velocity.z += acceleration_.z * deltaTime;

	// 加速度フィールドではパーティクルを削除しない
	return false;
}

bool AccelerationField::IsInField(const Vector3& point) const
{
	// ワールド座標でのAABBを取得
	AABB worldAABB = GetWorldAABB();

	// 点がAABB内にあるかチェック
	return IsCollision(worldAABB, point);
}

void AccelerationField::SetAreaSize(const Vector3& size)
{
	// 中心からの±sizeで設定（ローカル座標）
	area_.min = { -size.x, -size.y, -size.z };
	area_.max = { size.x, size.y, size.z };
	FixAABBMinMax(area_);
}

AABB AccelerationField::GetWorldAABB() const
{
	// フィールドの位置を取得
	Vector3 fieldPos = fieldTransform_.GetPosition();

	// ローカル座標のAABBをワールド座標に変換
	AABB worldAABB;
	worldAABB.min = {
		fieldPos.x + area_.min.x,
		fieldPos.y + area_.min.y,
		fieldPos.z + area_.min.z
	};
	worldAABB.max = {
		fieldPos.x + area_.max.x,
		fieldPos.y + area_.max.y,
		fieldPos.z + area_.max.z
	};

	return worldAABB;
}

void AccelerationField::CreateDebugShape()
{
	// 前回の線をクリア
	debugLineRenderer_->Reset();

	// ワールド座標でのAABBを取得
	AABB worldAABB = GetWorldAABB();

	// AABBの8頂点を計算（ワールド座標）
	Vector3 vertices[8] = {
		// 底面（min.y）
		{ worldAABB.min.x, worldAABB.min.y, worldAABB.min.z },	// 0: 左下手前
		{ worldAABB.max.x, worldAABB.min.y, worldAABB.min.z },	// 1: 右下手前
		{ worldAABB.max.x, worldAABB.min.y, worldAABB.max.z },	// 2: 右下奥
		{ worldAABB.min.x, worldAABB.min.y, worldAABB.max.z },	// 3: 左下奥

		// 上面（max.y）
		{ worldAABB.min.x, worldAABB.max.y, worldAABB.min.z },	// 4: 左上手前
		{ worldAABB.max.x, worldAABB.max.y, worldAABB.min.z },	// 5: 右上手前
		{ worldAABB.max.x, worldAABB.max.y, worldAABB.max.z },	// 6: 右上奥
		{ worldAABB.min.x, worldAABB.max.y, worldAABB.max.z }	// 7: 左上奥
	};

	// 底面の4本の線
	debugLineRenderer_->AddLine(vertices[0], vertices[1], debugColor_);
	debugLineRenderer_->AddLine(vertices[1], vertices[2], debugColor_);
	debugLineRenderer_->AddLine(vertices[2], vertices[3], debugColor_);
	debugLineRenderer_->AddLine(vertices[3], vertices[0], debugColor_);

	// 上面の4本の線
	debugLineRenderer_->AddLine(vertices[4], vertices[5], debugColor_);
	debugLineRenderer_->AddLine(vertices[5], vertices[6], debugColor_);
	debugLineRenderer_->AddLine(vertices[6], vertices[7], debugColor_);
	debugLineRenderer_->AddLine(vertices[7], vertices[4], debugColor_);

	// 縦の4本の線
	debugLineRenderer_->AddLine(vertices[0], vertices[4], debugColor_);
	debugLineRenderer_->AddLine(vertices[1], vertices[5], debugColor_);
	debugLineRenderer_->AddLine(vertices[2], vertices[6], debugColor_);
	debugLineRenderer_->AddLine(vertices[3], vertices[7], debugColor_);
}

void AccelerationField::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode(name_.c_str())) {
		// 基底クラスのImGuiを呼ぶ（共通設定）
		BaseField::ImGui();

		// 加速度フィールド固有の設定
		if (ImGui::CollapsingHeader("Acceleration Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::DragFloat3("Acceleration", &acceleration_.x, 0.1f, -10.0f, 10.0f);
			ImGui::Separator();
		}

		// AABB範囲設定
		if (ImGui::CollapsingHeader("Area (AABB)")) {
			bool aabbChanged = false;

			ImGui::Text("Local AABB Min");
			if (ImGui::DragFloat3("Min", &area_.min.x, 0.1f)) {
				aabbChanged = true;
			}

			ImGui::Text("Local AABB Max");
			if (ImGui::DragFloat3("Max", &area_.max.x, 0.1f)) {
				aabbChanged = true;
			}

			if (aabbChanged) {
				FixAABBMinMax(area_);
			}

			// サイズ表示
			Vector3 size = {
				area_.max.x - area_.min.x,
				area_.max.y - area_.min.y,
				area_.max.z - area_.min.z
			};
			ImGui::Text("Size: (%.2f, %.2f, %.2f)", size.x, size.y, size.z);

			ImGui::Separator();

			// ワールド座標のAABB表示
			AABB worldAABB = GetWorldAABB();
			ImGui::Text("World AABB Min: (%.2f, %.2f, %.2f)",
				worldAABB.min.x, worldAABB.min.y, worldAABB.min.z);
			ImGui::Text("World AABB Max: (%.2f, %.2f, %.2f)",
				worldAABB.max.x, worldAABB.max.y, worldAABB.max.z);

			ImGui::Separator();

			// クイック設定ボタン
			if (ImGui::Button("Set Small (2x2x2)")) {
				SetAreaSize({ 1.0f, 1.0f, 1.0f });
			}
			ImGui::SameLine();
			if (ImGui::Button("Set Medium (4x4x4)")) {
				SetAreaSize({ 2.0f, 2.0f, 2.0f });
			}
			ImGui::SameLine();
			if (ImGui::Button("Set Large (8x8x8)")) {
				SetAreaSize({ 4.0f, 4.0f, 4.0f });
			}
		}

		ImGui::TreePop();
	}
#endif
}