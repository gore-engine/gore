#ifndef VULKAN_VARIABLES
#define VULKAN_VARIABLES

#define HLSL_STRUCTUREDBUFFER(BufferName, Format) StructuredBuffer<Format> BufferName
#define HLSL_RW_STRUCTUREDBUFFER(BufferName, Format) RWStructuredBuffer<Format> BufferName

#define HLSL_TEXTURE_2D(TextureName, Format) Texture2D<Format> TextureName
#define HLSL_SAMPLER(SamplerName) SamplerState SamplerName

#define SAMPLE_TEXTURE_2D(TextureName, UV) TextureName.Sample(TextureName##Sampler, UV)
#define SAMPLE_TEXTURE_2D_SAMPLER(TextureName, SamplerName, UV) TextureName.Sample(SamplerName, UV)

#if SUPPORT_BINDLESS
    #define HLSL_ARRAY_TEXTURES(ArrayTextureName, Format) Texture2D<Format> ArrayTextureName[] 
    #define LOAD_ARRAY_TEXTURES(ArrayTextureName, Index) ArrayTextureName[Index]
#else
    #define HLSL_ARRAY_TEXTURES(ArrayTextureName, Format) Texture2D<Format> ArrayTextureName
    #define LOAD_ARRAY_TEXTURES(ArrayTextureName, Index) ArrayTextureName
#endif

#endif