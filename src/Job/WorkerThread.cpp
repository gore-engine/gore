#include "Prefix.h"

#include "WorkerThread.h"
#include "JobQueue.h"

#include "Core/Log.h"
#include "Platform/Threading.h"

#include <atomic>
#include <thread>
#include <vector>
#include <memory>

namespace gore
{

static unsigned int s_Concurrency = 0;
static std::vector<WorkerThread*> s_Threads;
static std::atomic<bool> s_ShouldExit = false;

void WorkerThread::Spawn()
{
    unsigned int concurrency = std::thread::hardware_concurrency() - 1;
    if (concurrency < 3)
        concurrency = 3;
    Spawn(concurrency);
}

void WorkerThread::Spawn(unsigned int threadCount)
{
    s_Concurrency = threadCount;

    JobQueue& mainThreadJobQueue = JobQueue::Create();
    for (unsigned int i = 0; i < threadCount; ++i)
    {
        auto* thread = new WorkerThread();
        thread->m_Thread = std::thread(ThreadFunction, thread, i);

        s_Threads.push_back(thread);
    }

    LOG_STREAM(INFO) << "Spawned " << threadCount << " worker threads." << std::endl;
}

void WorkerThread::Shutdown()
{
    s_ShouldExit = true;
    Wake();
    for (auto thread : s_Threads)
    {
        thread->m_Thread.join();
        delete thread;
    }
    s_Threads.clear();

    JobQueue::Destroy();
}

unsigned int WorkerThread::GetThreadCount()
{
    return s_Concurrency;
}

void WorkerThread::ThreadFunction(WorkerThread* self, unsigned int threadId)
{
    unsigned int currentThreadId = threadId;
    std::string threadName = "Worker " + std::to_string(currentThreadId);
    SetCurrentThreadName(threadName.c_str());

    JobQueue& jobQueue = JobQueue::Create();
    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(self->m_ShouldWaitMutex);
            self->m_ShouldWait.wait(lock, [self]() { return self->m_Awake.load(); });
        }

        if (s_ShouldExit)
            break;

        bool isValid;
        while (true)
        {
            std::shared_ptr<JobInvocation> job = jobQueue.GetJob(isValid);
            if (!isValid)
                break;
            job->Run();
            job->Wait();
        }
        self->m_Awake = false;
    }
    JobQueue::Destroy();
}

void WorkerThread::Wake()
{
    for (auto thread : s_Threads)
    {
        std::lock_guard<std::mutex> lock(thread->m_ShouldWaitMutex);
        thread->m_Awake = true;
        thread->m_ShouldWait.notify_one();
    }
}

}