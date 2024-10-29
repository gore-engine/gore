#pragma once

#include "Prefix.h"
#include "Draw.h"


namespace gore::renderer
{


struct RendererSort
{
    // custom compare function
    static operator()(const Renderer* a, const Renderer* b) const
    {
        return a->GetRenderQueue() < b->GetRenderQueue();
    }
};



} // namespace gore::renderer