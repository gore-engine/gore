#include "Prefix.h"

#include "Platform/LoadLibrary.h"

#include <windows.h>

namespace gore
{

void* LoadDynamicLibrary(const char* name)
{
    return reinterpret_cast<void*>(::LoadLibraryA(name));
}

void UnloadDynamicLibrary(void* lib)
{
    ::FreeLibrary(reinterpret_cast<HMODULE>(lib));
}

void* LoadSymbol(void* lib, const char* func)
{
    return reinterpret_cast<void*>(::GetProcAddress(reinterpret_cast<HMODULE>(lib), func));
}

} // namespace gore