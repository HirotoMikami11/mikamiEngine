#include "Texture.h"
#include "Logger.h"
#include "StringUtility.h"

bool Texture::LoadTexture(const std::string& filePath, DirectXCommon* dxCommon, uint32_t srvIndex) {
	// 既に読み込み済みの場合はスキップ
	if (IsValid() && filePath_ == filePath) {
		return true;
	}

	filePath_ = filePath;

	// テクスチャファイルを読み込み
	DirectX::ScratchImage mipImages = LoadTextureFile(filePath);
	if (mipImages.GetImageCount() == 0) {
		return false;
	}

	// メタデータを保存
	metadata_ = mipImages.GetMetadata();

	// テクスチャリソースを作成
	textureResource_ = CreateTextureResource(dxCommon->GetDeviceComPtr(), metadata_);
	if (!textureResource_) {
		return false;
	}

	// テクスチャデータをアップロード
	intermediateResource_ = UploadTextureData(
		textureResource_,
		mipImages,
		dxCommon->GetDeviceComPtr(),
		dxCommon->GetCommandListComPtr());

	auto descriptorManager = dxCommon->GetDescriptorManager();
	if (!descriptorManager) {
		Logger::Log(Logger::GetStream(), "DescriptorManager is null\n");
		return false;
	}

	// 指定されたインデックスでSRVを作成
	auto handleOpt = descriptorManager->CreateSRVForTexture2DAtIndex(
		textureResource_.Get(),
		metadata_.format,
		srvIndex,
		static_cast<uint32_t>(metadata_.mipLevels)
	);

	if (!handleOpt.has_value()) {
		Logger::Log(Logger::GetStream(), std::format("Failed to create SRV at index: {}\n", srvIndex));
		return false;
	}

	// ハンドルを保存
	descriptorHandle_ = handleOpt.value();
	cpuHandle_ = descriptorHandle_.cpuHandle;
	gpuHandle_ = descriptorHandle_.gpuHandle;
	srvIndex_ = srvIndex;

	Logger::Log(Logger::GetStream(), std::format("Texture loaded: {}\n", filePath));
	return true;
}

bool Texture::LoadTextureWithHandle(
	const std::string& filePath,
	DirectXCommon* dxCommon,
	const DescriptorHeapManager::DescriptorHandle& descriptorHandle) {

	// 既に読み込み済みの場合はスキップ
	if (IsValid() && filePath_ == filePath) {
		return true;
	}

	filePath_ = filePath;

	// テクスチャファイルを読み込み
	DirectX::ScratchImage mipImages = LoadTextureFile(filePath);
	if (mipImages.GetImageCount() == 0) {
		return false;
	}

	// メタデータを保存
	metadata_ = mipImages.GetMetadata();

	// テクスチャリソースを作成
	textureResource_ = CreateTextureResource(dxCommon->GetDeviceComPtr(), metadata_);
	if (!textureResource_) {
		return false;
	}

	// テクスチャデータをアップロード
	intermediateResource_ = UploadTextureData(
		textureResource_,
		mipImages,
		dxCommon->GetDeviceComPtr(),
		dxCommon->GetCommandListComPtr());

	auto descriptorManager = dxCommon->GetDescriptorManager();
	if (!descriptorManager) {
		Logger::Log(Logger::GetStream(), "DescriptorManager is null\n");
		return false;
	}

	// 既に割り当て済みのハンドルでSRVを作成
	descriptorManager->CreateSRVForTexture2DWithHandle(
		descriptorHandle,
		textureResource_.Get(),
		metadata_.format,
		static_cast<uint32_t>(metadata_.mipLevels)
	);

	// ハンドルを保存
	descriptorHandle_ = descriptorHandle;
	cpuHandle_ = descriptorHandle_.cpuHandle;
	gpuHandle_ = descriptorHandle_.gpuHandle;
	srvIndex_ = descriptorHandle_.index;

	Logger::Log(Logger::GetStream(), std::format("Texture loaded with handle: {}\n", filePath));
	return true;
}

void Texture::Unload(DirectXCommon* dxCommon) {
	if (!IsValid()) {
		return;
	}

	// SRVを解放
	if (srvIndex_ != INVALID_INDEX) {
		auto descriptorManager = dxCommon->GetDescriptorManager();
		if (descriptorManager) {
			descriptorManager->ReleaseSRV(srvIndex_);
		}
		srvIndex_ = INVALID_INDEX;
	}

	// リソースをクリア
	textureResource_.Reset();
	intermediateResource_.Reset();

	// ハンドルをクリア
	cpuHandle_ = {};
	gpuHandle_ = {};
	descriptorHandle_ = {};

	// その他の情報をクリア
	metadata_ = {};
	filePath_.clear();

	Logger::Log(Logger::GetStream(), std::format("Texture unloaded: {}\n", filePath_));
}

DirectX::ScratchImage Texture::LoadTextureFile(const std::string& filePath) {
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);

	if (FAILED(hr)) {
		Logger::Log(Logger::GetStream(), std::format("Failed to load texture: {}\n", filePath));
		return DirectX::ScratchImage{};
	}

	// ミップマップ生成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(
		image.GetImages(),
		image.GetImageCount(),
		image.GetMetadata(),
		DirectX::TEX_FILTER_SRGB,
		0,
		mipImages);

	if (FAILED(hr)) {
		Logger::Log(Logger::GetStream(), std::format("Failed to generate mipmaps for: {}\n", filePath));
		return image;
	}

	return mipImages;
}

Microsoft::WRL::ComPtr<ID3D12Resource> Texture::CreateTextureResource(
	const Microsoft::WRL::ComPtr<ID3D12Device>& device,
	const DirectX::TexMetadata& metadata) {

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);
	resourceDesc.Height = UINT(metadata.height);
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);
	resourceDesc.Format = metadata.format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&resource));

	if (FAILED(hr)) {
		Logger::Log(Logger::GetStream(), "Failed to create texture resource\n");
		return nullptr;
	}

	return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> Texture::UploadTextureData(
	Microsoft::WRL::ComPtr<ID3D12Resource> texture,
	const DirectX::ScratchImage& mipImages,
	const Microsoft::WRL::ComPtr<ID3D12Device>& device,
	const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList) {

	std::vector<D3D12_SUBRESOURCE_DATA> subresource;
	DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresource);

	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresource.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(device.Get(), intermediateSize);

	UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, UINT(subresource.size()), subresource.data());

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);

	return intermediateResource;
}
