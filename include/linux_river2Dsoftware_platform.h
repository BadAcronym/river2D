#pragma once

#include "river2D_main.h"

extern Window river2D_openWindow
(
    EngineData *engine
);

extern void river2D_drawFrame
(
    EngineData *engine
);

extern void river2D_resizeBackbuffer
(
    EngineData *engine,
    uint32_t   width,
    uint32_t   height
);

extern void river2D_updateBackbuffer
(
    EngineData *engine
);

extern void river2D_bltBuffer
(
    EngineData *engine
);

extern void river2D_init
(
    EngineData *engine
);

extern int32_t river2D_shutdown
(
    EngineData *engine
);
