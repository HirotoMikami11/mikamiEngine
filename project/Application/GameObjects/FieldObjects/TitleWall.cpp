#include "TitleWall.h"
#include "ImGui/ImGuiManager.h"
#include "JsonSettings.h"
#include <numbers>
#include <filesystem>

TitleWall::TitleWall()
	: dxCommon_(nullptr) {

	// 壁モデルサイズ（scale が 1 のときの実寸法）
	// {長さ（X方向に対応）, 高さ, 厚み（Z方向）}
	modelSize = { 60.0f, 30.0f, 1.0f };

	// 囲みたい領域（フルサイズ）。デフォルトは modelSize.x を幅・奥行きに使う
	areaSize_ = { modelSize.x, modelSize.x };
}

TitleWall::~TitleWall() {}

void TitleWall::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
	JsonSettings* json = JsonSettings::GetInstance();
	// JsonSettingsのグループを作成
	json->CreateGroup(kGroupPath_);
	json->LoadFiles();
	// デフォルト値を追加（既に存在する場合は追加されない）
	json->AddItem(kGroupPath_, "modelSize", modelSize);
	json->AddItem(kGroupPath_, "areaSize", areaSize_);

	// JSONファイルから値を読み込み
	modelSize = json->GetValue<Vector3>(kGroupPath_, "modelSize").value_or(modelSize);
	areaSize_ = json->GetValue<Vector2>(kGroupPath_, "areaSize").value_or(areaSize_);


	// 各壁モデルを初期化して transform をセットする
	for (int i = 0; i < 4; ++i) {
		walls_[i].obj = std::make_unique<Model3D>();
		walls_[i].obj->Initialize(dxCommon_, "titleWall");
		walls_[i].obj->SetColor(0x3F3A38FF);
		// SetName にインデックスを反映
		char nameBuf[32];
		snprintf(nameBuf, sizeof(nameBuf), "TitleWall[%d]", i);
		walls_[i].obj->SetName(nameBuf);

		// scale は固定 (1,1,1)
		walls_[i].transform.scale = { 1.0f, 1.0f, 1.0f };
	}

	// 初回 transform 計算＆適用
	UpdateTransforms();
}

void TitleWall::SaveToJson()
{
	JsonSettings* json = JsonSettings::GetInstance();
	json->SetValue(kGroupPath_, "modelSize", modelSize);
	json->SetValue(kGroupPath_, "areaSize", areaSize_);
	json->SaveFile(kGroupPath_);
}

void TitleWall::ApplyParameters()
{
	// JsonSettingsから現在の値を取得して適用
	JsonSettings* json = JsonSettings::GetInstance();

	// value_or()でデフォルト値を指定
	modelSize = json->GetValue<Vector3>(kGroupPath_, "modelSize").value_or(modelSize);
	areaSize_ = json->GetValue<Vector2>(kGroupPath_, "areaSize").value_or(areaSize_);

	// transformを再計算
	UpdateTransforms();
}

void TitleWall::SetAreaSize(const Vector2& areaSize)
{
	// 0 や負の値は受け付けない（最低値を設定）
	areaSize_.x = (areaSize.x > 0.0001f) ? areaSize.x : modelSize.x;
	areaSize_.y = (areaSize.y > 0.0001f) ? areaSize.y : modelSize.x;

	// JsonSettingsに反映
	JsonSettings::GetInstance()->SetValue(kGroupPath_, "areaSize", areaSize_);

	// 変更があったら transform を再計算して反映
	UpdateTransforms();
}

