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

    engine->backbuffer.info.bmiHeader.biSize   = sizeof(engine->backbuffer.info.bmiHeader);
    engine->backbuffer.info.bmiHeader.biWidth  = (long)engine->backbuffer.width;
    engine->backbuffer.info.bmiHeader.biHeight = -(long)engine->backbuffer.height;
    engine->backbuffer.info.bmiHeader.biPlanes = 1;
    engine->backbuffer.info.bmiHeader.biBitCount = 32;
    engine->backbuffer.info.bmiHeader.biCompression = BI_RGB;

    engine->backbuffer.data = VirtualAlloc(0, width * height * RIVER2D_BPP, MEM_COMMIT, PAGE_READWRITE);
    if(!engine->backbuffer.data)
    {
        fprintf(stderr, "\033[31;1;7mERROR: failed to resize backbuffer\033[0m");
    }
}

void river2D_init
(
    EngineData         *engine,
    River2D_Image      *planes
){
    engine->planes = planes;

    engine->windowName = "unnamed river2D application";

    if(engine->config.choices & RIVER2D_CHOICE_STATIC_CANVAS_BIT)
    {
        river2D_resizeBackbuffer(engine, engine->config.canvas_width, engine->config.canvas_height);
    }
    else
    {
        river2D_resizeBackbuffer(engine, engine->config.window_width, engine->config.window_height);
    }

    River2D_Time time = river2D_queryTime();
    engine->lastFrametime = time;
    engine->lastFPStime = time;
}

int32_t river2D_shutdown
(
    EngineData *engine
){
    // stuff
    DestroyWindow(engine->window);

    return 0;
}

// TODAY: (river2D #1) multi-thread.
// look at linux code for reference
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

    if(!engine->backbuffer.data)
    {
        fprintf(stderr, "\033[31;1;7mERROR: no image to composite onto.\033[0m\n");
        return;
    }

    // TODAY: (river2D #5) verify that both images are actually RGBA
    // (in other words, that there's enough space)
    // also validate that offset doesn't exceed buffer destination image

    uint64_t copyWidth  = image->width * RIVER2D_BPP;
    uint64_t srcCutoffX = cropWidth * RIVER2D_BPP;
    uint64_t bufWidth   = engine->backbuffer.width * RIVER2D_BPP;

    uint8_t *dest = (uint8_t*)engine->backbuffer.data + offsetDstY * bufWidth +
                    offsetDstX * RIVER2D_BPP;

    uint8_t *src  = image->data + offsetSrcY * copyWidth + offsetSrcX * RIVER2D_BPP;

    for(uint32_t y = 0; y < cropHeight; ++y)
    {
        for(uint32_t x = 0; x < srcCutoffX; x += RIVER2D_BPP)
        {
            uint64_t srcIndex = y * copyWidth + x;
            uint64_t dstIndex = y * bufWidth + x;
            if(src[srcIndex + 3])
            {
                dest[dstIndex]     = src[srcIndex];
                dest[dstIndex + 1] = src[srcIndex + 1];
                dest[dstIndex + 2] = src[srcIndex + 2];
                dest[dstIndex + 3] = src[srcIndex + 3];
            }
        }
    }
}

void river2D_bltBuffer
(
    EngineData *engine
){
    RECT clientRect;
    GetClientRect(engine->window, &clientRect);
    int width  = clientRect.right  - clientRect.left;
    int height = clientRect.bottom - clientRect.top;

    StretchDIBits(engine->context, 0, 0, width, height, 0, 0,
                  (int)engine->backbuffer.width, (int)engine->backbuffer.height,
                  engine->backbuffer.data, &engine->backbuffer.info, DIB_RGB_COLORS, SRCCOPY);
}
