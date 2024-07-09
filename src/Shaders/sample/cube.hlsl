#include "../ShaderLibrary/Core/Common.hlsl"
#include "../ShaderLibrary/GlobalConstantBuffer.hlsl"
#include "../ShaderLibrary/Utils/Cube.hlsl"

struct Attributes
{
    uint vertexID : SV_VertexID;
};

struct PerDrawData
{
    float4x4 m;
};

DESCRIPTOR_SET_BINDING(0, 3) ConstantBuffer<PerDrawData> perDrawData;

struct VertOut
{
    float4 pos : SV_Position;
    float3 color : COLOR;
};

VertOut vs(Attributes IN)
{
    VertOut vertOut;
    float4 objVertPos = float4(cube_pos[cube_indices[IN.vertexID]], 1);
    vertOut.pos = mul(_VPMatrix, mul(perDrawData.m, objVertPos));
    vertOut.pos.y *= -1.0f;
    vertOut.color = objVertPos.xyz + 0.5f;
    return vertOut;
}

float4 ps(VertOut vertOut) : SV_Target0
{
    return float4(vertOut.color, 1.0f);
}