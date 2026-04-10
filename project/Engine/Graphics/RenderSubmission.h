#pragma once
/// @file RenderSubmission.h
/// @brief Renderer に Submit するデータ構造体の定義
/// @details
///   Object3D や Sprite が Draw() 内で Renderer へ渡す描画リクエストの型定義。
///   GPU命令は含まない。純粋なデータ構造のみ。
///
///   【データフロー】
///   Object3D::Draw(renderer)
///     → UploadRingBuffer から Allocation を取得してCPUデータを書き込む
///     → ModelSubmission を構築して renderer->Submit() に渡す
///     ↓（GPU命令なし）
///   Object3DRenderer::Draw3D()
///     → submissions_ を処理してGPU描画命令を発行

#include <d3d12.h>
#include <cstdint>

// Mesh クラスの前方宣言
class Mesh;

/// <summary>
/// 描画グループ。ソート順とステート変更の最適化に使用する。
/// </summary>
enum class RenderGroup {
	Opaque,		/// 不透明オブジェクト（深度書き込みあり、前から後ろへ描画）
	AlphaBlend,	/// 半透明（深度書き込みなし、奥から手前へソートして描画）
	Add,		/// 加算合成（半透明と同様に奥から手前へ）
};

/// <summary>
/// 3Dオブジェクト 1メッシュ分の Submit データ
/// transformGpuAddr / materialGpuAddr は UploadRingBuffer::Allocate() で取得したアドレスを設定する。
/// </summary>
struct ModelSubmission {
	const Mesh* mesh;							/// 描画するメッシュ（ModelManager共有ポインタ）
	D3D12_GPU_VIRTUAL_ADDRESS transformGpuAddr;	/// TransformationMatrix の GPUアドレス（UploadRingBufferから取得）
	D3D12_GPU_VIRTUAL_ADDRESS materialGpuAddr;	/// MaterialData の GPUアドレス（UploadRingBufferから取得）
	D3D12_GPU_DESCRIPTOR_HANDLE textureHandle;	/// テクスチャ SRV の GPUハンドル
	RenderGroup group;							/// 描画グループ（ソート・ステート最適化に使用）
	float sortDepth;							/// カメラ空間Z値（AlphaBlend/Add の奥から手前ソートに使用）
};

/// <summary>
/// UIスプライト 1枚分の Submit データ
/// VB/IB はアンカーポイント依存の形状のため、各Spriteが自身のViewを持ち続ける。
/// </summary>
struct SpriteSubmission {
	const D3D12_VERTEX_BUFFER_VIEW* vbv;			/// Sprite自身の頂点バッファビュー
	const D3D12_INDEX_BUFFER_VIEW* ibv;				/// Sprite自身のインデックスバッファビュー
	uint32_t indexCount;							/// インデックス数（通常6: 四角形2トライアングル）
	D3D12_GPU_VIRTUAL_ADDRESS transformGpuAddr;		/// TransformationMatrix の GPUアドレス
	D3D12_GPU_VIRTUAL_ADDRESS materialGpuAddr;		/// SpriteMaterialData の GPUアドレス
	D3D12_GPU_DESCRIPTOR_HANDLE	textureHandle;		/// テクスチャ SRV の GPUハンドル
	int layerOrder;									/// 描画順（値が小さいほど先に描画される）
};
