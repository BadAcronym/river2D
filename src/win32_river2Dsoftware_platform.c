#include "river2D_main.h"

#include "win32_river2Dsoftware_platform.h"

#include <stdio.h>

void river2D_resizeBackbuffer
(
    EngineData *engine,
    uint32_t   width,
    uint32_t   height
){
    if(engine->backbuffer.data)
    {
        VirtualFree(engine->backbuffer.data, 0, MEM_RELEASE);
    }

    engine->backbuffer.width  = width;
    engine->backbuffer.height = height;

    engine->backbuffer.info.bmiHeader.biSize        = sizeof(engine->backbuffer.info.bmiHeader);
    engine->backbuffer.info.bmiHeader.biWidth       =  (long)engine->backbuffer.width;
    engine->backbuffer.info.bmiHeader.biHeight      = -(long)engine->backbuffer.height;
    engine->backbuffer.info.bmiHeader.biPlanes      = 1;
    engine->backbuffer.info.bmiHeader.biBitCount    = 32;
    engine->backbuffer.info.bmiHeader.biCompression = BI_RGB;

    engine->backbuffer.data = VirtualAlloc(0, width * height * RIVER2D_BPP, MEM_COMMIT, PAGE_READWRITE);
    if(!engine->backbuffer.data)
    {
        fprintf(stderr, "\033[31;1;7mERROR: failed to resize backbuffer.\033[0m");
    }
}

void river2D_init
(
    EngineData         *engine,
    River2D_Image      *planes
){
    engine->running = true;
    engine->planes  = planes;

    if(!engine->windowName)
    {
        engine->windowName = "unnamed river2D application";
    }

    if(engine->config.choices & RIVER2D_CHOICE_STATIC_CANVAS_BIT)
    {
        river2D_resizeBackbuffer(engine, engine->config.canvas_width, engine->config.canvas_height);
    }
    else
    {
        river2D_resizeBackbuffer(engine, engine->config.window_width, engine->config.window_height);
    }
}

int32_t river2D_shutdown
(
    EngineData *engine
){
    DeleteObject(engine->cursorMask);

    DestroyWindow(engine->window);

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
    if(pictop != RIVER2D_PICTOP_OVER)
    {
        fprintf(stderr, "\033[31;1;7mERROR: pictop %u not impletmented on windows.\033[0m\n", pictop);
        return;
    }

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

    // TODAY: (river2D #5) verify that both images are actually RGBA

    uint64_t copyWidth = image->width * RIVER2D_BPP;
    uint64_t bufWidth  = engine->backbuffer.width * RIVER2D_BPP;

    if(offsetDstX + cropWidth > engine->backbuffer.width)
    {
        cropWidth = engine->backbuffer.width - offsetDstX;
    }

    if(offsetDstY + cropHeight > engine->backbuffer.height)
    {
        cropHeight = engine->backbuffer.height - offsetDstY;
    }

    uint8_t *dst = (uint8_t*)engine->backbuffer.data + offsetDstY * bufWidth + offsetDstX * RIVER2D_BPP;
    uint8_t *src = image->data + offsetSrcY * copyWidth + offsetSrcX * RIVER2D_BPP;

    cropWidth *= RIVER2D_BPP;
    for(uint32_t y = 0; y < cropHeight; ++y)
    {
        for(uint32_t x = 0; x < cropWidth; x += RIVER2D_BPP)
        {
            uint64_t srcIndex = y * copyWidth + x;
            uint64_t dstIndex = y * bufWidth + x;
            if(src[srcIndex + 3])
            {
                dst[dstIndex]     = src[srcIndex];
                dst[dstIndex + 1] = src[srcIndex + 1];
                dst[dstIndex + 2] = src[srcIndex + 2];
                dst[dstIndex + 3] = src[srcIndex + 3];
            }
        }
    }
}

// TODO: figure out some bilinear or lanzcos or something for this, currently it looks awful
void river2D_bltBuffer
(
    EngineData *engine
){
    Dimensions dim = {0};
    dim = river2D_getWindowSize(engine);

    StretchDIBits(engine->context, 0, 0, dim.width, dim.height, 0, 0,
                  (int)engine->backbuffer.width, (int)engine->backbuffer.height,
                  engine->backbuffer.data, &engine->backbuffer.info, DIB_RGB_COLORS, SRCCOPY);
}
