#pragma once

#include "Prefix.h"

#include "Utilities/Hash/StdHash.h"

#include "Rendering/BindLayout.h"

#include <map>
#include <vector>

using namespace gore;

// custom injected hash function
namespace std
{
template <typename Key, typename Value>
struct hash<std::map<Key, Value>>
{
    size_t operator()(std::map<Key, Value> const& bindings) const
    {
        size_t result = 0;
        utils::hash_combine(result, bindings.size());
        for (auto const& binding : bindings)
        {
            utils::hash_combine(result, binding.first);
            utils::hash_combine(result, binding.second);
        }
        return result;
    }
};

template <typename T>
struct hash<std::vector<T>>
{
    size_t operator()(std::vector<T> const& bindings) const
    {
        size_t result = 0;
        utils::hash_combine(result, bindings.size());
        for (auto const& binding : bindings)
        {
            utils::hash_combine(result, binding);
        }
        return result;
    }
};

template <>
struct hash<gfx::BindLayoutCreateInfo>
{
    size_t operator()(gfx::BindLayoutCreateInfo const& bindings) const
    {
        size_t result = 0;
        utils::hash_combine(result, bindings.bindings.size());
        for (auto const& binding : bindings.bindings)
        {
            utils::hash_combine(result, binding.binding);
            utils::hash_combine(result, static_cast<uint8_t>(binding.type));
            utils::hash_combine(result, binding.descriptorCount);
            utils::hash_combine(result, static_cast<uint8_t>(binding.stage));
        }
        return result;
    }
};

template <>
struct hash<gfx::BindLayout>
{
    size_t operator()(gfx::BindLayout const& bindings) const
    {
        size_t result = 0;
        utils::hash_combine(result, bindings.layout);
        return result;
    }
};

} // namespace std
