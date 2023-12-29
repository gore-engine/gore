#include "Handle.h"

#include <vector>
#include <cassert>

namespace gore
{
template <typename ObjectType, typename ImplObjectType>
class Pool
{
    static constexpr uint32_t kListEndSentinel = 0xffffffff;
    struct PoolEntry
    {
        explicit PoolEntry(ImplObjectType& inObj) :
            obj(std::move(inObj))
        {
        }
        ImplObjectType obj = {};
        uint32_t gen       = 1;
        uint32_t nextFree  = kListEndSentinel;
    };
    uint32_t _freeListHead = kListEndSentinel;
    uint32_t _numObjects   = 0;

public:
    std::vector<PoolEntry> objects;

    Handle<ObjectType> create(ImplObjectType&& obj)
    {
        uint32_t idx = 0;
        if (_freeListHead != kListEndSentinel)
        {
            idx              = _freeListHead;
            _freeListHead    = objects[idx].nextFree_;
            objects[idx].obj = std::move(obj);
        }
        else
        {
            idx = (uint32_t)objects.size();
            objects.emplace_back(obj);
        }
        _numObjects++;
        return Handle<ObjectType>(idx, objects[idx].gen);
    }
    void destroy(Handle<ObjectType> handle)
    {
        if (handle.empty())
            return;
        assert(_numObjects > 0); // double deletion
        const uint32_t index = handle.index();
        assert(index < objects.size());
        assert(handle.gen() == objects[index].gen); // double deletion
        objects[index].obj = ImplObjectType{};
        objects[index].gen++;
        objects[index].nextFree_ = _freeListHead;
        _freeListHead            = index;
        _numObjects--;
    }
    const ImplObjectType* get(Handle<ObjectType> handle) const
    {
        if (handle.empty())
            return nullptr;

        const uint32_t index = handle.index();
        assert(index < objects.size());
        assert(handle.gen() == objects[index].gen); // accessing deleted object
        return &objects[index].obj;
    }
    ImplObjectType* get(Handle<ObjectType> handle)
    {
        if (handle.empty())
            return nullptr;

        const uint32_t index = handle.index();
        assert(index < objects.size());
        assert(handle.gen() == objects[index].gen); // accessing deleted object
        return &objects[index].obj;
    }
    Handle<ObjectType> findObject(const ImplObjectType* obj)
    {
        if (!obj)
            return {};

        for (size_t idx = 0; idx != objects.size(); idx++)
        {
            if (objects[idx].obj == *obj)
            {
                return Handle<ObjectType>((uint32_t)idx, objects[idx].gen);
            }
        }

        return {};
    }
    void clear()
    {
        objects.clear();
        _freeListHead = kListEndSentinel;
        _numObjects   = 0;
    }
    uint32_t num() const
    {
        return _numObjects;
    }
};
} // namespace gore