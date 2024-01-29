#ifndef GORE_FALLBACK_ERROR_SHADER
#define GORE_FALLBACK_ERROR_SHADER

#include "../GlobalConstantBuffer.hlsl"

struct Attributes
{
    float3 positionOS : POSITION;
};

struct PushConstant
{
    float4x4 _ObjectToWorldMatrix;
};

[[vk::push_constant]] PushConstant objToWorldPushConst;

struct Varyings
{
    float4 positionCS : SV_Position;
};

Varyings vs(Attributes IN)
{
    Varyings OUT;
    float4 positionOS = float4(IN.positionOS, 1.0f);
    OUT.positionCS = mul(_VPMatrix, mul(objToWorldPushConst._ObjectToWorldMatrix, positionOS));
    return OUT;
}

float4 ps(Varyings IN) : SV_Target0
{
    return float4(1, 0, 1, 1);
}

#endif