#ifndef VULKAN_VARIABLES
#define VULKAN_VARIABLES

#define STRUCTUREDBUFFER(BufferName, Format) StructuredBuffer<Format> BufferName
#define RW_STRUCTUREDBUFFER(BufferName, Format) RWStructuredBuffer<Format> BufferName

#define TEXTURE_2D(TextureName, Format) Texture2D<Format> TextureName
#define SAMPLER(SamplerName) SamplerState SamplerName

#define SAMPLE_TEXTURE_2D(TextureName, UV) TextureName.Sample(TextureName##Sampler, UV)

#endif