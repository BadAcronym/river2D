#include "river2D_main.h"
#include "linux_platform.h"

#include "X11/Xlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void X11init
(
    EngineData *engine
){
    engine->display = XOpenDisplay(0);
    if(!engine->display)
    {
        fprintf(stderr, "Failed to open default Display!\n");
    }

    engine->screen = DefaultScreenOfDisplay(engine->display);
    if(!engine->screen)
    {
        fprintf(stderr, "Failed to get default screen!\n");
    }

    engine->dimensions.width = WidthOfScreen(engine->screen);
    engine->dimensions.height = HeightOfScreen(engine->screen);

    engine->windowName = "river2D editor";
    engine->window = river2D_openWindow(engine);
    if(!engine->window)
    {
        fprintf(stderr, "Failed to create window!\n");
    }

    engine->context = XCreateGC(engine->display, engine->window, 0, 0);
    if(!engine->context)
    {
        fprintf(stderr, "Failed to create Graphics Context!\n");
    }

    river2D_resizeBackbuffer(engine, WidthOfScreen(engine->screen), HeightOfScreen(engine->screen));
    engine->running = true;

    //TODAY: load all the UI images and give their pointers to the array

    River2D_Image baseUI = {};
    river2D_loadImage("assets/image.png", &baseUI);

    engine->UI = XCreateImage(engine->display, DefaultVisual(engine->display, 0), 24,
                              ZPixmap, 0, (char*)baseUI.data, baseUI.dimensions.width,
                              baseUI.dimensions.height, 32, 0);
}

int32_t X11shutdown
(
    EngineData *engine
){
    XFreeGC(engine->display, engine->context);
    XDestroyWindow(engine->display, engine->window);
    XCloseDisplay(engine->display);

    return 0;
}

Window X11openWindow
(
    EngineData *engine
){
    Window window = XCreateWindow(engine->display, XDefaultRootWindow(engine->display), 0, 0,
                                  engine->dimensions.width, engine->dimensions.height,  0, 0,
                                  InputOutput, CopyFromParent, 0, 0);

    XStoreName(engine->display, window, engine->windowName);

    XSelectInput(engine->display, window, KeyPressMask | KeyReleaseMask |
                                  StructureNotifyMask);

    XMapWindow(engine->display, window);

    return window;
}

void X11drawFrame
(
    EngineData *engine
){
    //TODAY: move this to init, load premade UI to draw on top of

    //TODAY: figure out how to draw on part of viewport

    //TODAY: stretch it correctly (like with stretchDIBits)

    XPutImage(engine->display, engine->pixmap, engine->context, engine->UI, 0, 0, 0, 0,
              engine->UI->width, engine->UI->height);

    X11bltBuffer(engine);
}

void X11resizeBackbuffer
(
    EngineData *engine,
    uint32_t   width,
    uint32_t   height
){
    if(engine->pixmap)
    {
        XFreePixmap(engine->display, engine->pixmap);
    }

    engine->dimensions.width  = width;
    engine->dimensions.height = height;

    engine->pixmap = XCreatePixmap(engine->display, engine->window, engine->dimensions.width,
                                   engine->dimensions.height, 24);
}

void X11bltBuffer
(
    EngineData *engine
){
    XCopyArea(engine->display, engine->pixmap, engine->window, engine->context, 0, 0,
              engine->dimensions.width, engine->dimensions.height, 0, 0);

    XFlush(engine->display);
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
