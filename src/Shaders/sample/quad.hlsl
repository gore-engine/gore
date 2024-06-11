#include "../ShaderLibrary/Utils/Quad.hlsl"
#include "../ShaderLibrary/Core/Common.hlsl"

COMBINED_SAMPLE_TEXTURE(GLOBAL, 0, myTexture, float4)

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
    return SAMPLE_TEXTURE_2D(myTexture, v.uv);
}

