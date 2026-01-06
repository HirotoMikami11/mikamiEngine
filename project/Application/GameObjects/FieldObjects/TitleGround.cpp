#include "TitleGround.h"

TitleGround::TitleGround()
{
}

TitleGround::~TitleGround()
{
}

void TitleGround::Initialize(DirectXCommon* dxCommon, const Vector3& position)
{
	dxCommon_ = dxCommon;

	// 初期位置を保存
	initialPosition_ = position;

	// ゲームオブジェクト（3Dモデル）の初期化
	gameObject_ = std::make_unique<Model3D>();
	gameObject_->Initialize(dxCommon, "titleField");
	gameObject_->SetName("field");

	// 初期位置設定
	Vector3Transform defaultTransform{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		position
	};
	gameObject_->SetTransform(defaultTransform);

	gameObject_->SetColor(0x3F3A38FF);
}

void TitleGround::Update(const Matrix4x4& viewProjectionMatrix) {
	// ゲームオブジェクトの更新
	gameObject_->Update(viewProjectionMatrix);
}

void TitleGround::Draw() {
	// 地面の描画
	gameObject_->Draw();

}

void TitleGround::ImGui()
{
#ifdef USEIMGUI
	if (gameObject_) {
		gameObject_->ImGui();
	}

#endif
}

void TitleGround::SetZOffset(float zOffset) {
	zOffset_ = zOffset;
	
	// オフセットを適用した位置を設定
	if (gameObject_) {
		Vector3Transform transform;
		transform.translate= gameObject_->GetTransform().GetPosition();
		transform.scale = gameObject_->GetTransform().GetScale();
		transform.rotate = gameObject_->GetTransform().GetRotation();

		transform.translate = initialPosition_;
		transform.translate.z += zOffset_;
		gameObject_->SetTransform(transform);
	}
}
