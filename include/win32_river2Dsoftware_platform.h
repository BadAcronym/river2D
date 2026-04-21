#pragma once

#include "river2D_main.h"

#include <stdint.h>

#define EXPORT __declspec(dllexport)

EXPORT void init
(
    EngineData         *engine,
    River2D_Image      *planes
);

EXPORT int32_t shutdown
(
    EngineData *engine
);

EXPORT void bltBuffer
(
    EngineData *engine
);

EXPORT void compositeImage
(
    EngineData    *engine,
    River2D_Image *src,
    River2D_Image *dst,
    uint8_t       pictop,
    uint32_t      offsetDstX,
    uint32_t      offsetDstY,
    uint32_t      offsetSrcX,
    uint32_t      offsetSrcY,
    uint32_t      cropWidth,
    uint32_t      cropHeight
);

EXPORT void loadText
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
