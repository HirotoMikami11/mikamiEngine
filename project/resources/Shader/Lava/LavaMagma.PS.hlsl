#include "resources/Shader/Lava/LavaMagma.hlsli"

ConstantBuffer<LavaMagmaParameters> LavaMagmaParameter : register(b0);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

FullscreenPixelOutput main(FullscreenVertexOutput input)
{
    FullscreenPixelOutput output;
    
    // 元のテクスチャカラーを取得
    float32_t4 originalColor = gTexture.Sample(gSampler, input.texcoord);
    
    // ===== ShaderToyのmainImage関数を1:1で再現 =====
    
    // vec2 uv = fragCoord.xy / iResolution.xy;
    float2 uv = input.texcoord;
    
    // uv *= 7.5; → パラメータで制御
    uv *= LavaMagmaParameter.scale;
    
    // uv.x *= iResolution.x/iResolution.y;
    uint32_t2 texSize;
    gTexture.GetDimensions(texSize.x, texSize.y);
    uv.x *= (float)texSize.x / (float)texSize.y;
    
    // vec3 col1 = vec3(0.5, 0.0, 0.1); → パラメータから取得
    float3 col1 = LavaMagmaParameter.color1.rgb;
    float3 col2 = LavaMagmaParameter.color2.rgb;
    float3 col3 = LavaMagmaParameter.color3.rgb;
    float3 col4 = LavaMagmaParameter.color4.rgb;
    float3 col5 = LavaMagmaParameter.color5.rgb;
    float3 col6 = LavaMagmaParameter.color6.rgb;
    
    // vec2 speed = vec2(0.35, 0.35); → パラメータから取得
    float2 speed = float2(LavaMagmaParameter.speedX, LavaMagmaParameter.speedY);
    
    // vec2 nfbm = vec2( uv.x, uv.y - rand(vec2(1., 5.)));
    float2 nfbm = float2(
        uv.x,
        uv.y - Rand(float2(LavaMagmaParameter.randSeed1X, LavaMagmaParameter.randSeed1Y))
    );
    
    // float q = fbm(nfbm);
    float q = FBM(nfbm);
    
    // vec2 arg1 = vec2(uv + q +  iTime* speed.x -uv.x -uv.y);
    float2 arg1 = float2(
        uv + q + LavaMagmaParameter.time * speed.x - uv.x - uv.y
    );
    
    // vec2 arg2 = vec2 (uv + q - rand(vec2(100., 100.)) * speed.y);
    float2 arg2 = float2(
        uv + q - Rand(float2(LavaMagmaParameter.randSeed2X, LavaMagmaParameter.randSeed2Y)) * speed.y
    );
    
    // vec2 r = vec2(fbm(arg1 ), fbm(arg2));
    float2 r = float2(FBM(arg1), FBM(arg2));
    
    // vec2 agr3 = vec2(uv + r);
    float2 agr3 = float2(uv + r);
    
    // vec3 c = mix(col1, col2, fbm(agr3)) + mix(col3, col4, r.x) - mix(col5, col6, r.y);
    float3 c = lerp(col1, col2, FBM(agr3)) + lerp(col3, col4, r.x) - lerp(col5, col6, r.y);
    
    // 明るさ調整
    c *= LavaMagmaParameter.brightnessMultiplier;
    
    // 色の範囲をクランプ（負の値を防ぐが、明るい部分は許容）
    c = max(c, float3(0.0, 0.0, 0.0));
    
    // fragColor = vec4(c, 1.);
    float4 lavaColor = float4(c, 1.0);
    
    // 元画像との混合（ShaderToyにはない機能）
    float4 finalColor = lerp(originalColor, lavaColor, LavaMagmaParameter.mixRatio);
    
    // アルファ値は元の値を保持
    finalColor.a = originalColor.a;
    
    output.color = finalColor;
    
    return output;
}
