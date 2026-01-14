#include "resources/Shader/Lava/Lava.hlsli"

ConstantBuffer<LavaParameters> LavaParameter : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

// ShaderToyのrand関数をHLSLに変換
float Rand(float2 n)
{
    return frac(sin(dot(n, float2(12.9898, 4.1414))) * 43758.5453);
}

// ShaderToyのnoise関数をHLSLに変換
float NoiseShaderToy(float2 n)
{
    float2 d = float2(0.0, 1.0);
    float2 b = floor(n);
    float2 f = smoothstep(float2(0.0, 0.0), float2(1.0, 1.0), frac(n));
    
    return lerp(
        lerp(Rand(b), Rand(b + d.yx), f.x),
        lerp(Rand(b + d.xy), Rand(b + d.yy), f.x),
        f.y
    );
}

// ShaderToyのfbm関数をHLSLに変換
float FBMShaderToy(float2 n)
{
    float total = 0.0;
    float amplitude = 1.0;
    for (int i = 0; i < 5; i++)
    {
        total += NoiseShaderToy(n) * amplitude;
        n += n;
        amplitude *= 0.5;
    }
    return total;
}

FullscreenPixelOutput main(FullscreenVertexOutput input)
{
    FullscreenPixelOutput output;
    
    // 元のテクスチャカラー
    float32_t4 originalColor = gTexture.Sample(gSampler, input.texcoord);
    
    // UV座標
    float2 uv = input.texcoord;
    uv *= LavaParameter.scale;
    
    // アスペクト比調整
    uint32_t2 texSize;
    gTexture.GetDimensions(texSize.x, texSize.y);
    uv.x *= (float) texSize.x / (float) texSize.y;
    
    // 時間
    float time = LavaParameter.time;
    
    // ドメインワーピング（ShaderToyスタイル）
    float2 speed = float2(0.2, 0.15) * LavaParameter.speed;
    
    float2 nfbm = float2(uv.x, uv.y - time * 0.1);
    float q = FBMShaderToy(nfbm);
    
    float2 arg1 = uv + q + time * speed.x;
    float2 arg2 = uv + q - Rand(float2(100.0, 100.0)) * speed.y;
    float2 r = float2(FBMShaderToy(arg1), FBMShaderToy(arg2));
    
    // 溶岩の色を生成
    float2 warpedUV = uv + r * LavaParameter.distortionStrength;
    float pattern = FBMShaderToy(warpedUV);
    
    // 3色グラデーション（パラメータから取得）
    float4 lavaColor = ThreeColorGradient(
        pattern,
        LavaParameter.colorCool,
        LavaParameter.colorMid,
        LavaParameter.colorHot
    );
    
    // 追加のディテール
    lavaColor.rgb += lerp(
        LavaParameter.colorMid.rgb,
        LavaParameter.colorHot.rgb,
        r.x
    ) * 0.5;
    
    // 明るさ調整
    lavaColor.rgb *= LavaParameter.brightnessMultiplier;
    
    // ブレンディング
    float4 finalColor = lerp(originalColor, lavaColor, LavaParameter.mixRatio);
    finalColor.a = originalColor.a;
    
    output.color = finalColor;
    return output;
}