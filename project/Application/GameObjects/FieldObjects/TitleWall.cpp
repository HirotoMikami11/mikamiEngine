#include "TitleWall.h"
#include "ImGui/ImGuiManager.h"
#include "JsonSettings.h"
#include <numbers>
#include <filesystem>

TitleWall::TitleWall()
	: dxCommon_(nullptr)
	, zOffset_(0.0f)
{
	// 壁モデルサイズ（scale が 1 のときの実寸法）
	// {長さ（X方向に対応）, 高さ, 厚み（Z方向）}
	modelSize_ = { 60.0f, 30.0f, 1.0f };

	// 囲みたい領域（フルサイズ）。デフォルトは modelSize_.x を幅・奥行きに使う
	areaSize_ = { modelSize_.x, modelSize_.x };
}

TitleWall::~TitleWall() {}

void TitleWall::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
	JsonSettings* json = JsonSettings::GetInstance();

	// グローバル変数グループを作成
	auto groupName = GetGlobalVariableGroupName();
	json->CreateGroup(groupName);
	json->LoadFiles();

	// デフォルト値を追加（既に存在する場合は追加されない）
	json->AddItem(groupName, "modelSize", modelSize_);
	json->AddItem(groupName, "areaSize", areaSize_);
	json->AddItem(groupName, "zOffset", zOffset_);

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

	// グローバル変数から値を適用
	ApplyGlobalVariables();
}

void TitleWall::ApplyGlobalVariables()
{
	JsonSettings* json = JsonSettings::GetInstance();
	auto groupName = GetGlobalVariableGroupName();

	// パラメータを取得
	modelSize_ = json->GetVector3Value(groupName, "modelSize");
	areaSize_ = json->GetVector2Value(groupName, "areaSize");
	zOffset_ = json->GetFloatValue(groupName, "zOffset");

	// transformを再計算
	UpdateTransforms();
}

void TitleWall::SetAreaSize(const Vector2& areaSize)
{
	// 0 や負の値は受け付けない（最低値を設定）
	areaSize_.x = (areaSize.x > 0.0001f) ? areaSize.x : modelSize_.x;
	areaSize_.y = (areaSize.y > 0.0001f) ? areaSize.y : modelSize_.x;

	// JsonSettingsに反映
	JsonSettings::GetInstance()->SetValue(GetGlobalVariableGroupName(), "areaSize", areaSize_);

	// 変更があったら transform を再計算して反映
	UpdateTransforms();
}

void TitleWall::SetZOffset(float zOffset)
{
	zOffset_ = zOffset;

	// JsonSettingsに反映
	JsonSettings::GetInstance()->SetValue(GetGlobalVariableGroupName(), "zOffset", zOffset_);

	// オフセットが変更されたら transform を再計算
	UpdateTransforms();
}

void TitleWall::UpdateTransforms()
{
	// modelSize_: scale==1 のときの実寸
	const float modelHalfX = modelSize_.x * 0.5f;
	const float modelHalfY = modelSize_.y * 0.5f;
	const float modelHalfZ = modelSize_.z * 0.5f;

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
	// 右側と左側の壁のみ描画（手前と奥は描画しない）
	if (walls_[1].obj) walls_[1].obj->Draw();
	if (walls_[3].obj) walls_[3].obj->Draw();
}

void TitleWall::ImGui()
{
#ifdef USEIMGUI
	JsonSettings* json = JsonSettings::GetInstance();
	auto groupName = GetGlobalVariableGroupName();

	if (ImGui::TreeNode("TitleWalls")) {
		bool changed = false;

		// エリアサイズの調整
		if (ImGui::DragFloat2("Area", &areaSize_.x, 0.1f, 0.0f, 100.0f, "%.2f")) {
			json->SetValue(groupName, "areaSize", areaSize_);
			changed = true;
		}

		// モデルサイズの調整
		if (ImGui::DragFloat3("ModelSize", &modelSize_.x, 0.1f, 0.0f, 100.0f, "%.2f")) {
			json->SetValue(groupName, "modelSize", modelSize_);
			changed = true;
		}

		// Z座標オフセットの調整
		if (ImGui::DragFloat("Z Offset", &zOffset_, 0.1f, -100.0f, 100.0f, "%.2f")) {
			json->SetValue(groupName, "zOffset", zOffset_);
			changed = true;
		}

		if (changed) {
			// 入力を検証して適用
			if (areaSize_.x <= 0.0f) areaSize_.x = modelSize_.x;
			if (areaSize_.y <= 0.0f) areaSize_.y = modelSize_.x;

			ApplyGlobalVariables();
		}

		ImGui::Separator();

		// モデル情報
		for (int i = 0; i < 4; ++i) {
			if (walls_[i].obj) walls_[i].obj->ImGui();
		}

		ImGui::TreePop();
	}
#endif
}