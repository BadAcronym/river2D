#ifndef RV_SOFTWARE_LINUX
#define RV_SOFTWARE_LINUX

#include "river2D_main.h"

extern void river2D_loadText
(
    EngineData         *engine,
    rvLoadTextSettings *settings
);

extern void river2D_bltBuffer
(
    EngineData *engine,
    uint8_t factor
);

#endif
