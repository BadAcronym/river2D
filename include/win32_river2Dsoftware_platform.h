#ifndef RV_SOFTWARE_WIN32
#define RV_SOFTWARE_WIN32

#include "river2D_main.h"

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
    u08        pictop,
    u32        offsetDstX,
    u32        offsetDstY,
    u32        offsetSrcX,
    u32        offsetSrcY,
    u32        cropWidth,
    u32        cropHeight
);

EXPORT void _loadText
(
    EngineData *engine,
    RiverImage *image,
    StringView *sv,
    u08        font,
    u16        charsize,
    u32        spacing,
    u32        offsetX,
    u32        offsetY
);

#endif
