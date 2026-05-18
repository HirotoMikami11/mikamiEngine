#include "resources/Shader/Skybox/Skybox.hlsli"

struct TransformationMatrix
{
    float32_t4x4 WVP;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    // .xyww でz成分をwに置き換える → NDC深度 = z/w = 1.0（最遠）
    // これによりSkyboxは常に他の全オブジェクトより奥に描画される
    output.position = mul(input.position, gTransformationMatrix.WVP).xyww;
    // 頂点座標をそのままキューブマップのサンプリング方向として使う
    output.texcoord = input.position.xyz;
    return output;
}
