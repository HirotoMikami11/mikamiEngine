#include "resources/Shader/BoxFilter/BoxFilter.hlsli"

ConstantBuffer<BoxFilterParameters> gBoxFilterParameters : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

FullscreenPixelOutput main(FullscreenVertexOutput input)
{
    FullscreenPixelOutput output;

    uint32_t textureWidth = 0;
    uint32_t textureHeight = 0;
    gTexture.GetDimensions(textureWidth, textureHeight);

    // 1. uvStepSize算出
    float32_t2 uvStepSize = float32_t2(1.0f / max(1u, textureWidth), 1.0f / max(1u, textureHeight));

    int32_t kernelSize = clamp(gBoxFilterParameters.kernelSize, 1, 9);
    int32_t radius = kernelSize / 2;

    float32_t4 sumColor = float32_t4(0.0f, 0.0f, 0.0f, 0.0f);

    // 2. カーネルサイズ分ループ
    [loop]
    for (int32_t y = -radius; y <= radius; ++y)
    {
        [loop]
        for (int32_t x = -radius; x <= radius; ++x)
        {
            // 3. 現在のtexcoord算出
            float32_t2 currentTexcoord = input.texcoord + float32_t2(x, y) * uvStepSize;

            // 4. 色に対してかけて、足す
            sumColor += gTexture.Sample(gSampler, currentTexcoord);
        }
    }

    float32_t kernelArea = float32_t(kernelSize * kernelSize);
    output.color = sumColor / kernelArea;

    return output;
}
