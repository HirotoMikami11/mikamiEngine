#include "resources/Shader/Object3d/Object3d.hlsli"
struct Material
{
    float32_t4 color; //色
    int32_t enableLighting; //ライティングするか否か
    int32_t useLambertianReflectance; //ランバート反射を利用するかどうか
    float32_t4x4 uvTransform; //uvTransform
};
ConstantBuffer<Material> gMaterial : register(b0);

struct DirectionalLight
{
    float32_t4 color; //色
    float32_t3 direction; //方向
    float32_t intensity; //強度
};
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

Texture2D<float32_t4> gTexture : register(t0); //SRVのregisterはt
SamplerState gSampler : register(s0); //Samplerはs

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{

    PixelShaderOutput output;

    
    //UV座標を変換する
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    //texture.colorのa値が0.5以下のときPixelを破棄(空白で塗りつぶされないように)
    if (textureColor.a <= 0.5f)
    {
        discard;
    }
    
    if (gMaterial.enableLighting != 0)//Lightingする場合
    {
        ///最初に宣言
        float cos = 0;
        
        //ランバート反射を使うかどうか
        if (gMaterial.useLambertianReflectance != 0)
        {
            cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
        }
        else
        {
            float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
            cos = pow(NdotL * 0.5 + 0.5f, 2.0f);
        }
        output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
    }
    else
    { ////Lightingしない場合
      //サンプリングしたtextureの色とマテリアルん色を乗算して合成する
        output.color.rgb = gMaterial.color.rgb * textureColor.rgb;
    }

    // アルファ値：テクスチャとマテリアルの両方を乗算
    output.color.a = gMaterial.color.a * textureColor.a;
    
   //output.colorのa値が0のときPixelを破棄(空白で塗りつぶされないように)
    if (output.color.a == 0.0)
    {
        discard;
    }
    
    return output;
}