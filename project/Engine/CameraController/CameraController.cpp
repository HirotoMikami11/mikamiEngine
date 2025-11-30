#include "CameraController.h"
#include "ImGui/ImGuiManager.h"
#include "Input.h"
#include <algorithm>
#include <vector>

CameraController* CameraController::GetInstance() {
	static CameraController instance;
	return &instance;
}

void CameraController::Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& rotation) {
	RegisterBuiltInCameras(dxCommon, position, rotation);
	SetActiveCamera("normal");
}

void CameraController::Finalize() {
	// 全てのカメラを破棄してリソース解放
	registeredCameras_.clear();

	// アクティブカメラIDをリセット
	activeCameraId_ = "normal";
	lastActiveCameraId_ = "normal";
}

void CameraController::RegisterBuiltInCameras(DirectXCommon* dxCommon, const Vector3& initialPosition, const Vector3& initialRotation) {
	// Normalカメラを登録
	auto normalCamera = std::make_unique<NormalCamera>();
	normalCamera->Initialize(dxCommon, initialPosition, initialRotation);
	registeredCameras_["normal"] = { std::move(normalCamera), true };

	// Debugカメラを登録
	auto debugCamera = std::make_unique<DebugCamera>();
	debugCamera->Initialize(dxCommon, initialPosition, initialRotation);
	registeredCameras_["debug"] = { std::move(debugCamera), true };
}

void CameraController::RegisterCamera(const std::string& cameraId, std::unique_ptr<BaseCamera> camera) {
	if (!camera) {
		return;
	}

	registeredCameras_[cameraId] = { std::move(camera), false };
}

void CameraController::UnregisterCamera(const std::string& cameraId) {
	auto it = registeredCameras_.find(cameraId);
	if (it == registeredCameras_.end()) {
		return;
	}

	// エンジン標準カメラは削除不可
	if (it->second.isEngineCamera) {
		return;
	}

	// 現在アクティブなカメラを削除する場合は、normalカメラに切り替え
	if (activeCameraId_ == cameraId) {
		activeCameraId_ = "normal";
	}

	registeredCameras_.erase(it);
}

void CameraController::SetActiveCamera(const std::string& cameraId) {
	auto it = registeredCameras_.find(cameraId);
	if (it == registeredCameras_.end()) {
		return;
	}

	if (!it->second.camera->IsActive()) {
		return;
	}

	activeCameraId_ = cameraId;
}

void CameraController::Update() {
	// カメラシェイク更新（固定60FPS想定、実際はdeltaTimeを使用）
	cameraShake_.Update(1.0f / 60.0f);

	// アクティブなカメラのみ更新
	BaseCamera* activeCamera = GetActiveCamera();
	if (activeCamera) {
		activeCamera->Update();
	}

	// アクティブカメラが無効になった場合はnormalカメラに戻す
	if (!activeCamera || !activeCamera->IsActive()) {
		SetActiveCamera("normal");
		activeCamera = GetActiveCamera();
		if (activeCamera) {
			activeCamera->Update();
		}
	}

	HandleDebugInput();
}

void CameraController::HandleDebugInput() {
#ifdef USEIMGUI
	// Shift + TAB でデバッグカメラの切り替え
	if (Input::GetInstance()->IsKeyTrigger(DIK_TAB) &&
		Input::GetInstance()->IsKeyDown(DIK_LSHIFT)) {
		ToggleDebugCamera();
	}
#endif
}

void CameraController::ToggleDebugCamera() {
	if (activeCameraId_ == "debug") {
		SetActiveCamera(lastActiveCameraId_);
	} else {
		lastActiveCameraId_ = activeCameraId_;
		SetActiveCamera("debug");
	}
}

// === カメラシェイク関連 ===

void CameraController::StartCameraShake(float duration, float amplitude) {
	cameraShake_.StartShake(duration, amplitude);
}

void CameraController::StartMultiCameraShake(float duration, float amplitude, float frequency) {
	cameraShake_.StartMultiShake(duration, amplitude, frequency);
}

void CameraController::StopCameraShake() {
	cameraShake_.StopShake();
}

bool CameraController::IsCameraShaking() const {
	return cameraShake_.IsShaking();
}

// === カメラ情報取得 ===

BaseCamera* CameraController::GetActiveCamera() const {
	auto it = registeredCameras_.find(activeCameraId_);
	return (it != registeredCameras_.end()) ? it->second.camera.get() : nullptr;
}

Matrix4x4 CameraController::GetCameraMatrix() const {
	BaseCamera* activeCamera = GetActiveCamera();
	if (!activeCamera) {
		return MakeIdentity4x4();
	}

	Matrix4x4 cameraMatrix = activeCamera->GetCameraMatrix();

	// シェイクオフセットを適用
	if (cameraShake_.IsShaking()) {
		Vector3 shakeOffset = cameraShake_.GetOffset();
		Matrix4x4 shakeMatrix = MakeTranslateMatrix(shakeOffset);
		cameraMatrix = Matrix4x4Multiply(shakeMatrix, cameraMatrix);
	}

	return cameraMatrix;
}

Matrix4x4 CameraController::GetViewProjectionMatrix() const {
	BaseCamera* activeCamera = GetActiveCamera();
	if (!activeCamera) {
		return MakeIdentity4x4();
	}

	// 通常のビュープロジェクション行列を取得
	Matrix4x4 viewProjection = activeCamera->GetViewProjectionMatrix();

	// シェイクオフセットを適用
	if (cameraShake_.IsShaking()) {
		Vector3 shakeOffset = cameraShake_.GetOffset();

		// シェイクを逆変換としてビュー行列に適用
		Matrix4x4 inverseShakeMatrix = MakeTranslateMatrix(Multiply(shakeOffset,-1.0f));
		viewProjection = Matrix4x4Multiply(inverseShakeMatrix, viewProjection);
	}

	return viewProjection;
}

