#pragma once

#include "n3d_ctr.hpp"

namespace n3d
{
    extern IEngine* g_sys;
    extern CTRRenderer* ctrr;

    extern unique_ptr<CTRVertexCache> vertexCache;
}
