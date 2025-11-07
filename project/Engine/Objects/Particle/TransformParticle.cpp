#include "TransformParticle.h"

void TransformParticle::Initialize(DirectXCommon* dxCommon)
{
	const uint32_t kNumInstance = 10;
	// トランスフォーム用のリソースを作成
	transformResource_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformationMatrix) * kNumInstance);

	// トランスフォームデータにマップ
	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformData_));

	// デフォルト設定で初期化
	SetDefaultTransform();

	// SRVの作成
	MakeStructuredBufferSRV(dxCommon);
}

void TransformParticle::UpdateMatrix(const Matrix4x4& viewProjectionMatrix)
{
	for (size_t index = 0; index < kNumInstance; ++index)
	{
		transformData_[index].World = MakeAffineMatrix(
			transform_[index].scale,
			transform_[index].rotate,
			transform_[index].translate);

		if (parent_) {
			transformData_[index].World = Matrix4x4Multiply(
				transformData_[index].World,
				parent_->GetWorldMatrix());
		}

		transformData_[index].WVP = Matrix4x4Multiply(
			transformData_[index].World,
			viewProjectionMatrix);
	}
}

void TransformParticle::MakeStructuredBufferSRV(DirectXCommon* dxCommon)
{
	auto descriptorManager = dxCommon->GetDescriptorManager();

	// 構造化バッファ用のSRVを作成
	srvHandle = descriptorManager->CreateSRVForStructuredBuffer(
		transformResource_.Get(),
		kNumInstance,
		sizeof(TransformationMatrix)
	);

	// エラーチェック
	if (!srvHandle.isValid) {
		Logger::Log(Logger::GetStream(), "Failed to create SRV for TransformParticle\n");
		assert(false && "SRV creation failed");
	}

}

void TransformParticle::SetDefaultTransform() {
	// デフォルト値に設定
	for (size_t index = 0; index < kNumInstance; ++index)
	{
		// GPU側のデータも単位行列で初期化
		transformData_->World = MakeIdentity4x4();
		transformData_->WVP = MakeIdentity4x4();
		transform_[index].scale = { 1.0f, 1.0f, 1.0f };
		transform_[index].rotate = { 0.0f, 0.0f, 0.0f };
		transform_[index].translate = { index * 0.1f, index * -0.1f, index * 0.1f };
	}
}

void TransformParticle::AddPosition(const Vector3& Position)
{
	for (size_t index = 0; index < kNumInstance; ++index)
	{
		transform_[index].translate.x += Position.x;
		transform_[index].translate.y += Position.y;
		transform_[index].translate.z += Position.z;
	}
}

void TransformParticle::AddRotation(const Vector3& rotation)
{
	for (size_t index = 0; index < kNumInstance; ++index)
	{
		transform_[index].rotate.x += rotation.x;
		transform_[index].rotate.y += rotation.y;
		transform_[index].rotate.z += rotation.z;
	}
}

void TransformParticle::AddScale(const Vector3& Scale)
{
	for (size_t index = 0; index < kNumInstance; ++index)
	{
		transform_[index].scale.x += Scale.x;
		transform_[index].scale.y += Scale.y;
		transform_[index].scale.z += Scale.z;
	}
}