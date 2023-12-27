#include "Prefix.h"

#include "Platform/LoadLibrary.h"

#include <dlfcn.h>

namespace gore
{

void* LoadDynamicLibrary(const char* name)
{
    return dlopen(name, RTLD_LOCAL | RTLD_NOW);
}

void UnloadDynamicLibrary(void* lib)
{
    dlclose(lib);
}

void* LoadSymbol(void* lib, const char* func)
{
    return dlsym(lib, func);
}

} // namespace gore