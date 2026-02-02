#include "river2D_main.h"
#include "linux_river2Dsoftware_platform.h"

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

internal Visual* findVisual
(
    Display *display
){
    Visual *visual = {0};

    XVisualInfo visualInfo = {0};
    visualInfo.screen = DefaultScreen(display);
    visualInfo.depth  = RIVER2D_PIXDEPTH;

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
           foundVisuals[i].depth == RIVER2D_PIXDEPTH
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

    Window window = XCreateWindow(engine->display, XDefaultRootWindow(engine->display),
                                  0, 0, engine->width, engine->height, 0, RIVER2D_PIXDEPTH,
                                  InputOutput, engine->visual, valuemask, &attributes);

    XStoreName(engine->display, window, engine->windowName);
    XSelectInput(engine->display, window, KeyPressMask | KeyReleaseMask | StructureNotifyMask);
    XMapWindow(engine->display, window);

    return window;
}

//TODAY: rework to simple free/alloc like win32 equiv
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

    engine->width  = width;
    engine->height = height;
}

void river2D_init
(
    EngineData         *engine,
    River2D_Image      *planes
){
    river2D_loadConfig(&engine->config);

    engine->width  = engine->config.width;
    engine->height = engine->config.height;

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

    engine->visual = findVisual(engine->display);
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

    //TESTING: pixmap from backbuffer?
    // engine->backbuffer.pixmap = XCreatePixmap(engine->display, engine->window,
    //                                           engine->config.width, engine->config.height,
    //                                           RIVER2D_PIXDEPTH);

    if(!(engine->config.choices & RIVER2D_CHOICE_STATIC_CANVAS_BIT))
    {
        river2D_resizeBackbuffer(engine, engine->config.width, engine->config.height);
    }

    River2D_Time time;
    river2D_queryTime(&time);
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

//FIXME: nothing fails, but nothing displays, either.
void river2D_bltBuffer
(
    EngineData *engine
){
    XImage *bufImg = XCreateImage(engine->display, engine->visual, RIVER2D_PIXDEPTH, ZPixmap,
                                  0, (char*)engine->backbuffer.data, engine->config.width,
                                  engine->config.height, RIVER2D_SCANLINE, 0);

    Pixmap pixmap = XCreatePixmap(engine->display, engine->window,
                                  engine->config.width, engine->config.height,
                                  RIVER2D_PIXDEPTH);

    XPutImage(engine->display, pixmap, engine->context, bufImg, 0, 0, 0, 0,
              engine->backbuffer.width, engine->backbuffer.height);

    XCopyArea(engine->display, pixmap, engine->window, engine->context, 0, 0,
              engine->config.width, engine->config.height, 0, 0);

    XFlush(engine->display);

    XSetForeground(engine->display, engine->context, 0x00000000);
    XFillRectangle(engine->display, pixmap, engine->context, 0, 0,
                   engine->width, engine->height);

    XFree(bufImg);
    XFreePixmap(engine->display, pixmap);
}
