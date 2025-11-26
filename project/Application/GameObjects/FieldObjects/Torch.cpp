#include "torch.h"
#include "ImGui/ImGuiManager.h"
#include "JsonSettings.h"
#include <numbers>

Torch::Torch()
	: dxCommon_(nullptr)
	, globalScale_({ 1.0f, 1.0f, 1.0f })
	, globalColor_({ 1.0f, 1.0f, 1.0f, 1.0f })
{
	// 12個のトーチのデフォルト配置（例：円形に配置）
	const float radius = 25.0f;
	const float angleStep = (2.0f * std::numbers::pi_v<float>) / kTorchCount_;

	for (int i = 0; i < kTorchCount_; ++i) {
		float angle = angleStep * i;
		positions_[i] = {
			radius * std::cos(angle),
			0.0f,
			radius * std::sin(angle)
		};

		// 中心を向くように回転
		rotations_[i] = {
			0.0f,
			angle + std::numbers::pi_v<float>,  // 中心を向く
			0.0f
		};
	}
}

Torch::~Torch() {}

void Torch::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
	JsonSettings* json = JsonSettings::GetInstance();

	// JsonSettingsのグループを作成
	json->CreateGroup(kGroupPath_);
	json->LoadFiles();

	// デフォルト値を追加（既に存在する場合は追加されない）
	json->AddItem(kGroupPath_, "globalScale", globalScale_);
	json->AddItem(kGroupPath_, "globalColor", globalColor_);

	// 各トーチの座標と回転をJsonに追加
	for (int i = 0; i < kTorchCount_; ++i) {
		char posKey[32], rotKey[32];
		snprintf(posKey, sizeof(posKey), "position%d", i);
		snprintf(rotKey, sizeof(rotKey), "rotation%d", i);

		json->AddItem(kGroupPath_, posKey, positions_[i]);
		json->AddItem(kGroupPath_, rotKey, rotations_[i]);
	}

	// JSONファイルから値を読み込み
	globalScale_ = json->GetValue<Vector3>(kGroupPath_, "globalScale").value_or(globalScale_);
	globalColor_ = json->GetValue<Vector4>(kGroupPath_, "globalColor").value_or(globalColor_);

	for (int i = 0; i < kTorchCount_; ++i) {
		char posKey[32], rotKey[32];
		snprintf(posKey, sizeof(posKey), "position%d", i);
		snprintf(rotKey, sizeof(rotKey), "rotation%d", i);

		positions_[i] = json->GetValue<Vector3>(kGroupPath_, posKey).value_or(positions_[i]);
		rotations_[i] = json->GetValue<Vector3>(kGroupPath_, rotKey).value_or(rotations_[i]);
	}

	// 各トーチモデルを初期化
	for (int i = 0; i < kTorchCount_; ++i) {
		torches_[i].obj = std::make_unique<Model3D>();

		torches_[i].obj->Initialize(dxCommon_, "torch", "white2x2");

		// SetName にインデックスを反映
		char nameBuf[32];
		snprintf(nameBuf, sizeof(nameBuf), "Torch[%d]", i);
		torches_[i].obj->SetName(nameBuf);
	}

	// 初回 transform 計算＆適用
	UpdateTransforms();
}

void Torch::SaveToJson()
{
	JsonSettings* json = JsonSettings::GetInstance();

	json->SetValue(kGroupPath_, "globalScale", globalScale_);

	for (int i = 0; i < kTorchCount_; ++i) {
		char posKey[32], rotKey[32];
		snprintf(posKey, sizeof(posKey), "position%d", i);
		snprintf(rotKey, sizeof(rotKey), "rotation%d", i);

		json->SetValue(kGroupPath_, posKey, positions_[i]);
		json->SetValue(kGroupPath_, rotKey, rotations_[i]);
	}

	json->SaveFile(kGroupPath_);
}

