#pragma once
#include <cstdint>

/// <summary>
/// ポストエフェクトの識別子
/// このIDを使ってエフェクトの種類を判別してScene側でPostEffectを使いやすくする
/// </summary>
enum class PostEffectId : uint8_t {
	DepthFog,
	DepthOfField,
	Outline,
	RGBShift,
	LineGlitch,
	Grayscale,
	Vignette,
	DamageVignette,
	Binarization
};
