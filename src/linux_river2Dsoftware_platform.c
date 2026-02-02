#include "river2D_main.h"

#include <math.h>
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
    XVisualInfo *foundVisuals = XGetVisualInfo(display, VisualScreenMask | VisualDepthMask,
                                               &visualInfo, &numVisuals);
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
    engine->backbuffer.data = calloc(width * height * RIVER2D_BPP, 1);
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

    for(uint8_t i = 0; i < RIVER2D_MAX_THREADS; ++i)
    {
        engine->pool.scaleData[i] = (ScaleData*)malloc(sizeof(ScaleData));
        engine->pool.pictopData[i] = (PictopData*)malloc(sizeof(PictopData));
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

//TODAY: (river2 #15) improve multi-threading to actually max out the CPU
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

    uint32_t copyWidth = image->width * RIVER2D_BPP;
    uint32_t bufWidth  = engine->backbuffer.width * RIVER2D_BPP;
    uint8_t* dst = (uint8_t*)engine->backbuffer.data + offsetDstY * bufWidth +
           offsetDstX * RIVER2D_BPP;
    uint8_t* src  = image->data + offsetSrcY * copyWidth + offsetSrcX * RIVER2D_BPP;

    //TODAY: send this data to a command queue
}

internal void *blt
(
    void *data
){
    ScaleData *sd = (ScaleData*)data;
    uint32_t bltWidth = sd->ogWidth * sd->factor;

    for(uint32_t y = 0; y < sd->threadHeight; ++y)
    {
        for(uint32_t x = 0; x < sd->ogWidth; ++x)
        {
            for(uint8_t i = 0; i < sd->factor; ++i)
            {
                *sd->dst++ = *sd->src;
            }
            ++sd->src;
        }
        for(uint8_t i = 0; i < sd->factor - 1; ++i)
        {
            memcpy(sd->dst, sd->dst - bltWidth, bltWidth * RIVER2D_BPP);
        }
    }

    pthread_exit(0);
}

