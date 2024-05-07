[[vk::combinedImageSampler]] [[vk::binding(0, 0)]]
Texture2D _MainTex;
[[vk::combinedImageSampler]] [[vk::binding(0, 0)]]
SamplerState _MainTexSampler;

static const float2 quad_pos[6] = 
{
    float2(-.5f, -.5f),
    float2(.5f, .5f),
    float2(-.5f, .5f),
    float2(-.5f, -.5f),
    float2(.5f, -.5f),
    float2(.5f, .5f)
};

static const float2 quad_uv[6] =
{
    float2(0.0f, 1.0f),
    float2(1.0f, 0.0f),
    float2(0.0f, 0.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 1.0f),
    float2(1.0f, 0.0f)
};

struct Attributes
{
    uint vertexID : SV_VertexID;
};

struct Varyings
{
    float4 position : SV_Position;
    float2 texcoord0 : TEXCOORD0;
};

struct PushConstant
{
    float4x4 m;
};

[[vk::push_constant]] PushConstant mvpPushConst;

Varyings vs(Attributes IN)
{
    Varyings v;

    v.position = float4(quad_pos[IN.vertexID], 0, 1);
    v.texcoord0 = quad_uv[IN.vertexID];

    return v;
}

float4 ps(Varyings v) : SV_Target0
{
    float4 color = _MainTex.Sample(_MainTexSampler, v.texcoord0);

    return color;
}