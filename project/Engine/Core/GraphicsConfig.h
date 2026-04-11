#pragma once
#include <cstdint>

/// <summary>
/// グラフィックス関連の設定値を集約するクラス
/// </summary>
class GraphicsConfig {
public:
	///*-----------------------------------------------------------------------*///
	///								ウィンドウ									///
	///*-----------------------------------------------------------------------*///
	static const uint32_t kClientWidth = 1280;
	static const uint32_t kClientHeight = 720;

	///*-----------------------------------------------------------------------*///
	///							ディスクリプタヒープサイズ							///
	///*-----------------------------------------------------------------------*///

	static const uint32_t kRTVHeapSize = 6;   // スワップチェーン2+ オフスクリーン描画1、+pingpong切り替えで2、+FinalPass1
	static const uint32_t kDSVHeapSize = 2;   // メイン + オフスクリーン
	static const uint32_t kSRVHeapSize = 128; // テクスチャ + ImGui
	///*-----------------------------------------------------------------------*///
	///							ディスクリプタインデックス							///
	///*-----------------------------------------------------------------------*///

	// RTV[0]SwapChain0, [1]SwapChain1, [2]Offscreen
	static const uint32_t kSwapChainRTV0Index = 0;
	static const uint32_t kSwapChainRTV1Index = 1;

	// DSV[0]Main
	static const uint32_t kMainDSVIndex = 0;

	// ImGui専用SRVインデックス
	static const uint32_t kImGuiSRVIndex = 0;

	///*-----------------------------------------------------------------------*///
	///							レンダラー最大スロット数							///
	///*-----------------------------------------------------------------------*///
	
	static const uint32_t kMaxObject3DTransforms = 2048;	/// Object3DRenderer Transform スロット数
	static const uint32_t kMaxObject3DMaterials = 2048;		/// Object3DRenderer Material スロット数
	static const uint32_t kMaxSprites = 1024;				/// SpriteRenderer スロット数
	static const uint32_t kMaxLineCount = 32768;			/// LineRenderer 線分数（2^15）
	// Particle の最大数は ParticleGroup::Initialize(maxParticles) でグループごとに指定する設計のため
	// ここには定義しない。グループ数上限は ParticleSystem 側で管理


private:

	// ::で呼び出すためにインスタンス化しないように設定
	GraphicsConfig() = delete;
	~GraphicsConfig() = delete;
	GraphicsConfig(const GraphicsConfig&) = delete;
	GraphicsConfig& operator=(const GraphicsConfig&) = delete;
};