#include "river2D_main.h"
#include "imgsurf_main.h"

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
    if(engine->backbuffer.pixmap)
    {
        XFreePixmap(engine->display, engine->backbuffer.pixmap);
    }
    engine->backbuffer.pixmap = XCreatePixmap(engine->display, engine->window, width, height, RIVER2D_PIXDEPTH);
    engine->backbuffer.width  = width;
    engine->backbuffer.height = height;
}

void river2D_init
(
    EngineData         *engine,
    River2D_Image      *planes
){
    engine->running = true;
    engine->planes  = planes;

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

    // NOTE: do we even need this anymore. dividing constants... when is it ever gonna be 24bpp?
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
        fprintf(stderr, "\033[31m\nERROR: failed to create window!.\n\033[0m");
    }

    engine->context = XCreateGC(engine->display, engine->window, 0, 0);
    if(!engine->context)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to Graphics Context!.\n\033[0m");
    }

    if(engine->config.choices & RIVER2D_CHOICE_STATIC_CANVAS_BIT)
    {
        river2D_resizeBackbuffer(engine, engine->config.canvas_width, engine->config.canvas_height);
    }
    else
    {
        river2D_resizeBackbuffer(engine, engine->config.window_width, engine->config.window_height);
    }

    engine->backbuffer.pixmap = XCreatePixmap(engine->display, XDefaultRootWindow(engine->display),
                                              engine->backbuffer.width, engine->backbuffer.height, 32);

    engine->backbuffer.picture = XRenderCreatePicture(engine->display, engine->backbuffer.pixmap, engine->format, 0, 0);
    if(!engine->backbuffer.picture)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XRenderPicture for backbuffer.\n\033[0m");
    }

    // TODO: verify if we need this
    engine->blitDstPict = XRenderCreatePicture(engine->display, engine->window, engine->format, 0, 0);
}

int32_t river2D_shutdown
(
    EngineData *engine
){
    for(uint8_t i = 0; i < RIVER2D_MAX_PLANES; ++i)
    {
        river2D_destroyImage(&engine->planes[i]);
    }

    XRenderFreePicture(engine->display, engine->backbuffer.picture);
    XRenderFreePicture(engine->display, engine->blitDstPict);

    XFreeGC(engine->display, engine->context);
    XDestroyWindow(engine->display, engine->window);
    XCloseDisplay(engine->display);

    return 0;
}

void river2D_compositeImage
(
    EngineData    *engine,
    River2D_Image *src,
    River2D_Image *dst,
    uint8_t       pictop,
    uint32_t      offsetDstX,
    uint32_t      offsetDstY,
    uint32_t      offsetSrcX,
    uint32_t      offsetSrcY,
    uint32_t      cropWidth,
    uint32_t      cropHeight
){
    if(!src)
    {
        fprintf(stderr, "\033[31;1;7mERROR: no image to composite with.\033[0m\n");
        return;
    }
    if(!src->data)
    {
        fprintf(stderr, "\033[31;1;7mERROR: src->data is nullptr.\033[0m\n");
        return;
    }

    if(!dst)
    {
        fprintf(stderr, "\033[31;1;7mERROR: no image to composite onto.\033[0m\n");
        return;
    }
    if(!dst->data)
    {
        fprintf(stderr, "\033[31;1;7mERROR: dst->data is nullptr.\033[0m\n");
        return;
    }

    if(!src->picture)
    {
        fprintf(stderr, "\033[31;1;7mERROR: src was created incorrectly.\033[0m\n");
        fprintf(stderr, "image->path: %s\n",     src->path);
        fprintf(stderr, "image->picture: %lu\n", src->picture);
        fprintf(stderr, "image->width: %u\n",    src->width);
        fprintf(stderr, "image->height: %u\n",   src->height);
        abort();
    }
    if(!dst->picture)
    {
        fprintf(stderr, "\033[31;1;7mERROR: dst was created incorrectly.\033[0m\n");
        fprintf(stderr, "image->path: %s\n",     dst->path);
        fprintf(stderr, "image->picture: %lu\n", dst->picture);
        fprintf(stderr, "image->width: %u\n",    dst->width);
        fprintf(stderr, "image->height: %u\n",   dst->height);
        abort();
    }

    XRenderComposite(engine->display, pictop, src->picture, None, dst->picture, offsetSrcX, offsetSrcY,
                     0, 0, offsetDstX, offsetDstY, cropWidth, cropHeight);
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
    XRenderSetPictureTransform(engine->display, engine->backbuffer.picture, &transform);
    XRenderSetPictureFilter(engine->display, engine->backbuffer.picture, FilterNearest, 0, 0);

    XRenderComposite(engine->display, PictOpSrc, engine->backbuffer.picture, 0, engine->blitDstPict,
                     0, 0, 0, 0, 0, 0, engine->config.window_width, engine->config.window_height);
}
