#include "../ShaderLibrary/Core/Common.hlsl"
#include "../ShaderLibrary/Core/GlobalConstantBuffer.hlsl"

struct Attributes
{
    float3 positionOS : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct PerDrawData
{
    float4x4 m[128];
};

DESCRIPTOR_SET_BINDING(0, 3) ConstantBuffer<PerDrawData> perDrawData;

struct Varyings
{
    float4 positionCS : SV_Position;
};

Varyings vs(Attributes IN)
{
    Varyings v;
    float4 objVertPos = float4(IN.positionOS, 1);
    v.positionCS = mul(_DirectionalLightVPMatrix, mul(perDrawData.m[0], objVertPos));
    return v;
}

void ps(Varyings v)
{
}

