#pragma once

#include <cstdint>

#include <memory>
#include <string>
#include <vector>

#include "Export.h"

namespace gore
{

class Window;

class Time;
class InputSystem;
class RenderSystem;

ENGINE_CLASS(App)
{
public:
    App(int argc, char** argv);
    virtual ~App();

    NON_COPYABLE(App);

    static App* Get();

    int Run(int width, int height, const char* title);

    [[nodiscard]] Window* GetWindow() const
    {
        return m_Window;
    }

protected:
    virtual void Initialize() = 0;
    virtual void Update()     = 0;
    virtual void Shutdown()   = 0;
    virtual void EndofFrame() = 0;

    [[nodiscard]] bool HasArg(const std::string& arg) const;

    virtual void OnWindowResize(Window* window, int width, int height);

    friend class Window;

private:
    friend class FileSystem;
    std::vector<std::string> m_Args;
    std::string m_ExecutablePath;

protected:
    Time* m_TimeSystem;
    InputSystem* m_InputSystem;
    RenderSystem* m_RenderSystem;

private:
    Window* m_Window;
};

} // namespace gore