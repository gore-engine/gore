struct Attributes
{
    uint vertexID : SV_VertexID;
};

struct Varyings
{
    float4 position : SV_Position;
    float3 color : COLOR;
};

static const float2 triangle_pos[3] = 
{
    float2(.5f, -.5f),
    float2(.5f, .5f),
    float2(-.5f, .5f)
};

static const float3 triangle_color[3] =
{
    float3(1, 0, 0),
    float3(0, 1, 0),
    float3(0, 0, 1)
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