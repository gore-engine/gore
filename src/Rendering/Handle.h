#pragma once
#include <cstddef>

namespace gore
{
// Non-ref counted handles; based on:
// https://enginearchitecture.realtimerendering.com/downloads/reac2023_modern_mobile_rendering_at_hypehype.pdf
template <typename ObjectDesc>
class Handle final
{
public:
    Handle() = default;

    bool empty() const
    {
        return _gen == 0;
    }
    bool valid() const
    {
        return _gen != 0;
    }
    uint32_t index() const
    {
        return _index;
    }
    uint32_t gen() const
    {
        return _gen;
    }
    void* indexAsVoid() const
    {
        return reinterpret_cast<void*>(static_cast<ptrdiff_t>(_index));
    }
    bool operator==(const Handle<ObjectDesc>& other) const
    {
        return _index == other._index && _gen == other._gen;
    }
    bool operator!=(const Handle<ObjectDesc>& other) const
    {
        return _index != other._index || _gen != other._gen;
    }
    // allow conditions 'if (handle)'
    explicit operator bool() const
    {
        return _gen != 0;
    }

private:
    Handle(uint32_t index, uint32_t gen) :
        _index(index),
        _gen(gen){};

    template <typename ObjectType_, typename ImplObjectType>
    friend class Pool;

    uint32_t _index = 0;
    uint32_t _gen   = 0;
};
} // namespace gore