#ifndef GORE_QUAD_HLSL_INCLUDE
#define GORE_QUAD_HLSL_INCLUDE

// Quad vertices

static const float2 quad_pos[4] = 
{
    float2(-.5f, -.5f),
    float2(.5f, -.5f),
    float2(.5f, .5f),
    float2(-.5f, .5f)
};

static const float2 quad_uv[4] = 
{
    float2(0, 1),
    float2(1, 1),
    float2(1, 0),
    float2(0, 0)
};

static const float3 quad_color[4] =
{
    float3(1, 0, 0),
    float3(0, 1, 0),
    float3(0, 0, 1),
    float3(1, 1, 1)
};

static const uint quad_indices[6] = 
{
    0, 1, 2,
    0, 2, 3
};

#endif // GORE_QUAD_HLSL_INCLUDE