#include "../ShaderLibrary/GlobalConstantBuffer.hlsl"

struct Attributes
{
    float3 positionOS : POSITION;
};

struct PushConstant
{
    float4x4 m;
};

[[vk::push_constant]] PushConstant mvpPushConst;

struct VertOut
{
    float4 pos : SV_Position;
    float3 color : COLOR;
};

VertOut vs(Attributes IN)
{
    VertOut vertOut;
    float4 objVertPos = float4(IN.positionOS, 1);
    vertOut.pos = mul(_VPMatrix, mul(mvpPushConst.m, objVertPos));
    vertOut.pos.y *= -1.0f;
    vertOut.color = objVertPos.xyz + 0.5f;
    return vertOut;
}

float4 ps(VertOut vertOut) : SV_Target0
{
    return float4(vertOut.color, 1.0f);
}