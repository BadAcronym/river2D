#ifndef RV_SOFTWARE_WIN32
#define RV_SOFTWARE_WIN32

#include "river2D_main.h"

#include <stdint.h>

#define EXPORT __declspec(dllexport)

EXPORT void _init
(
    EngineData *engine,
    RiverImage *planes
);

EXPORT int32_t _shutdown
(
    EngineData *engine
);

EXPORT void _bltBuffer
(
    EngineData *engine
);

EXPORT void _compositeImage
(
    EngineData *engine,
    RiverImage *src,
    RiverImage *dst,
    uint8_t    pictop,
    uint32_t   offsetDstX,
    uint32_t   offsetDstY,
    uint32_t   offsetSrcX,
    uint32_t   offsetSrcY,
    uint32_t   cropWidth,
    uint32_t   cropHeight
);

EXPORT void _loadText
(
    EngineData *engine,
    RiverImage *image,
    StringView *sv,
    uint8_t    font,
    uint16_t   charsize,
    uint32_t   spacing,
    uint32_t   offsetX,
    uint32_t   offsetY
);

#endif
