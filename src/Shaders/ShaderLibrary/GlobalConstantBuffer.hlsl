#ifndef GORE_GLOBAL_CONSTANT_BUFFER
#define GORE_GLOBAL_CONSTANT_BUFFER

[[vk::binding(0, 0)]] cbuffer GlobalConstantBuffer
{
    float4x4 _VPMatrix;
};

#ifndef USE_UNIFIED_GEOMETRY_BUFFER
#define USE_UNIFIED_GEOMETRY_BUFFER (1)
#endif

#if USE_UNIFIED_GEOMETRY_BUFFER
#include "./UGB/UGBGlobalBindGroup.hlsl"
#endif

#endif