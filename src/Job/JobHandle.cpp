#include <cassert>
#include "Prefix.h"

#include "JobHandle.h"
#include "JobQueue.h"

namespace gore
{

JobHandle::JobHandle(std::nullptr_t) :
    job(nullptr)
{
}

JobHandle::JobHandle(const std::shared_ptr<JobInvocation>& job) :
    job(job)
{
}

bool JobHandle::operator==(std::nullptr_t) const
{
    return job == nullptr;
}

void JobHandle::Wait()
{
    if (job != nullptr)
    {
        job->Wait();
    }
}

JobInvocation::JobInvocation() :
    job(nullptr),
    function(),
    parent(nullptr),
    unfinishedJobs(0)
{
}

void JobInvocation::Run()
{
    function(job);
    Finish();
}

void JobInvocation::Wait() const
{
    JobQueue& jobQueue = JobQueue::Get();
    while (unfinishedJobs > 0)
    {
        bool success;
        std::shared_ptr<JobInvocation> jobToRun = jobQueue.GetJob(success);
        if (success)
        {
            jobToRun->Run();
        }
    }
}

void JobInvocation::Finish()
{
    int unfinished = --unfinishedJobs;

    if (unfinished <= 0 && parent != nullptr)
    {
        parent->Finish();
    }
}

} // namespace gore