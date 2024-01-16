#pragma once

#include "Export.h"

#include <string>

typedef struct GLFWwindow GLFWwindow;

namespace gore
{

class App;

ENGINE_CLASS(Window)
{
public:
    explicit Window(App* app, int width, int height);
    ~Window();

    NON_COPYABLE(Window);

    [[nodiscard]] GLFWwindow* Get() const { return m_Window; }

    void GetSize(int* width, int* height) const;

    [[nodiscard]] void* GetNativeHandle() const
    {
        return m_NativeHandle;
    }

    [[nodiscard]] bool ShouldClose() const;

    void SetTitle(const std::string& title);

private:
    App* m_App;

    GLFWwindow* m_Window;
    void* m_NativeHandle;

    static void OnWindowResizeCallback(GLFWwindow * window, int width, int height);

    void CreateNativeHandle();
    void DestroyNativeHandle();
};

} // namespace gore
