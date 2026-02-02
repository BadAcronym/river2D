#include "river2D_main.h"

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <pthread.h>

internal Visual* findVisual
(
    Display *display,
    uint8_t depth
){
    Visual *visual = {0};

    XVisualInfo visualInfo = {0};
    visualInfo.screen = DefaultScreen(display);
    visualInfo.depth  = depth;

    int numVisuals;
    XVisualInfo *foundVisuals = XGetVisualInfo(display, VisualScreenMask | VisualDepthMask, &visualInfo, &numVisuals);
    if(!foundVisuals)
    {
        fprintf(stderr, "No valid visuals could be found for the desired depth of %i.\n", visualInfo.depth);
        return 0;
    }

    XWindowAttributes rootAttributes = {0};
    XGetWindowAttributes(display, XDefaultRootWindow(display), &rootAttributes);

    for(int i = 0; i < numVisuals; ++i)
    {
        if(foundVisuals[i].class == rootAttributes.visual->class &&
           foundVisuals[i].depth == depth
        ){
            visual = foundVisuals[i].visual;
            break;
        }
    }

    XFree(foundVisuals);

    return visual;
}

Window river2D_openWindow
(
    EngineData *engine
){
    unsigned long valuemask = CWBackPixel | CWBorderPixel | CWColormap | CWOverrideRedirect;

    XSetWindowAttributes attributes;
    attributes.background_pixel  = BlackPixel(engine->display, DefaultScreen(engine->display));
    attributes.border_pixel      = BlackPixel(engine->display, DefaultScreen(engine->display));
    attributes.colormap = XCreateColormap(engine->display, XDefaultRootWindow(engine->display),
                                          engine->visual, AllocNone);
    attributes.override_redirect = false;

    Window window = XCreateWindow(engine->display, XDefaultRootWindow(engine->display), 0, 0, engine->config.width, engine->config.height,
                                  0, engine->config.depth, InputOutput, engine->visual, valuemask, &attributes);

    XStoreName(engine->display, window, engine->windowName);
    XSelectInput(engine->display, window, KeyPressMask | KeyReleaseMask | StructureNotifyMask);
    XMapWindow(engine->display, window);

    return window;
}

void river2D_resizeBackbuffer
(
    EngineData *engine,
    uint32_t   width,
    uint32_t   height
){
    if(engine->backbuffer.data)
    {
        free(engine->backbuffer.data);
    }
    engine->backbuffer.data   = calloc(width * height * RIVER2D_BPP, 1);
    engine->backbuffer.width  = width;
    engine->backbuffer.height = height;
}

