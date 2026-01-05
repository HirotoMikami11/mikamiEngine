#include "TitleTorch.h"
#include "ImGui/ImGuiManager.h"
#include "JsonSettings.h"
#include "GameTimer.h"
#include "LightManager.h"
#include <numbers>

TitleTorch::TitleTorch()
	: dxCommon_(nullptr)
	, titleWall_(nullptr)
	, globalScale_({ 1.5f, 3.0f, 1.5f })
	, globalColor_({ 0.21078431606292725f, 0.08679354935884476f, 0.0f, 1.0f })
	, zOffset_(0.0f)
	, distanceFromWall_(1.5f)
{
	// 元のTorch.jsonの配置を基準に初期値を設定
	// 右側（0~3）
	positions_[0] = { 0.0f, 7.0f, -9.5f };
	positions_[1] = { 0.0f, 7.0f, 7.7f };
	positions_[2] = { 0.0f, 7.0f, 21.35f };
	positions_[3] = { 0.0f, 7.0f, -21.3f };

	// 左側（4~7）
	positions_[4] = { 0.0f, 7.0f, -9.5f };
	positions_[5] = { 0.0f, 7.0f, 7.7f };
	positions_[6] = { 0.0f, 7.0f, 21.35f };
	positions_[7] = { 0.0f, 7.0f, -21.3f };

	// 元のTorch.jsonの回転を設定
	// 右側（0~3）
	rotations_[0] = { 0.0f, 3.1415927410125732f, 0.4363323152065277f };
	rotations_[1] = { 0.0f, 3.1415927410125732f, 0.43633225560188293f };
	rotations_[2] = { 0.0f, 3.1415927410125732f, 0.4363323152065277f };
	rotations_[3] = { 0.0f, 3.1415927410125732f, 0.4363323152065277f };

	// 左側（4~7）
	rotations_[4] = { 0.0f, 3.1415927410125732f, -0.4363323152065277f };
	rotations_[5] = { 0.0f, 3.1415927410125732f, -0.43633225560188293f };
	rotations_[6] = { 0.0f, 3.1415927410125732f, -0.4363323152065277f };
	rotations_[7] = { 0.0f, 3.1415927410125732f, -0.4363323152065277f };
}

TitleTorch::~TitleTorch() {}

void TitleTorch::Initialize(DirectXCommon* dxCommon, TitleWall* titleWall, int segmentIndex)
{
	dxCommon_ = dxCommon;
	titleWall_ = titleWall;
	segmentIndex_ = segmentIndex;

	JsonSettings* json = JsonSettings::GetInstance();

	// JsonSettingsのグループを作成
	json->CreateGroup(kGroupPath_);
	json->LoadFiles();

	// デフォルト値を追加
	json->AddItem(kGroupPath_, "globalScale", globalScale_);
	json->AddItem(kGroupPath_, "globalColor", globalColor_);
	json->AddItem(kGroupPath_, "distanceFromWall", distanceFromWall_);

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
	distanceFromWall_ = json->GetValue<float>(kGroupPath_, "distanceFromWall").value_or(distanceFromWall_);

	for (int i = 0; i < kTorchCount_; ++i) {
		char posKey[32], rotKey[32];
		snprintf(posKey, sizeof(posKey), "position%d", i);
		snprintf(rotKey, sizeof(rotKey), "rotation%d", i);

		positions_[i] = json->GetValue<Vector3>(kGroupPath_, posKey).value_or(positions_[i]);
		rotations_[i] = json->GetValue<Vector3>(kGroupPath_, rotKey).value_or(rotations_[i]);
	}

	// TitleWallの位置情報から自動配置
	AutoPlaceTorches();

	// 各トーチモデルを初期化
	for (int i = 0; i < kTorchCount_; ++i) {
		torches_[i].obj = std::make_unique<Model3D>();
		torches_[i].obj->Initialize(dxCommon_, "torch", "white2x2");

		// SetName にインデックスを反映
		char nameBuf[32];
		snprintf(nameBuf, sizeof(nameBuf), "TitleTorch[%d]", i);
		torches_[i].obj->SetName(nameBuf);
	}

	// 初回 transform 計算＆適用
	UpdateTransforms();

	// ポイントライトを追加
	LightManager* lightManager = LightManager::GetInstance();
	for (int i = 0; i < kTorchCount_; ++i) {
		Vector3 lightPos = positions_[i];
		lightPos.z += zOffset_;
		torchLights_[i] = lightManager->AddPointLight(
			lightPos,
			Vector4{ 1.0f, 0.5f, 0.0f, 1.0f }, // オレンジ色
			1.5f,
			15.0f,
			2.0f
		);
	}

	// パーティクルを追加
	ParticleEditor* particleEditor = ParticleEditor::GetInstance();
	for (int i = 0; i < kTorchCount_; ++i)
	{
		// セグメントインデックスを含めた一意の名前を生成
		std::string instanceName = std::format("TitleFire_Seg{}[{}]", segmentIndex_, i);

		// 既存インスタンスがあれば削除（重複回避）
		auto* existingInstance = particleEditor->GetInstance(instanceName);
		if (existingInstance) {
			particleEditor->DestroyInstance(instanceName);
		}

		particleEditor->CreateInstance("Fire", instanceName);
		particleInstance_[i] = particleEditor->GetInstance(instanceName);

		if (particleInstance_[i]) {
			Vector3 particlePos = positions_[i];
			particlePos.z += zOffset_;
			particleInstance_[i]->GetEmitter("FireEmitter")->GetTransform().SetPosition(particlePos);
		}
	}

	// ちらつき速度をずらして初期化
	for (int i = 0; i < kTorchCount_; ++i) {
		flickerTime_[i] = 0.0f;
		flickerSpeed_[i] = 2.0f + (i * 0.2f);
		flickerOffset_[i] = i * 0.5f;
	}
}

