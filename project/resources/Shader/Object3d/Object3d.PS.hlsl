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

// 統合ライティングデータ
ConstantBuffer<LightingData> gLighting : register(b1);

// カメラ情報（鏡面反射用）
ConstantBuffer<Camera> gCamera : register(b2);

Texture2D<float32_t4> gTexture : register(t0); //SRVのregisterはt
SamplerState gSampler : register(s0); //Samplerはs

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

// 拡散反射を計算する関数
float3 CalculateDiffuse(float32_t3 normal, float32_t3 lightDir, float32_t3 lightColor, float32_t lightIntensity)
{
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
        // HalfLambertと同じ
        float NdotL = dot(normal, lightDir);
        diffuse = pow(NdotL * 0.5f + 0.5f, 2.0f);
    }
    
    return lightColor * diffuse * lightIntensity;
}

// 鏡面反射を計算する関数（PhongSpecular用）
float3 CalculateSpecular(float32_t3 normal, float32_t3 lightDir, float32_t3 toEye, float32_t3 lightColor, float32_t lightIntensity)
{
    if (gMaterial.lightingMode != 3) // PhongSpecularモードでない場合
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
    
    // ハーフベクトルを計算
    float3 halfVector = normalize(lightDir + toEye);
    float NDotH = dot(normal, halfVector);
    
    // 鏡面反射の強度を計算
    float specularPow = pow(saturate(NDotH), gMaterial.shininess);
    
    // 鏡面反射の色（白色）
    return float3(1.0f, 1.0f, 1.0f) * specularPow * lightColor * lightIntensity;
}

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
        float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
        
        // 合計の拡散反射と鏡面反射
        float3 totalDiffuse = float3(0.0f, 0.0f, 0.0f);
        float3 totalSpecular = float3(0.0f, 0.0f, 0.0f);
        
        // 1. 平行光源の計算
        float3 directionalLightDir = normalize(-gLighting.directionalLight.direction);
        totalDiffuse += CalculateDiffuse(
            normal,
            directionalLightDir,
            gLighting.directionalLight.color.rgb,
            gLighting.directionalLight.intensity
        );
        totalSpecular += CalculateSpecular(
            normal,
            directionalLightDir,
            toEye,
            gLighting.directionalLight.color.rgb,
            gLighting.directionalLight.intensity
        );
        
        // 2. ポイントライトの計算（有効な数だけループ）
        for (int i = 0; i < gLighting.numPointLights; i++)
        {
            PointLight pointLight = gLighting.pointLights[i];
            
            // ライトへの方向ベクトルを計算
            float3 pointLightDir = normalize(pointLight.position - input.worldPosition);
            
            // ライトまでの距離を計算
            float distance = length(pointLight.position - input.worldPosition);
            
            // 距離減衰を計算（指数によるコントロール）
            float attenuation = pow(saturate(-distance / pointLight.radius + 1.0), pointLight.decay);
            
            // 拡散反射を計算
            float3 diffuse = CalculateDiffuse(
                normal,
                pointLightDir,
                pointLight.color.rgb,
                pointLight.intensity
            );
            
            // 鏡面反射を計算
            float3 specular = CalculateSpecular(
                normal,
                pointLightDir,
                toEye,
                pointLight.color.rgb,
                pointLight.intensity
            );
            
            // 減衰を適用して合計に加算
            totalDiffuse += diffuse * attenuation;
            totalSpecular += specular * attenuation;
        }
        
        // 拡散反射にマテリアル色とテクスチャ色を適用
        float3 diffuseColor = gMaterial.color.rgb * textureColor.rgb * totalDiffuse;
        
        // 拡散反射と鏡面反射を合成
        output.color.rgb = diffuseColor + totalSpecular;
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
