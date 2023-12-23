#pragma once

#include <windows.h>

namespace gore
{

struct Win32Window
{
    HINSTANCE m_hInstance;
    HWND m_hWnd;
};

} // namespace gore
