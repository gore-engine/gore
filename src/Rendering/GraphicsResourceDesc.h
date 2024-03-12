#pragma once

#include "GraphicsResourcePrefix.h"

namespace gore
{

struct BufferDesc final
{
    const char* debugName = "Noname VertexBufferDesc";
    uint32_t byteSize     = 0;
    BufferUsage usage     = BufferUsage::Vertex;
    MemoryUsage memUsage  = MemoryUsage::GPU;
};

struct TextureDesc final
{
    const char* debugName = "Noname Texture_2D_R8G8B8A8_SRGB";
    TextureType type      = TextureType::Tex2D;
    // use dimentions.z for different texturetypes
    // glm::ivec3 dimentions = glm::ivec3(0);
    GraphicsFormat format = GraphicsFormat::RGBA8_SRGB;
    MemoryUsage memUsage  = MemoryUsage::GPU;
    uint32_t width        = 1;
    uint32_t height       = 1;
    uint32_t depth        = 1;
    int numMips           = 1;
    int numLayers         = 1;
    int numSamples        = 1;
    uint8_t* data         = nullptr;
    uint32_t dataSize     = 0;
};

// TODO: Add more options
// Default values are linear, repeat
struct SamplerDesc final
{
    const char* debugName   = "Noname SamplerDesc";
    SamplerFilter minFilter = SamplerFilter::Linear;
    SamplerFilter magFilter = SamplerFilter::Linear;

    SamplerMipmapMode mipmapMode = SamplerMipmapMode::Linear;

    SamplerAddressMode addressModeU = SamplerAddressMode::Repeat;
    SamplerAddressMode addressModeV = SamplerAddressMode::Repeat;
    SamplerAddressMode addressModeW = SamplerAddressMode::Repeat;
};

struct MaterialDesc final
{
    const char* debugName = "Noname MaterialDesc";
};

struct ShaderModuleDesc final
{
    const char* debugName = "Noname ShaderModuleDesc";
    uint8_t* byteCode     = nullptr;
    uint32_t byteSize     = 0;
    const char* entryFunc = "main";
};
} // namespace gore