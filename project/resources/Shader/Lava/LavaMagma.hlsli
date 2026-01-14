#include "resources/Shader/FullscreenTriangle/FullscreenTriangle.hlsli"

// 溶岩マグマパラメータ（ShaderToy完全再現版）
struct LavaMagmaParameters
{
    float32_t time;                 // 時間（アニメーション用）
    float32_t speedMultiplier;      // 速度倍率
    float32_t scale;                // UVスケール
    float32_t mixRatio;             // 元画像との混合比
    
    float32_t speedX;               // ShaderToyのspeed.x
    float32_t speedY;               // ShaderToyのspeed.y
    float32_t randSeed1X;           // rand(vec2(1., 5.))のX
    float32_t randSeed1Y;           // rand(vec2(1., 5.))のY
    
    float32_t randSeed2X;           // rand(vec2(100., 100.))のX
    float32_t randSeed2Y;           // rand(vec2(100., 100.))のY
    float32_t padding1;
    float32_t padding2;
    
    float32_t4 color1;              // ShaderToyのcol1
    float32_t4 color2;              // ShaderToyのcol2
    float32_t4 color3;              // ShaderToyのcol3
    float32_t4 color4;              // ShaderToyのcol4
    float32_t4 color5;              // ShaderToyのcol5
    float32_t4 color6;              // ShaderToyのcol6
    
    float32_t brightnessMultiplier; // 明るさ調整
    float32_t3 padding3;            // 16バイト境界調整用
};

// ===== ShaderToyのコードを1:1で再現 =====

// ShaderToyのrand関数（COSを使用）
// float rand(vec2 n) {
//     return fract(cos(dot(n, vec2(5.9898, 4.1414))) * 65899.89956);
// }
float Rand(float2 n)
{
    return frac(cos(dot(n, float2(5.9898, 4.1414))) * 65899.89956);
}

// ShaderToyのnoise関数（smoothstepを使用）
// float noise( in vec2 n )
// {
//     const vec2 d = vec2(0.0, 1.0);
//     vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
//     return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
// }
float Noise(float2 n)
{
    const float2 d = float2(0.0, 1.0);
    float2 b = floor(n);
    float2 f = smoothstep(float2(0.0, 0.0), float2(1.0, 1.0), frac(n));
    
    return lerp(
        lerp(Rand(b), Rand(b + d.yx), f.x),
        lerp(Rand(b + d.xy), Rand(b + d.yy), f.x),
        f.y
    );
}

// ShaderToyのfbm関数（4オクターブ、amplitude *= 0.51）
// float fbm(vec2 n) {
//     float total = 0.0;
//     float amplitude = 1.0;
//     for (int i = 0; i < 4; i++){
//         total += noise(n) * amplitude;
//         n += n;
//         amplitude *= 0.51;
//     }
//     return total;
// }
float FBM(float2 n)
{
    float total = 0.0;
    float amplitude = 1.0;
    
    for (int i = 0; i < 4; i++)
    {
        total += Noise(n) * amplitude;
        n += n;  // ShaderToyの "n += n" をそのまま再現
        amplitude *= 0.51;  // ShaderToyの 0.51 をそのまま使用
    }
    
    return total;
}
