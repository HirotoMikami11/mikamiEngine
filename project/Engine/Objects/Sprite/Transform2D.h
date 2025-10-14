#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cassert>

#include "DirectXCommon.h"
#include "MyFunction.h"
#include "Logger.h"


/// <summary>
/// 2D用のトランスフォーム構造体
/// </summary>
struct Vector2Transform {
	Vector2 scale{ 1.0f, 1.0f };      // X,Yスケール
	float rotateZ = 0.0f;           // Z軸回転のみ
	Vector2 translate{ 0.0f, 0.0f };  // X,Y座標
};


/// <summary>
/// 2Dスプライト専用のTransformクラス
/// </summary>
class Transform2D final
{

public:
	Transform2D() = default;
	~Transform2D() = default;

	/// <summary>
	/// トランスフォームの初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonの初期化</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 行列の更新（2D用）
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション</param>
	void UpdateMatrix(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// デフォルトの値を設定する
	/// </summary>
	void SetDefaultTransform();


	/// <summary>
	/// ImGui用のデバッグ表示
	/// </summary>
	void ImGui();

	// Getter
	const Vector2Transform& GetTransform() const { return transform_; }
	Vector2 GetPosition() const { return transform_.translate; }
	float GetRotation() const { return transform_.rotateZ; }
	Vector2 GetScale() const { return transform_.scale; }
	float GetDepth() const { return 0.0f; }  // 互換性のため常に0を返す

	Matrix4x4 GetWorldMatrix() const { return transformData_->World; }
	Matrix4x4 GetWVPMatrix() const { return transformData_->WVP; }
	ID3D12Resource* GetResource() const { return transformResource_.Get(); }

	/// トランスフォームデータの直接取得（ImGui用）
	TransformationMatrix* GetTransformDataPtr() const { return transformData_; }

	// Setter
	void SetTransform(const Vector2Transform& newTransform) { transform_ = newTransform; }
	void SetScale(const Vector2& scale) { transform_.scale = scale; }
	void SetRotation(float rotateZ) { transform_.rotateZ = rotateZ; }
	void SetPosition(const Vector2& translate) { transform_.translate = translate; }

private:
	// GPU用トランスフォームリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformResource_;
	// トランスフォームデータへのポインタ
	TransformationMatrix* transformData_ = nullptr;

	// CPU側のトランスフォーム値（2D用）
	Vector2Transform transform_{
		.scale{1.0f, 1.0f},
		.rotateZ = 0.0f,
		.translate{0.0f, 0.0f}
	};
};