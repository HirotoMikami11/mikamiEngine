// カメラ情報
struct Camera
{
    float32_t3 worldPosition; // カメラのワールド座標
    float32_t padding; // アライメント調整用
};

// 平行光源
struct DirectionalLight
{
    float32_t4 color; // 色
    float32_t3 direction; // 方向
    float32_t intensity; // 強度
};

// ポイントライト
struct PointLight
{
    float32_t4 color; // 色
    float32_t3 position; // 位置
    float32_t intensity; // 強度
    float32_t radius; // 影響範囲
    float32_t decay; // 減衰率
    float32_t2 padding; // アライメント調整
};

// 統合ライティングデータ
struct LightingData
{
    DirectionalLight directionalLight; // 平行光源
    PointLight pointLights[32]; // ポイントライト（最大32個）
    int32_t numPointLights; // 有効なポイントライト数
    float32_t3 padding; // アライメント調整
};

struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
    float32_t3 worldPosition : POSITION0; // ワールド座標
};
