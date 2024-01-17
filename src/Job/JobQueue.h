#pragma once

#include "Export.h"

#include "JobHandle.h"

#include <thread>
#include <list>
#include <mutex>
#include <memory>

namespace gore
{

ENGINE_CLASS(JobQueue)
{
public:
    static JobQueue& Create();
    static void Destroy();

    static JobQueue& Get();

    void Push(const std::shared_ptr<JobInvocation>& job);

    std::shared_ptr<JobInvocation> GetJob(bool& isValid);

private:
    JobQueue();
    ~JobQueue();

    std::thread::id m_ThreadId;
    std::list<std::shared_ptr<JobInvocation>> m_JobQueue;
    std::mutex m_Mutex;

    std::shared_ptr<JobInvocation> Pop(bool& isValid);
    std::shared_ptr<JobInvocation> Steal(bool& isValid);
};

} // namespace gore
