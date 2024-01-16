#pragma once

#include "Export.h"

#include <type_traits>
#include <vector>

namespace gore
{

class JobBase;

class JobHandle
{
public:
    JobHandle(std::nullptr_t) { job = nullptr; }
    void Wait();
private:
    JobBase* job;
};

class JobBase
{
public:
    virtual void Run(size_t invocationCount) = 0;
    virtual JobHandle Schedule(size_t invocationCount, size_t batchSize, JobHandle dependency) = 0;
};

template<typename T>
concept AggregateType = std::is_aggregate_v<T>;

template<AggregateType ReadOnlyDataT, AggregateType ReadWriteDataT, AggregateType SharedDataT>
ENGINE_CLASS(Job) : JobBase
{
public:
    void Run(size_t invocationCount);
    JobHandle Schedule(size_t invocationCount, size_t batchSize, JobHandle dependency = nullptr);

    virtual void Execute(size_t invocationIndex, const ReadOnlyDataT& readOnlyData, ReadWriteDataT& readWriteData, const SharedDataT& sharedData) = 0;

    void Reserve(size_t capacity);
    void Append(ReadOnlyDataT readOnlyData, ReadWriteDataT readWriteData);
    void Clear();
private:
    // starting from C++17, the STL default allocator will handle alignment for us.
    // see https://timsong-cpp.github.io/cppwp/n4659/default.allocator#allocator.members-2
    // and https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0035r4.html

    std::vector<ReadOnlyDataT> m_ReadOnlyData;
    std::vector<ReadWriteDataT> m_ReadWriteData;
    SharedDataT m_SharedData;
};

}