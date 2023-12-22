#pragma once

#include <X11/Xlib.h>

namespace gore
{

struct X11Window
{
    Display* display;
    Window window;
};

} // namespace gore