void TitleWall::UpdateTransforms()
{
	// modelSize: scale==1 のときの実寸
	const float modelHalfX = modelSize.x * 0.5f;
	const float modelHalfY = modelSize.y * 0.5f;
	const float modelHalfZ = modelSize.z * 0.5f;

	// area の半分
	const float areaHalfX = areaSize_.x * 0.5f;
	const float areaHalfZ = areaSize_.y * 0.5f;

	// Y位置（モデルの中心が原点にある想定 modelHalfY を使う）
	const float yPos = modelHalfY;

	// 各面の回転（前, 右, 後, 左）
	const float rotY[4] = {
		0.0f,
		std::numbers::pi_v<float> / 2.0f,
		std::numbers::pi_v<float>,
		-std::numbers::pi_v<float> / 2.0f
	};

	// 手前 (-Z)
	walls_[0].transform.rotate = { 0.0f, rotY[0], 0.0f };
	walls_[0].transform.translate = { 0.0f, yPos, -(areaHalfZ + modelHalfZ) + zOffset_ };
	walls_[0].transform.scale = { 1.0f, 1.0f, 1.0f };
	walls_[0].obj->SetTransform(walls_[0].transform);

	// 右 (+X)
	walls_[1].transform.rotate = { 0.0f, rotY[1], 0.0f };
	walls_[1].transform.translate = { (areaHalfX + modelHalfZ), yPos, 0.0f + zOffset_ };
	walls_[1].transform.scale = { 1.0f, 1.0f, 1.0f };
	walls_[1].obj->SetTransform(walls_[1].transform);

	// 奥 (+Z)
	walls_[2].transform.rotate = { 0.0f, rotY[2], 0.0f };
	walls_[2].transform.translate = { 0.0f, yPos, (areaHalfZ + modelHalfZ) + zOffset_ };
	walls_[2].transform.scale = { 1.0f, 1.0f, 1.0f };
	walls_[2].obj->SetTransform(walls_[2].transform);

	// 左 (-X)
	walls_[3].transform.rotate = { 0.0f, rotY[3], 0.0f };
	walls_[3].transform.translate = { -(areaHalfX + modelHalfZ), yPos, 0.0f + zOffset_ };
	walls_[3].transform.scale = { 1.0f, 1.0f, 1.0f };
	walls_[3].obj->SetTransform(walls_[3].transform);
}

void TitleWall::Update(const Matrix4x4& viewProjectionMatrix)
{
	for (int i = 0; i < 4; ++i) {
		if (walls_[i].obj) walls_[i].obj->Update(viewProjectionMatrix);
	}
}

void TitleWall::Draw()
{


		if (walls_[1].obj) walls_[1].obj->Draw();
		if (walls_[3].obj) walls_[3].obj->Draw();
	
}

void TitleWall::ImGui()
{
#ifdef USEIMGUI

	JsonSettings* json = JsonSettings::GetInstance();

	if (ImGui::TreeNode("Walls")) {
		// areaSize の表示/編集
		Vector2 a = areaSize_;
		bool changed = false;


		if (ImGui::DragFloat2("Area", &a.x, 0.1f, 0.0f, 100.0f, "%.2f")) {
			json->SetValue(kGroupPath_, "areaSize", a);
			changed = true;
		}


		if (ImGui::DragFloat3("ModelSize", &modelSize.x, 0.1f, 0.0f, 100.0f, "%.2f")) {
			json->SetValue(kGroupPath_, "modelSize", modelSize);
			changed = true;
		}

		if (changed) {
			// 入力を検証して適用
			if (a.x <= 0.0f) a.x = modelSize.x;
			if (a.y <= 0.0f) a.y = modelSize.x;
			areaSize_.x = a.x;
			areaSize_.y = a.y;

			// modelSize の変更に合わせて area デフォルトを変える等するならここで処理
			UpdateTransforms();
		}
		// 保存ボタン
		if (ImGui::Button("Save TitleWall Settings")) {
			SaveToJson();
		}

		for (int i = 0; i < 4; ++i) {
			if (walls_[i].obj) walls_[i].obj->ImGui();
		}

		ImGui::TreePop();
	}

#endif
}
void TitleWall::SetZOffset(float zOffset) {
	zOffset_ = zOffset;
	// オフセットが変更されたら transform を再計算
	UpdateTransforms();
}
