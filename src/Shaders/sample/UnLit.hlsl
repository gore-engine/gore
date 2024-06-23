#include "../ShaderLibrary/GlobalConstantBuffer.hlsl"

#define MAIN_LIGHT_DIRECTION float3(0.0f, -1.0f, 0.0f)

struct Attributes
{
    float3 positionOS : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct PushConstant
{
    float4x4 m;
};

[[vk::push_constant]] PushConstant mvpPushConst;

struct Varyings
{
    float4 positionCS : SV_Position;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

Varyings vs(Attributes IN)
{
    Varyings v;
    float4 objVertPos = float4(IN.positionOS, 1);
    v.positionCS = mul(_VPMatrix, mul(mvpPushConst.m, objVertPos));
    v.positionCS.y *= -1.0f;
    v.uv = IN.uv;
    v.normal = IN.normal;
    return v;
}

float4 ps(Varyings v) : SV_Target0
{
    float3 normal = normalize(v.normal);
    float3 lightDir = normalize(MAIN_LIGHT_DIRECTION);
    float lightIntensity = saturate(dot(normal, lightDir) * 0.5f + 0.5f);
    float2 uv = v.uv * .5f + .5f;
    return float4(lightIntensity, lightIntensity, lightIntensity, 1.0f);
}