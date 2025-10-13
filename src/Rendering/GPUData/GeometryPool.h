#pragma once

#include "Utilities/Allocator/HashAllocatePool.h"

using namespace gore::utils;

namespace gore::gfx
{
struct GeometryData
{
    uint32_t vertexCount  = 0;
    uint32_t indexCount   = 0;
    uint32_t vertexOffset = 0;
    uint32_t indexOffset  = 0;

    uint32_t texcoord0Offset = 0;
    uint32_t texcoord1Offset = 0;
    uint32_t texcoord2Offset = 0;
    uint32_t texcoord3Offset = 0;

    uint32_t normalOffset  = 0;
    uint32_t tangentOffset = 0;
    uint32_t pad0          = 0;
    uint32_t pad1          = 0;
};
} // namespace gore::gfx