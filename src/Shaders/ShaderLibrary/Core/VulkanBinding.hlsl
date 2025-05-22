#ifndef VULKAN_BINDING
#define VULKAN_BINDING

// Set 0 is for global resources, set 1 is for shader resources, set 2 is for material resources
// eg. Declare your resources like this:
// COMBINED_SAMPLE_TEXTURE(GLOBAL, 0, myTexture, float4)
// COMBINED_SAMPLE_TEXTURE(GLOBAL, 1, myMaterialTexture, float4)
// COMBINED_SAMPLE_TEXTURE(SHADER, 0, myShaderTexture, float4)
// COMBINED_SAMPLE_TEXTURE(MATERIAL, 0, myMaterialTexture, float4)
// Binding Model based on resource update frequency
#define GLOBAL_BINDING_DESCRIPTOR_SET 0
#define SHADER_BINDING_DESCRIPTOR_SET 1
#define MATERIAL_BINDING_DESCRIPTOR_SET 2

// Helper macros for binding
#define COMBINED_IMAGE_SAMPLER [[vk::combinedImageSampler]]
#define DESCRIPTOR_SET_BINDING(Binding, Set) [[vk::binding(Binding, Set)]]

// Resource binding types
#define COMBINED_SAMPLE_TEXTURE(Region, Binding, TextureName, Format)\
COMBINED_IMAGE_SAMPLER DESCRIPTOR_SET_BINDING(Binding, Region##_BINDING_DESCRIPTOR_SET) HLSL_TEXTURE_2D(TextureName, Format); \
COMBINED_IMAGE_SAMPLER DESCRIPTOR_SET_BINDING(Binding, Region##_BINDING_DESCRIPTOR_SET) HLSL_SAMPLER(TextureName##Sampler);

#define TEXTURE_2D(Region, Binding, TextureName, Format) DESCRIPTOR_SET_BINDING(Binding, Region##_BINDING_DESCRIPTOR_SET) HLSL_TEXTURE_2D(TextureName, Format)

#define SAMPLER(Region, Binding, SamplerName) DESCRIPTOR_SET_BINDING(Binding, Region##_BINDING_DESCRIPTOR_SET) HLSL_SAMPLER(SamplerName)

#define ARRAY_TEXTURES(Region, Binding, ArrayTextureName, Format) DESCRIPTOR_SET_BINDING(Binding, Region##_BINDING_DESCRIPTOR_SET) HLSL_ARRAY_TEXTURES(ArrayTextureName, Format)
#endif