void TitleTorch::SaveToJson()
{
	JsonSettings* json = JsonSettings::GetInstance();

	json->SetValue(kGroupPath_, "globalScale", globalScale_);
	json->SetValue(kGroupPath_, "globalColor", globalColor_);
	json->SetValue(kGroupPath_, "distanceFromWall", distanceFromWall_);

	for (int i = 0; i < kTorchCount_; ++i) {
		char posKey[32], rotKey[32];
		snprintf(posKey, sizeof(posKey), "position%d", i);
		snprintf(rotKey, sizeof(rotKey), "rotation%d", i);

		json->SetValue(kGroupPath_, posKey, positions_[i]);
		json->SetValue(kGroupPath_, rotKey, rotations_[i]);
	}

	json->SaveFile(kGroupPath_);
}

void TitleTorch::ApplyParameters()
{
	JsonSettings* json = JsonSettings::GetInstance();

	globalScale_ = json->GetValue<Vector3>(kGroupPath_, "globalScale").value_or(globalScale_);
	globalColor_ = json->GetValue<Vector4>(kGroupPath_, "globalColor").value_or(globalColor_);
	distanceFromWall_ = json->GetValue<float>(kGroupPath_, "distanceFromWall").value_or(distanceFromWall_);

	for (int i = 0; i < kTorchCount_; ++i) {
		char posKey[32], rotKey[32];
		snprintf(posKey, sizeof(posKey), "position%d", i);
		snprintf(rotKey, sizeof(rotKey), "rotation%d", i);

		positions_[i] = json->GetValue<Vector3>(kGroupPath_, posKey).value_or(positions_[i]);
		rotations_[i] = json->GetValue<Vector3>(kGroupPath_, rotKey).value_or(rotations_[i]);
	}

	UpdateTransforms();
}

void TitleTorch::AutoPlaceTorches()
{
	if (!titleWall_) return;

	// TitleWallのエリアサイズを取得（JsonSettingsから）
	// TitleWall.json: areaSize = [25.0, 60.0]
	const float wallAreaHalfX = 25.0f * 0.5f; // 12.5
	const float wallModelHalfZ = 1.0f * 0.5f; // 0.5

	// 壁の実際の位置
	const float rightWallX = wallAreaHalfX + wallModelHalfZ; // 13.0
	const float leftWallX = -(wallAreaHalfX + wallModelHalfZ); // -13.0

	// トーチの位置（壁から内側に配置）
	const float rightTorchX = rightWallX - distanceFromWall_;
	const float leftTorchX = leftWallX + distanceFromWall_;

	// 右側のトーチ（0~3）のX座標を更新
	for (int i = 0; i < 4; ++i) {
		positions_[i].x = rightTorchX;
	}

	// 左側のトーチ（4~7）のX座標を更新
	for (int i = 4; i < 8; ++i) {
		positions_[i].x = leftTorchX;
	}
}

