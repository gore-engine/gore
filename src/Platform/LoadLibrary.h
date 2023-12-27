#pragma once

namespace gore
{

void* LoadDynamicLibrary(const char* name);
void UnloadDynamicLibrary(void* lib);
void* LoadSymbol(void* lib, const char* func);

}