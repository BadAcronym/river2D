#pragma once

#include "river2D_main.h"

// TODO: (river2D #7) allow for other font colours
// TODO: (river2D #17) allow for linebreaks and other escape codes
extern void river2D_loadText
(
    EngineData    *engine,
    River2D_Image *image,
    const char    *text,
    uint8_t       font,
    uint16_t      charsize,
    uint32_t      spacing,
    uint32_t      offsetX,
    uint32_t      offsetY
);

extern void river2D_bltBuffer
(
    EngineData *engine,
    uint8_t factor
);
