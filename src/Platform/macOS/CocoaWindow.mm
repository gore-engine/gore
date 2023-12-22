#include "Prefix.h"

#include "Core/App.h"
#import "CocoaWindow.h"

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

namespace gore
{

void App::InitNativeWindowHandle()
{
    auto* w              = new CocoaWindow();
    w->layer             = [CAMetalLayer layer];
    m_NativeWindowHandle = w;

    NSWindow* window = glfwGetCocoaWindow(m_Window);
    [window.contentView setWantsLayer:YES];
    [window.contentView setLayer:w->layer];
}

void App::DestroyNativeWindowHandle()
{
    auto* w  = static_cast<CocoaWindow*>(m_NativeWindowHandle);
    w->layer = nil;
    delete static_cast<CocoaWindow*>(m_NativeWindowHandle);
}

} // namespace gore