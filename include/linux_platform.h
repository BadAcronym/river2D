#pragma once

#include "river2D_main.h"
#include "X11/Xlib.h"

#define openWindow         X11openWindow
#define drawFrame          X11drawFrame
#define allocateBackbuffer X11allocateBackbuffer
#define updateBackbuffer   X11updateBackbuffer
#define bltBuffer          X11bltBuffer
#define queryTime          X11queryTime

#define Backbuffer         X11Backbuffer

typedef struct X11Backbuffer
{
    void       *address;
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

//TODO: do we need this? look at CPong
extern Backbuffer* X11allocateBackbuffer
(
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
