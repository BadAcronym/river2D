#include "river2D_main.h"
#include "linux_river2Dsoftware_platform.h"

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

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

internal Visual* findVisual
(
    Display *display
){
    Visual *visual = {0};

    XVisualInfo visualInfo = {0};
    visualInfo.screen = DefaultScreen(display);
    visualInfo.depth  = RIVER2D_PIXDEPTH;

    int numVisuals;
    XVisualInfo *foundVisuals = XGetVisualInfo(display, VisualScreenMask | VisualDepthMask, &visualInfo,
                                               &numVisuals);
    if(!foundVisuals)
    {
        fprintf(stderr, "No valid visuals could be found for the desired depth of %i.\n", visualInfo.depth);
        return 0;
    }

    XWindowAttributes rootAttributes = {0};
    XGetWindowAttributes(display, XDefaultRootWindow(display), &rootAttributes);

    //TODO: add 24-bit fallback visual
    for(int i = 0; i < numVisuals; ++i)
    {
        if(foundVisuals[i].class == rootAttributes.visual->class &&
           foundVisuals[i].depth == RIVER2D_PIXDEPTH
        ){
            visual = foundVisuals[i].visual;
            break;
        }
    }

    XFree(foundVisuals);

    return visual;
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

    engine->visual = findVisual(engine->display);
    if(!engine->visual)
    {
        fprintf(stderr, "No matching visual could be found.\n");
    }

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

    engine->compDestImg = XCreateImage(engine->display, engine->visual, RIVER2D_PIXDEPTH, ZPixmap, 0,
                                      0, engine->width, engine->height,
                                      RIVER2D_SCANLINE, 0);
    if(!engine->compDestImg)
    {
        fprintf(stderr, "Failed to create compDestImg!\n");
    }

    engine->compSrcImg = XCreateImage(engine->display, engine->visual, RIVER2D_PIXDEPTH, ZPixmap, 0,
                                      0, engine->width, engine->height,
                                      RIVER2D_SCANLINE, 0);
    if(!engine->compSrcImg)
    {
        fprintf(stderr, "Failed to create compSrcImg!\n");
    }
}

//TODO: not safely shutting down for some reason... why?
int32_t river2D_shutdown
(
    EngineData *engine
){
    XFreePixmap(engine->display, engine->backbuffer);
    XFreePixmap(engine->display, engine->compDestBuf);
    XFreePixmap(engine->display, engine->compSrcBuf);

    XDestroyImage(engine->compDestImg);
    XDestroyImage(engine->compSrcImg);

    XFreeGC(engine->display, engine->context);
    XDestroyWindow(engine->display, engine->window);
    XCloseDisplay(engine->display);

    return 0;
}

