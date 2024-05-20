#include "../ShaderLibrary/Utils/Quad.hlsl"

struct Attributes
{
    uint vertexID : SV_VertexID;
};

struct Varyings
{
    float4 position : SV_Position;
    float3 color : COLOR;
};

Varyings vs(Attributes IN)
{
    Varyings v;

    uint index = quad_indices[IN.vertexID];

    v.position = float4(quad_pos[index], 0, 1);
    v.color = quad_color[index];

    return v;
}

float4 ps(Varyings v) : SV_Target0
{
    return float4(v.color, 1.0f);
}

