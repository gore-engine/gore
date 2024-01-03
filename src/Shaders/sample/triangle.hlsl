static const float2 vertPos[3] =
{
    float2( 0.5f,  0.5f),
    float2( 0.0f, -0.5f),
    float2(-0.5f,  0.5f)
};

static const float3 vertColor[3] =
{
    float3(1.0f, 0.0f, 0.0f),
    float3(0.0f, 1.0f, 0.0f),
    float3(0.0f, 0.0f, 1.0f)
};

struct VertOut
{
    float4 pos : SV_Position;
    float3 fragColor : COLOR;
};

VertOut vs(int vertID : SV_VertexID)
{
    VertOut vertOut;
    vertOut.pos = float4(vertPos[vertID], 0.0f, 1.0f);
    vertOut.fragColor = vertColor[vertID];
    return vertOut;
}

float4 ps(VertOut vertOut) : SV_Target0
{
    return float4(vertOut.fragColor, 1.0f);
}