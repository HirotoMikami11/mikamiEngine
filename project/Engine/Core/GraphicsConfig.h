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

	static const uint32_t kRTVHeapSize = 5;   // スワップチェーン2+ オフスクリーン描画1、+pingpong切り替えで2
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

	static const uint32_t kImGuiSRVIndex = 0;           // ImGui専用SRVインデックス


private:

	// ::で呼び出すためにインスタンス化しないように設定
	GraphicsConfig() = delete;
	~GraphicsConfig() = delete;
	GraphicsConfig(const GraphicsConfig&) = delete;
	GraphicsConfig& operator=(const GraphicsConfig&) = delete;
};