#pragma once
#include <Windows.h>

///DirectX12
#include<d3d12.h>
#include<dxgi1_6.h>
#include <dxgidebug.h>
///DirectXTex
#include"../externals/DirectXTex/DirectXTex.h"
#include"../externals/DirectXTex/d3dx12.h"
///DXC
#include <dxcapi.h>
#include <chrono>
#include <wrl.h>
#include"WinApp.h"
#include"DescriptorHeapManager.h"		//ディスクリプタヒープ管理
#include"PSOFactory.h"					//PSO作成
#include"FrameTimer.h"					//フレームタイマー
/// <summary>
/// DirectX
/// </summary>
class DirectXCommon
{
public:
	//クラス内でのみ,namespace省略(エイリアステンプレート)
	template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	void Initialize(WinApp* winApp);
	void Finalize();

	/// <summary>
	/// 描画前の処理
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 描画後の処理
	/// </summary>
	void PostDraw();
	/// <summary>
	/// フレーム開始処理（コマンドリストの準備）
	/// </summary>
	void BeginFrame();

	/// <summary>
	/// フレーム終了処理（コマンドリストの実行と次フレーム準備）
	/// </summary>
	void EndFrame();

	/// <summary>
	/// シェーダーをコンパイルする関数
	/// </summary>
	static ComPtr<IDxcBlob> CompileShader(
		const std::wstring& filePath,
		const wchar_t* profile,
		ComPtr<IDxcUtils> dxcUtils,
		ComPtr<IDxcCompiler3> dxcCompiler,
		ComPtr<IDxcIncludeHandler> includeHandler);


	//*-----------------------------------------------------------------------*//
	//									ゲッター  								//
	//*-----------------------------------------------------------------------*//
	///ゲッター

	ID3D12Device* GetDevice() const { return device.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }
	ID3D12CommandQueue* GetCommandQueue() const { return commandQueue.Get(); }
	IDXGISwapChain4* GetSwapChain() const { return swapChain.Get(); }
	ID3D12RootSignature* GetRootSignature() const { return rootSignature.Get(); }
	ID3D12PipelineState* GetPipelineState() const { return graphicsPipelineState.Get(); }
	ID3D12RootSignature* GetSpriteRootSignature() const { return spriteRootSignature.Get(); }
	ID3D12PipelineState* GetSpritePipelineState() const { return spritePipelineState.Get(); }
	ID3D12RootSignature* GetLineRootSignature() const { return lineRootSignature.Get(); }
	ID3D12PipelineState* GetLinePipelineState() const { return linePipelineState.Get(); }

	///参照で返すゲッター？
	const ComPtr<ID3D12Device>& GetDeviceComPtr() const { return device; }
	const ComPtr<ID3D12GraphicsCommandList>& GetCommandListComPtr() const { return commandList; }

	// DXC関連のゲッター
	IDxcUtils* GetDxcUtils() const { return dxcUtils.Get(); }
	IDxcCompiler3* GetDxcCompiler() const { return dxcCompiler.Get(); }
	IDxcIncludeHandler* GetIncludeHandler() const { return includeHandler.Get(); }

	// DescriptorHeapManager関連
	DescriptorHeapManager* GetDescriptorManager() const { return descriptorManager_.get(); }
	ID3D12DescriptorHeap* GetSRVDescriptorHeap() const { return descriptorManager_->GetSRVHeap(); }

