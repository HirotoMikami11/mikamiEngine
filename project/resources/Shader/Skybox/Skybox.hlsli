// Skybox マテリアルデータ
struct SkyboxMaterialData
{
    float32_t4 color;
};

// 頂点シェーダー入力（位置のみ）
struct VertexShaderInput
{
    float32_t4 position : POSITION0;
};

// 頂点シェーダー出力
struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t3 texcoord : TEXCOORD0; // TextureCubeのサンプリング方向
};

// ピクセルシェーダー出力
struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};
