#include "TreasureBox.h"

TreasureBox::TreasureBox()
{
}

TreasureBox::~TreasureBox()
{
}

void TreasureBox::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;

	// ゲームオブジェクト（3Dモデル）の初期化
	gameObject_ = std::make_unique<Model3D>();
	gameObject_->Initialize(dxCommon, "treasureBox");
	gameObject_->SetName("treasureBox");

	// 初期位置設定
	Vector3Transform defaultTransform{
		{3.0f, 3.0f, 3.0f},
		{0.0f, 0.6f, 0.0f},
		{0.0f,3.95f,14.83f}
	};
	gameObject_->SetTransform(defaultTransform);

	gameObject_->SetColor(0xFFFFFFFF);
}

void TreasureBox::Update(const Matrix4x4& viewProjectionMatrix) {
	// ゲームオブジェクトの更新
	gameObject_->Update(viewProjectionMatrix);
}

void TreasureBox::Draw() {
	// 地面の描画
	gameObject_->Draw();

}

void TreasureBox::ImGui()
{
#ifdef USEIMGUI
	if (gameObject_) {
		gameObject_->ImGui();
	}

#endif
}

