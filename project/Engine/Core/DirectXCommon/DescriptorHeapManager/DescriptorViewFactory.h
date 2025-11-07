#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cstdint>

#include "Logger.h"

/// <summary>
/// ディスクリプタビュー（SRV/RTV/DSV/UAV）の作成を担当するファクトリークラス
/// </summary>
class DescriptorViewFactory {
public:
	DescriptorViewFactory() = default;
	~DescriptorViewFactory() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="device">D3D12デバイス</param>
	void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();


	// SRV作成


	/// <summary>
	/// Texture2D用のSRVを作成
	/// </summary>
	/// <param name="resource">テクスチャリソース</param>
	/// <param name="cpuHandle">作成先のCPUハンドル</param>
	/// <param name="format">フォーマット</param>
	/// <param name="mipLevels">ミップレベル数</param>
	void CreateSRVForTexture2D(
		ID3D12Resource* resource,
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		DXGI_FORMAT format,
		uint32_t mipLevels = 1);

	/// <summary>
	/// Texture3D用のSRVを作成
	/// </summary>
	/// <param name="resource">3Dテクスチャリソース</param>
	/// <param name="cpuHandle">作成先のCPUハンドル</param>
	/// <param name="format">フォーマット</param>
	/// <param name="mipLevels">ミップレベル数</param>
	void CreateSRVForTexture3D(
		ID3D12Resource* resource,
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		DXGI_FORMAT format,
		uint32_t mipLevels = 1);

	/// <summary>
	/// TextureCube用のSRVを作成
	/// </summary>
	/// <param name="resource">キューブマップリソース</param>
	/// <param name="cpuHandle">作成先のCPUハンドル</param>
	/// <param name="format">フォーマット</param>
	/// <param name="mipLevels">ミップレベル数</param>
	void CreateSRVForTextureCube(
		ID3D12Resource* resource,
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		DXGI_FORMAT format,
		uint32_t mipLevels = 1);

	/// <summary>
	/// StructuredBuffer用のSRVを作成
	/// </summary>
	/// <param name="resource">バッファリソース</param>
	/// <param name="cpuHandle">作成先のCPUハンドル</param>
	/// <param name="numElements">要素数</param>
	/// <param name="structureByteStride">1要素のバイトサイズ</param>
	void CreateSRVForStructuredBuffer(
		ID3D12Resource* resource,
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		uint32_t numElements,
		uint32_t structureByteStride);

	/// <summary>
	/// RawBuffer（ByteAddressBuffer）用のSRVを作成
	/// </summary>
	/// <param name="resource">バッファリソース</param>
	/// <param name="cpuHandle">作成先のCPUハンドル</param>
	/// <param name="numElements">要素数（4バイト単位）</param>
	void CreateSRVForRawBuffer(
		ID3D12Resource* resource,
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		uint32_t numElements);

	/// <summary>
	/// TypedBuffer用のSRVを作成
	/// </summary>
	/// <param name="resource">バッファリソース</param>
	/// <param name="cpuHandle">作成先のCPUハンドル</param>
	/// <param name="format">バッファのフォーマット</param>
	/// <param name="numElements">要素数</param>
	void CreateSRVForTypedBuffer(
		ID3D12Resource* resource,
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		DXGI_FORMAT format,
		uint32_t numElements);


	// RTV作成

	/// <summary>
	/// Texture2D用のRTVを作成
	/// </summary>
	/// <param name="resource">レンダーターゲットリソース</param>
	/// <param name="cpuHandle">作成先のCPUハンドル</param>
	/// <param name="format">フォーマット</param>
	void CreateRTVForTexture2D(
		ID3D12Resource* resource,
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		DXGI_FORMAT format);


	// DSV作成

	/// <summary>
	/// Texture2D用のDSVを作成
	/// </summary>
	/// <param name="resource">深度ステンシルリソース</param>
	/// <param name="cpuHandle">作成先のCPUハンドル</param>
	/// <param name="format">フォーマット</param>
	void CreateDSVForTexture2D(
		ID3D12Resource* resource,
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		DXGI_FORMAT format);


	// UAV作成


	/// <summary>
	/// Texture2D用のUAVを作成
	/// </summary>
	/// <param name="resource">テクスチャリソース</param>
	/// <param name="cpuHandle">作成先のCPUハンドル</param>
	/// <param name="format">フォーマット</param>
	void CreateUAVForTexture2D(
		ID3D12Resource* resource,
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		DXGI_FORMAT format);

	/// <summary>
	/// StructuredBuffer用のUAVを作成
	/// </summary>
	/// <param name="resource">バッファリソース</param>
	/// <param name="cpuHandle">作成先のCPUハンドル</param>
	/// <param name="numElements">要素数</param>
	/// <param name="structureByteStride">1要素のバイトサイズ</param>
	void CreateUAVForStructuredBuffer(
		ID3D12Resource* resource,
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		uint32_t numElements,
		uint32_t structureByteStride);

	/// <summary>
	/// 初期化済みかチェック
	/// </summary>
	bool IsInitialized() const { return isInitialized_; }

private:
	// D3D12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device_;

	// 初期化フラグ
	bool isInitialized_ = false;
};