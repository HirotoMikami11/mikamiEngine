#include "resources/Shader/Object3d/Object3d.hlsli"

struct Material
{
    float32_t4 color; //色
    int32_t enableLighting; //ライティングするか否か
    int32_t lightingMode; //ライティングモード（0:None, 1:Lambert, 2:HalfLambert, 3:PhongSpecular）
    float32_t shininess; //光沢度（鏡面反射の鋭さ）
    float32_t padding; //アライメント調整用
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

// カメラ情報（鏡面反射用）
ConstantBuffer<Camera> gCamera : register(b2);

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
    
    if (gMaterial.enableLighting != 0) //Lightingする場合
    {
        float3 normal = normalize(input.normal);
        float3 lightDir = normalize(-gDirectionalLight.direction);
        
        // 拡散反射の計算
        float diffuse = 0.0f;
        
        // ライティングモードによって拡散反射を計算
        if (gMaterial.lightingMode == 1) // Lambert
        {
            diffuse = saturate(dot(normal, lightDir));
        }
        else if (gMaterial.lightingMode == 2) // HalfLambert
        {
            float NdotL = dot(normal, lightDir);
            diffuse = pow(NdotL * 0.5f + 0.5f, 2.0f);
        }
        else if (gMaterial.lightingMode == 3) // PhongSpecular
        {
            //halfLambertと同じ
            float NdotL = dot(normal, lightDir);
            diffuse = pow(NdotL * 0.5f + 0.5f, 2.0f);
            
        }
        
        // 拡散反射の色を計算
        float3 diffuseColor = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * diffuse * gDirectionalLight.intensity;
        
        // 鏡面反射の計算（PhongSpecularモードの場合のみ）
        float3 specularColor = float3(0.0f, 0.0f, 0.0f);
        if (gMaterial.lightingMode == 3) // PhongSpecular
        {
            // カメラへの方向ベクトルを計算
            float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
            
            // 反射ベクトルを計算（入射ベクトルを反射）
            float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
            float NDotH = dot(normal, halfVector);
            
            // 鏡面反射の強度を計算
            float specularPow = pow(saturate(NDotH), gMaterial.shininess);
            
            // 鏡面反射の色（白色）
            specularColor = float3(1.0f, 1.0f, 1.0f) * specularPow * gDirectionalLight.color.rgb * gDirectionalLight.intensity;
        }
        
        // 拡散反射と鏡面反射を合成
        output.color.rgb = diffuseColor + specularColor;
    }
    else
    { //Lightingしない場合
        //サンプリングしたtextureの色とマテリアルの色を乗算して合成する
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