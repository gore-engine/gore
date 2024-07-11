#ifndef GORE_CUBE_HLSL_INCLUDE
#define GORE_CUBE_HLSL_INCLUDE

// Cube vertices

static const float3 cube_pos[8] = 
{
    float3(-.5f, -.5f, -.5f),
    float3(.5f, -.5f, -.5f),
    float3(.5f, .5f, -.5f),
    float3(-.5f, .5f, -.5f),
    float3(-.5f, -.5f, .5f),
    float3(.5f, -.5f, .5f),
    float3(.5f, .5f, .5f),
    float3(-.5f, .5f, .5f)
};

static const uint cube_indices[36] = 
{
    0, 1, 2,
    0, 2, 3,
    1, 5, 6,
    1, 6, 2,
    5, 4, 7,
    5, 7, 6,
    4, 0, 3,
    4, 3, 7,
    3, 2, 6,
    3, 6, 7,
    4, 5, 1,
    4, 1, 0
};

#endif // GORE_CUBE_HLSL_INCLUDE