#pragma once
#include "BaseCamera.h"
#include "BaseSystem/DirectXCommon/DirectXCommon.h"

/// <summary>
/// 通常カメラ(定点から指定方向を見るカメラ)
/// </summary>
class NormalCamera : public BaseCamera {
public:

	NormalCamera();
	~NormalCamera() override;

	/// <summary>
	/// カメラの初期化
	/// </summary>
	/// <param name="position">初期位置</param>
	/// <param name="rotation">初期回転（デフォルト：{0,0,0}）</param>
	void Initialize(const Vector3& position, const Vector3& rotation = { 0.0f, 0.0f, 0.0f }) override;

	/// <summary>
	/// カメラの更新
	/// </summary>
	void Update() override;

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui() override;


	Matrix4x4 GetViewProjectionMatrix() const override { return viewProjectionMatrix_; }
	Matrix4x4 GetSpriteViewProjectionMatrix() const override { return spriteViewProjectionMatrix_; }
	Vector3 GetPosition() const override { return cameraTransform_.translate; }
	Vector3 GetRotation() const override { return cameraTransform_.rotate; }
	void SetPosition(const Vector3& position) override { cameraTransform_.translate = position; }
	std::string GetCameraType() const override { return "Normal"; }

	// NormalCamera固有機能
	// カメラの回転を設定
	void SetRotation(const Vector3& rotation) { cameraTransform_.rotate = rotation; }
	// トランスフォーム全体を設定
	void SetTransform(const Vector3Transform& transform) { cameraTransform_ = transform; }

	/// <summary>
	/// カメラを指定した座標を向くように設定
	/// </summary>
	/// <param name="target">注視点</param>
	/// <param name="up">上方向ベクトル</param>
	void LookAt(const Vector3& target, const Vector3& up = { 0.0f, 1.0f, 0.0f });



private:

	// 3Dカメラ用のトランスフォーム
	Vector3Transform cameraTransform_;

	// 初期値保存用
	Vector3 initialPosition_;
	Vector3 initialRotation_;

	float fov_ = 0.45f;
	float nearClip_ = 0.1f;
	float farClip_ = 1000.0f;
	float aspectRatio_ = (float(GraphicsConfig::kClientWidth) / float(GraphicsConfig::kClientHeight));

	// 行列 
	Matrix4x4 viewMatrix_;
	//プロジェクション行列
	Matrix4x4 projectionMatrix_;			//3D用のプロジェクション行列
	Matrix4x4 spriteProjectionMatrix_;		//スプライト用のプロジェクション行列
	//ビュープロジェクション行列
	Matrix4x4 viewProjectionMatrix_;		// 3D用ビュープロジェクション行列
	Matrix4x4 spriteViewProjectionMatrix_;	// スプライト用ビュープロジェクション行列
	// スプライト用ビュープロジェクション行列を使用するかどうか
	bool useSpriteViewProjectionMatrix_;

	/// <summary>
	/// 指定座標・回転でデフォルト値を設定
	/// </summary>
	/// <param name="position">初期座標</param>
	/// <param name="rotation">初期回転</param>
	void SetDefaultCamera(const Vector3& position, const Vector3& rotation = { 0.0f, 0.0f, 0.0f });

	/// <summary>
	/// 3D用行列の更新
	/// </summary>
	void UpdateMatrix();

	/// <summary>
	/// スプライト用行列の更新
	/// </summary>
	void UpdateSpriteMatrix();
};