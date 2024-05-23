#include "../ShaderLibrary/Utils/Quad.hlsl"

[[vk::combinedImageSampler]][[vk::binding(0, 0)]]
Texture2D<float4> myTexture;
[[vk::combinedImageSampler]][[vk::binding(0, 0)]]
SamplerState mySampler;

struct Attributes
{
    uint vertexID : SV_VertexID;
};

struct Varyings
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

Varyings vs(Attributes IN)
{
    Varyings v;

    uint index = quad_indices[IN.vertexID];

    v.position = float4(quad_pos[index], 0, 1);
    v.uv = quad_uv[index];
    return v;
}

float4 ps(Varyings v) : SV_Target0
{
    return myTexture.Sample(mySampler, v.uv);
}

