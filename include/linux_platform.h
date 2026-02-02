#pragma once

#include "river2D_main.h"
#include "X11/Xlib.h"

#define shutdownRiver2D            X11shutdown
#define initRiver2D                X11init

#define river2D_openWindow         X11openWindow
#define river2D_drawFrame          X11drawFrame
#define river2D_resizeBackbuffer   X11resizeBackbuffer
#define river2D_updateBackbuffer   X11updateBackbuffer
#define river2D_bltBuffer          X11bltBuffer
#define river2D_queryTime          X11queryTime

#define EngineData                 X11EngineData
#define Backbuffer                 X11Backbuffer

typedef struct X11EngineData
{
    bool              running;
    Display           *display;
    Screen            *screen;
    Window            window;
    GC                context;
    River2DControlMap controls;
    Pixmap            pixmap;
    Dimensions        dimensions;
    const char*       windowName;
    XImage            *UI;
    Config            config;
}
X11EngineData;

extern Window X11openWindow
(
    EngineData *engine
);

extern void X11drawFrame
(
    EngineData *engine
);

extern void X11resizeBackbuffer
(
    EngineData *engine,
    uint32_t   width,
    uint32_t   height
);

extern void X11updateBackbuffer
(
    EngineData *engine
);

extern void X11bltBuffer
(
    EngineData *engine
);

extern uint64_t X11queryTime
(
    bool nano
);

extern void X11init
(
    EngineData *engine
);

extern int32_t X11shutdown
(
    EngineData *engine
);
