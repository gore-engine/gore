#include "Core/Common.hlsl"
#include "Core/GlobalConstantBuffer.hlsl"

TEXTURE_2D(GLOBAL, 1, _DirectionalShadowmap, float4);
SAMPLER(GLOBAL, 2, _DirectionalShadowmapSampler);