void river2D_init
(
    EngineData         *engine,
    River2D_Image      *planes
){
    river2D_loadConfig(&engine->config);

    engine->planes = planes;

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

    engine->visual = findVisual(engine->display, engine->config.depth);
    if(!engine->visual)
    {
        fprintf(stderr, "No matching visual could be found.\n");
    }

    engine->windowName = "unnamed river2D application";
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

    if(!(engine->config.choices & RIVER2D_CHOICE_STATIC_CANVAS_BIT))
    {
        river2D_resizeBackbuffer(engine, engine->config.width, engine->config.height);
    }

    River2D_Time time = river2D_queryTime();
    engine->lastFrametime  = time;
    engine->lastFPStime    = time;
    engine->playerAnimTime = time;
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

int32_t river2D_shutdown
(
    EngineData *engine
){
    for(uint8_t i = 0; i < RIVER2D_MAX_THREADS; ++i)
    {
        // free(engine->pool.pictopData[i]);
        // free(engine->pool.pictopData[i]);
    }
    for(uint8_t i = 0; i < RIVER2D_MAX_PLANES; ++i)
    {
        river2D_destroyImage(&engine->planes[i]);
    }

    free(engine->backbuffer.data);

    XFreeGC(engine->display, engine->context);
    XDestroyWindow(engine->display, engine->window);
    XCloseDisplay(engine->display);

    return 0;
}

void river2D_compositeImage
(
    EngineData    *engine,
    River2D_Image *image,
    uint8_t       pictop,
    uint32_t      offsetDstX,
    uint32_t      offsetDstY,
    uint32_t      offsetSrcX,
    uint32_t      offsetSrcY,
    uint32_t      cropWidth,
    uint32_t      cropHeight
){
    //TODO: (river2D #6) deal with alpha and actual compositing instead of just overlaying/copying
    if(pictop != RIVER2D_PICTOP_OVER)
    {
        fprintf(stderr, "\033[33;1;7mSORRY: only RIVER2D_PICTOP_OVER implemented for now. :/\033[0m\n");
        return;
    }

    if(!image)
    {
        fprintf(stderr, "\033[31;1;7mERROR: no image to composite with.\033[0m\n");
        return;
    }
    if(!image->data)
    {
        fprintf(stderr, "\033[31;1;7mERROR: image->data is nullptr.\033[0m\n");
        return;
    }

    if(!engine->backbuffer.data)
    {
        fprintf(stderr, "\033[31;1;7mERROR: no image to composite onto.\033[0m\n");
        return;
    }

    //TODAY: (river2D #5) verify source image channelcount, destination (backbuf) is always RGB w/o A
    //also validate that offset doesn't exceed buffer destination image

    uint32_t bufWidth  = engine->backbuffer.width * RIVER2D_BPP;
    uint32_t copyWidth = image->width * RIVER2D_BPP;

    uint8_t *dest = (uint8_t*)engine->backbuffer.data + offsetDstY * bufWidth + offsetDstX * RIVER2D_BPP;
    uint8_t *src  = image->data + offsetSrcY * copyWidth + offsetSrcX * RIVER2D_BPP;

    for(uint32_t y = 0; y < cropHeight; ++y)
    {
        for(uint32_t x = 0; x < cropWidth; ++x)
        {
            uint8_t *dstIndexed = &dest[y * bufWidth  + x * RIVER2D_BPP];
            uint8_t *srcIndexed = &src [y * copyWidth + x * RIVER2D_BPP];

            if(srcIndexed[3])
            {
                memcpy(dstIndexed, srcIndexed, RIVER2D_BPP);
            }
        }
    }
}

// FIXME: factor 2 segfault
void river2D_bltBuffer
(
    EngineData *engine,
    uint8_t    factor
){
    if(factor == 0)
    {
        fprintf(stderr, "\033[33;3;1m0 is an invalid scaling factor.\033[0m\n");
        return;
    }

    XImage   *bufImg   = 0;
    uint64_t bltWidth  = engine->backbuffer.width  * factor;
    uint64_t bltHeight = engine->backbuffer.height * factor;

    if(factor == 1)
    {
        bufImg = XCreateImage(engine->display, engine->visual, engine->config.depth, ZPixmap, 0,
                              (char*)engine->backbuffer.data, bltWidth, bltHeight, RIVER2D_SCANLINE, 0);
        if(!bufImg)
        {
            fprintf(stderr, "\033[30;3;1mERROR: could not create bufImg.\033[0m\n");
            return;
        }
    }
    else
    {
        bufImg = XCreateImage(engine->display, engine->visual, engine->config.depth, ZPixmap, 0,
                              0, bltWidth, bltHeight, RIVER2D_SCANLINE, 0);
        if(!bufImg)
        {
            fprintf(stderr, "\033[30;3;1mERROR: could not create bufImg.\033[0m\n");
            return;
        }
        bufImg->data = malloc(bltWidth * bltHeight * RIVER2D_BPP);

        uint64_t bltWidthBytes = bltWidth  * RIVER2D_BPP;

        for(uint32_t y = 0; y < engine->backbuffer.height; y += factor)
        {
            for(uint32_t x = 0; x < engine->backbuffer.width; x += RIVER2D_BPP)
            {
                uint32_t ogPixel = engine->backbuffer.data[y * bltWidthBytes + x];
                for(uint8_t i = 0; i < factor; ++i)
                {
                    bufImg->data[y * bltWidthBytes + x * factor + i * RIVER2D_BPP] = ogPixel;
                }
            }
            for(uint8_t i = 1; i < factor; ++i)
            {
                memcpy((char*)&bufImg->data[(y + i) * bltWidthBytes], (char*)bufImg->data + y * bltWidthBytes, bltWidthBytes);
            }
        }

    }

    Pixmap pixmap = XCreatePixmapFromBitmapData(engine->display, engine->window, (char*)bufImg->data,
                                                bltWidth, bltHeight, 0, 0, engine->config.depth);
    if(!pixmap)
    {
        fprintf(stderr, "\033[30;3;1mERROR: could not create pixmap.\033[0m\n");
        return;
    }

    XPutImage(engine->display, pixmap, engine->context, bufImg, 0, 0, 0, 0, bltWidth, bltHeight);

    XCopyArea(engine->display, pixmap, engine->window, engine->context, 0, 0, bltWidth, bltHeight, 0, 0);

    XFlush(engine->display);

    if(bufImg->data && factor != 1)
    {
        free(bufImg->data);
    }
    XFree(bufImg);
    XFreePixmap(engine->display, pixmap);
}
