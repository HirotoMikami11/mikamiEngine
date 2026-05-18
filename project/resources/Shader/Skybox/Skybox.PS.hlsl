#include "resources/Shader/Skybox/Skybox.hlsli"

ConstantBuffer<SkyboxMaterialData> gMaterial : register(b0);
TextureCube<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    // texcoord(= 頂点のposition.xyz)をキューブマップの方向ベクトルとしてサンプリング
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    output.color = textureColor * gMaterial.color;
    return output;
}
