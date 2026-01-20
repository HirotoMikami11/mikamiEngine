#include "resources/Shader/Binarization/Binarization.hlsli"

ConstantBuffer<BinarizationParameters> BinarizationParameter : register(b0);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

FullscreenPixelOutput main(FullscreenVertexOutput input)
{
    FullscreenPixelOutput output;
    
    // テクスチャサンプリング
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    // 輝度を計算
    float luminance = CalculateLuminance(textureColor.rgb);
    
    // ディザリングを適用（オプション）
    if (BinarizationParameter.ditherStrength > 0.0)
    {
        // ピクセル座標を取得（整数座標）
        uint32_t2 texSize;
        gTexture.GetDimensions(texSize.x, texSize.y);
        int2 pixelPos = int2(input.texcoord * float2(texSize));
        
        // ベイヤーディザパターンを取得
        float bayerValue = GetBayerValue(pixelPos);
        
        // ディザノイズを輝度に加算（強度で調整）
        float ditherNoise = (bayerValue - 0.5) * BinarizationParameter.ditherStrength;
        luminance += ditherNoise;
    }
    
    // 閾値で2値化（smoothnessで境界をぼかす）
    float t = SmoothThreshold(
        luminance,
        BinarizationParameter.threshold,
        BinarizationParameter.smoothness
    );
    
    // 2つの色を補間
    output.color = lerp(BinarizationParameter.color1, BinarizationParameter.color2, t);
    
    // アルファ値は元の値を保持
    output.color.a = textureColor.a;
    
    return output;
}
