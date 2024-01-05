#include "Handle.h"

#include <vector>
#include <cassert>

namespace gore
{
template <typename ObjectDesc, typename ImplObjectType>
class Pool
{
    static constexpr uint32_t kListEndSentinel = 0xffffffff;
    struct PoolEntryDesc
    {
        explicit PoolEntryDesc(ObjectDesc& inObj) :
            objDesc(std::move(inObj))
        {
        }
        ObjectDesc objDesc = {};
        uint32_t gen       = 1;
        uint32_t nextFree  = kListEndSentinel;
    };
    uint32_t _freeListHead = kListEndSentinel;
    uint32_t _numObjects   = 0;

public:
    std::vector<PoolEntryDesc> objectDesc;
    std::vector<ImplObjectType> objects;

    Handle<ObjectDesc> create(ObjectDesc&& desc, ImplObjectType&& obj)
    {
        uint32_t idx = 0;
        if (_freeListHead != kListEndSentinel)
        {
            idx                     = _freeListHead;
            _freeListHead           = objectDesc[idx].nextFree;
            objectDesc[idx].objDesc = std::move(desc);
            objects[idx]            = std::move(obj);
        }
        else
        {
            idx = (uint32_t)objectDesc.size();
            objectDesc.emplace_back(desc);
            objects.emplace_back(obj);
        }
        _numObjects++;
        return Handle<ObjectDesc>(idx, objectDesc[idx].gen);
    }
    void destroy(Handle<ObjectDesc> handle)
    {
        if (handle.empty())
            return;
        assert(_numObjects > 0); // double deletion
        const uint32_t index = handle.index();
        assert(index < objectDesc.size());
        assert(handle.gen() == objectDesc[index].gen); // double deletion
        objectDesc[index].objDesc = ObjectDesc{};
        objectDesc[index].gen++;
        objectDesc[index].nextFree = _freeListHead;
        _freeListHead               = index;
        _numObjects--;
    }
    const ImplObjectType* get(Handle<ObjectDesc> handle) const
    {
        if (handle.empty())
            return nullptr;

        const uint32_t index = handle.index();
        assert(index < objectDesc.size());
        assert(handle.gen() == objectDesc[index].gen); // accessing deleted object
        return &objects[index];
    }
    ImplObjectType* get(Handle<ObjectDesc> handle)
    {
        if (handle.empty())
            return nullptr;

        const uint32_t index = handle.index();
        assert(index < objectDesc.size());
        assert(handle.gen() == objectDesc[index].gen); // accessing deleted object
        return &objects[index];
    }
    Handle<ObjectDesc> findObject(const ImplObjectType* obj)
    {
        if (!obj)
            return {};

        for (size_t idx = 0; idx != objectDesc.size(); idx++)
        {
            if (objects[idx] == *obj)
            {
                return Handle<ObjectDesc>((uint32_t)idx, objectDesc[idx].gen);
            }
        }

        return {};
    }
    void clear()
    {
        objectDesc.clear();
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