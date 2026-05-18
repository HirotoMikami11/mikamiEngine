#pragma once
#include <string>
#include <d3d12.h>
#include "MyFunction.h"

/// <summary>
/// スカイボックス
/// </summary>
class Skybox {
public:
	/// <summary>
	/// キューブマップの画像を設定(dds形式)
	/// </summary>
	void SetCubemap(const std::string& tag);

	/// <summary>
	/// 色倍率を設定（デフォルト: {1,1,1,1}）
	/// </summary>
	void SetColor(const Vector4& color) { color_ = color; }

	/// <summary>
	/// Skybox を描画する
	/// </summary>
	void Draw() const;

	bool IsLoaded() const { return isLoaded_; }

private:
	D3D12_GPU_DESCRIPTOR_HANDLE cubemapHandle_{};
	Vector4 color_{ 1.0f, 1.0f, 1.0f, 1.0f };
	bool isLoaded_ = false;
};
