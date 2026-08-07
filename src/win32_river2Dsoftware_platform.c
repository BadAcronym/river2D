#include "river2D_main.h"

#include "win32_river2Dsoftware_platform.h"

#include <stdio.h>

void rvResizeBackbuffer
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

    engine->backbuffer.data = VirtualAlloc(0, width * height * RV_BPP,
                                           MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if(!engine->backbuffer.data)
    {
        fprintf(stderr, "\033[31;1;7mERROR: failed to resize backbuffer.\033[0m");
    }
}

void init
(
    EngineData *engine,
    RiverImage *planes
){
    engine->running = true;
    engine->planes  = planes;

    if(!engine->windowName)
    {
        engine->windowName = "unnamed river2D application";
    }

    if(engine->config.choices & RV_CHOICE_STATIC_CANVAS_BIT)
    {
        rvCreateImage(engine, &engine->backbuffer,
                            engine->config.canvas_width, engine->config.canvas_height);
    }
    else
    {
        rvCreateImage(engine, &engine->backbuffer,
                            engine->config.window_width, engine->config.window_height);
    }
}

int32_t shutdown
(
    EngineData *engine
){
    DeleteObject(engine->cursorMask);
    DestroyWindow(engine->window);

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
    if(pictop != RV_PICTOP_OVER)
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

    uint64_t copyWidth = src->width * RV_BPP;
    uint64_t bufWidth  = dst->width * RV_BPP;

    if(offsetDstX + cropWidth > engine->backbuffer.width)
    {
        cropWidth = engine->backbuffer.width - offsetDstX;
    }

    if(offsetDstY + cropHeight > engine->backbuffer.height)
    {
        cropHeight = engine->backbuffer.height - offsetDstY;
    }

    uint8_t *dst_data = (uint8_t*)dst->data +
                        offsetDstY * bufWidth + offsetDstX * RV_BPP;
    uint8_t *src_data = src->data + offsetSrcY * copyWidth + offsetSrcX * RV_BPP;

    cropWidth *= RV_BPP;
    for(uint32_t y = 0; y < cropHeight; ++y)
    {
        for(uint32_t x = 0; x < cropWidth; x += RV_BPP)
        {
            uint64_t srcIndex = y * copyWidth + x;
            uint64_t dstIndex = y * bufWidth + x;
            if(src_data[srcIndex + 3])
            {
                dst_data[dstIndex]     = src_data[srcIndex];
                dst_data[dstIndex + 1] = src_data[srcIndex + 1];
                dst_data[dstIndex + 2] = src_data[srcIndex + 2];
                dst_data[dstIndex + 3] = src_data[srcIndex + 3];
            }
        }
    }
}

void bltBuffer
(
    EngineData *engine
){
    Dimensions dim = {0};
    dim = rvGetWindowSize(engine);

    StretchDIBits(engine->context, 0, 0, dim.width, dim.height, 0, 0,
                  (int)engine->backbuffer.width, (int)engine->backbuffer.height,
                  engine->backbuffer.data, &engine->backbuffer.info, DIB_RGB_COLORS,
                  SRCCOPY);
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
        char character = 0x7F;
        if(i < sv->size)
        {
            character = sv->data[i];
        }

        if(character < 0x21 || character > 0x7F)
        {
            continue;
        }

        uint32_t charBigX = (uint32_t)(character - 0x21) * charsize % fontImgWidth;
        uint32_t charBigY = (uint32_t)(character - 0x21) * charsize / fontImgWidth;

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
}
