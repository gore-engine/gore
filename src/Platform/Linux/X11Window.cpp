#include "Prefix.h"

#include "Windowing/Window.h"
#include "X11Window.h"

#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace gore
{

void Window::CreateNativeHandle()
{
    auto* w        = new X11Window();
    w->display     = glfwGetX11Display();
    w->window      = glfwGetX11Window(m_Window);
    m_NativeHandle = w;
}

void Window::DestroyNativeHandle()
{
    delete static_cast<X11Window*>(m_NativeHandle);
}

} // namespace gore