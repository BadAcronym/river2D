#pragma once

#include "river2D_main.h"

extern void river2D_loadText
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

extern void river2D_bltBuffer
(
    EngineData *engine,
    uint8_t factor
);
