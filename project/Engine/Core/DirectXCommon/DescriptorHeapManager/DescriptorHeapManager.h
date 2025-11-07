#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <cassert>
#include <optional>
#include <memory>

#include "GraphicsConfig.h"
#include "Logger.h"
#include "DescriptorViewFactory.h"

/// <summary>
/// ディスクリプタヒープの管理を行うクラス
/// RTVヒープ、DSVヒープ、SRVヒープの統一管理を行う
/// DescriptorViewFactoryと連携してビュー作成を行う
/// </summary>
class DescriptorHeapManager {
public:
	/// <summary>
	/// ディスクリプタヒープの種別
	/// </summary>
	enum class HeapType {
		RTV,		// レンダーターゲットビュー
		DSV,		// デプスステンシルビュー 
		SRV			// シェーダーリソースビュー
	};

	/// <summary>
	/// ディスクリプタハンドル情報
	/// </summary>
	struct DescriptorHandle {
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		uint32_t index;
		bool isValid;

		DescriptorHandle() : cpuHandle{}, gpuHandle{}, index(0), isValid(false) {}
		DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu, uint32_t idx)
			: cpuHandle(cpu), gpuHandle(gpu), index(idx), isValid(true) {
		}
	};

public:
	DescriptorHeapManager() = default;
	~DescriptorHeapManager() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="device">D3D12デバイス</param>
	void Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	//																			//
	//						ディスクリプタヒープ取得								//
	//																			//

	ID3D12DescriptorHeap* GetRTVHeap() const { return rtvHeap_.Get(); }
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetRTVHeapComPtr() const { return rtvHeap_; }

	ID3D12DescriptorHeap* GetDSVHeap() const { return dsvHeap_.Get(); }
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDSVHeapComPtr() const { return dsvHeap_; }

	ID3D12DescriptorHeap* GetSRVHeap() const { return srvHeap_.Get(); }
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSRVHeapComPtr() const { return srvHeap_; }

	//																			//
	//						ディスクリプタサイズ取得								//
	//																			//

	uint32_t GetRTVDescriptorSize() const { return rtvDescriptorSize_; }
	uint32_t GetDSVDescriptorSize() const { return dsvDescriptorSize_; }
	uint32_t GetSRVDescriptorSize() const { return srvDescriptorSize_; }

	//																			//
	//							割り当てと解放										//
	//																			//

	DescriptorHandle AllocateRTV();
	DescriptorHandle AllocateDSV();
	DescriptorHandle AllocateSRV();

	void ReleaseRTV(uint32_t index);
	void ReleaseDSV(uint32_t index);
	void ReleaseSRV(uint32_t index);

	//																			//
	//						指定したインデックスを予約								//
	//																			//

	std::optional<DescriptorHandle> ReserveRTV(uint32_t index);
	std::optional<DescriptorHandle> ReserveDSV(uint32_t index);
	std::optional<DescriptorHandle> ReserveSRV(uint32_t index);

	//																			//
	//							ハンドル取得										//
	//																			//

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(HeapType type, uint32_t index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(HeapType type, uint32_t index) const;

	//																			//
	//						統計情報とチェック									//
	//																			//

	uint32_t GetAvailableCount(HeapType type) const;
	uint32_t GetUsedCount(HeapType type) const;
	bool IsIndexUsed(HeapType type, uint32_t index) const;

	//																			//
	//				SRV作成関数（DescriptorViewFactoryへのラッパー）				//
	//																			//

	/// <summary>
	/// Texture2D用のSRVを作成（自動でSRVを割り当て）
	/// </summary>
	DescriptorHandle CreateSRVForTexture2D(
		ID3D12Resource* resource,
		DXGI_FORMAT format,
		uint32_t mipLevels = 1);

	/// <summary>
	/// Texture2D用のSRVを作成（指定インデックスで予約）
	/// </summary>
	std::optional<DescriptorHandle> CreateSRVForTexture2DAtIndex(
		ID3D12Resource* resource,
		DXGI_FORMAT format,
		uint32_t index,
		uint32_t mipLevels = 1);

	/// <summary>
	/// Texture3D用のSRVを作成（自動でSRVを割り当て）
	/// </summary>
	DescriptorHandle CreateSRVForTexture3D(
		ID3D12Resource* resource,
		DXGI_FORMAT format,
		uint32_t mipLevels = 1);

	/// <summary>
	/// TextureCube用のSRVを作成（自動でSRVを割り当て）
	/// </summary>
	DescriptorHandle CreateSRVForTextureCube(
		ID3D12Resource* resource,
		DXGI_FORMAT format,
		uint32_t mipLevels = 1);

	/// <summary>
	/// StructuredBuffer用のSRVを作成（自動でSRVを割り当て）
	/// </summary>
	DescriptorHandle CreateSRVForStructuredBuffer(
		ID3D12Resource* resource,
		uint32_t numElements,
		uint32_t structureByteStride);

	/// <summary>
	/// StructuredBuffer用のSRVを作成（指定インデックスで予約）
	/// </summary>
	std::optional<DescriptorHandle> CreateSRVForStructuredBufferAtIndex(
		ID3D12Resource* resource,
		uint32_t numElements,
		uint32_t structureByteStride,
		uint32_t index);

	/// <summary>
	/// RawBuffer用のSRVを作成（自動でSRVを割り当て）
	/// </summary>
	DescriptorHandle CreateSRVForRawBuffer(
		ID3D12Resource* resource,
		uint32_t numElements);

	/// <summary>
	/// TypedBuffer用のSRVを作成（自動でSRVを割り当て）
	/// </summary>
	DescriptorHandle CreateSRVForTypedBuffer(
		ID3D12Resource* resource,
		DXGI_FORMAT format,
		uint32_t numElements);

	/// <summary>
	/// 既に割り当て済みのハンドルに対してTexture2D用のSRVを作成
	/// </summary>
	void CreateSRVForTexture2DWithHandle(
		const DescriptorHandle& handle,
		ID3D12Resource* resource,
		DXGI_FORMAT format,
		uint32_t mipLevels = 1);

	/// <summary>
	/// 既に割り当て済みのハンドルに対してStructuredBuffer用のSRVを作成
	/// </summary>
	void CreateSRVForStructuredBufferWithHandle(
		const DescriptorHandle& handle,
		ID3D12Resource* resource,
		uint32_t numElements,
		uint32_t structureByteStride);

	//																			//
	//				RTV/DSV/UAV作成関数（DescriptorViewFactoryへのラッパー）		//
	//																			//

	/// <summary>
	/// Texture2D用のRTVを作成（自動でRTVを割り当て）
	/// </summary>
	DescriptorHandle CreateRTVForTexture2D(
		ID3D12Resource* resource,
		DXGI_FORMAT format);

	/// <summary>
	/// Texture2D用のDSVを作成（自動でDSVを割り当て）
	/// </summary>
	DescriptorHandle CreateDSVForTexture2D(
		ID3D12Resource* resource,
		DXGI_FORMAT format);

	/// <summary>
	/// DescriptorViewFactoryを取得
	/// </summary>
	DescriptorViewFactory* GetViewFactory() { return viewFactory_.get(); }
	const DescriptorViewFactory* GetViewFactory() const { return viewFactory_.get(); }

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		D3D12_DESCRIPTOR_HEAP_TYPE type,
		uint32_t numDescriptors,
		bool shaderVisible);

	std::optional<uint32_t> FindAvailableIndex(const std::vector<bool>& usedFlags) const;
	bool IsValidIndex(HeapType type, uint32_t index) const;

private:
	// D3D12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device_;

	// ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_;

	// ディスクリプタサイズ
	uint32_t rtvDescriptorSize_ = 0;
	uint32_t dsvDescriptorSize_ = 0;
	uint32_t srvDescriptorSize_ = 0;

	// 使用状況を管理するフラグ
	std::vector<bool> rtvUsedFlags_;
	std::vector<bool> dsvUsedFlags_;
	std::vector<bool> srvUsedFlags_;

	// ビューファクトリー（責務の分離）
	std::unique_ptr<DescriptorViewFactory> viewFactory_;

	// 初期化フラグ
	bool isInitialized_ = false;
};