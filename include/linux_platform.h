#pragma once

#include "river2D_main.h"
#include "X11/Xlib.h"

#define river2D_openWindow         X11openWindow
#define river2D_drawFrame          X11drawFrame
#define river2D_resizeBackbuffer   X11resizeBackbuffer
#define river2D_updateBackbuffer   X11updateBackbuffer
#define river2D_bltBuffer          X11bltBuffer
#define river2D_queryTime          X11queryTime
#define river2D_shutdown           X11shutdown

#define Backbuffer                 X11Backbuffer

typedef struct X11Backbuffer
{
    Display    *display;
    Window     window;
    GC         gc;
    Pixmap     pixmap;
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
    Backbuffer *buf
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
    Backbuffer *buf
);

extern uint64_t X11queryTime
(
    bool nano
);

extern int32_t X11shutdown
(
    Display *display,
    Window  window,
    GC      gc
);
