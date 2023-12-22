#include "Prefix.h"

#include "Core/App.h"
#include "Win32Window.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace gore
{

void App::InitNativeWindowHandle()
{
    auto* w              = new Win32Window();
    w->m_hWnd            = glfwGetWin32Window(m_Window);
    w->m_hInstance       = GetModuleHandle(NULL);
    m_NativeWindowHandle = w;
}

void App::DestroyNativeWindowHandle()
{
    delete static_cast<Win32Window*>(m_NativeWindowHandle);
}

} // namespace gore