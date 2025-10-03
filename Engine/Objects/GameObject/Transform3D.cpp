#include "Transform3D.h"

void Transform3D::Initialize(DirectXCommon* dxCommon)
{
	const uint32_t kNumInstance = 10;
	// トランスフォーム用のリソースを作成
	transformResource_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformationMatrix) * kNumInstance);

	// トランスフォームデータにマップ
	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformData_));

	// デフォルト設定で初期化
	SetDefaultTransform();
}

void Transform3D::UpdateMatrix(const Matrix4x4& viewProjectionMatrix)
{
	for (size_t index = 0; index < kNumInstance; ++index)
	{

		// トランスフォームデータを更新（ローカル→ワールド変換行列）
		transformData_->World = MakeAffineMatrix(transform_[index].scale, transform_[index].rotate, transform_[index].translate);

		// 親があれば親のワールド行列を掛ける
		if (parent_) {
			transformData_->World = Matrix4x4Multiply(transformData_->World, parent_->GetWorldMatrix());
		}

		// ビュープロジェクション行列を掛け算してWVP行列を計算
		transformData_->WVP = Matrix4x4Multiply(transformData_->World, viewProjectionMatrix);
	}
}


void Transform3D::MakeStructuredBufferSRV(DirectXCommon* dxCommon)
{


	///textureとSRVの設定が異なる
	// SRV設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = kNumInstance;
	srvDesc.Buffer.StructureByteStride = sizeof(TransformationMatrix); //構造化バッファの1要素分のサイズ

	srvHandle = dxCommon->GetDescriptorManager()->AllocateSRV();
	// SRVの生成
	dxCommon->GetDevice()->CreateShaderResourceView(transformResource_.Get(), &srvDesc, srvHandle.cpuHandle);
}


void Transform3D::SetDefaultTransform() {

	// デフォルト値に設定

	for (size_t index = 0; index < kNumInstance; ++index)
	{	
		// GPU側のデータも単位行列で初期化
		transformData_->World = MakeIdentity4x4();
		transformData_->WVP = MakeIdentity4x4();
		transform_[index].scale = { 1.0f, 1.0f, 1.0f };
		transform_[index].rotate = { 0.0f, 0.0f, 0.0f };
		transform_[index].translate = { index * 0.1f, 0.0f, 0.0f };

	}


}

void Transform3D::AddPosition(const Vector3& Position)
{
	for (size_t index = 0; index < kNumInstance; ++index)
	{
		transform_[index].translate.x += Position.x;
		transform_[index].translate.y += Position.y;
		transform_[index].translate.z += Position.z;
	}

}

void Transform3D::AddRotation(const Vector3& rotation)
{
	for (size_t index = 0; index < kNumInstance; ++index)
	{
		transform_[index].rotate.x += rotation.x;
		transform_[index].rotate.y += rotation.y;
		transform_[index].rotate.z += rotation.z;
	}


}

void Transform3D::AddScale(const Vector3& Scale)
{
	for (size_t index = 0; index < kNumInstance; ++index)
	{
		transform_[index].scale.x += Scale.x;
		transform_[index].scale.y += Scale.y;
		transform_[index].scale.z += Scale.z;
	}

}