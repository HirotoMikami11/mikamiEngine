#include "resources/Shader/GaussianFilter/GaussianFilter.hlsli"

ConstantBuffer<GaussianFilterParameters> gGaussianFilterParameters : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

static const float32_t PI = 3.14159265359f;

float gauss(float x, float y, float sigma)
{
    float exponent = -(x * x + y * y) * rcp(2.0f * sigma * sigma);
    float denominator = 2.0f * PI * sigma * sigma;
    return exp(exponent) * rcp(denominator);
}

FullscreenPixelOutput main(FullscreenVertexOutput input)
{
    FullscreenPixelOutput output;

    uint32_t textureWidth = 0;
    uint32_t textureHeight = 0;
    gTexture.GetDimensions(textureWidth, textureHeight);

    float32_t2 uvStepSize = float32_t2(1.0f / max(1u, textureWidth), 1.0f / max(1u, textureHeight));

    int32_t kernelSize = clamp(gGaussianFilterParameters.kernelSize, 1, 9);
    int32_t radius = kernelSize / 2;
    float sigma = max(gGaussianFilterParameters.sigma, 0.001f);

    float32_t3 sumColor = float32_t3(0.0f, 0.0f, 0.0f);
    float sumWeight = 0.0f;

    [loop]
    for (int32_t y = -radius; y <= radius; ++y)
    {
        [loop]
        for (int32_t x = -radius; x <= radius; ++x)
        {
            float32_t2 currentTexcoord = input.texcoord + float32_t2(x, y) * uvStepSize;
            float weight = gauss((float)x, (float)y, sigma);

            sumColor += gTexture.Sample(gSampler, currentTexcoord).rgb * weight;
            sumWeight += weight;
        }
    }

    sumColor *= rcp(max(sumWeight, 0.000001f));

    output.color = float32_t4(sumColor, gTexture.Sample(gSampler, input.texcoord).a);
    return output;
}
