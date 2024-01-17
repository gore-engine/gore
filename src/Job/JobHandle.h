#pragma once

#include "Export.h"

#include <atomic>
#include <memory>
#include <functional>

namespace gore
{

class JobBase;

ENGINE_CLASS(JobInvocation)
{
public:
    JobInvocation();

    NON_COPYABLE(JobInvocation);

    JobBase* job;
    std::atomic<int> unfinishedJobs;
    char padding[4];
    std::shared_ptr<JobInvocation> parent;
    std::function<void(JobBase*)> function;

    void Run();
    void Wait() const;
    void Finish();
};

ENGINE_CLASS(JobHandle)
{
public:
    JobHandle(std::nullptr_t);
    JobHandle(const std::shared_ptr<JobInvocation>& job);

    bool operator==(std::nullptr_t) const;

    void Wait();
private:
    std::shared_ptr<JobInvocation> job;
};

} // namespace gore
