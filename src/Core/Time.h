#pragma once

#include "Export.h"

#include "Core/System.h"

#include <chrono>

ENGINE_API_FUNC(float, GetDeltaTime);
ENGINE_API_FUNC(float, GetTotalTime);

namespace gore
{

class Time final : System
{
public:
    explicit Time(App* app);
    ~Time() override;

    Time(const Time&)            = delete;
    Time& operator=(const Time&) = delete;
    Time(Time&&)                 = delete;
    Time& operator=(Time&&)      = delete;

    void Initialize() override;
    void Update() override;
    void Shutdown() override;

    float GetDeltaTime();
    float GetTotalTime();

private:
    std::chrono::time_point<std::chrono::steady_clock> m_LastTime;
    std::chrono::time_point<std::chrono::steady_clock> m_CurrentTime;
    std::chrono::time_point<std::chrono::steady_clock> m_StartTime;
};

} // namespace gore
