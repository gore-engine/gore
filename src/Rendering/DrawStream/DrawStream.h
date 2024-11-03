#pragma once

#include "Prefix.h"
#include "Draw.h"


namespace gore::renderer
{
union DrawStateMask
{
    struct
    {
        uint32_t pipeline : 1;
        uint32_t bindgroup0 : 1;
        uint32_t bindgroup1 : 1;
        uint32_t bindgroup2 : 1;
        uint32_t indexBuffer : 1;
        uint32_t vertexBuffer : 1;
        uint32_t dynamicBuffer : 1;
        uint32_t indexOffset : 1;
        uint32_t vertexOffset : 1;
        uint32_t instanceOffset : 1;
        uint32_t instanceCount : 1;
        uint32_t dynamicBufferOffset : 1;

        uint32_t pack : 20;
    };

    uint32_t mask;
};
static_assert(sizeof(DrawStateMask) == 4, "DrawStateMask should be 4 bytes");

struct DrawStream
{
};
} // namespace gore::renderer