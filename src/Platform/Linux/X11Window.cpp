#include "Prefix.h"

#include "Core/App.h"
#include "X11Window.h"

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace gore
{

void App::InitNativeWindowHandle()
{
    auto* w              = new X11Window();
    w->display           = glfwGetX11Display();
    w->window            = glfwGetX11Window(m_Window);
    m_NativeWindowHandle = w;
}

void App::DestroyNativeWindowHandle()
{
    delete static_cast<X11Window*>(m_NativeWindowHandle);
}

} // namespace gore