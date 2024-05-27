#pragma once

#include "Prefix.h"

#include "HashCaching.h"

#include <unordered_map>

namespace gore::gfx
{
struct ResourceCache final
{
    std::unordered_map<std::size_t, BindLayout> bindLayouts;
};
} // namespace gore::gfx