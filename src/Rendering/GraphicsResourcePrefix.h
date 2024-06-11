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

enum class LogicOp : uint8_t
{
    Clear,
    And,
    AndReverse,
    Copy,
    AndInverted,
    NoOp,
    Xor,
    Or,
    Nor,
    Equivalent,
    Invert,
    OrReverse,
    CopyInverted,
    OrInverted,
    Nand,
    Set,
    Count
};

enum class CompareOp : uint8_t
{
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    AlwaysTrue,
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

FLAG_ENUM_CLASS(ShaderStage, uint8_t)