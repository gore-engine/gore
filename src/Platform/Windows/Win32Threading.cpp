#include "Prefix.h"

#include "Platform/Threading.h"

#include <Windows.h>

#include <string>
#include <cstring>

namespace gore
{

void SetCurrentThreadName(const char* name)
{
    std::wstring wname(name, name + strlen(name) + 1);
    HRESULT hr = SetThreadDescription(GetCurrentThread(), wname.c_str());
}

}