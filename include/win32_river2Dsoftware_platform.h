#pragma once

#include "river2D_main.h"

#include <stdint.h>

#define EXPORT __declspec(dllexport)

EXPORT void river2D_init
(
    EngineData         *engine,
    River2D_Image      *planes
);

EXPORT int32_t river2D_shutdown
(
    EngineData *engine
);

EXPORT void river2D_bltBuffer
(
    EngineData *engine
);
