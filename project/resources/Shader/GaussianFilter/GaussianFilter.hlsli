#include "resources/Shader/FullscreenTriangle/FullscreenTriangle.hlsli"

struct GaussianFilterParameters
{
    int32_t kernelSize;
    float32_t sigma;
    float32_t2 padding;
};
