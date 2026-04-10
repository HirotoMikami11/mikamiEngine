#pragma once
/// @file UploadRingBuffer.h
/// @brief フレームごとにリセットする定数バッファ用アップロードリングバッファ
/// @details
///   D3D12のCBV要件（256バイトアライン）を満たしながら、大量の定数バッファを
///   効率的に管理するためのテンプレートクラス。
///
///   【設計方針】
///   - 起動時に capacity 個分のUPLOADヒープを1回だけ確保する
///   - BeginFrame() でカウンタを0にリセットするだけ（O(1)、再確保なし）
///   - Allocate() でCPUポインタとGPUアドレスのペアを返す
///   - Free() は不要（フレームリセットで自動解放）
///
///   【使用例】
///   // 初期化（起動時に1回）
///   UploadRingBuffer<TransformationMatrix> ring;
///   ring.Initialize(device, 2048);
///
///   // フレーム先頭（Engine::StartDrawOffscreen内）
///   ring.BeginFrame();
///
///   // 使用時（Object3D::Draw内など）
///   auto alloc = ring.Allocate();
///   alloc.cpuPtr->WVP = wvpMatrix;
///   commandList->SetGraphicsRootConstantBufferView(1, alloc.gpuAddr);

#include <d3d12.h>
#include <wrl.h>
#include <cassert>
#include "MyFunction.h"

/// <summary>
/// バッファに格納するデータ型（TransformationMatrix, MaterialData など）
/// </summary>
template<typename T>

/// <summary>
/// フレームごとにリセットするアップロードリングバッファ
/// </summary>
class UploadRingBuffer {
public:

	/// <summary>
	/// Allocate() の戻り値。CPU書き込み先とGPUアドレスのペア。
	/// </summary>
	struct Allocation {
		T* cpuPtr;							///< CPU側書き込みポインタ（Mapされたアドレス）
		D3D12_GPU_VIRTUAL_ADDRESS gpuAddr;	///< シェーダーへ渡すGPUアドレス
	};

	UploadRingBuffer() = default;
	~UploadRingBuffer() = default;

	// コピー・ムーブ禁止（GPUリソースを管理するため）
	UploadRingBuffer(const UploadRingBuffer&) = delete;
	UploadRingBuffer& operator=(const UploadRingBuffer&) = delete;

	/// <summary>
	/// 初期化。capacity 個分のUPLOADヒープを1回だけ確保する。
	/// sizeof(T) が256バイト未満の場合でも、CBV要件に合わせて自動でアライン
	/// </summary>
	/// <param name="device">D3D12デバイス</param>
	/// <param name="capacity">1フレームに確保できる最大スロット数</param>
	void Initialize(ID3D12Device* device, uint32_t capacity) {
		assert(device != nullptr && "UploadRingBuffer::Initialize: device is null");
		assert(capacity > 0 && "UploadRingBuffer::Initialize: capacity must be > 0");

		capacity_ = capacity;

		// D3D12 CBVの要件: GPUアドレスは256バイト境界にアライン
		// stride_ = sizeof(T) を256バイト単位に切り上げた値
		stride_ = (static_cast<uint32_t>(sizeof(T)) + 255u) & ~255u;

		// capacity 個分を一括確保（以降は追加確保しない）
		const size_t totalBytes = static_cast<size_t>(stride_) * capacity_;
		buffer_ = CreateBufferResource(device, totalBytes);
		assert(buffer_ != nullptr && "UploadRingBuffer::Initialize: buffer creation failed");

		// 永続マップ（Unmapは不要。Upload ヒープなので常時CPU書き込み可）
		buffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedTop_));
		assert(mappedTop_ != nullptr && "UploadRingBuffer::Initialize: Map failed");

		baseGpuAddr_ = buffer_->GetGPUVirtualAddress();
	}

	/// <summary>
	/// GPU リソースを明示的に解放する。
	///	シングルトンのデストラクタよりも先に解放しないとリソースリークになる。
	/// </summary>
	void Finalize() {
		if (mappedTop_) {
			buffer_->Unmap(0, nullptr);
			mappedTop_ = nullptr;
		}
		buffer_.Reset();
		baseGpuAddr_ = 0;
		capacity_ = 0;
		stride_ = 0;
		currentIndex_ = 0;
	}

	/// <summary>
	/// カウンタを0にリセットするだけ（O(1)）
	/// </summary>
	void BeginFrame() {
		currentIndex_ = 0;
	}

	/// <summary>
	/// 1スロットを確保して Allocation を返す。
	/// </summary>
	/// <returns>CPU書き込みポインタとGPUアドレスのペア</returns>
	Allocation Allocate() {

		// スロットが足りなくなったらcapacity_ を増やす
		assert(currentIndex_ < capacity_ &&
			"UploadRingBuffer::Allocate: capacity exceeded. Increase kMax*** constant.");

		const uint32_t idx = currentIndex_++;
		return {
			reinterpret_cast<T*>(mappedTop_ + static_cast<size_t>(stride_) * idx),
			baseGpuAddr_ + static_cast<D3D12_GPU_VIRTUAL_ADDRESS>(stride_) * idx
		};
	}


	// 現フレームの使用スロット数を返す（デバッグ・ImGui表示用）
	uint32_t GetUsedCount() const { return currentIndex_; }

	//最大スロット数を返す
	uint32_t GetCapacity() const { return capacity_; }

	// 初期化済みかどうかを返す
	bool IsInitialized() const { return buffer_ != nullptr; }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer_;					/// 一括確保したUPLOADバッファ
	uint8_t* mappedTop_ = nullptr;									/// Map済みの先頭CPUアドレス
	D3D12_GPU_VIRTUAL_ADDRESS baseGpuAddr_ = 0;						/// バッファ先頭のGPUアドレス
	uint32_t capacity_ = 0;											/// 最大スロット数
	uint32_t stride_ = 0;											/// 1スロットのバイト数（256バイトアライン済み）
	uint32_t currentIndex_ = 0;										/// 現フレームの次割り当てインデックス
};
