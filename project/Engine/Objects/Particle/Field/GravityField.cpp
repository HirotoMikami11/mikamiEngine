#include "GravityField.h"
#include "ImGui/ImGuiManager.h"

void GravityField::Initialize(DirectXCommon* dxCommon)
{
	// 基底クラスの初期化を呼ぶ
	BaseField::Initialize(dxCommon);

	// デフォルトのデバッグカラー（マゼンタ）
	debugColor_ = { 1.0f, 0.0f, 1.0f, 1.0f };
}

bool GravityField::ApplyEffect(ParticleState& particle, float deltaTime)
{
	// フィールドの中心位置を取得
	Vector3 fieldCenter = fieldTransform_.GetPosition();

	// パーティクルからフィールド中心へのベクトル
	Vector3 toCenter = {
		fieldCenter.x - particle.transform.translate.x,
		fieldCenter.y - particle.transform.translate.y,
		fieldCenter.z - particle.transform.translate.z
	};

	// 距離を計算
	float distance = Length(toCenter);

	// 削除範囲内の場合、パーティクルを削除
	if (distance < deleteRadius_) {
		return true;
	}

	// 効果範囲外の場合は何もしない
	if (distance > effectRadius_) {
		return false;
	}

	// 方向を正規化
	Vector3 direction = Normalize(toCenter);

	// 距離に応じた重力の強さ（距離の2乗に反比例）
	float distanceFactor = 1.0f - (distance / effectRadius_);
	float gravityForce = gravityStrength_ * distanceFactor * distanceFactor;

	// 重力を速度に加算
	particle.velocity.x += direction.x * gravityForce * deltaTime;
	particle.velocity.y += direction.y * gravityForce * deltaTime;
	particle.velocity.z += direction.z * gravityForce * deltaTime;

	return false;
}

bool GravityField::IsInField(const Vector3& point) const
{
	// フィールドの中心位置を取得
	Vector3 fieldCenter = fieldTransform_.GetPosition();

	// 点からフィールド中心までの距離を計算
	Vector3 toCenter = {
		fieldCenter.x - point.x,
		fieldCenter.y - point.y,
		fieldCenter.z - point.z
	};

	float distance = Length(toCenter);

	// 効果範囲内かチェック
	return distance <= effectRadius_;
}

void GravityField::CreateDebugShape()
{
	// フィールドの中心位置を取得
	Vector3 fieldCenter = fieldTransform_.GetPosition();

	// 効果範囲の球体を描画
	debugDrawLineSystem_->DrawSphere(fieldCenter, effectRadius_, debugColor_);

	// 削除範囲の球体も描画（半透明の赤）
	Vector4 deleteColor = { 1.0f, 0.0f, 0.0f, 0.5f };
	debugDrawLineSystem_->DrawSphere(fieldCenter, deleteRadius_, deleteColor);
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
			ImGui::DragFloat("Effect Radius", &effectRadius_, 0.1f, 0.1f, 20.0f);
			ImGui::DragFloat("Delete Radius", &deleteRadius_, 0.1f, 0.0f, effectRadius_);

			// 削除範囲が効果範囲を超えないようにする
			if (deleteRadius_ > effectRadius_) {
				deleteRadius_ = effectRadius_;
			}

			ImGui::Separator();
		}

		ImGui::TreePop();
	}
#endif
}

json GravityField::SerializeParameters() const
{
	return json{
		{"gravityStrength", gravityStrength_},
		{"effectRadius", effectRadius_},
		{"deleteRadius", deleteRadius_}
	};
}

void GravityField::DeserializeParameters(const json& j)
{
	if (j.contains("gravityStrength")) {
		gravityStrength_ = j["gravityStrength"];
	}

	if (j.contains("effectRadius")) {
		effectRadius_ = j["effectRadius"];
	}

	if (j.contains("deleteRadius")) {
		deleteRadius_ = j["deleteRadius"];
	}
}