#pragma once
#include "Prefix.h"

enum class MemoryUsage
{
    AUTO,
    CPU,
    GPU,
    CPU_TO_GPU,
    GPU_TO_CPU,
    Count
};

enum class BufferUsage
{
    Vertex,
    Index,
    Uniform,
    Storage,
    TransferSrc,
    TransferDst,
    Indirect,
    RayTracing,
    AccelerationStructure,
    Count
};

enum class TextureType
{
    Tex2D,
    Tex3D,
    Cube,
    Count
};

enum class SamplerFilter
{
    Nearest,
    Linear,
    Count
};

enum class SamplerAddressMode
{
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder,
    MirrorClampToEdge,
    Count
};

enum class SamplerMipmapMode
{
    Nearest,
    Linear,
    Count
};

enum class ShaderStage : uint8_t
{
    Vertex = 1 << 0,
    Fragment = 1 << 1,

    Compute = 1 << 2,

    // Geometry,

    // tessellation    
    // Hull,
    // Domain,
    
    // mesh shading
    Task = 1 << 3,
    Mesh = 1 << 4
    // TODO: RayTracing
};

inline ShaderStage operator|(ShaderStage a, ShaderStage b)
{
    return static_cast<ShaderStage>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline ShaderStage operator&(ShaderStage a, ShaderStage b)
{
    return static_cast<ShaderStage>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
