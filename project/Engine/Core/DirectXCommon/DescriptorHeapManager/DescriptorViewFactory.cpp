#include "DescriptorViewFactory.h"
#include <cassert>
#include <format>

void DescriptorViewFactory::Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device) {
	device_ = device;
	isInitialized_ = true;
	Logger::Log(Logger::GetStream(), "DescriptorViewFactory initialized successfully!\n");
}

void DescriptorViewFactory::Finalize() {
	device_.Reset();
	isInitialized_ = false;
	Logger::Log(Logger::GetStream(), "DescriptorViewFactory finalized.\n");
}

// SRV作成

void DescriptorViewFactory::CreateSRVForTexture2D(
	ID3D12Resource* resource,
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
	DXGI_FORMAT format,
	uint32_t mipLevels) {

	assert(isInitialized_ && "DescriptorViewFactory is not initialized");
	assert(resource != nullptr && "Resource is null");

	// SRV設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = mipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	// SRVの生成
	device_->CreateShaderResourceView(resource, &srvDesc, cpuHandle);

	Logger::Log(Logger::GetStream(),
		std::format("[ViewFactory] Created Texture2D SRV (MipLevels: {})\n", mipLevels));
}

void DescriptorViewFactory::CreateSRVForTexture3D(
	ID3D12Resource* resource,
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
	DXGI_FORMAT format,
	uint32_t mipLevels) {

	assert(isInitialized_ && "DescriptorViewFactory is not initialized");
	assert(resource != nullptr && "Resource is null");

	// SRV設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Texture3D.MipLevels = mipLevels;
	srvDesc.Texture3D.MostDetailedMip = 0;
	srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;

	// SRVの生成
	device_->CreateShaderResourceView(resource, &srvDesc, cpuHandle);

	Logger::Log(Logger::GetStream(),
		std::format("[ViewFactory] Created Texture3D SRV (MipLevels: {})\n", mipLevels));
}

void DescriptorViewFactory::CreateSRVForTextureCube(
	ID3D12Resource* resource,
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
	DXGI_FORMAT format,
	uint32_t mipLevels) {

	assert(isInitialized_ && "DescriptorViewFactory is not initialized");
	assert(resource != nullptr && "Resource is null");

	// SRV設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = mipLevels;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

	// SRVの生成
	device_->CreateShaderResourceView(resource, &srvDesc, cpuHandle);

	Logger::Log(Logger::GetStream(),
		std::format("[ViewFactory] Created TextureCube SRV (MipLevels: {})\n", mipLevels));
}

void DescriptorViewFactory::CreateSRVForStructuredBuffer(
	ID3D12Resource* resource,
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
	uint32_t numElements,
	uint32_t structureByteStride) {

	assert(isInitialized_ && "DescriptorViewFactory is not initialized");
	assert(resource != nullptr && "Resource is null");
	assert(numElements > 0 && "NumElements must be greater than 0");
	assert(structureByteStride > 0 && "StructureByteStride must be greater than 0");

	// SRV設定（構造化バッファ用）
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.StructureByteStride = structureByteStride;

	// SRVの生成
	device_->CreateShaderResourceView(resource, &srvDesc, cpuHandle);

	Logger::Log(Logger::GetStream(),
		std::format("[ViewFactory] Created StructuredBuffer SRV (NumElements: {}, Stride: {})\n",
			numElements, structureByteStride));
}

void DescriptorViewFactory::CreateSRVForRawBuffer(
	ID3D12Resource* resource,
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
	uint32_t numElements) {

	assert(isInitialized_ && "DescriptorViewFactory is not initialized");
	assert(resource != nullptr && "Resource is null");
	assert(numElements > 0 && "NumElements must be greater than 0");

	// SRV設定（RawBuffer用）
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.StructureByteStride = 0;

	// SRVの生成
	device_->CreateShaderResourceView(resource, &srvDesc, cpuHandle);

	Logger::Log(Logger::GetStream(),
		std::format("[ViewFactory] Created RawBuffer SRV (NumElements: {})\n", numElements));
}

void DescriptorViewFactory::CreateSRVForTypedBuffer(
	ID3D12Resource* resource,
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
	DXGI_FORMAT format,
	uint32_t numElements) {

	assert(isInitialized_ && "DescriptorViewFactory is not initialized");
	assert(resource != nullptr && "Resource is null");
	assert(numElements > 0 && "NumElements must be greater than 0");

	// SRV設定（TypedBuffer用）
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.StructureByteStride = 0;

	// SRVの生成
	device_->CreateShaderResourceView(resource, &srvDesc, cpuHandle);

	Logger::Log(Logger::GetStream(),
		std::format("[ViewFactory] Created TypedBuffer SRV (NumElements: {})\n", numElements));
}


// RTV作成

void DescriptorViewFactory::CreateRTVForTexture2D(
	ID3D12Resource* resource,
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
	DXGI_FORMAT format) {

	assert(isInitialized_ && "DescriptorViewFactory is not initialized");
	assert(resource != nullptr && "Resource is null");

	// RTV設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	// RTVの生成
	device_->CreateRenderTargetView(resource, &rtvDesc, cpuHandle);

	Logger::Log(Logger::GetStream(), "[ViewFactory] Created Texture2D RTV\n");
}


// DSV作成

void DescriptorViewFactory::CreateDSVForTexture2D(
	ID3D12Resource* resource,
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
	DXGI_FORMAT format) {

	assert(isInitialized_ && "DescriptorViewFactory is not initialized");
	assert(resource != nullptr && "Resource is null");

	// DSV設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = format;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.Texture2D.MipSlice = 0;

	// DSVの生成
	device_->CreateDepthStencilView(resource, &dsvDesc, cpuHandle);

	Logger::Log(Logger::GetStream(), "[ViewFactory] Created Texture2D DSV\n");
}


// UAV作成


void DescriptorViewFactory::CreateUAVForTexture2D(
	ID3D12Resource* resource,
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
	DXGI_FORMAT format) {

	assert(isInitialized_ && "DescriptorViewFactory is not initialized");
	assert(resource != nullptr && "Resource is null");

	// UAV設定
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = format;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	uavDesc.Texture2D.PlaneSlice = 0;

	// UAVの生成
	device_->CreateUnorderedAccessView(resource, nullptr, &uavDesc, cpuHandle);

	Logger::Log(Logger::GetStream(), "[ViewFactory] Created Texture2D UAV\n");
}

void DescriptorViewFactory::CreateUAVForStructuredBuffer(
	ID3D12Resource* resource,
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
	uint32_t numElements,
	uint32_t structureByteStride) {

	assert(isInitialized_ && "DescriptorViewFactory is not initialized");
	assert(resource != nullptr && "Resource is null");
	assert(numElements > 0 && "NumElements must be greater than 0");
	assert(structureByteStride > 0 && "StructureByteStride must be greater than 0");

	// UAV設定（構造化バッファ用）
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	uavDesc.Buffer.NumElements = numElements;
	uavDesc.Buffer.StructureByteStride = structureByteStride;
	uavDesc.Buffer.CounterOffsetInBytes = 0;

	// UAVの生成
	device_->CreateUnorderedAccessView(resource, nullptr, &uavDesc, cpuHandle);

	Logger::Log(Logger::GetStream(),
		std::format("[ViewFactory] Created StructuredBuffer UAV (NumElements: {}, Stride: {})\n",
			numElements, structureByteStride));
}