#include "Ground.h"

Ground::Ground()
{
}

Ground::~Ground()
{
}

void Ground::Initialize(DirectXCommon* dxCommon, const Vector3& position)
{
	dxCommon_ = dxCommon;

	// ゲームオブジェクト（3Dモデル）の初期化
	gameObject_ = std::make_unique<Model3D>();
	gameObject_->Initialize(dxCommon, "field");
	gameObject_->SetName("field");

	// 初期位置設定
	Vector3Transform defaultTransform{
		{1.0f, 1.0f, 1.0f},	// scale
		{0.0f, 0.0f, 0.0f},	// rotate
		position			// translate
	};
	gameObject_->SetTransform(defaultTransform);

	gameObject_->SetColor(0x3F3A38FF);
}

void Ground::Update(const Matrix4x4& viewProjectionMatrix) {
	// ゲームオブジェクトの更新
	gameObject_->Update(viewProjectionMatrix);
}

void Ground::Draw() {
	// 地面の描画
	gameObject_->Draw();

}

void Ground::ImGui()
{
#ifdef USEIMGUI
	if (gameObject_) {
		gameObject_->ImGui();
	}

#endif
}
