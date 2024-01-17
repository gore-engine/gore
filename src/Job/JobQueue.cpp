#include "Prefix.h"

#include "JobQueue.h"
#include "JobHandle.h"
#include "WorkerThread.h"

#include "Core/Log.h"

#include <cassert>
#include <map>
#include <shared_mutex>

namespace gore
{

// TODO: we might want to restrict JobQueue to only be instantiated in worker threads
static thread_local JobQueue* t_JobQueue;
static std::map<std::thread::id, JobQueue*> s_JobQueues;
static std::shared_mutex s_JobQueuesMutex;

JobQueue::JobQueue()
    : m_ThreadId(std::this_thread::get_id())
{
    std::unique_lock<std::shared_mutex> lock(s_JobQueuesMutex);
    assert(s_JobQueues.find(m_ThreadId) == s_JobQueues.end());
    s_JobQueues[m_ThreadId] = this;
}

JobQueue::~JobQueue()
{
    std::unique_lock<std::shared_mutex> lock(s_JobQueuesMutex);
    assert(s_JobQueues.find(m_ThreadId) != s_JobQueues.end());
    s_JobQueues.erase(m_ThreadId);
}

JobQueue& JobQueue::Get()
{
    assert(std::this_thread::get_id() == t_JobQueue->m_ThreadId);
    return *t_JobQueue;
}

void JobQueue::Push(const std::shared_ptr<JobInvocation>& job)
{
    assert(std::this_thread::get_id() == m_ThreadId);

    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_JobQueue.push_back(job);
    }

    WorkerThread::Wake();
}

std::shared_ptr<JobInvocation> JobQueue::Pop(bool& isValid)
{
    assert(std::this_thread::get_id() == m_ThreadId);
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (!m_JobQueue.empty())
        {
            std::shared_ptr<JobInvocation> job = m_JobQueue.back();
            m_JobQueue.pop_back();
            isValid = true;
            return job;
        }
    }
    isValid = false;
    return nullptr;
}

std::shared_ptr<JobInvocation> JobQueue::Steal(bool& isValid)
{
    assert(std::this_thread::get_id() != m_ThreadId);
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (!m_JobQueue.empty())
        {
            std::shared_ptr<JobInvocation> job = m_JobQueue.front();
            m_JobQueue.pop_front();
            isValid = true;
            return job;
        }
    }
    isValid = false;
    return nullptr;
}

std::shared_ptr<JobInvocation> JobQueue::GetJob(bool& isValid)
{
    // first, try pop one from self
    std::shared_ptr<JobInvocation> job = Pop(isValid);
    if (isValid)
        return job;

    // then, try steal one from others
    // TODO: use random number generator to pick a random thread to steal from?
    {
        std::shared_lock<std::shared_mutex> lock(s_JobQueuesMutex);
        for (auto& pair : s_JobQueues)
        {
            if (!(pair.first != m_ThreadId))
                continue;
            job = pair.second->Steal(isValid);
            if (isValid)
                return job;
        }
    }

    // if we still don't have one, yield and return invalid
    std::this_thread::yield();
    isValid = false;
    return nullptr;
}

JobQueue& JobQueue::Create()
{
    assert(t_JobQueue == nullptr);
    t_JobQueue = new JobQueue();
    return *t_JobQueue;
}

void JobQueue::Destroy()
{
    assert(t_JobQueue != nullptr);
    delete t_JobQueue;
    t_JobQueue = nullptr;
}

}