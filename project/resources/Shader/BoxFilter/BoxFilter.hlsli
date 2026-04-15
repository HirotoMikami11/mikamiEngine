#include "resources/Shader/FullscreenTriangle/FullscreenTriangle.hlsli"

struct BoxFilterParameters
{
    int32_t kernelSize;
    float32_t3 padding;
};