Window river2D_openWindow
(
    EngineData *engine
){

    unsigned long valuemask = CWBackPixel | CWBorderPixel | CWColormap | CWOverrideRedirect;

    XSetWindowAttributes attributes;
    attributes.background_pixel  = BlackPixel(engine->display, DefaultScreen(engine->display));
    attributes.border_pixel      = BlackPixel(engine->display, DefaultScreen(engine->display));
    attributes.colormap          = XCreateColormap(engine->display, XDefaultRootWindow(engine->display),
                                                   engine->visual, AllocNone);
    attributes.override_redirect = false;

    Window window = XCreateWindow(engine->display, XDefaultRootWindow(engine->display),
                                  0, 0, engine->width, engine->height, 0, RIVER2D_PIXDEPTH,
                                  InputOutput, engine->visual, valuemask, &attributes);

    XStoreName(engine->display, window, engine->windowName);
    XSelectInput(engine->display, window, KeyPressMask | KeyReleaseMask | StructureNotifyMask);
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

//TODO: multi-thread some of this?
void river2D_compositeImage
(
    EngineData    *engine,
    River2D_Image *destImg,
    River2D_Image *srcImg
){
    if(!destImg->data)
    {
        fprintf(stderr, "No image to composite onto.\n");
        return;
    }
    else if(!srcImg->data)
    {
        fprintf(stderr, "No image to composite with.\n");
        return;
    }

    XRenderPictFormat *format = XRenderFindStandardFormat(engine->display, PictStandardARGB32);

    if(destImg->width != engine->width || destImg->height != engine->height)
    {
        river2D_resizeXImage(engine, engine->compDestImg, destImg->width, destImg->height);
    }
    memcpy(engine->compDestImg->data, destImg->data, destImg->width * destImg->height * RIVER2D_BPP);

    if(srcImg->width != engine->width || srcImg->height != engine->height)
    {
        river2D_resizeXImage(engine, engine->compSrcImg, srcImg->width, srcImg->height);
    }
    memcpy(engine->compDestImg->data, srcImg->data, srcImg->width * srcImg->height * RIVER2D_BPP);

    XPutImage(engine->display, engine->compDestBuf, engine->context, engine->compDestImg, 0, 0, 0, 0,
              destImg->width, destImg->height);

    XPutImage(engine->display, engine->compSrcBuf, engine->context, engine->compSrcImg, 0, 0, 0, 0,
              srcImg->width, srcImg->height);

    Picture destPict = XRenderCreatePicture(engine->display, engine->compDestBuf, format, 0, 0);
    Picture srcPict  = XRenderCreatePicture(engine->display, engine->compSrcBuf,  format, 0, 0);
    Picture compPict = XRenderCreatePicture(engine->display, engine->backbuffer,  format, 0, 0);

    XRenderComposite(engine->display, PictOpOver, destPict, srcPict, compPict,
                     0, 0, 0, 0, 0, 0, destImg->width, destImg->height);

    XRenderFreePicture(engine->display, destPict);
    XRenderFreePicture(engine->display, srcPict);
    XRenderFreePicture(engine->display, compPict);
}

void river2D_resizeBackbuffer
(
    EngineData *engine,
    uint32_t   width,
    uint32_t   height
){
    engine->width  = width;
    engine->height = height;

    if(engine->backbuffer)
    {
        XFreePixmap(engine->display, engine->backbuffer);
    }
    engine->backbuffer = XCreatePixmap(engine->display, engine->window, engine->width, engine->height,
                                       RIVER2D_PIXDEPTH);
    if(!engine->backbuffer)
    {
        fprintf(stderr, "Failed to resize backbuffer Pixmap!\n");
    }

    if(engine->compDestBuf)
    {
        XFreePixmap(engine->display, engine->compDestBuf);
    }
    engine->compDestBuf = XCreatePixmap(engine->display, engine->window, engine->width, engine->height,
                                       RIVER2D_PIXDEPTH);
    if(!engine->compDestBuf)
    {
        fprintf(stderr, "Failed to resize compDestBuf Pixmap!\n");
    }

    if(engine->compSrcBuf)
    {
        XFreePixmap(engine->display, engine->compSrcBuf);
    }
    engine->compSrcBuf = XCreatePixmap(engine->display, engine->window, engine->width, engine->height,
                                       RIVER2D_PIXDEPTH);
    if(!engine->compSrcBuf)
    {
        fprintf(stderr, "Failed to resize compSrcBuf Pixmap!\n");
    }
}

void river2D_resizeXImage
(
    EngineData *engine,
    XImage     *ximage,
    uint32_t   width,
    uint32_t   height
){
    if(ximage)
    {
        XDestroyImage(ximage);
    }

    ximage = XCreateImage(engine->display, engine->visual, RIVER2D_PIXDEPTH, ZPixmap, 0,
                          0, width, height, RIVER2D_SCANLINE, 0);
    if(!ximage)
    {
        fprintf(stderr, "Failed to resize XImage!\n");
    }

    ximage->data = (char*)malloc(width * height * RIVER2D_BPP);

    if(!ximage->data)
    {
        fprintf(stderr, "Failed to allocate resized XImage memory!\n");
    }
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
