#pragma once

#include <cstdint>

#include "Export.h"

namespace gore
{

ENGINE_STRUCT(Rect)
{
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
};

}; // namespace gore