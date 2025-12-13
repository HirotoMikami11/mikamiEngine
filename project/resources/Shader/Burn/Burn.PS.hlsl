#include "resources/Shader/Burn/Burn.hlsli"

// 定数バッファ（燃焼エフェクトのパラメータ）
ConstantBuffer<BurnParameters> BurnParameter : register(b0);

// テクスチャとサンプラー
Texture2D<float32_t4> gTexture : register(t0); // 入力カラーテクスチャ
SamplerState gSampler : register(s0);

// ピクセルシェーダーのメインエントリーポイント
FullscreenPixelOutput main(FullscreenVertexOutput input)
{
    FullscreenPixelOutput output;
    
    // 1. 元のテクスチャカラーを取得
    float32_t4 originalColor = gTexture.Sample(gSampler, input.texcoord);
    
    // 2. 燃焼パターンを計算
    // 戻り値: 0.0f = 燃えていない、1.0f = 完全に燃えた
    float32_t burnPattern = CalculateBurnPattern(input.texcoord, BurnParameter);
    
    // 3. 燃焼の段階に応じて色を決定
    // burnPattern = 0.0f: 燃えていない（元の画像）
    // burnPattern = 0.0f～0.5f: 燃焼境界（炎の色のグラデーション）
    // burnPattern = 0.5f～1.0f: 燃焼中心部（暗い炎→黒へフェード）
    
    float32_t4 finalColor;
    
    if (burnPattern < 0.01f)
    {
        // 燃えていない部分：元の画像をそのまま表示
        finalColor = originalColor;
    }
    else if (burnPattern < 0.5f)
    {
        // 燃焼境界：炎の色を表示
        // burnPattern 0.0f～0.5f を 0.0f～1.0f にマッピング
        float32_t edgeProgress = burnPattern * 2.0f;
        
        // エッジの色をグラデーション表示
        // 外側（黄色）から内側（オレンジ）へ滑らかに変化
        float32_t4 fireColor = lerp(BurnParameter.edgeColor, BurnParameter.burnColor, edgeProgress);
        
        // 元の画像と炎の色をブレンド
        finalColor = lerp(originalColor, fireColor, edgeProgress);
        
        // 炎の明るさを追加（グロー効果）
        // 外側ほど明るく光る
        float32_t glow = (1.0f - edgeProgress) * 0.5f;
        finalColor.rgb += glow * BurnParameter.edgeColor.rgb;
    }
    else
    {
        // 燃え尽きた部分：黒に向かってフェード
        // burnPattern 0.5f～1.0f を 0.0f～1.0f にマッピング
        float32_t burnedProgress = (burnPattern - 0.5f) * 2.0f;
        
        // 炎の色から黒へ段階的にフェード
        float32_t4 darkColor = float32_t4(0.0f, 0.0f, 0.0f, 1.0f);
        float32_t4 fireColor = BurnParameter.burnColor * 0.5f; // 暗めの炎色
        
        finalColor = lerp(fireColor, darkColor, burnedProgress);
    }
    
    // アルファ値は常に1.0（完全不透明）
    finalColor.a = 1.0f;
    
    // 4. 最終結果を出力
    output.color = finalColor;
    return output;
}
