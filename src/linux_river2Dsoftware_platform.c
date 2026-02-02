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
    unsigned long valuemask = CWBackPixel | CWBorderPixel |
                              CWColormap  | CWOverrideRedirect;

    XSetWindowAttributes attributes;
    attributes.background_pixel  = BlackPixel(engine->display, DefaultScreen(engine->display));
    attributes.background_pixmap = 0;
    attributes.border_pixel      = BlackPixel(engine->display, DefaultScreen(engine->display));
    attributes.border_pixmap     = 0;
    attributes.colormap          = XCreateColormap(engine->display, XDefaultRootWindow(engine->display),
                                          engine->visual, AllocNone);
    attributes.override_redirect = false;

    Window window = XCreateWindow(engine->display, XDefaultRootWindow(engine->display), 0, 0,
                                  engine->config.window_width, engine->config.window_height,
                                  0, RIVER2D_PIXDEPTH, InputOutput, engine->visual, valuemask, &attributes);

    XStoreName(engine->display, window, engine->windowName);
    XSelectInput(engine->display, window, KeyPressMask    | KeyReleaseMask    | PointerMotionMask |
                                          ButtonPressMask | ButtonReleaseMask | ButtonMotionMask  | StructureNotifyMask);
    XMapWindow(engine->display, window);

    return window;
}

void river2D_resizeBackbuffer
(
    EngineData *engine,
    uint32_t   width,
    uint32_t   height
){
    if(engine->compSrcPict)
    {
        XRenderFreePicture(engine->display, engine->compSrcPict);
        engine->compSrcPict = 0;
    }
    if(engine->compDstPict)
    {
        XRenderFreePicture(engine->display, engine->compDstPict);
        engine->compDstPict = 0;
    }

    if(engine->backbuffer.pixmap)
    {
        XFreePixmap(engine->display, engine->backbuffer.pixmap);
    }
    engine->backbuffer.pixmap = XCreatePixmap(engine->display, engine->window, width, height, RIVER2D_PIXDEPTH);
    engine->backbuffer.width  = width;
    engine->backbuffer.height = height;

    if(engine->compBuffer.pixmap)
    {
        XFreePixmap(engine->display, engine->compBuffer.pixmap);
    }
    engine->compBuffer.pixmap = XCreatePixmap(engine->display, engine->window, width, height, RIVER2D_PIXDEPTH);
    engine->compBuffer.width  = width;
    engine->compBuffer.height = height;

    engine->compSrcPict = XRenderCreatePicture(engine->display, engine->compBuffer.pixmap, engine->format, 0, 0);
    if(!engine->compSrcPict)
    {
        fprintf(stderr, "Failed to create compSrcPict!\n");
        return;
    }

    engine->compDstPict = XRenderCreatePicture(engine->display, engine->backbuffer.pixmap, engine->format, 0, 0);
    if(!engine->compDstPict)
    {
        fprintf(stderr, "Failed to create compDestPict!\n");
        XRenderFreePicture(engine->display, engine->compSrcPict);
        return;
    }
}

void river2D_init
(
    EngineData         *engine,
    River2D_Image      *planes
){
    engine->running = true;
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

    engine->visual = findVisual(engine->display, RIVER2D_PIXDEPTH);
    if(!engine->visual)
    {
        fprintf(stderr, "No matching visual could be found.\n");
    }

    if(RIVER2D_PIXDEPTH / 8 == 4)
    {
        engine->format = XRenderFindStandardFormat(engine->display, PictStandardARGB32);
    }
    else
    {
        engine->format = XRenderFindStandardFormat(engine->display, PictStandardRGB24);
    }

    if(!engine->format)
    {
        fprintf(stderr, "No matching format could be found.\n");
    }

    if(!engine->windowName)
    {
        engine->windowName = "unnamed river2D application";
    }
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

    if(engine->config.choices & RIVER2D_CHOICE_STATIC_CANVAS_BIT)
    {
        river2D_resizeBackbuffer(engine, engine->config.canvas_width, engine->config.canvas_height);
    }
    else
    {
        river2D_resizeBackbuffer(engine, engine->config.window_width, engine->config.window_height);
    }

    engine->blitSrcPict = XRenderCreatePicture(engine->display, engine->backbuffer.pixmap, engine->format, 0, 0);
    engine->blitDstPict = XRenderCreatePicture(engine->display, engine->window, engine->format, 0, 0);

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
    for(uint8_t i = 0; i < RIVER2D_MAX_PLANES; ++i)
    {
        river2D_destroyImage(&engine->planes[i]);
    }

    XRenderFreePicture(engine->display, engine->compSrcPict);
    XRenderFreePicture(engine->display, engine->compDstPict);
    XRenderFreePicture(engine->display, engine->blitSrcPict);
    XRenderFreePicture(engine->display, engine->blitDstPict);

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

    if(!engine->backbuffer.pixmap)
    {
        fprintf(stderr, "\033[31;1;7mERROR: no image to composite onto.\033[0m\n");
        return;
    }

    XImage *compSrcImg  = XCreateImage(engine->display, engine->visual, RIVER2D_PIXDEPTH, ZPixmap, 0,
                                      (char*)image->data, image->width, image->height, RIVER2D_SCANLINE, 0);

    XImage *compDestImg = XCreateImage(engine->display, engine->visual, RIVER2D_PIXDEPTH, ZPixmap, 0,
                                      0, engine->backbuffer.width, engine->backbuffer.height, RIVER2D_SCANLINE, 0);
    if(!compDestImg)
    {
        fprintf(stderr, "Failed to create compDestImg!\n");
        return;
    }

    XPutImage(engine->display, engine->compBuffer.pixmap, engine->context, compSrcImg, 0, 0, 0, 0,
              image->width, image->height);

    XRenderComposite(engine->display, pictop, engine->compSrcPict, None, engine->compDstPict, offsetSrcX, offsetSrcY,
                     0, 0, offsetDstX, offsetDstY, cropWidth, cropHeight);

    compSrcImg->data = 0;
    XDestroyImage(compSrcImg);

    if(compDestImg)
    {
        XDestroyImage(compDestImg);
    }
}

void river2D_bltBuffer
(
    EngineData *engine
){
    double x_s = (double)engine->backbuffer.width  / (double)engine->config.window_width;
    double y_s = (double)engine->backbuffer.height / (double)engine->config.window_height;

    XTransform transform =
    {{
        {XDoubleToFixed(x_s),                   0,                   0},
        {                  0, XDoubleToFixed(y_s),                   0},
        {                  0,                   0, XDoubleToFixed(1.0)}
    }};
    XRenderSetPictureTransform(engine->display, engine->blitSrcPict, &transform);
    XRenderSetPictureFilter(engine->display, engine->blitSrcPict, FilterNearest, 0, 0);

    XRenderComposite(engine->display, PictOpSrc, engine->blitSrcPict, 0, engine->blitDstPict,
                     0, 0, 0, 0, 0, 0, engine->config.window_width, engine->config.window_height);
}
