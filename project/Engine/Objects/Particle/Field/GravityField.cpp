#include "GravityField.h"
#include "Managers/ImGui/ImGuiManager.h"
#include <cmath>

void GravityField::Initialize(DirectXCommon* dxCommon)
{
	// 基底クラスの初期化を呼ぶ
	BaseField::Initialize(dxCommon);

	// デフォルトのデバッグカラー（マゼンタ）
	debugColor_ = { 1.0f, 0.0f, 1.0f, 1.0f };
}

bool GravityField::ApplyEffect(ParticleState& particle, float deltaTime)
{
	// フィールド中心座標を取得
	Vector3 center = GetCenter();

	// パーティクルから中心へのベクトル
	Vector3 toCenter = {
		center.x - particle.transform.translate.x,
		center.y - particle.transform.translate.y,
		center.z - particle.transform.translate.z
	};

	// 中心からの距離を計算
	float distance = Length(toCenter);

	// 中心に到達したら削除
	if (distance < deleteRadius_) {
		return true;	// パーティクルを削除
	}

	// 重力を適用（距離が0に近い場合は何もしない）
	if (distance > 0.001f) {
		// 正規化して方向ベクトルを取得
		Vector3 direction = Normalize(toCenter);

		// 重力による加速度を適用
		particle.velocity.x += direction.x * gravityStrength_ * deltaTime;
		particle.velocity.y += direction.y * gravityStrength_ * deltaTime;
		particle.velocity.z += direction.z * gravityStrength_ * deltaTime;
	}

	// パーティクルは削除しない
	return false;
}

bool GravityField::IsInField(const Vector3& point) const
{
	// フィールド中心座標を取得
	Vector3 center = fieldTransform_.GetPosition();

	// 中心からの距離を計算
	Vector3 diff = {
		point.x - center.x,
		point.y - center.y,
		point.z - center.z
	};
	float distance = Length(diff);

	// 効果範囲内かチェック
	return distance <= effectRadius_;
}

void GravityField::DrawCircle(const Vector3& center, float radius, int axis, int segments, const Vector4& color)
{
	const float angleStep = 2.0f * 3.14159265f / segments;

	for (int i = 0; i < segments; ++i) {
		float angle1 = i * angleStep;
		float angle2 = (i + 1) * angleStep;

		Vector3 p1, p2;

		// 軸に応じて円を描画
		switch (axis) {
		case 0: // YZ平面（X軸周り）
			p1 = { center.x, center.y + radius * std::cos(angle1), center.z + radius * std::sin(angle1) };
			p2 = { center.x, center.y + radius * std::cos(angle2), center.z + radius * std::sin(angle2) };
			break;
		case 1: // XZ平面（Y軸周り）
			p1 = { center.x + radius * std::cos(angle1), center.y, center.z + radius * std::sin(angle1) };
			p2 = { center.x + radius * std::cos(angle2), center.y, center.z + radius * std::sin(angle2) };
			break;
		case 2: // XY平面（Z軸周り）
			p1 = { center.x + radius * std::cos(angle1), center.y + radius * std::sin(angle1), center.z };
			p2 = { center.x + radius * std::cos(angle2), center.y + radius * std::sin(angle2), center.z };
			break;
		}

		debugLineRenderer_->AddLine(p1, p2, color);
	}
}

void GravityField::CreateDebugShape()
{
	// 前回の線をクリア
	debugLineRenderer_->Reset();

	// フィールド中心座標を取得
	Vector3 center = GetCenter();

	// 効果範囲の球体を3つの円で表現
	const int segments = 32;	// 円の分割数

	// 3つの軸周りに円を描画
	DrawCircle(center, effectRadius_, 0, segments, debugColor_);	// YZ平面
	DrawCircle(center, effectRadius_, 1, segments, debugColor_);	// XZ平面
	DrawCircle(center, effectRadius_, 2, segments, debugColor_);	// XY平面

	// 削除範囲を異なる色で表示（赤色）
	Vector4 deleteColor = { 1.0f, 0.0f, 0.0f, 1.0f };
	DrawCircle(center, deleteRadius_, 0, segments / 2, deleteColor);
	DrawCircle(center, deleteRadius_, 1, segments / 2, deleteColor);
	DrawCircle(center, deleteRadius_, 2, segments / 2, deleteColor);
}

void GravityField::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode(name_.c_str())) {
		// 基底クラスのImGuiを呼ぶ（共通設定）
		BaseField::ImGui();

		// 重力フィールド固有の設定
		if (ImGui::CollapsingHeader("Gravity Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::DragFloat("Gravity Strength", &gravityStrength_, 0.1f, 0.0f, 20.0f);
			ImGui::DragFloat("Effect Radius", &effectRadius_, 0.1f, 0.1f, 50.0f);
			ImGui::DragFloat("Delete Radius", &deleteRadius_, 0.01f, 0.01f, 2.0f);

			ImGui::Separator();

			// 中心座標の表示
			Vector3 center = GetCenter();
			ImGui::Text("Center: (%.2f, %.2f, %.2f)", center.x, center.y, center.z);
		}

		ImGui::TreePop();
	}
#endif
}