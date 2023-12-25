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
class RenderSystem;

ENGINE_CLASS(App)
{
public:
    App(int argc, char** argv);
    virtual ~App();

    NON_COPYABLE(App);

    int Run(int width, int height, const char* title);

protected:
    virtual void Initialize() = 0;
    virtual void Update()     = 0;
    virtual void Render()     = 0;
    virtual void Shutdown()   = 0;

    [[nodiscard]] bool HasArg(const std::string& arg) const;

    virtual void OnWindowResize(Window* window, int width, int height);

    [[nodiscard]] Window* GetWindow() const
    {
        return m_Window;
    }

    friend class Window;
private:
    std::vector<std::string> m_Args;

private:
    Time* m_TimeSystem;
    RenderSystem* m_RenderSystem;

    Window* m_Window;
};

} // namespace gore