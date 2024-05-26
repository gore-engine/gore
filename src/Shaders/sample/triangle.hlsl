#include "../ShaderLibrary/Utils/Triangle.hlsl"

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
    v.position = float4(triangle_pos[IN.vertexID], 0, 1);
    v.color = triangle_color[IN.vertexID];
    return v;
}

float4 ps(Varyings v) : SV_Target0
{
    return float4(v.color, 1.0f);
}