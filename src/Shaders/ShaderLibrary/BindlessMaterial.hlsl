#pragma once
#include "Core/Common.hlsl"


ARRAY_TEXTURES(MATERIAL, 0, _Albedo, float4);
SAMPLER(MATERIAL, 1, _AlbedoSampler);
// ARRAY_TEXTURES(MATERIAL, 1, _Normal, float4);
// ARRAY_TEXTURES(MATERIAL, 2, _Mask, float4);