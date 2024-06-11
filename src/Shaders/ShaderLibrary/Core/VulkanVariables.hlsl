#ifndef VULKAN_VARIABLES
#define VULKAN_VARIABLES

#define HLSL_STRUCTUREDBUFFER(BufferName, Format) StructuredBuffer<Format> BufferName
#define HLSL_RW_STRUCTUREDBUFFER(BufferName, Format) RWStructuredBuffer<Format> BufferName

#define HLSL_TEXTURE_2D(TextureName, Format) Texture2D<Format> TextureName
#define HLSL_SAMPLER(SamplerName) SamplerState SamplerName

#define SAMPLE_TEXTURE_2D(TextureName, UV) TextureName.Sample(TextureName##Sampler, UV)
#define SAMPLE_TEXTURE_2D_SAMPLER(TextureName, SamplerName, UV) TextureName.Sample(SamplerName, UV)

#endif