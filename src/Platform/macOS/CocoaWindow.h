#pragma once

#ifdef __OBJC__
@class CAMetalLayer;
#else
typedef void CAMetalLayer;
#endif

namespace gore
{

struct CocoaWindow
{
    CAMetalLayer* layer;
};

} // namespace gore
