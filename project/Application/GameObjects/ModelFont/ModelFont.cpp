#include "ModelFont.h"
#include <numbers>

ModelFont::ModelFont()
{
}

ModelFont::~ModelFont()
{
}

void ModelFont::Initialize(DirectXCommon* dxCommon, const std::string& modelTag, const Vector3& position)
{
	directXCommon_ = dxCommon;

	// ゲームオブジェクト（3Dモデル）の初期化
	gameObject_ = std::make_unique<Model3D>();
	gameObject_->Initialize(dxCommon, modelTag);
	gameObject_->SetName(modelTag);

	// 初期位置設定
	Vector3Transform defaultTransform{
		{1.0f, 1.0f, 1.0f},	// scale
		{0.0f,  std::numbers::pi_v<float>, 0.0f},	// rotate
		position			// translate
	};
	gameObject_->SetTransform(defaultTransform);

	gameObject_->SetColor(0xFFFFFFFF);
}

void ModelFont::Update(const Matrix4x4& viewProjectionMatrix) {
	// ゲームオブジェクトの更新
	gameObject_->Update(viewProjectionMatrix);
}

void ModelFont::Draw(const Light& directionalLight) {
	// 地面の描画
	gameObject_->Draw(directionalLight);
}

void ModelFont::ImGui()
{
#ifdef USEIMGUI
	if (gameObject_) {
		gameObject_->ImGui();
	}

#endif
}
