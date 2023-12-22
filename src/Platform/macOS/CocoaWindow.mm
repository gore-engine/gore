#include "Prefix.h"

#include "Windowing/Window.h"
#import "CocoaWindow.h"

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

namespace gore
{

void Window::CreateNativeHandle()
{
    auto* w        = new CocoaWindow();
    w->layer       = [CAMetalLayer layer];
    m_NativeHandle = w;

    NSWindow* window = glfwGetCocoaWindow(m_Window);
    [window.contentView setWantsLayer:YES];
    [window.contentView setLayer:w->layer];
}

void Window::DestroyNativeHandle()
{
    auto* w  = static_cast<CocoaWindow*>(m_NativeHandle);
    w->layer = nil;
    delete w;
}

} // namespace gore