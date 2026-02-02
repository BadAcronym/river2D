#include "river2D_main.h"
#include "linux_river2Dsoftware_platform.h"

#include <stdio.h>

#define __USE_POSIX199309
#include <time.h>

void river2D_loadConfig
(
    River2D_Config *config
){
    //TODO: parse & load from file
    config->static_canvas_enable = false;
    config->backgrounds = 4;
    // config->static_canvas_width  = 1280;
    // config->static_canvas_height = 720;
}

void river2D_init
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

    engine->width  = WidthOfScreen(engine->screen);
    engine->height = HeightOfScreen(engine->screen);

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

    river2D_loadConfig(&engine->config);
    engine->running = true;

    if(!engine->config.static_canvas_enable)
    {
        river2D_resizeBackbuffer(engine, WidthOfScreen(engine->screen), HeightOfScreen(engine->screen));
    }
}

int32_t river2D_shutdown
(
    EngineData *engine
){
    XFreeGC(engine->display, engine->context);
    XDestroyWindow(engine->display, engine->window);
    XCloseDisplay(engine->display);

    return 0;
}

Window river2D_openWindow
(
    EngineData *engine
){
    Window window = XCreateWindow(engine->display, XDefaultRootWindow(engine->display), 0, 0,
                                  engine->width, engine->height,  0, 0,
                                  InputOutput, CopyFromParent, 0, 0);

    XStoreName(engine->display, window, engine->windowName);

    XSelectInput(engine->display, window, KeyPressMask | KeyReleaseMask |
                                  StructureNotifyMask);

    XMapWindow(engine->display, window);

    return window;
}

void river2D_drawFrame
(
    EngineData *engine
){
    //TEST: draw something...
    XDrawRectangle(engine->display, engine->backbuffer, engine->context,
                   engine->width, engine->height, 0, 0);

    river2D_bltBuffer(engine);
}

void river2D_compositeImage
(
    EngineData    *engine,
    River2D_Image *img
){
    if(!img->data)
    {
        fprintf(stderr, "No image to draw.\n");
    }

    Visual visual       = {0};
    visual.visualid     = DirectColor;
    visual.bits_per_rgb = 8;

    //TODAY: use xrender to composit onto the backbuffer
    //shenanigans going on here. I need to figure out some order of operations
    //and conventions.
    XRenderPictFormat *format = XRenderFindStandardFormat(engine->display, PictStandardARGB32);

    Picture backbuffer = XRenderCreatePicture(engine->display, engine->backbuffer, format, 0, 0);
    Picture composite  = XRenderCreatePicture(engine->display, engine->comp_canvas, format, 0, 0);

    XRenderComposite(engine->display, PictOpOver, composite, 0, backbuffer,
                     0, 0, 0, 0, 0, 0, img->width, img->height);

    XImage *ximage = XCreateImage(engine->display, &visual, 24, ZPixmap, 0, (char*)img->data,
                                  img->width, img->height, 32, RIVER2D_BPP * img->width);
    if(!ximage->data)
    {
        fprintf(stderr, "Failed to create XImage.\n");
    }

    XPutImage(engine->display, engine->backbuffer, engine->context, ximage,
              0, 0, 0, 0, img->width, img->height);
}

void river2D_resizeBackbuffer
(
    EngineData *engine,
    uint32_t   width,
    uint32_t   height
){
    if(engine->backbuffer)
    {
        XFreePixmap(engine->display, engine->backbuffer);
    }

    engine->width  = width;
    engine->height = height;

    engine->backbuffer = XCreatePixmap(engine->display, engine->window, engine->width, engine->height, 24);
}

void river2D_bltBuffer
(
    EngineData *engine
){
    XCopyArea(engine->display, engine->backbuffer, engine->window,
              engine->context, 0, 0, engine->width, engine->height, 0, 0);

    XFlush(engine->display);
}

uint64_t river2D_queryTime
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
