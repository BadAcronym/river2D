#include "river2D_main.h"
#include "imgsurf_main.h"

#include <stdio.h>
#include <memory.h>
#include <pthread.h>

f_internal Visual* findVisual
(
    EngineData *engine,
    uint8_t    depth
){
    Visual  *visual  = {0};
    Display *display = engine->display;

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
    engine->xGetWinAttr(display, XDefaultRootWindow(display), &rootAttributes);

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

Window rvOpenWindow
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
                                  0, RV_PIXDEPTH, InputOutput,
                                  engine->visual, valuemask, &attributes);

    XStoreName(engine->display, window, engine->windowName);
    XSelectInput(engine->display, window,
                 KeyPressMask    | KeyReleaseMask    | PointerMotionMask |
                 ButtonPressMask | ButtonReleaseMask | ButtonMotionMask  |
                 StructureNotifyMask);
    XMapWindow(engine->display, window);

    return window;
}

void rvResizeBackbuffer
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
                                              width, height, RV_PIXDEPTH);
    engine->backbuffer.width  = width;
    engine->backbuffer.height = height;
}

void init
(
    EngineData *engine,
    RiverImage *planes
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

    engine->visual = findVisual(engine, RV_PIXDEPTH);
    if(!engine->visual)
    {
        fprintf(stderr, "No matching visual could be found.\n");
    }

    engine->format = XRenderFindStandardFormat(engine->display, PictStandardARGB32);

    if(!engine->format)
    {
        fprintf(stderr, "No matching format could be found.\n");
    }

    if(!engine->windowName)
    {
        engine->windowName = "unnamed river2D application";
    }
    engine->window = rvOpenWindow(engine);
    if(!engine->window)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create window!.\n\033[0m");
    }

    engine->context = XCreateGC(engine->display, engine->window, 0, 0);
    if(!engine->context)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to Graphics Context!.\n\033[0m");
    }

    if(engine->config.choices & RV_CHOICE_STATIC_CANVAS_BIT)
    {
        rvCreateImage(engine, &engine->backbuffer, engine->config.canvas_width,
                      engine->config.canvas_height);
    }
    else
    {
        rvCreateImage(engine, &engine->backbuffer, engine->config.window_width,
                      engine->config.window_height);
    }

    if(!engine->backbuffer.picture)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XRenderPicture "
                "for backbuffer.\n\033[0m");
    }

    engine->blitDstPict = XRenderCreatePicture(engine->display, engine->window,
                                               engine->format, 0, 0);
}

int32_t shutdown
(
    EngineData *engine
){
    for(uint8_t i = 0; i < RV_MAX_PLANES; ++i)
    {
        rvDestroyImage(&engine->planes[i]);
    }

    rvDestroyImage(&engine->backbuffer);

    XFreeGC(engine->display, engine->context);
    XDestroyWindow(engine->display, engine->window);
    XCloseDisplay(engine->display);

    return 0;
}

void compositeImage
(
    EngineData *engine,
    RiverImage *src,
    RiverImage *dst,
    uint8_t    pictop,
    uint32_t   offsetSrcX,
    uint32_t   offsetSrcY,
    uint32_t   offsetDstX,
    uint32_t   offsetDstY,
    uint32_t   cropWidth,
    uint32_t   cropHeight
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
        fprintf(stderr, "image->path: "PRI_SV"\n", ARG_SV(src->path));
        fprintf(stderr, "image->picture: %lu\n",   src->picture);
        fprintf(stderr, "image->width: %u\n",      src->width);
        fprintf(stderr, "image->height: %u\n",     src->height);
        return;
    }
    if(!dst->picture)
    {
        fprintf(stderr, "\033[31;1;7mERROR: dst was created incorrectly.\033[0m\n");
        fprintf(stderr, "image->path: "PRI_SV"\n", ARG_SV(dst->path));
        fprintf(stderr, "image->picture: %lu\n",   dst->picture);
        fprintf(stderr, "image->width: %u\n",      dst->width);
        fprintf(stderr, "image->height: %u\n",     dst->height);
        return;
    }

    XRenderComposite(engine->display, pictop, src->picture, None, dst->picture,
                     (int)offsetSrcX, (int)offsetSrcY, 0, 0,
                     (int)offsetDstX, (int)offsetDstY, cropWidth, cropHeight);
}

void bltBuffer
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

void loadText
(
    EngineData *engine,
    RiverImage *image,
    StringView *sv,
    uint8_t    font,
    uint16_t   charsize,
    uint32_t   spacing,
    uint32_t   offsetX,
    uint32_t   offsetY
){
    if(!engine->planes[font].data)
    {
        fprintf(stderr, "\033[31;3;1mERROR: Font not found. Check loaded planes."
                "\033[0m\n");
        return;
    }

    if(!image)
    {
        fprintf(stderr, "\033[31;3;1mERROR: Destination image is null.\033[0m\n");
        return;
    }

    uint32_t minTextWidth = (charsize + spacing) * (uint32_t)sv->size;

    if(image->width < minTextWidth || image->height < charsize)
    {
        rvDestroyImage(image);
    }

    if(!image->data)
    {
        rvCreateImage(engine, image, minTextWidth, charsize);
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
    uint32_t imageChars = (image->width) / ((charsize + spacing));

    for(uint32_t i = 0; i < imageChars; ++i)
    {
        char character = 0x20;
        if(i < sv->size && sv->data[i])
        {
            character = sv->data[i];
        }

        uint32_t charBigX = (uint32_t)(character - 0x21) * charsize % fontImgWidth;
        uint32_t charBigY = (uint32_t)(character - 0x21) * charsize / fontImgWidth;

        if(character == 0x20)
        {
            charBigX = (uint32_t)(0x5F) * charsize % fontImgWidth;
            charBigY = (uint32_t)(0x5F) * charsize / fontImgWidth;
        }
        else if(character == RV_ASCII_CURSOR)
        {
            charBigX = (uint32_t)(0x5E) * charsize % fontImgWidth;
            charBigY = (uint32_t)(0x5E) * charsize / fontImgWidth;
        }
        else if(character < 0x20)
        {
            continue;
        }

        uint64_t trueSrcOffset = (charBigY * charsize * fontImgWidth + charBigX) *
                                 RV_BPP;
        uint64_t trueDestOffset = (offsetY * image->width + offsetX + i *
                                  (charsize + spacing)) * RV_BPP;

        uint8_t* charloc = engine->planes[font].data + trueSrcOffset;
        uint8_t* destloc = image->data + trueDestOffset;

        for(uint32_t j = 0; j < charsize; ++j)
        {
            uint8_t* charlineLoc = charloc + j * fontImgWidth * RV_BPP;
            uint8_t* destlineLoc = destloc + j * image->width * RV_BPP;

            memcpy(destlineLoc, charlineLoc, charsize * RV_BPP);
        }
    }

    XImage *img = XCreateImage(engine->display, engine->visual, 32, ZPixmap, 0,
                               (char*)image->data, image->width, image->height, 32, 0);
    XPutImage(engine->display, image->pixmap, engine->context, img, 0, 0, 0, 0,
              image->width, image->height);

    img->data = NULL;
    XDestroyImage(img);
}