	DXGI_SWAP_CHAIN_DESC1 GetSwapChainDesc() const { return swapChainDesc; }
	ID3D12Resource* GetSwapChainResource(int index) const { return swapChainResources[index].Get(); }
	D3D12_RENDER_TARGET_VIEW_DESC GetRTVDesc() { return rtvDesc; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(int index) const { return rtvHandles[index]; }

	// PSOFactory関連
	PSOFactory* GetPSOFactory() const { return psoFactory_.get(); }

private:



	/// <summary>
	/// デバッグレイヤーを作成する
	/// </summary>
	void MakeDebugLayer();

	/// <summary>
	/// DXGIFactoryを作成
	/// </summary>
	void MakeDXGIFactory();

	/// <summary>
	/// コマンドQueue,Allocator,Listの作成
	/// </summary>
	void InitializeCommand();

	/// <summary>
	/// SwapChainの作成
	/// </summary>
	void MakeSwapChain(WinApp* winApp);

	/// <summary>
	/// RTVを作成する
	/// </summary>
	void MakeRTV();

	//SRVはテクスチャの部分で作成

	/// <summary>
	/// DSVを作成する
	/// </summary>
	void MakeDSV();
	ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(ComPtr<ID3D12Device> device, int32_t width, int32_t height);

	/// <summary>
	/// フェンスとイベントを作成する
	/// </summary>
	void MakeFenceEvent();

	/// <summary>
	/// DXC
	/// </summary>
	void InitalizeDXC();

	/// <summary>
	/// PSOFactoryを初期化する
	/// </summary>
	void InitializePSOFactory();

	/// <summary>
	/// PSOを作成する
	/// </summary>
	void MakePSO();

	/// <summary>
	/// 2D用のPSOを作成する
	/// </summary>
	void MakeSpritePSO();

	/// <summary>
	/// 線分描画用のPSOを作成する
	/// </summary>
	void MakeLinePSO();

	/// <summary>
	/// ViewportとScissor
	/// </summary>
	void MakeViewport();

	/// <summary>
	/// FPS固定処理の初期化
	 /// </summary>
	void InitializeFixFPS();

	/// <summary>
	/// FPS固定処理の更新
	/// </summary>
	void UpdateFixFPS();

#ifdef USEIMGUI
	///デバッグレイヤー
	ComPtr<ID3D12Debug1> debugController;
	bool useDebugLayer_ = true;
#endif
	//DXGIFactory
	HRESULT hr;
	ComPtr<IDXGIFactory7> dxgiFactory;
	ComPtr<IDXGIAdapter4> useAdapter;
	ComPtr<ID3D12Device> device;

	//initailzeCommand
	ComPtr<ID3D12CommandQueue> commandQueue;
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> commandList;

	ComPtr<IDXGISwapChain4> swapChain;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	// スワップチェーンのバッファ
	ComPtr<ID3D12Resource> swapChainResources[2];

	//rtv
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};//rtvの設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	//dsv
	ComPtr<ID3D12Resource> depthStencilResource;
	ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;

	//Fence
	ComPtr<ID3D12Fence> fence;
	uint64_t fenceValue;


	//DXC
	ComPtr<IDxcUtils> dxcUtils;
	ComPtr<IDxcCompiler3> dxcCompiler;
	ComPtr<IDxcIncludeHandler> includeHandler;

	// PSOFactory
	std::unique_ptr<PSOFactory> psoFactory_;
	//3D用PSO
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> graphicsPipelineState;

	//スプライト用PSO
	ComPtr<ID3D12RootSignature> spriteRootSignature;
	ComPtr<ID3D12PipelineState> spritePipelineState;

	//線分用PSO
	ComPtr<ID3D12RootSignature> lineRootSignature;
	ComPtr<ID3D12PipelineState> linePipelineState;


	//ビューポート
	D3D12_VIEWPORT viewport{};
	//シザー矩形
	D3D12_RECT scissorRect{};
	//TransitionBarrier
	D3D12_RESOURCE_BARRIER barrier{};

	//ディスクリプタヒープの管理をする
	std::unique_ptr<DescriptorHeapManager> descriptorManager_;

	// FPS固定関連
	std::chrono::steady_clock::time_point reference_;

	// 1/60秒ぴったりの時間(マイクロ秒)
	static constexpr std::chrono::microseconds kMinTime{
		static_cast<uint64_t>(1000000.0f / 60.0f)
	};

	// 1/60秒よりわずかに短い時間(マイクロ秒) - モニター倍数対策
	static constexpr std::chrono::microseconds kMinCheckTime{
		static_cast<uint64_t>(1000000.0f / 65.0f)
	};


};

