// static const float3 vertPos[8] =
// {
//     float3(-0.5f, -0.5f, -0.5f),
//     float3(-0.5f, -0.5f,  0.5f),
//     float3(-0.5f,  0.5f, -0.5f),
//     float3(-0.5f,  0.5f,  0.5f),
//     float3( 0.5f, -0.5f, -0.5f),
//     float3( 0.5f, -0.5f,  0.5f),
//     float3( 0.5f,  0.5f, -0.5f),
//     float3( 0.5f,  0.5f,  0.5f)
// };

// static const int vertIndex[36] =
// {
//     2, 1, 0, 2, 3, 1, // -X
//     4, 5, 6, 6, 5, 7, // +X
//     0, 1, 5, 0, 5, 4, // -Y
//     2, 6, 3, 3, 6, 7, // +Y
//     0, 4, 2, 2, 4, 6, // -Z
//     1, 3, 5, 5, 3, 7  // +Z
// };

struct Attributes
{
    float3 positionOS : POSITION;
};

struct PushConstant
{
    float4x4 m;
    float4x4 vp;
};

[[vk::push_constant]]
PushConstant mvpPushConst;

struct VertOut
{
    float4 pos : SV_Position;
    float3 color : COLOR;
};

VertOut vs(Attributes IN)
{
    VertOut vertOut;
    float4 objVertPos = float4(IN.positionOS, 1.0f);
    vertOut.pos = mul(mvpPushConst.vp, mul(mvpPushConst.m, objVertPos));
    vertOut.pos.y *= -1.0f;
    vertOut.color = objVertPos.xyz + 0.5f;
    return vertOut;
}

float4 ps(VertOut vertOut) : SV_Target0
{
    return float4(vertOut.color, 1.0f);
}