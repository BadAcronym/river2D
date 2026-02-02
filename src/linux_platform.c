//NOTE: only X11 support for now.

#include "river2D_main.h"
#include "linux_platform.h"

#include "X11/Xlib.h"

#include <cstdlib>
#include <stdio.h>

Window X11openWindow
(
    Display     *display,
    Dimensions  dimensions,
    const char* windowName
){
    Window window = XCreateWindow(display, XDefaultRootWindow(display), 0, 0,
                                  dimensions.width, dimensions.height,  0, 0,
                                  InputOutput, CopyFromParent, 0, 0);

    XStoreName(display, window, windowName);

    XSelectInput(display, window, KeyPressMask | KeyReleaseMask
                                  | StructureNotifyMask);

    XMapWindow(display, window);

    return window;
}

void X11drawFrame
(
    Display    *display,
    Dimensions dimensions,
    Window     window,
    GC         gc
){
    //TODAY: load image from stbi or something
    XSetBackground(display, gc, 0x000000);
    XFillRectangle(display, window, gc, 0, 0, dimensions.width, dimensions.height);
    XFlush(display);
}

void X11resizeBackbuffer
(
    Backbuffer  *buf,
    Dimensions  dimensions
){
    if(buf->memory)
    {
        free(buf->memory);
    }

    buf->dimensions = dimensions;
    buf->memory = malloc(buf->dimensions.width * buf->dimensions.height * River2D_BPP);

    //debug
    printf("%s", "resized Backbuffer to ");
    printf("%u", buf->dimensions.width);
    printf("%s", "x");
    printf("%u", buf->dimensions.height);
    printf("%s", "\n");
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
