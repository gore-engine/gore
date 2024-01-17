#pragma once

#include "JobHandle.h"

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace gore
{

class WorkerThread
{
public:
    static void Spawn();
    static void Spawn(unsigned int threadCount);
    static void Shutdown();

    static unsigned int GetThreadCount();

    static void Wake();

private:
    std::thread m_Thread;

    static void ThreadFunction(WorkerThread* self, unsigned int threadId);

    std::mutex m_ShouldWaitMutex;
    std::condition_variable m_ShouldWait;
    std::atomic<bool> m_Awake = false;
};

}

