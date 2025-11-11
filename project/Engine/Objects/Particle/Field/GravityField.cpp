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

void GravityField::CreateDebugShape()
{

	// フィールド中心座標を取得
	Vector3 center = GetCenter();

	// 削除範囲を異なる色で表示（赤色）
	Vector4 deleteColor = { 1.0f, 0.0f, 0.0f, 1.0f };

	debugDrawLineSystem_->DrawSphere(center, effectRadius_, debugColor_);
	debugDrawLineSystem_->DrawSphere(center, deleteRadius_, deleteColor);

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