#pragma once

#include "river2D_main.h"

extern Window river2D_openWindow
(
    EngineData *engine
);

extern void river2D_resizeBackbuffer
(
    EngineData *engine,
    uint32_t   width,
    uint32_t   height
);
