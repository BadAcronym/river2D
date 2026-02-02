//NOTE: only X11 support for now.

#include "river2D_main.h"
#include "linux_platform.h"

#include "X11/Xlib.h"

Window X11openWindow
(
    Display     *display,
    Dimensions  dimensions,
    const char* windowName
){
    Window window = XCreateWindow(display, XDefaultRootWindow(display), 0, 0,
                                  dimensions.width, dimensions.height, 0, 0,
                                  InputOutput, CopyFromParent, 0, 0);

    XStoreName(display, window, windowName);
    XSelectInput(display, window, KeyPressMask|KeyReleaseMask|StructureNotifyMask);
    XMapWindow(display, window);

    return window;
}

Backbuffer* X11allocateBackbuffer
(
    Dimensions dimensions
){
    Backbuffer *buf = {0};

    //TODO:

    return buf;
}

void X11updateBackbuffer
(
    Backbuffer *buf
){
}

void X11bltBuffer
(
    Backbuffer *buf,
    void       *window
){
    //TODO: blt backbuffer to X11 window
}

uint64_t X11queryTime
(
){
    //TODO: equiv of QueryPerformanceCounter
    return 0;
}