void TitleTorch::UpdateTransforms()
{
	// 各トーチのトランスフォームを更新
	for (int i = 0; i < kTorchCount_; ++i) {
		torches_[i].transform.scale = globalScale_;
		torches_[i].transform.rotate = rotations_[i];
		torches_[i].transform.translate = positions_[i];
		torches_[i].transform.translate.z += zOffset_; // Z座標オフセットを適用

		if (torches_[i].obj) {
			torches_[i].obj->SetTransform(torches_[i].transform);
			torches_[i].obj->SetColor(globalColor_);
		}
	}
}

void TitleTorch::Update(const Matrix4x4& viewProjectionMatrix)
{
	// GameTimerからデルタタイムを取得
	GameTimer& gameTimer = GameTimer::GetInstance();
	float deltaTime = gameTimer.GetDeltaTime();

	LightManager* lightManager = LightManager::GetInstance();

	for (int i = 0; i < kTorchCount_; ++i) {
		// 時間を進める
		flickerTime_[i] += deltaTime;

		// 複数の正弦波を組み合わせて自然な揺らぎを作る
		float flicker1 = std::sin(flickerTime_[i] * flickerSpeed_[i] + flickerOffset_[i]);
		float flicker2 = std::sin(flickerTime_[i] * flickerSpeed_[i] * 2.3f) * 0.5f;
		float flicker3 = std::sin(flickerTime_[i] * flickerSpeed_[i] * 3.7f) * 0.3f;

		float flickerValue = (flicker1 + flicker2 + flicker3) / 1.8f; // -1 ~ 1
		float intensity = baseIntensity_ + (flickerValue * flickerAmount_);

		// ライトの強度を更新
		Vector3 lightPos = positions_[i];
		lightPos.z += zOffset_;
		torchLights_[i]->SetPosition(lightPos);
		torchLights_[i]->SetIntensity(intensity);

		// パーティクルのスケールも連動させる
		if (particleInstance_[i]) {
			float scale = 1.0f + (flickerValue * 0.2f);
			particleInstance_[i]->GetEmitter("FireEmitter")->SetParticleStartScale({ scale, scale, scale });

			// パーティクルの位置も更新
			Vector3 particlePos = positions_[i];
			particlePos.z += zOffset_;
			particleInstance_[i]->GetEmitter("FireEmitter")->GetTransform().SetPosition(particlePos);
		}

		if (torches_[i].obj) {
			torches_[i].obj->Update(viewProjectionMatrix);
		}
	}
}

void TitleTorch::Draw()
{
	for (int i = 0; i < kTorchCount_; ++i) {
		if (torches_[i].obj) {
			torches_[i].obj->Draw();
		}
	}
}

void TitleTorch::ImGui()
{
#ifdef USEIMGUI

	JsonSettings* json = JsonSettings::GetInstance();

	if (ImGui::TreeNode("TitleTorches")) {
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

		// 壁からの距離設定
		if (ImGui::DragFloat("Distance From Wall", &distanceFromWall_, 0.1f, 0.0f, 10.0f, "%.2f")) {
			json->SetValue(kGroupPath_, "distanceFromWall", distanceFromWall_);
			AutoPlaceTorches(); // 自動配置を再計算
			changed = true;
		}

		// 自動配置ボタン
		if (ImGui::Button("Auto Place from TitleWall")) {
			AutoPlaceTorches();
			changed = true;
		}

		ImGui::Separator();

		// 各トーチの設定
		for (int i = 0; i < kTorchCount_; ++i) {
			char labelBuf[32];
			snprintf(labelBuf, sizeof(labelBuf), "TitleTorch[%d] %s", i, (i < 4) ? "(Right)" : "(Left)");

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
		if (ImGui::Button("Save TitleTorch Settings")) {
			SaveToJson();
		}

		ImGui::TreePop();
	}

#endif
}

void TitleTorch::SetZOffset(float zOffset)
{
	zOffset_ = zOffset;

	// トランスフォームを更新
	UpdateTransforms();

	// ポイントライトとパーティクルの位置も更新
	for (int i = 0; i < kTorchCount_; ++i) {
		Vector3 pos = positions_[i];
		pos.z += zOffset_;

		if (torchLights_[i]) {
			torchLights_[i]->SetPosition(pos);
		}

		if (particleInstance_[i]) {
			particleInstance_[i]->GetEmitter("FireEmitter")->GetTransform().SetPosition(pos);
		}
	}
}