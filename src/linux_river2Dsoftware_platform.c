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
    XVisualInfo *foundVisuals = XGetVisualInfo(display,
                                               VisualScreenMask | VisualDepthMask,
                                               &visualInfo, &numVisuals);
    if(!foundVisuals)
    {
        fprintf(stderr, "No valid visuals could be found "
                "for the desired depth of %i.\n", visualInfo.depth);
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
    attributes.background_pixel  = BlackPixel(engine->display,
                                              DefaultScreen(engine->display));
    attributes.background_pixmap = 0;
    attributes.border_pixel      = BlackPixel(engine->display,
                                              DefaultScreen(engine->display));
    attributes.border_pixmap     = 0;
    attributes.colormap          = XCreateColormap(engine->display,
                                                   XDefaultRootWindow(engine->display),
                                                   engine->visual, AllocNone);
    attributes.override_redirect = false;

    Window window = XCreateWindow(engine->display,
                                  XDefaultRootWindow(engine->display), 0, 0,
                                  engine->config.window_width,
                                  engine->config.window_height,
                                  0, RIVER2D_PIXDEPTH, InputOutput,
                                  engine->visual, valuemask, &attributes);

    XStoreName(engine->display, window, engine->windowName);
    XSelectInput(engine->display, window,
                 KeyPressMask    | KeyReleaseMask    | PointerMotionMask |
                 ButtonPressMask | ButtonReleaseMask | ButtonMotionMask  |
                 StructureNotifyMask);
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
    engine->backbuffer.pixmap = XCreatePixmap(engine->display, engine->window,
                                              width, height, RIVER2D_PIXDEPTH);
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

    // NOTE: do we even need this anymore. dividing constants... when is it ever
    // gonna be 24bpp?
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
        river2D_createImage(engine, &engine->backbuffer,
                            engine->config.canvas_width,
                            engine->config.canvas_height);
    }
    else
    {
        river2D_createImage(engine, &engine->backbuffer, engine->config.window_width,
                            engine->config.window_height);
    }

    if(!engine->backbuffer.picture)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XRenderPicture for backbuffer.\n\033[0m");
    }

    // TODO: verify if we need this
    engine->blitDstPict = XRenderCreatePicture(engine->display, engine->window,
                                               engine->format, 0, 0);
}

int32_t river2D_shutdown
(
    EngineData *engine
){
    for(uint8_t i = 0; i < RIVER2D_MAX_PLANES; ++i)
    {
        river2D_destroyImage(&engine->planes[i]);
    }

    river2D_destroyImage(&engine->backbuffer);

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

    // TODO: (river2D #5) verify that both images are actually RGBA

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

    XRenderComposite(engine->display, pictop, src->picture, None, dst->picture,
                     (int)offsetSrcX, (int)offsetSrcY, 0, 0,
                     (int)offsetDstX, (int)offsetDstY, cropWidth, cropHeight);
}

void river2D_bltBuffer
(
    EngineData *engine
){
    float x_s = (float)engine->backbuffer.width  / (float)engine->config.window_width;
    float y_s = (float)engine->backbuffer.height / (float)engine->config.window_height;

    XTransform transform =
    {{
        {XDoubleToFixed(x_s),                   0,                   0},
        {                  0, XDoubleToFixed(y_s),                   0},
        {                  0,                   0, XDoubleToFixed(1.0)}
    }};
    XRenderSetPictureTransform(engine->display, engine->backbuffer.picture, &transform);
    XRenderSetPictureFilter(engine->display, engine->backbuffer.picture,
                            FilterNearest, 0, 0);

    XRenderComposite(engine->display, PictOpSrc, engine->backbuffer.picture, 0,
                     engine->blitDstPict, 0, 0, 0, 0, 0, 0,
                     engine->config.window_width, engine->config.window_height);
}

void river2D_loadText
(
    EngineData    *engine,
    River2D_Image *image,
    const char    *text,
    uint8_t       font,
    uint16_t      charsize,
    uint32_t      spacing,
    uint32_t      offsetX,
    uint32_t      offsetY
){
    if(!engine->planes[font].data)
    {
        fprintf(stderr, "\033[31;3;1mERROR: Font not found. Check loaded planes.\033[0m\n");
        return;
    }

    if(!image)
    {
        fprintf(stderr, "\033[31;3;1mERROR: Destination image is null.\033[0m\n");
        return;
    }

    uint32_t minTextWidth = (charsize + spacing) * (uint32_t)strlen(text);

    if(image->width < minTextWidth || image->height < charsize)
    {
        river2D_destroyImage(image);
    }

    if(!image->data)
    {
        river2D_createImage(engine, image, minTextWidth, charsize);
    }

    if(offsetX > image->width)
    {
        fprintf(stderr, "offsetX too large.\n");
        return;
    }
    if(offsetY > image->height)
    {
        fprintf(stderr, "offsetY too large.\n");
        return;
    }
    uint32_t fontImgWidth = engine->planes[font].width;

    for(uint32_t i = 0; text[i] != '\0'; ++i)
    {
        if(text[i] < 0x21 || text[i] > 0x7F)
        {
            continue;
        }

        uint32_t charBigX = (uint32_t)(text[i] - 0x21) * charsize % fontImgWidth;
        uint32_t charBigY = (uint32_t)(text[i] - 0x21) * charsize / fontImgWidth;

        uint64_t trueSrcOffset  = (charBigY * charsize * fontImgWidth + charBigX) * RIVER2D_BPP;
        uint64_t trueDestOffset = (offsetY * image->width + offsetX + i * (charsize + spacing)) * RIVER2D_BPP;

        uint8_t* charloc = engine->planes[font].data + trueSrcOffset;
        uint8_t* destloc = image->data + trueDestOffset;

        for(uint32_t j = 0; j < charsize; ++j)
        {
            uint8_t* charlineLoc = charloc + j * fontImgWidth * RIVER2D_BPP;
            uint8_t* destlineLoc = destloc + j * image->width * RIVER2D_BPP;

            memcpy(destlineLoc, charlineLoc, charsize * RIVER2D_BPP);
        }
    }

    XImage *img = XCreateImage(engine->display, engine->visual, 32, ZPixmap, 0, (char*)image->data, image->width, image->height, 32, 0);
    XPutImage(engine->display, image->pixmap, engine->context, img, 0, 0, 0, 0, image->width, image->height);

    img->data = NULL;
    XDestroyImage(img);
}
