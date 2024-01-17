#include "Prefix.h"

#include "Platform/Threading.h"

#include <pthread.h>

namespace gore
{

void SetCurrentThreadName(const char* name)
{
    pthread_setname_np(pthread_self(), name);
}

}
