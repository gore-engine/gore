#include "Prefix.h"

#include "Windowing/Window.h"
#include "Win32Window.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace gore
{

void Window::CreateNativeHandle()
{
    auto* w        = new Win32Window();
    w->m_hWnd      = glfwGetWin32Window(m_Window);
    w->m_hInstance = GetModuleHandle(NULL);
    m_NativeHandle = w;
}

void Window::DestroyNativeHandle()
{
    delete static_cast<Win32Window*>(m_NativeHandle);
}

} // namespace gore