#ifndef GORE_TRIANGLE_HLSL_INCLUDE
#define GORE_TRIANGLE_HLSL_INCLUDE

// Triangle vertices

static const float2 triangle_pos[3] = 
{
    float2(.0f, -.5f),
    float2(.5f, .5f),
    float2(-.5f, .5f)
};

static const float3 triangle_color[3] =
{
    float3(1, 0, 0),
    float3(0, 1, 0),
    float3(0, 0, 1)
};

#endif // GORE_TRIANGLE_HLSL_INCLUDE