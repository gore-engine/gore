#pragma once

#include "Export.h"

#include "JobHandle.h"
#include "JobQueue.h"

#include "Core/Log.h"

#include <type_traits>
#include <vector>
#include <utility>
#include <stdexcept>
#include <cmath>
#include <cassert>

namespace gore
{

#ifdef __cpp_lib_hardware_interference_size
constexpr std::size_t hardware_constructive_interference_size = std::hardware_constructive_interference_size;
constexpr std::size_t hardware_destructive_interference_size = std::hardware_destructive_interference_size;
#else
constexpr std::size_t hardware_constructive_interference_size = 64;
constexpr std::size_t hardware_destructive_interference_size = 64;
#endif

class JobBase
{
public:
    virtual void Run() = 0;
    virtual JobHandle Schedule(size_t batchSize, JobHandle dependency) = 0;
};

template<typename T>
concept AggregateType = std::is_aggregate_v<T>;

template<AggregateType ReadOnlyDataT, AggregateType ReadWriteDataT, AggregateType SharedDataT>
class Job : public JobBase
{
public:
    Job() : m_ReadOnlyData(), m_ReadWriteData(), m_SharedData{}, m_InvocationCount(0) {}
    virtual ~Job() = default;

    void Run() final
    {
        for (size_t i = 0; i < m_InvocationCount; ++i)
        {
            Execute(i, m_ReadOnlyData[i], m_ReadWriteData[i], m_SharedData);
        }
    }

    JobHandle Schedule(size_t batchSize, JobHandle dependency) final
    {
        float batchCountF = static_cast<float>(m_InvocationCount) / static_cast<float>(batchSize);
        int batchCount = batchCountF <= 1.0f ? 1 : static_cast<int>(std::ceil(batchCountF));

//        if (batchSize < PreferredBatchSize())
//        {
//            LOG_STREAM(WARNING) << "Batch size is smaller than preferred. This may result in false sharing. Consider change the batch size to " << PreferredBatchSize() << " or higher." << std::endl;
//        }

        if (dependency == nullptr)
        {
            JobQueue& queue = JobQueue::Get();

            std::shared_ptr<JobInvocation> emptyParentJob = std::make_shared<JobInvocation>();
            emptyParentJob->job = this;
            emptyParentJob->function = [](JobBase*) {};
            emptyParentJob->parent = nullptr;
            emptyParentJob->unfinishedJobs = batchCount + 1;

            queue.Push(emptyParentJob);

            for (int i = 0; i < batchCount; ++i)
            {
                std::shared_ptr<JobInvocation> job = std::make_shared<JobInvocation>();
                job->job = this;
                int startIndex = i * static_cast<int>(batchSize);
                int endIndex = std::min(startIndex + batchSize, m_InvocationCount);
                job->function = [startIndex, endIndex](JobBase* job)
                {
                    Job* j = static_cast<Job*>(job);
                    for (int i = startIndex; i < endIndex; ++i)
                    {
                        j->Execute(i, j->m_ReadOnlyData[i], j->m_ReadWriteData[i], j->m_SharedData);
                    }
                };
                job->parent = emptyParentJob;
                job->unfinishedJobs = 1;

                queue.Push(job);
            }

            return {emptyParentJob};
        }
        else
        {
            // TODO
            throw std::runtime_error("Not implemented");
        }
    }

    JobHandle Schedule(JobHandle dependency = nullptr)
    {
        return Schedule(PreferredBatchSize(), dependency);
    }
    JobHandle Schedule(size_t batchSize)
    {
        return Schedule(batchSize, nullptr);
    }

    virtual void Execute(size_t invocationIndex, const ReadOnlyDataT& readOnlyData, ReadWriteDataT& readWriteData, const SharedDataT& sharedData) = 0;

    void Reserve(size_t capacity)
    {
        m_ReadOnlyData.reserve(capacity);
        m_ReadWriteData.reserve(capacity);
    }

    void Append(ReadOnlyDataT readOnlyData, ReadWriteDataT readWriteData)
    {
        m_ReadOnlyData.push_back(readOnlyData);
        m_ReadWriteData.push_back(readWriteData);
        ++m_InvocationCount;
    }

    void SetShared(SharedDataT sharedData)
    {
        m_SharedData = sharedData;
    }

    void Clear()
    {
        m_ReadOnlyData.clear();
        m_ReadWriteData.clear();
        m_SharedData = {};
        m_InvocationCount = 0;
    }

    const std::vector<ReadWriteDataT>& GetReadWriteData() const
    {
        return m_ReadWriteData;
    }
private:
    // starting from C++17, the STL default allocator will handle alignment for us.
    // see https://timsong-cpp.github.io/cppwp/n4659/default.allocator#allocator.members-2
    // and https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0035r4.html

    std::vector<ReadOnlyDataT> m_ReadOnlyData;
    std::vector<ReadWriteDataT> m_ReadWriteData;
    SharedDataT m_SharedData;
    size_t m_InvocationCount;

    [[nodiscard]] size_t PreferredBatchSize() const
    {
        constexpr size_t align = alignof(ReadWriteDataT);
        constexpr size_t size = sizeof(ReadWriteDataT);

        constexpr size_t stride = (size + align - 1) & ~(align - 1);

        constexpr size_t cacheLineSize = hardware_destructive_interference_size;

        constexpr float batchSizeF = static_cast<float>(cacheLineSize) / static_cast<float>(stride);
        size_t batchSize = batchSizeF <= 1.0f ? 1 : static_cast<size_t>(std::ceil(batchSizeF));

        return batchSize;
    }
};

}