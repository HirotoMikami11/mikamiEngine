#include "Camera.h"
#include "Managers/ImGui/ImGuiManager.h"
#include <numbers>

NormalCamera::NormalCamera()
	: directXCommon_(nullptr)
	, cameraTransform_{}
	, cameraMatrix_{}
	, viewProjectionMatrix_{}
	, spriteViewProjectionMatrix_{}
	, viewMatrix_{}
	, projectionMatrix_{}
	, spriteProjectionMatrix_{}
	, useSpriteViewProjectionMatrix_(true)
	, cameraForGPUData_(nullptr) {
}

NormalCamera::~NormalCamera() {
	// CameraForGPUリソースをUnmap
	if (cameraForGPUResource_ && cameraForGPUData_) {
		cameraForGPUResource_->Unmap(0, nullptr);
		cameraForGPUData_ = nullptr;
	}
}

void NormalCamera::Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& rotation) {
	// DirectXCommonを保存
	directXCommon_ = dxCommon;

	// 初期値を保存
	initialPosition_ = position;
	initialRotation_ = rotation;
	// 指定座標・回転でデフォルト値を設定
	SetDefaultCamera(position, rotation);

	// CameraForGPU用のリソースを作成
	cameraForGPUResource_ = CreateBufferResource(directXCommon_->GetDevice(), sizeof(CameraForGPU));
	cameraForGPUResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPUData_));

	// 初期化
	UpdateCameraForGPU();
}

void NormalCamera::Update() {
	// カメラの行列を更新
	UpdateMatrix();
	// スプライト用の行列を更新
	UpdateSpriteMatrix();
	// カメラ情報（GPU用）を更新
	UpdateCameraForGPU();
}

void NormalCamera::SetDefaultCamera(const Vector3& position, const Vector3& rotation) {
	// デフォルト値に設定（座標・回転は引数で指定）
	cameraTransform_.scale = { 1.0f, 1.0f, 1.0f };
	cameraTransform_.rotate = rotation;
	cameraTransform_.translate = position;

	// カメラパラメータのデフォルト値
	fov_ = 0.45f;
	nearClip_ = 0.1f;
	farClip_ = 1000.0f;
	aspectRatio_ = (float(GraphicsConfig::kClientWidth) / float(GraphicsConfig::kClientHeight));

	// 初期行列計算
	cameraMatrix_ = MakeAffineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate);
	viewMatrix_ = Matrix4x4Inverse(cameraMatrix_);

	// プロジェクション行列は最初に作っておく
	projectionMatrix_ = MakePerspectiveFovMatrix(fov_, aspectRatio_, nearClip_, farClip_);
	spriteProjectionMatrix_ = MakeOrthograpicMatrix(
		0.0f, 0.0f,
		float(GraphicsConfig::kClientWidth),
		float(GraphicsConfig::kClientHeight),
		0.0f, 1000.0f
	);

	viewProjectionMatrix_ = Matrix4x4Multiply(viewMatrix_, projectionMatrix_);
	spriteViewProjectionMatrix_ = MakeIdentity4x4();

	// スプライトを画面に表示できるように初期化
	useSpriteViewProjectionMatrix_ = true;
}

void NormalCamera::UpdateMatrix() {
	// 3D用のビュープロジェクション行列を計算
	cameraMatrix_ = MakeAffineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate);
	viewMatrix_ = Matrix4x4Inverse(cameraMatrix_);
	viewProjectionMatrix_ = Matrix4x4Multiply(viewMatrix_, projectionMatrix_);
}

void NormalCamera::UpdateSpriteMatrix() {
	// フラグで使うと判断したときだけスプライトの行列を計算する
	if (useSpriteViewProjectionMatrix_) {
		// スプライト用のビュープロジェクション行列を計算
		Matrix4x4 spriteViewMatrix = MakeIdentity4x4();
		spriteViewProjectionMatrix_ = Matrix4x4Multiply(spriteViewMatrix, spriteProjectionMatrix_);
	}
}

void NormalCamera::UpdateCameraForGPU() {
	if (cameraForGPUData_) {
		cameraForGPUData_->worldPosition = cameraTransform_.translate;
		cameraForGPUData_->padding = 0.0f;
	}
}

void NormalCamera::LookAt(const Vector3& target, const Vector3& up) {
	// カメラ位置からターゲットへの方向ベクトルを計算
	Vector3 forward = Normalize(Subtract(target, cameraTransform_.translate));

	// Y軸回転（Yaw）を計算
	float yaw = atan2f(forward.x, forward.z);

	// X軸回転（Pitch）を計算
	float pitch = asinf(-forward.y);

	// 回転角度を設定
	cameraTransform_.rotate = { pitch, yaw, 0.0f };
}

void NormalCamera::ImGui() {
#ifdef USEIMGUI
	ImGui::Text("NormalCamera");
	ImGui::Separator();

	// カメラの位置と回転を表示・編集
	if (ImGui::DragFloat3("Position", &cameraTransform_.translate.x, 0.1f, -1000.0f, 1000.0f)) {
		// 位置変更時は行列を更新
		UpdateMatrix();
	}

	if (ImGui::DragFloat3("Rotation", &cameraTransform_.rotate.x, 0.01f, -std::numbers::pi_v<float>, std::numbers::pi_v<float>)) {
		// 回転変更時は行列を更新
		UpdateMatrix();
	}

	ImGui::Separator();

	// カメラパラメータ
	if (ImGui::SliderFloat("FOV", &fov_, 0.1f, 3.0f) ||
		ImGui::SliderFloat("Near Clip", &nearClip_, 0.01f, 10.0f) ||
		ImGui::SliderFloat("Far Clip", &farClip_, 10.0f, 1000.0f)) {

		// パラメータ変更時はプロジェクション行列を再計算
		projectionMatrix_ = MakePerspectiveFovMatrix(fov_, aspectRatio_, nearClip_, farClip_);
		UpdateMatrix();
	}

	ImGui::Separator();

	ImGui::Text("Camera Type: %s", GetCameraType().c_str());
	ImGui::Separator();

	// リセットボタン
	if (ImGui::Button("Reset Camera")) {
		SetDefaultCamera(initialPosition_, initialRotation_);
	}
#endif
}