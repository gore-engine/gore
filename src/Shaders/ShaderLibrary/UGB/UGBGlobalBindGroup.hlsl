#ifndef GORE_UNIFIED_GEOMETRY_BUFFER_GLOBAL_BINDGROUP
#define GORE_UNIFIED_GEOMETRY_BUFFER_GLOBAL_BINDGROUP

#ifndef UGB_BINDING
#define UGB_BINDING 1
#endif

struct VertexData
{
    float3 positionOS;
#ifdef VERTEX_DATA_HAS_NORMAL
    float3 normalOS;
#endif
#ifdef VERTEX_DATA_HAS_TEXCOORD0
    float2 texcoord0;
#endif
};
// TODO: use VertexxData for simple test
[[vk::binding(UGB_BINDING)]] StructuredBuffer<VertexData> _VertexDataBuffer;

#endif