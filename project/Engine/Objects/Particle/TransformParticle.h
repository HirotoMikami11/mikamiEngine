#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cassert>

#include "DirectXCommon.h"
#include "MyFunction.h"
#include "Logger.h"

class TransformParticle final
{

public:
	TransformParticle() = default;
	~TransformParticle() = default;

	/// <summary>
	/// トランスフォームの初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonの初期化</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 行列の更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション</param>
	void UpdateMatrix(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// デフォルトの値を設定する
	/// </summary>
	void SetDefaultTransform();

	/// <summary>
	/// ストラクチャードバッファー用のSRVを作成する
	/// </summary>
	void MakeStructuredBufferSRV(DirectXCommon* dxCommon);

	//Getter
	const Vector3Transform& GetTransform() const { return transform_[0]; }

	Vector3 GetPosition() const { return transform_[0].translate; }
	Vector3 GetRotation() const { return transform_[0].rotate; }
	Vector3 GetScale() const { return transform_[0].scale; }

	Matrix4x4 GetWorldMatrix() const { return transformData_->World; };
	Matrix4x4 GetWVPMatrix() const { return transformData_->WVP; };
	ID3D12Resource* GetResource() const { return transformResource_.Get(); }
	DescriptorHeapManager::DescriptorHandle GetSRV() const { return srvHandle; }
	///トランスフォームデータの直接取得（ImGui用）
	TransformationMatrix* GetTransformDataPtr() const { return transformData_; }

	//Setter
	void SetTransform(const Vector3Transform& newTransform) { transform_[0] = newTransform; }
	void SetScale(const Vector3& scale) { transform_[0].scale = scale; }
	void SetRotation(const Vector3& rotate) { transform_[0].rotate = rotate; }
	void SetPosition(const Vector3& translate) { transform_[0].translate = translate; }

	/// <summary>
	/// 親オブジェクトを設定
	/// </summary>
	/// <param name="parent">親のTransformParticleへのポインタ</param>
	void SetParent(const TransformParticle* parent) { parent_ = parent; }

	/// <summary>
	/// 親オブジェクトを取得
	/// </summary>
	/// <returns>親のTransformParticleへのポインタ</returns>
	const TransformParticle* GetParent() const { return parent_; }

	///指定した値で回転
	void AddPosition(const Vector3& Position);
	void AddRotation(const Vector3& rotation);
	void AddScale(const Vector3& Scale);

private:

	//一旦生成する量を決めておく
	const uint32_t kNumInstance = 10;

	// GPU用トランスフォームリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> transformResource_;
	// トランスフォームデータへのポインタ（Map済み）
	TransformationMatrix* transformData_;

	//instanceingするような場合のSRV設定
	DescriptorHeapManager::DescriptorHandle srvHandle;

	// CPU側のトランスフォーム値
	Vector3Transform transform_[10];

	// 親となるTransformParticleへのポインタ
	const TransformParticle* parent_ = nullptr;
};