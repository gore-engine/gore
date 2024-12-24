#include "../ShaderLibrary/GlobalBinding.hlsl"

struct Attributes
{
    float3 positionOS : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct Varyings
{
    float4 positionCS : SV_Position;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct PerDrawData
{
    float4x4 m;
};

DESCRIPTOR_SET_BINDING(0, 3) ConstantBuffer<PerDrawData> perDrawData;

Varyings vs(Attributes IN)
{
    Varyings v;
    float4 objVertPos = float4(IN.positionOS, 1);
    v.positionCS = mul(_VPMatrix, mul(perDrawData.m, objVertPos));
    v.uv = IN.uv;
    v.normal = IN.normal;
    return v;
}

float4 ps(Varyings v) : SV_Target0
{
    float2 uv = v.uv / 0.57735f * .5f + .5f;
    return float4(uv, 0.0f, 1.0f);
}