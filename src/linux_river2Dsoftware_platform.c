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
    //TODO: parse & load from engine file
    config->static_canvas_enable = false;
    config->backgrounds = 4;
    config->width  = 1280;
    config->height = 720;
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
    river2D_loadConfig(&engine->config);
    engine->width = engine->config.width;
    engine->height = engine->config.height;

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

    engine->visual = findVisual(engine->display);
    if(!engine->visual)
    {
        fprintf(stderr, "No matching visual could be found.\n");
    }

    engine->format = XRenderFindStandardFormat(engine->display, PictStandardARGB32);
    if(!engine->format)
    {
        fprintf(stderr, "No matching format could be found.\n");
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

    engine->running = true;
    if(!engine->config.static_canvas_enable)
    {
        //TODO: get from config, worry about scaling
        river2D_resizeBackbuffer(engine, engine->config.width, engine->config.height);
    }

    River2D_Time time = river2D_queryTime();
    engine->lastFrametime = time;
    engine->lastFPStime = time;
}

void river2D_destroyImage
(
    River2D_Image *image
){
    if(!image)
    {
        fprintf(stderr, "No image to be freed.\n");
        return;
    }

    if(image->data)
    {
        free(image->data);
        image->data = 0;
    }
}

//TODO: not safely shutting down for some reason... why?
int32_t river2D_shutdown
(
    EngineData *engine
){
    XFreePixmap(engine->display, engine->backbuffer);
    XFreePixmap(engine->display, engine->compBuffer);

    XFreeGC(engine->display, engine->context);
    XDestroyWindow(engine->display, engine->window);
    XCloseDisplay(engine->display);

    for(uint8_t i = 0; i < RIVER2D_MAX_PLANES; ++i)
    {
        river2D_destroyImage(&engine->planes[i]);
    }

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

void river2D_loadText
(
    EngineData    *engine,
    River2D_Image *image,
    const char    *text,
    uint8_t       font,
    uint16_t      charsize,
    uint32_t      spacing,
    uint32_t      offsetY,
    uint32_t      offsetX
){
    if(!engine->planes[font].data)
    {
        fprintf(stderr, "Font not found. Check loaded planes.\n");
        return;
    }

    if(!image)
    {
        fprintf(stderr, "Destination image is null.\n");
        return;
    }

    if(!image->data)
    {
        image->data = malloc(engine->width * engine->height * RIVER2D_BPP);
        memset(image->data, 0, engine->width * engine->height * RIVER2D_BPP);
        image->width = engine->width;
        image->height = engine->height;
    }

    if(offsetY > image->height)
    {
        fprintf(stderr, "offsetY too large.\n");
        return;
    }
    if(offsetX > image->width)
    {
        fprintf(stderr, "offsetX too large.\n");
        return;
    }
    uint8_t fontImgWidth = engine->planes[font].width;

    for(uint32_t i = 0; text[i] != '\0'; ++i)
    {
        if(text[i] < 33 || text[i] > 127)
        {
            continue;
        }
        uint8_t  charBigY = (text[i] - 33) * charsize / fontImgWidth;
        uint8_t  charBigX = (text[i] - 33) * charsize % fontImgWidth;

        uint64_t trueSrcOffset = (charBigY * charsize * fontImgWidth + charBigX) * RIVER2D_BPP;
        uint64_t trueDestOffset = (offsetY * image->width + offsetX + i * (charsize + spacing)) * RIVER2D_BPP;

        uint8_t* charloc = engine->planes[font].data + trueSrcOffset;
        uint8_t* destloc = image->data + trueDestOffset;

        for(uint32_t j = 0; j < charsize; ++j)
        {
            uint8_t* charlineLoc = charloc + j * fontImgWidth * RIVER2D_BPP;
            uint8_t* destlineLoc = destloc + j * image->width * RIVER2D_BPP;

            memset(destlineLoc, 0, 2 * charsize * RIVER2D_BPP);
            memcpy(destlineLoc, charlineLoc, charsize * RIVER2D_BPP);
        }
    }
}

//TODO: multi-thread some of this?
void river2D_compositeImage
(
    EngineData    *engine,
    River2D_Image *image,
    uint8_t       pictop
){
    if(pictop > RIVER2D_PICTOP_MAXIMUM)
    {
        fprintf(stderr, "Invalid pictop.\n");
        return;
    }

    if(!engine->backbuffer)
    {
        fprintf(stderr, "No image to composite onto.\n");
        return;
    }

    if(!image)
    {
        fprintf(stderr, "No image to composite with.\n");
        return;
    }

    if(!image->data)
    {
        fprintf(stderr, "No data to composite with.\n");
        return;
    }

    XImage *compSrcImg = XCreateImage(engine->display, engine->visual, RIVER2D_PIXDEPTH, ZPixmap, 0,
                                      0, image->width, image->height,
                                      RIVER2D_SCANLINE, 0);
    if(!compSrcImg)
    {
        fprintf(stderr, "Failed to create compSrcImg!\n");
        return;
    }
    compSrcImg->data = malloc(image->width * image->height * RIVER2D_BPP);
    memcpy(compSrcImg->data, image->data, image->width * image->height * RIVER2D_BPP);

    XImage *compDestImg = XCreateImage(engine->display, engine->visual, RIVER2D_PIXDEPTH, ZPixmap, 0,
                                      0, engine->width, engine->height,
                                      RIVER2D_SCANLINE, 0);
    if(!compDestImg)
    {
        fprintf(stderr, "Failed to create compDestImg!\n");
        return;
    }

    Picture compSrcPict  = XRenderCreatePicture(engine->display, engine->compBuffer, engine->format, 0, 0);
    if(!compSrcPict)
    {
        fprintf(stderr, "Failed to create compSrcPict!\n");
        return;
    }

    Picture compDestPict = XRenderCreatePicture(engine->display, engine->backbuffer, engine->format, 0, 0);
    if(!compDestPict)
    {
        fprintf(stderr, "Failed to create compDestPict!\n");
        XRenderFreePicture(engine->display, compSrcPict);
        return;
    }

    if(image->width > engine->width && image->height > engine->height)
    {
        river2D_resizeBackbuffer(engine, image->width, image->height);
    }

    XPutImage(engine->display, engine->compBuffer, engine->context, compSrcImg, 0, 0, 0, 0,
              image->width, image->height);

    XRenderComposite(engine->display, pictop, compSrcPict, None, compDestPict,
                     0, 0, 0, 0, 0, 0, engine->width, engine->height);

    XRenderFreePicture(engine->display, compSrcPict);
    XRenderFreePicture(engine->display, compDestPict);

    XDestroyImage(compSrcImg);
    if(compDestImg)
    {
        XDestroyImage(compDestImg);
    }
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
    engine->backbuffer = XCreatePixmap(engine->display, engine->window, width, height, RIVER2D_PIXDEPTH);
    if(!engine->backbuffer)
    {
        fprintf(stderr, "Failed to resize backbuffer Pixmap!\n");
    }

    if(engine->compBuffer)
    {
        XFreePixmap(engine->display, engine->compBuffer);
    }
    engine->compBuffer = XCreatePixmap(engine->display, engine->window, width, height, RIVER2D_PIXDEPTH);
    if(!engine->compBuffer)
    {
        fprintf(stderr, "Failed to resize compSrcBuf Pixmap!\n");
    }

    engine->width  = width;
    engine->height = height;
}

void river2D_bltBuffer
(
    EngineData *engine
){
    XCopyArea(engine->display, engine->backbuffer, engine->window,
              engine->context, 0, 0, engine->width, engine->height, 0, 0);

    XFlush(engine->display);

    XSetForeground(engine->display, engine->context, 0x00000000);
    XFillRectangle(engine->display, engine->backbuffer, engine->context, 0, 0, engine->width, engine->height);
}

River2D_Time river2D_queryTime
(
    void
){
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);

    River2D_Time time;
    time.s  = spec.tv_sec;
    time.ms = spec.tv_nsec / 1000000;
    time.us = spec.tv_nsec / 1000;
    time.ns = spec.tv_nsec;

    return time;
}
