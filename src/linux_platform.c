#include "river2D_main.h"
#include "linux_platform.h"

#include "X11/Xlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

    XSelectInput(display, window, KeyPressMask | KeyReleaseMask |
                                  StructureNotifyMask);

    XMapWindow(display, window);

    return window;
}

void X11drawFrame
(
    Backbuffer *buf
){
    XSetBackground(buf->display, buf->gc, 0xFFFFFF);
    XFillRectangle(buf->display, buf->window, buf->gc, 0, 0, buf->dimensions.width, buf->dimensions.height);

    XFlush(buf->display);
}

void X11resizeBackbuffer
(
    Backbuffer  *buf,
    Dimensions  dimensions
){
    if(buf->pixmap)
    {
        XFreePixmap(buf->display, buf->pixmap);
    }

    buf->dimensions = dimensions;
    buf->pixmap = XCreatePixmap(buf->display, buf->window, dimensions.width, dimensions.height, 24);
}

void X11bltBuffer
(
    Backbuffer *buf
){
    XCopyArea(buf->display, buf->pixmap, buf->window, buf->gc, 0, 0,
              buf->dimensions.width, buf->dimensions.height, 0, 0);
}

uint64_t X11queryTime
(
    bool nano
){
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    if(nano)
    {
        return spec.tv_nsec;
    }

    return spec.tv_sec;
}

int32_t X11shutdown
(
    Display *display,
    Window  window,
    GC      gc
){
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}
