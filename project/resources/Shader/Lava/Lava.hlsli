#include "resources/Shader/FullscreenTriangle/FullscreenTriangle.hlsli"

// 溶岩パラメータ
struct LavaParameters
{
    float32_t time;                 // 時間（アニメーション用）
    float32_t speed;                // 流れる速度
    float32_t scale;                // ノイズスケール
    float32_t distortionStrength;   // 歪みの強度
    
    float32_t brightnessMultiplier; // 明るさ
    float32_t octaves;              // ノイズのオクターブ数
    float32_t blendMode;            // ブレンドモード
    float32_t mixRatio;             // 元画像との混合比
    
    float32_t4 colorHot;            // 高温部の色
    float32_t4 colorMid;            // 中温部の色
    float32_t4 colorCool;           // 低温部の色
    float32_t3 padding;             // 16バイト境界調整用
};

// ハッシュ関数（疑似乱数生成）
float Hash21(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return frac(p.x * p.y);
}

// 2Dノイズ関数（Value Noise）
float Noise2D(float2 p)
{
    float2 i = floor(p);
    float2 f = frac(p);
    
    // スムーズステップ補間
    f = f * f * (3.0 - 2.0 * f);
    
    // 4つの角のハッシュ値
    float a = Hash21(i);
    float b = Hash21(i + float2(1.0, 0.0));
    float c = Hash21(i + float2(0.0, 1.0));
    float d = Hash21(i + float2(1.0, 1.0));
    
    // バイリニア補間
    return lerp(lerp(a, b, f.x), lerp(c, d, f.x), f.y);
}

// FBM（Fractional Brownian Motion）- 複数オクターブのノイズを重ねる
float FBM(float2 p, int octaves)
{
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for (int i = 0; i < octaves; i++)
    {
        value += amplitude * Noise2D(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    
    return value;
}

// 回転行列を適用
float2 Rotate(float2 p, float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    return float2(p.x * c - p.y * s, p.x * s + p.y * c);
}

// 3色グラデーション（3つの色を滑らかに補間）
float4 ThreeColorGradient(float t, float4 colorCool, float4 colorMid, float4 colorHot)
{
    float4 color;
    
    if (t < 0.5)
    {
        // 低温から中温へ
        float localT = t * 2.0;
        color = lerp(colorCool, colorMid, localT);
    }
    else
    {
        // 中温から高温へ
        float localT = (t - 0.5) * 2.0;
        color = lerp(colorMid, colorHot, localT);
    }
    
    return color;
}

// ドメインワーピング（UV座標を歪める）
float2 DomainWarp(float2 uv, float time, float strength, float scale)
{
    // 2つの異なるノイズパターンで歪み
    float2 q = float2(
        FBM(uv * scale + float2(time * 0.1, 0.0), 4),
        FBM(uv * scale + float2(0.0, time * 0.1), 4)
    );
    
    float2 r = float2(
        FBM(uv * scale + 4.0 * q + float2(time * 0.15, 0.0), 4),
        FBM(uv * scale + 4.0 * q + float2(0.0, time * 0.15), 4)
    );
    
    return uv + r * strength;
}
