#pragma once

#include <X11/Xlib.h>

namespace gore
{

struct X11Window
{
    Display* display;
    // otherwise it conflicts with the gore::Window class (X11, sigh ...)
    ::Window window;
};

} // namespace gore