void Torch::ApplyParameters()
{
	// JsonSettingsから現在の値を取得して適用
	JsonSettings* json = JsonSettings::GetInstance();

	globalScale_ = json->GetValue<Vector3>(kGroupPath_, "globalScale").value_or(globalScale_);
	globalColor_ = json->GetValue<Vector4>(kGroupPath_, "globalColor").value_or(globalColor_);

	for (int i = 0; i < kTorchCount_; ++i) {
		char posKey[32], rotKey[32];
		snprintf(posKey, sizeof(posKey), "position%d", i);
		snprintf(rotKey, sizeof(rotKey), "rotation%d", i);

		positions_[i] = json->GetValue<Vector3>(kGroupPath_, posKey).value_or(positions_[i]);
		rotations_[i] = json->GetValue<Vector3>(kGroupPath_, rotKey).value_or(rotations_[i]);
	}

	// transformを再計算
	UpdateTransforms();
}

void Torch::UpdateTransforms()
{
	// 各トーチのトランスフォームを更新
	for (int i = 0; i < kTorchCount_; ++i) {
		torches_[i].transform.scale = globalScale_;
		torches_[i].transform.rotate = rotations_[i];
		torches_[i].transform.translate = positions_[i];

		if (torches_[i].obj) {
			torches_[i].obj->SetTransform(torches_[i].transform);
			torches_[i].obj->SetColor(globalColor_);
		}
	}
}

void Torch::Update(const Matrix4x4& viewProjectionMatrix)
{
	for (int i = 0; i < kTorchCount_; ++i) {
		if (torches_[i].obj) {
			torches_[i].obj->Update(viewProjectionMatrix);
		}
	}
}

void Torch::Draw()
{
	for (int i = 0; i < kTorchCount_; ++i) {
		if (torches_[i].obj) {
			torches_[i].obj->Draw();
		}
	}
}

void Torch::ImGui()
{
#ifdef USEIMGUI

	JsonSettings* json = JsonSettings::GetInstance();

	if (ImGui::TreeNode("Torches")) {
		bool changed = false;

		// 全体のスケール設定
		if (ImGui::DragFloat3("Global Scale", &globalScale_.x, 0.01f, 0.01f, 10.0f, "%.2f")) {
			json->SetValue(kGroupPath_, "globalScale", globalScale_);
			changed = true;
		}
		// 全体の色設定
		if (ImGui::ColorEdit4("Global Color", &globalColor_.x)) {
			json->SetValue(kGroupPath_, "globalColor", globalColor_);
			changed = true;
		}
		ImGui::Separator();

		// 各トーチの設定
		for (int i = 0; i < kTorchCount_; ++i) {
			char labelBuf[32];
			snprintf(labelBuf, sizeof(labelBuf), "Torch[%d]", i);

			if (ImGui::TreeNode(labelBuf)) {
				char posKey[32], rotKey[32];
				snprintf(posKey, sizeof(posKey), "position%d", i);
				snprintf(rotKey, sizeof(rotKey), "rotation%d", i);

				// 座標設定
				if (ImGui::DragFloat3("Position", &positions_[i].x, 0.1f, -100.0f, 100.0f, "%.2f")) {
					json->SetValue(kGroupPath_, posKey, positions_[i]);
					changed = true;
				}

				// 回転設定（度数法で表示）
				Vector3 rotationDeg = {
					rotations_[i].x * (180.0f / std::numbers::pi_v<float>),
					rotations_[i].y * (180.0f / std::numbers::pi_v<float>),
					rotations_[i].z * (180.0f / std::numbers::pi_v<float>)
				};

				if (ImGui::DragFloat3("Rotation (deg)", &rotationDeg.x, 1.0f, -180.0f, 180.0f, "%.1f")) {
					rotations_[i] = {
						rotationDeg.x * (std::numbers::pi_v<float> / 180.0f),
						rotationDeg.y * (std::numbers::pi_v<float> / 180.0f),
						rotationDeg.z * (std::numbers::pi_v<float> / 180.0f)
					};
					json->SetValue(kGroupPath_, rotKey, rotations_[i]);
					changed = true;
				}

				// 各トーチのImGui（詳細表示用）
				if (torches_[i].obj) {
					torches_[i].obj->ImGui();
				}

				ImGui::TreePop();
			}
		}

		// 変更があった場合、transformを更新
		if (changed) {
			UpdateTransforms();
		}

		ImGui::Separator();

		// 保存ボタン
		if (ImGui::Button("Save Torch Settings")) {
			SaveToJson();
		}

		ImGui::TreePop();
	}

#endif
}