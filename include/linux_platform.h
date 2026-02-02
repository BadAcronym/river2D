#pragma once

#include "river2D_main.h"
#include "X11/Xlib.h"

#define openWindow         X11openWindow
#define drawFrame          X11drawFrame
#define resizeBackbuffer   X11resizeBackbuffer
#define updateBackbuffer   X11updateBackbuffer
#define bltBuffer          X11bltBuffer
#define queryTime          X11queryTime

#define Backbuffer         X11Backbuffer

typedef struct X11Backbuffer
{
    void       *memory;
    Dimensions dimensions;
}
X11Backbuffer;

extern Window X11openWindow
(
    Display     *display,
    Dimensions  dimensions,
    const char* windowName
);

extern void X11drawFrame
(
    Display    *display,
    Dimensions dimensions,
    Window     window,
    GC         gc
);

extern void X11resizeBackbuffer
(
    Backbuffer *buf,
    Dimensions dimensions
);

extern void X11updateBackbuffer
(
    Backbuffer *buf
);

extern void X11bltBuffer
(
    Backbuffer *buf,
    void       *window
);

extern uint64_t X11queryTime
(
);
