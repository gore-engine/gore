#pragma once

#include <cstdint>

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "Export.h"

typedef struct GLFWwindow GLFWwindow;

namespace gore
{

ENGINE_CLASS(App)
{
public:
    App(int argc, char** argv);
    virtual ~App();

    App(const App&)            = delete;
    App& operator=(const App&) = delete;
    App(App&&)                 = delete;
    App& operator=(App&&)      = delete;

    int Run(uint32_t width, uint32_t height, const char* title);

    [[nodiscard]] void* GetNativeWindowHandle() const
    {
        return m_NativeWindowHandle;
    }
    void GetWindowSize(int* width, int* height) const;

protected:
    virtual void Initialize()                             = 0;
    virtual void Update(float deltaTime, float totalTime) = 0;
    virtual void Render(float deltaTime, float totalTime) = 0;
    virtual void Shutdown()                               = 0;

    virtual void OnWindowResize(int width, int height) = 0;

    bool HasArg(const std::string& arg) const;
    void SetWindowTitle(const std::string& title);

private:
    std::vector<std::string> m_Args;

    std::chrono::time_point<std::chrono::steady_clock> m_LastTime;
    std::chrono::time_point<std::chrono::steady_clock> m_StartTime;

    GLFWwindow* m_Window;
    void* m_NativeWindowHandle;

    void MainLoop();

    void InitNativeWindowHandle();
    void DestroyNativeWindowHandle();

    static void OnWindowResizeCallback(GLFWwindow * window, int width, int height);

};

} // namespace gore