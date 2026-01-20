#include "resources/Shader/FullscreenTriangle/FullscreenTriangle.hlsli"

// 2値化パラメータ
struct BinarizationParameters
{
    float32_t threshold;        // 閾値（0.0～1.0）
    float32_t ditherStrength;   // ディザの強度（0.0=OFF, 1.0=MAX）
    float32_t smoothness;       // 境界のぼかし（0.0=カクカク）
    float32_t padding;          // 16バイト境界調整用
    
    float32_t4 color1;          // 暗い側の色
    float32_t4 color2;          // 明るい側の色
};

// 輝度計算（人間の視覚に基づく重み付け）
float CalculateLuminance(float3 color)
{
    // ITU-R BT.709の係数を使用
    return dot(color, float3(0.2126, 0.7152, 0.0722));
}

// ベイヤーディザリング用の4x4パターン行列
float GetBayerValue(int2 pixelPos)
{
    // 4x4 Bayer matrix
    const float bayerMatrix[4][4] = {
        { 0.0 / 16.0,  8.0 / 16.0,  2.0 / 16.0, 10.0 / 16.0 },
        { 12.0 / 16.0,  4.0 / 16.0, 14.0 / 16.0,  6.0 / 16.0 },
        { 3.0 / 16.0, 11.0 / 16.0,  1.0 / 16.0,  9.0 / 16.0 },
        { 15.0 / 16.0,  7.0 / 16.0, 13.0 / 16.0,  5.0 / 16.0 }
    };
    
    int x = pixelPos.x % 4;
    int y = pixelPos.y % 4;
    
    return bayerMatrix[y][x];
}

// スムースステップ補間（滑らかな境界）
float SmoothThreshold(float value, float threshold, float smoothness)
{
    if (smoothness <= 0.0)
    {
        // smoothnessが0なら通常のstep関数（カクカク）
        return step(threshold, value);
    }
    else
    {
        // smoothstepで滑らかに補間
        float edge0 = threshold - smoothness;
        float edge1 = threshold + smoothness;
        return smoothstep(edge0, edge1, value);
    }
}
