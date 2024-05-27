#pragma once

#include "Prefix.h"

namespace gore::utils
{
    template <typename T>
    struct StdHash
    {
        size_t operator()(const T& value) const
        {
            return std::hash<T>()(value);
        }
    };

    template <typename T>
    inline void hash_combine(std::size_t& seed, const T& v)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
} // namespace gore::utils