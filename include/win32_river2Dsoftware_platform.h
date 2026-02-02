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

EXPORT void river2D_compositeImage
(
    EngineData    *engine,
    River2D_Image *image,
    uint8_t       pictop,
    uint32_t      offsetX,
    uint32_t      offsetY,
    uint32_t      cropX,
    uint32_t      cropY
);

EXPORT void river2D_loadText
(
    EngineData    *engine,
    River2D_Image *image,
    const char    *text,
    uint8_t       font,
    uint16_t      charsize,
    uint32_t      spacing,
    uint32_t      offsetY,
    uint32_t      offsetX
);
