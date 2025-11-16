#include "Wall.h"
#include "ImGui/ImGuiManager.h"
#include <numbers>

Wall::Wall() {}
Wall::~Wall() {}

void Wall::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;

	// areaSize_ はデフォルトで {modelSize.x, modelSize.x} に初期化済み
	// 各壁モデルを初期化して transform をセットする
	for (int i = 0; i < 4; ++i) {
		walls_[i].obj = std::make_unique<Model3D>();
		// モデル名/マテリアル名は既存仕様に合わせてください
		walls_[i].obj->Initialize(dxCommon_, "wall", "white2x2");
		
		// SetName にインデックスを反映
		char nameBuf[32];
		snprintf(nameBuf, sizeof(nameBuf), "Wall[%d]", i);
		walls_[i].obj->SetName(nameBuf);

		// scale は固定 (1,1,1)
		walls_[i].transform.scale = { 1.0f, 1.0f, 1.0f };
	}

	// 初回 transform 計算＆適用
	UpdateTransforms();
}

void Wall::SetAreaSize(const Vector2& areaSize)
{
	// 0 や負の値は受け付けない（最低値を設定）
	areaSize_.x = (areaSize.x > 0.0001f) ? areaSize.x : modelSize.x;
	areaSize_.y = (areaSize.y > 0.0001f) ? areaSize.y : modelSize.x;

	// 変更があったら transform を再計算して反映
	UpdateTransforms();
}

void Wall::UpdateTransforms()
{
	// modelSize: scale==1 のときの実寸
	const float modelHalfX = modelSize.x * 0.5f;
	const float modelHalfY = modelSize.y * 0.5f;
	const float modelHalfZ = modelSize.z * 0.5f;

	// area の半分
	const float areaHalfX = areaSize_.x * 0.5f;
	const float areaHalfZ = areaSize_.y * 0.5f;

	// Y位置（モデルの中心が原点にある想定 -> 床 y=0 にしたい場合は modelHalfY を使う）
	const float yPos = modelHalfY;

	// 各面の回転（前, 右, 後, 左）
	const float rotY[4] = {
		0.0f,
		std::numbers::pi_v<float> / 2.0f,
		std::numbers::pi_v<float>,
		-std::numbers::pi_v<float> / 2.0f
	};

	// 前 (北, -Z)
	walls_[0].transform.rotate = { 0.0f, rotY[0], 0.0f };
	walls_[0].transform.translate = { 0.0f, yPos, -(areaHalfZ + modelHalfZ) };
	walls_[0].transform.scale = { 1.0f, 1.0f, 1.0f };
	walls_[0].obj->SetTransform(walls_[0].transform);

	// 右 (東, +X)
	walls_[1].transform.rotate = { 0.0f, rotY[1], 0.0f };
	walls_[1].transform.translate = { (areaHalfX + modelHalfZ), yPos, 0.0f };
	walls_[1].transform.scale = { 1.0f, 1.0f, 1.0f };
	walls_[1].obj->SetTransform(walls_[1].transform);

	// 後 (南, +Z)
	walls_[2].transform.rotate = { 0.0f, rotY[2], 0.0f };
	walls_[2].transform.translate = { 0.0f, yPos, (areaHalfZ + modelHalfZ) };
	walls_[2].transform.scale = { 1.0f, 1.0f, 1.0f };
	walls_[2].obj->SetTransform(walls_[2].transform);

	// 左 (西, -X)
	walls_[3].transform.rotate = { 0.0f, rotY[3], 0.0f };
	walls_[3].transform.translate = { -(areaHalfX + modelHalfZ), yPos, 0.0f };
	walls_[3].transform.scale = { 1.0f, 1.0f, 1.0f };
	walls_[3].obj->SetTransform(walls_[3].transform);
}

void Wall::Update(const Matrix4x4& viewProjectionMatrix)
{
	for (int i = 0; i < 4; ++i) {
		if (walls_[i].obj) walls_[i].obj->Update(viewProjectionMatrix);
	}
}

void Wall::Draw(const Light& light)
{
	for (int i = 0; i < 4; ++i) {

		if (i == 0) continue;

		if (walls_[i].obj) walls_[i].obj->Draw(light);
	}
}

void Wall::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode("Player")) {
		// areaSize の表示/編集（変更があれば再配置）
		float ax = areaSize_.x;
		float az = areaSize_.y;
		bool changed = false;

		changed |= ImGui::InputFloat("Area X (width)", &ax);
		changed |= ImGui::InputFloat("Area Z (depth)", &az);

		// 参考用に modelSize も表示（編集可能であれば再配置される）
		float mX = modelSize.x;
		float mY = modelSize.y;
		float mZ = modelSize.z;
		if (ImGui::InputFloat("Model size X (length)", &mX)) { modelSize.x = mX; changed = true; }
		if (ImGui::InputFloat("Model size Y (height)", &mY)) { modelSize.y = mY; changed = true; }
		if (ImGui::InputFloat("Model size Z (thickness)", &mZ)) { modelSize.z = mZ; changed = true; }

		if (changed) {
			// 入力を検証して適用
			if (ax <= 0.0f) ax = modelSize.x;
			if (az <= 0.0f) az = modelSize.x;
			areaSize_.x = ax;
			areaSize_.y = az;

			// modelSize の変更に合わせて area デフォルトを変える等するならここで処理
			// 今回は単純に transform を再計算
			UpdateTransforms();
		}

		// オリジナルのモデル ImGui 呼び出し（各 Model3D が持つ UI）
		for (int i = 0; i < 4; ++i) {
			if (walls_[i].obj) walls_[i].obj->ImGui();
		}
	
		ImGui::TreePop();
	}

#endif
}