Matrix4x4 CameraController::GetViewProjectionMatrixSprite() const {
	BaseCamera* activeCamera = GetActiveCamera();
	if (!activeCamera) {
		return MakeIdentity4x4();
	}

	// スプライト用は揺れを適用しない（UI要素は揺らさない）
	return activeCamera->GetSpriteViewProjectionMatrix();
}

Vector3 CameraController::GetPosition() const {
	BaseCamera* activeCamera = GetActiveCamera();
	Vector3 basePosition = activeCamera ? activeCamera->GetPosition() : Vector3{ 0.0f, 0.0f, 0.0f };

	// シェイクオフセットを加算
	if (cameraShake_.IsShaking()) {
		return Add(basePosition, cameraShake_.GetOffset());
	}

	return basePosition;
}

void CameraController::SetPosition(const Vector3& position) {
	BaseCamera* activeCamera = GetActiveCamera();
	if (activeCamera) {
		activeCamera->SetPosition(position);
	}
}

Vector3 CameraController::GetForward() const {
	BaseCamera* activeCamera = GetActiveCamera();
	if (!activeCamera) {
		return Vector3{ 0.0f, 0.0f, 1.0f }; // デフォルトの前方向
	}

	// カメラの回転情報から前方向ベクトルを計算
	Vector3 rotation = activeCamera->GetRotation();

	// 回転角度から前方向ベクトルを計算
	Vector3 forward;
	forward.x = std::sin(rotation.y) * std::cos(rotation.x);
	forward.y = -std::sin(rotation.x);
	forward.z = std::cos(rotation.y) * std::cos(rotation.x);

	// 正規化して返す
	return Normalize(forward);
}

ID3D12Resource* CameraController::GetCameraForGPUResource() const {
	BaseCamera* activeCamera = GetActiveCamera();
	return activeCamera ? activeCamera->GetCameraForGPUResource() : nullptr;
}

bool CameraController::IsRegistered(const std::string& cameraId) const {
	return registeredCameras_.find(cameraId) != registeredCameras_.end();
}

BaseCamera* CameraController::GetCamera(const std::string& cameraId) const {
	auto it = registeredCameras_.find(cameraId);
	return (it != registeredCameras_.end()) ? it->second.camera.get() : nullptr;
}

std::vector<std::string> CameraController::GetRegisteredCameraIds() const {
	std::vector<std::string> ids;
	ids.reserve(registeredCameras_.size());

	for (const auto& [id, info] : registeredCameras_) {
		ids.push_back(id);
	}

	return ids;
}

Matrix4x4 CameraController::GetViewMatrixWithShake() const {
	BaseCamera* activeCamera = GetActiveCamera();
	if (!activeCamera) {
		return MakeIdentity4x4();
	}

	// 基本のビュー行列を取得
	Matrix4x4 viewMatrix = Matrix4x4Inverse(activeCamera->GetCameraMatrix());

	// シェイクオフセットを適用
	if (cameraShake_.IsShaking()) {
		Vector3 shakeOffset = cameraShake_.GetOffset();
		Matrix4x4 shakeMatrix = MakeTranslateMatrix(Multiply(shakeOffset, -1.0f));
		viewMatrix = Matrix4x4Multiply(shakeMatrix, viewMatrix);
	}

	return viewMatrix;
}

void CameraController::ImGui() {
#ifdef USEIMGUI
	ImGui::Begin("CameraController");

	// 現在の状態表示
	ImGui::Text("Active Camera: %s", activeCameraId_.c_str());
	BaseCamera* activeCamera = GetActiveCamera();
	if (activeCamera) {
		ImGui::Text("Camera Type: %s", activeCamera->GetCameraType().c_str());
		Vector3 pos = activeCamera->GetPosition();
		ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
		Vector3 forward = GetForward();
		ImGui::Text("Forward: (%.2f, %.2f, %.2f)", forward.x, forward.y, forward.z);
	}

	ImGui::Separator();

	// カメラ切り替えUI
	ImGui::Text("Cameras:");
	for (const auto& [id, info] : registeredCameras_) {
		ImGui::PushID(id.c_str());

		bool isActive = (id == activeCameraId_);
		bool isAvailable = info.camera && info.camera->IsActive();

		// アクティブなカメラの場合は緑色のボタンにする
		if (isActive) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.8f, 0.2f, 0.8f));
		}

		// ボタンサイズ
		if (ImGui::Button(id.c_str(), ImVec2(120, 0))) {
			if (isAvailable && !isActive) {
				SetActiveCamera(id);
			}
		}

		// アクティブなカメラの場合は色をリセット
		if (isActive) {
			ImGui::PopStyleColor(2);
			ImGui::SameLine();
			ImGui::Text("(Active)");
		} else {
			ImGui::SameLine();
			ImGui::Text("(%s%s)",
				isAvailable ? "Available" : "Unavailable",
				info.isEngineCamera ? " Engine" : "");
		}

		ImGui::PopID();
	}

	ImGui::Separator();

	// アクティブカメラのImGui
	if (activeCamera) {
		MyImGui::CenterText("Camera Data");
		ImGui::Separator();
		activeCamera->ImGui();
	}

	ImGui::Separator();
	// カメラシェイクのImGui表示
	cameraShake_.ImGui();

	ImGui::Separator();
	// 操作説明
	ImGui::Text("Controls:");
	ImGui::Text("Shift + TAB: Toggle Debug Camera");

	ImGui::End();
#endif
}

