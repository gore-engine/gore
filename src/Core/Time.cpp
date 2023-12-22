#include "Prefix.h"

#include "Time.h"

namespace gore
{

static Time* g_Time = nullptr;

Time::Time(App* app) :
    System(app)
{
    g_Time = this;
}

Time::~Time()
{
    g_Time = nullptr;
}

void Time::Initialize()
{
    m_LastTime    = std::chrono::steady_clock::now();
    m_CurrentTime = m_LastTime;
    m_StartTime   = m_LastTime;
}

void Time::Update()
{
    m_LastTime    = m_CurrentTime;
    m_CurrentTime = std::chrono::steady_clock::now();
}

void Time::Shutdown()
{
}

float Time::GetDeltaTime()
{
    return std::chrono::duration<float>(m_CurrentTime - m_LastTime).count();
}

float Time::GetTotalTime()
{
    return std::chrono::duration<float>(m_CurrentTime - m_StartTime).count();
}

} // namespace gore

float GetDeltaTime()
{
    // check g_Time
    return gore::g_Time->GetDeltaTime();
}

float GetTotalTime()
{
    // check g_Time
    return gore::g_Time->GetTotalTime();
}