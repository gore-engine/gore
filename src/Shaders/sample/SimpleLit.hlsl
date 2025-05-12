#include "../ShaderLibrary/GlobalBinding.hlsl"
#include "../ShaderLibrary/ShadowPassBinding.hlsl"
#include "../ShaderLibrary/BindlessMaterial.hlsl"

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
    float4 positionWS : TEXCOORD1;
    float3 normal : NORMAL;
};

struct PerDrawData
{
    float4x4 objToWorld;
};

DESCRIPTOR_SET_BINDING(0, 3) ConstantBuffer<PerDrawData> perDrawData;

Varyings vs(Attributes IN)
{
    Varyings v;
    float4 objVertPos = float4(IN.positionOS, 1);
    float4 positionWS = mul(perDrawData.objToWorld, objVertPos);
    v.positionWS = positionWS;
    v.positionCS = mul(_VPMatrix, positionWS);
    v.uv = IN.uv;
    v.normal = IN.normal;
    return v;
}

float4 ps(Varyings v) : SV_Target0
{
    float2 uv = v.uv / 0.57735f * .5f + .5f;
    float4 shadowCoord = mul(_DirectionalLightVPMatrix, v.positionWS);
    shadowCoord.xyz /= shadowCoord.w;

    // Apply bias to avoid shadow acne
    shadowCoord.z += 0.005f;

    // Sample the shadow map and perform depth comparison
    float shadowMapDepth = _DirectionalShadowmap.Sample(_DirectionalShadowmapSampler, shadowCoord.xy).r;
    float shadowFactor = shadowCoord.z < shadowMapDepth ? 1.0f : 0.0f;

    Texture2D albedo = LOAD_ARRAY_TEXTURES(_Albedo, 0);

    return albedo.Sample(_AlbedoSampler, uv);
}