void river2D_bltBuffer
(
    EngineData *engine,
    uint8_t    factor
){
    //TODAY: (river2D #4) oh boy. now it's time to create a way to stretch this thing.
    //probably will have to treat each pixel in the backbuffer as a vertex.
    //I can pass the backbuffer through a multi-threaded function which goes through each pixel
    //in the desired buffer size (which will be the bufImg here) and calculates its value, based
    //on some filtering of the source pixel and possibly its neighbours. I only want to scale up
    //here, not down. If the desired buffer size is bigger than the window, I don't care. Or that
    //is to say, that should probably be handled by some other downscaling function. for now,
    //let's upscale.

    if(factor == 0)
    {
        fprintf(stderr, "\033[33;3;1m0 is an invalid scaling factor.\033[0m\n");
    }

    XImage *bufImg = 0;
    int bltWidth   = 0;
    int bltHeight  = 0;

    //TODAY: (river2D #4): we are now multi-threaded but again just slower than our single-
    //threaded counterpart. is this some sort of aliasing problem?
    //try calculating the starting y index for both dest and src before spawning the thread,
    //maybe that helps.
    //don't pass the original src[] at all, possibly that is waited upon

    if(factor != 1)
    {
        bltWidth  = engine->backbuffer.width  * factor;
        bltHeight = engine->backbuffer.height * factor;

        bufImg = XCreateImage(engine->display, engine->visual, engine->config.depth,
                              ZPixmap, 0, 0, bltWidth, bltHeight,
                              RIVER2D_SCANLINE, 0);

        bufImg->data = malloc(bltWidth * bltHeight * engine->config.depth / 8);

        uint32_t y = 0;
        uint32_t stopHeight = engine->backbuffer.height - RIVER2D_MAX_THREADS;
        uint32_t threadHeight = floor(engine->backbuffer.height / RIVER2D_MAX_THREADS);

        struct sched_param threadParam =
        {
            .sched_priority = 90
        };

        // pthread_attr_t threadAttr;
        //
        // if(pthread_attr_init(&threadAttr))
        // {
        //     fprintf(stderr, "pthread_attr_init failed.\n");
        // }
        // if(pthread_attr_setschedpolicy(&threadAttr, SCHED_FIFO))
        // {
        //     fprintf(stderr, "pthread_attr_setschedpolicy failed.\n");
        // }
        // if(pthread_attr_setschedparam(&threadAttr, &threadParam))
        // {
        //     fprintf(stderr, "pthread_attr_setschedparam failed.\n");
        // }
        // if(pthread_attr_setinheritsched(&threadAttr, PTHREAD_EXPLICIT_SCHED))
        // {
        //     fprintf(stderr, "pthread_attr_setinheritsched failed.\n");
        // }

        if(!threadHeight)
        {
            threadHeight = 1;
        }

        //WIP: debug
        // fprintf(stderr, "sd.threadHeight: %u\n", scaleData.threadHeight);
        // fprintf(stderr, "sd.ogWidth: %u\n", scaleData.ogWidth);
        // fprintf(stderr, "bltWidth: %u\n", bltWidth);
        // fprintf(stderr, "factor: %u\n", scaleData.factor);
        // fprintf(stderr, "stopHeight: %u\n\n", stopHeight);

        #ifdef RIVER2D_PROFILING_BLT
        River2D_Time time = river2D_queryTime();
        #endif

        for(uint8_t i = 0; i < RIVER2D_MAX_THREADS && y < engine->backbuffer.height; ++i)
        {
            if(y > stopHeight)
            {
                break;
            }

            //TODAY: calc src and dst
            engine->pool.scaleData[i]->src = (uint32_t*)&engine->backbuffer.data[y * engine->backbuffer.width];
            engine->pool.scaleData[i]->dst = (uint32_t*)&bufImg->data[y * bufImg->width];
            engine->pool.scaleData[i]->factor = factor;
            engine->pool.scaleData[i]->ogWidth = engine->backbuffer.width;
            engine->pool.scaleData[i]->threadHeight = threadHeight;

            if(pthread_create(&engine->pool.threads[i], 0, blt,
                              (void*)engine->pool.scaleData[i]))
            {
                fprintf(stderr, "pthread_create failed.\n");
            }

            //WIP: debug:
            // int policy;
            // struct sched_param param;
            // pthread_getschedparam(engine->pool.threads[i], &policy, &param);
            // fprintf(stderr, "blt thread created with policy %i and prio %i\n", policy, param.sched_priority);
            //

            y += threadHeight;
        }

        #ifdef RIVER2D_PROFILING_BLT
        River2D_Time dispatchTime = river2D_deltaTime(&time);
        engine->dispatchTime.s  += dispatchTime.s;
        engine->dispatchTime.ns += dispatchTime.ns;
        if(engine->dispatchTime.ns > 1000000000)
        {
            ++engine->dispatchTime.s;
            engine->dispatchTime.ns = 0;
        }

        time = river2D_queryTime();
        #endif

        //FIXME: this would overlap
        // for(; y < engine->backbuffer.height; ++y)
        // {
        //     for(uint32_t x = 0; x < engine->backbuffer.width; ++x)
        //     {
        //         for(uint8_t i = 0; i < factor; ++i)
        //         {
        //             *dst++ = *src;
        //         }
        //         ++src;
        //     }
        //     for(uint8_t i = 0; i < factor - 1; ++i)
        //     {
        //         memcpy(&dst[bltWidth * i], &dst[bltWidth * i - 1], bltWidth * RIVER2D_BPP);
        //     }
        // }

        #ifdef RIVER2D_PROFILING_BLT
        River2D_Time singleTime = river2D_deltaTime(&time);
        engine->singleTime.s  += singleTime.s;
        engine->singleTime.ns += singleTime.ns;
        if(engine->singleTime.ns > 1000000000)
        {
            ++engine->singleTime.s;
            engine->singleTime.ns = 0;
        }

        time = river2D_queryTime();
        #endif

        for(uint8_t i = 0; i < RIVER2D_MAX_THREADS; ++i)
        {
            if(engine->pool.threads[i])
            {
                pthread_join(engine->pool.threads[i], 0);
                engine->pool.threads[i] = 0;
            }
        }
        // pthread_attr_destroy(&threadAttr);

        #ifdef RIVER2D_PROFILING_BLT
        River2D_Time idleTime = river2D_deltaTime(&time);
        engine->idleTime.s  += idleTime.s;
        engine->idleTime.ns += idleTime.ns;
        if(engine->idleTime.ns > 1000000000)
        {
            ++engine->idleTime.s;
            engine->idleTime.ns = 0;
        }
        #endif
    }
    else
    {
        bltWidth  = engine->backbuffer.width;
        bltHeight = engine->backbuffer.height;
        bufImg = XCreateImage(engine->display, engine->visual, engine->config.depth, ZPixmap, 0,
                              (char*)engine->backbuffer.data, bltWidth, bltHeight, RIVER2D_SCANLINE, 0);
    }

    Pixmap pixmap = XCreatePixmapFromBitmapData(engine->display, engine->window, (char*)engine->backbuffer.data,
                                                bltWidth, bltHeight, 0x000000, 0x000000, engine->config.depth);

    XPutImage(engine->display, pixmap, engine->context, bufImg, 0, 0, 0, 0, bltWidth, bltHeight);

    XCopyArea(engine->display, pixmap, engine->window, engine->context, 0, 0, bltWidth, bltHeight, 0, 0);

    XFlush(engine->display);

    if(factor > 1)
    {
        free(bufImg->data);
    }
    XFree(bufImg);
    XFreePixmap(engine->display, pixmap);
}
