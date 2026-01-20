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

// エリアライト（矩形ライト）の計算関数
float3 CalculateRectLightContribution(
    RectLight rectLight,
    float3 worldPos,
    float3 normal,
    float3 toEye)
{
    // 矩形の4つの頂点を計算
    float3 halfWidth = rectLight.tangent * (rectLight.width * 0.5f);
    float3 halfHeight = rectLight.bitangent * (rectLight.height * 0.5f);
    
    // 矩形の中心から各頂点への位置
    float3 corners[4];
    corners[0] = rectLight.position - halfWidth - halfHeight; // 左下
    corners[1] = rectLight.position + halfWidth - halfHeight; // 右下
    corners[2] = rectLight.position + halfWidth + halfHeight; // 右上
    corners[3] = rectLight.position - halfWidth + halfHeight; // 左上
    
    // 矩形の最も近い点を見つける（簡易近似）
    float3 toLight = rectLight.position - worldPos;
    float distToCenter = length(toLight);
    
    // 矩形のローカル座標系での位置を計算
    float3 localPos = worldPos - rectLight.position;
    float xProj = dot(localPos, rectLight.tangent);
    float yProj = dot(localPos, rectLight.bitangent);
    float zProj = dot(localPos, rectLight.normal);
    
    // 矩形上の最も近い点を計算（クランプ）
    float clampedX = clamp(xProj, -rectLight.width * 0.5f, rectLight.width * 0.5f);
    float clampedY = clamp(yProj, -rectLight.height * 0.5f, rectLight.height * 0.5f);
    
    // 矩形上の最も近い点のワールド座標
    float3 closestPoint = rectLight.position + rectLight.tangent * clampedX + rectLight.bitangent * clampedY;
    
    // 最も近い点への方向ベクトル
    float3 toLightFromClosest = closestPoint - worldPos;
    float distance = length(toLightFromClosest);
    float3 lightDir = toLightFromClosest / distance;
    
    // 両面ライティング: 法線の向きに関係なくライティング
    float NdotL = dot(rectLight.normal, -lightDir);
    
    // 矩形の裏側からの照明は無効
    if (NdotL < 0.0f)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
    
    // 距離減衰を計算（最大距離を矩形の対角線 + α として計算）
    float maxDistance = sqrt(rectLight.width * rectLight.width + rectLight.height * rectLight.height) * 2.0f;
    float attenuation = pow(saturate(1.0f - distance / maxDistance), rectLight.decay);
    
    // 矩形の面積による強度補正（面積が大きいほど明るい）
    float area = rectLight.width * rectLight.height;
    float areaFactor = sqrt(area) * 0.5f; // 面積係数（調整可能）
    
    // 拡散反射を計算
    float3 diffuse = CalculateDiffuse(
        normal,
        lightDir,
        rectLight.color.rgb,
        rectLight.intensity * areaFactor
    );
    
    // 鏡面反射を計算
    float3 specular = CalculateSpecular(
        normal,
        lightDir,
        toEye,
        rectLight.color.rgb,
        rectLight.intensity * areaFactor
    );
    
    // 距離減衰を適用して返す
    return (diffuse + specular) * attenuation;
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
        
        // 3. スポットライトの計算（有効な数だけループ）
        for (int j = 0; j < gLighting.numSpotLights; j++)
        {
            SpotLight spotLight = gLighting.spotLights[j];
            
            // ライトへの方向ベクトルを計算
            float3 spotLightDir = normalize(spotLight.position - input.worldPosition);
            
            // ライトまでの距離を計算
            float distance = length(spotLight.position - input.worldPosition);
            
            // 距離による減衰を計算
            float distanceAttenuation = pow(saturate(-distance / spotLight.distance + 1.0), spotLight.decay);
            
            // 入射角を計算（表面からライトへの方向 と スポットライトの方向 の内積）
            float cosAngle = dot(spotLightDir, -spotLight.direction);
            
            // フォールオフ係数を計算
            // cosAngleがcosFalloffStart（内側）より大きい場合は1.0（最大）
            // cosAngleがcosAngle（外側）より小さい場合は0.0（影響なし）
            // その間は線形補間
            float falloffFactor = saturate((cosAngle - spotLight.cosAngle) / (spotLight.cosFalloffStart - spotLight.cosAngle));
            
            // 拡散反射を計算
            float3 diffuse = CalculateDiffuse(
                normal,
                spotLightDir,
                spotLight.color.rgb,
                spotLight.intensity
            );
            
            // 鏡面反射を計算
            float3 specular = CalculateSpecular(
                normal,
                spotLightDir,
                toEye,
                spotLight.color.rgb,
                spotLight.intensity
            );
            
            // 距離減衰とフォールオフを適用して合計に加算
            totalDiffuse += diffuse * distanceAttenuation * falloffFactor;
            totalSpecular += specular * distanceAttenuation * falloffFactor;
        }
        
        // 4. エリアライト（矩形ライト）の計算（有効な数だけループ）
        for (int k = 0; k < gLighting.numRectLights; k++)
        {
            RectLight rectLight = gLighting.rectLights[k];
            
            // エリアライトの寄与を計算
            float3 rectLightContribution = CalculateRectLightContribution(
                rectLight,
                input.worldPosition,
                normal,
                toEye
            );
            
            // 合計に加算
            totalDiffuse += rectLightContribution;
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
