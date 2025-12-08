#include "resources/Shader/FullscreenTriangle/FullscreenTriangle.hlsli"

// アウトラインエフェクトのパラメータ構造体
struct OutlineParameters
{
    float32_t4 outlineColor; // アウトラインの色
    
    float32_t depthThreshold; // 深度差の閾値
    float32_t normalThreshold; // 法線差の閾値（将来の拡張用）
    float32_t outlineThickness; // アウトラインの太さ
    float32_t outlineStrength; // アウトラインの強度
    
    float32_t2 screenSize; // スクリーンサイズ
    float32_t2 padding; // パディング
};

// 深度値を線形化する関数
float32_t LinearizeDepth(float32_t depth)
{
    float32_t nearPlane = 0.1f;
    float32_t farPlane = 1000.0f;
    
    // 深度値が1.0に近い場合は明示的にfarPlaneとして扱う
    if (depth >= 0.9999f)
    {
        return farPlane;
    }
    
    // 透視投影行列の逆変換
    float32_t z_ndc = depth * 2.0f - 1.0f; // [0,1] -> [-1,1]
    float32_t linearDepth = (2.0f * nearPlane * farPlane) / (farPlane + nearPlane - z_ndc * (farPlane - nearPlane));
    
    return clamp(linearDepth, nearPlane, farPlane);
}
