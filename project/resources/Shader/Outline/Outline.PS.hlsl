#include "resources/Shader/Outline/Outline.hlsli"

// 定数バッファ
ConstantBuffer<OutlineParameters> gOutlineParams : register(b0);

// テクスチャとサンプラー
Texture2D<float32_t4> gColorTexture : register(t0); // カラーテクスチャ
Texture2D<float32_t> gDepthTexture : register(t1); // 深度テクスチャ
SamplerState gSampler : register(s0);

// Sobelフィルタを使用した深度ベースのエッジ検出
float32_t DetectEdge(float32_t2 texcoord, float32_t2 texelSize, float32_t thickness)
{
    // Sobelカーネル（3x3）
    // X方向の勾配検出
    float32_t sobelX[9] =
    {
        -1.0, 0.0, 1.0,
        -2.0, 0.0, 2.0,
        -1.0, 0.0, 1.0
    };
    
    // Y方向の勾配検出
    float32_t sobelY[9] =
    {
        -1.0, -2.0, -1.0,
         0.0, 0.0, 0.0,
         1.0, 2.0, 1.0
    };
    
    // 太さに応じてサンプリングオフセットを調整
    float32_t2 offset = texelSize * thickness;
    
    // 3x3のサンプリング位置
    float32_t2 offsets[9] =
    {
        float32_t2(-offset.x, offset.y), // 左上
        float32_t2(0.0, offset.y), // 上
        float32_t2(offset.x, offset.y), // 右上
        float32_t2(-offset.x, 0.0), // 左
        float32_t2(0.0, 0.0), // 中央
        float32_t2(offset.x, 0.0), // 右
        float32_t2(-offset.x, -offset.y), // 左下
        float32_t2(0.0, -offset.y), // 下
        float32_t2(offset.x, -offset.y) // 右下
    };
    
    // 各位置の深度値をサンプリングして線形化
    float32_t depths[9];
    for (int i = 0; i < 9; i++)
    {
        float32_t rawDepth = gDepthTexture.Sample(gSampler, texcoord + offsets[i]).r;
        depths[i] = LinearizeDepth(rawDepth);
    }
    
    // Sobelフィルタを適用
    float32_t gradientX = 0.0;
    float32_t gradientY = 0.0;
    
    for (int j = 0; j < 9; j++)
    {
        gradientX += depths[j] * sobelX[j];
        gradientY += depths[j] * sobelY[j];
    }
    
    // 勾配の大きさを計算
    float32_t gradientMagnitude = sqrt(gradientX * gradientX + gradientY * gradientY);
    
    return gradientMagnitude;
}

// ピクセルシェーダーのメインエントリーポイント
FullscreenPixelOutput main(FullscreenVertexOutput input)
{
    FullscreenPixelOutput output;
    
    // テクセルサイズを計算
    float32_t2 texelSize = 1.0 / gOutlineParams.screenSize;
    
    // 元のカラーをサンプリング
    float32_t4 originalColor = gColorTexture.Sample(gSampler, input.texcoord);
    
    // エッジを検出
    float32_t edgeIntensity = DetectEdge(input.texcoord, texelSize, gOutlineParams.outlineThickness);
    
    // 深度閾値でエッジを判定
    float32_t edgeFactor = smoothstep(
        gOutlineParams.depthThreshold * 0.5,
        gOutlineParams.depthThreshold,
        edgeIntensity
    );
    
    // エッジ強度を適用
    edgeFactor *= gOutlineParams.outlineStrength;
    
    // アウトライン色と元の色をブレンド
    float32_t4 finalColor = lerp(originalColor, gOutlineParams.outlineColor, edgeFactor);
    
    output.color = finalColor;
    
    return output;
}
