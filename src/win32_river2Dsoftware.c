#include "river2D_main.h"
#include "rv_print_macros.h"

#include "win32_river2Dsoftware_platform.h"

void rvResizeBackbuffer
(
    EngineData *engine,
    u32   width,
    u32   height
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
        RV_ERROR("failed to resize backbuffer.");
    }
}

void _init
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
                      engine->config.canvasWidth, engine->config.canvasHeight);
    }
    else
    {
        rvCreateImage(engine, &engine->backbuffer,
                      engine->config.windowWidth, engine->config.windowHeight);
    }
}

i32 _shutdown
(
    EngineData *engine
){
    DeleteObject(engine->cursorMask);
    DestroyWindow(engine->window);

    return 0;
}

void _compositeImage
(
    EngineData *engine,
    RiverImage *src,
    RiverImage *dst,
    u08        pictop,
    u32        offsetSrcX,
    u32        offsetSrcY,
    u32        offsetDstX,
    u32        offsetDstY,
    u32        cropWidth,
    u32        cropHeight
){
    if(pictop != RV_PICTOP_OVER)
    {
        RV_ERROR("pictop %u not implemented on windows.", pictop);
        return;
    }

    if(!src)
    {
        RV_ERROR("no image to composite with.");
        return;
    }
    if(!src->data)
    {
        RV_ERROR("src->data is nullptr.");
        return;
    }

    if(!dst)
    {
        RV_ERROR("no image to composite onto.");
        return;
    }
    if(!dst->data)
    {
        RV_ERROR("dst->data is nullptr.");
        return;
    }

    u64 copyWidth = src->width * RV_BPP;
    u64 bufWidth  = dst->width * RV_BPP;

    if(offsetDstX + cropWidth > engine->backbuffer.width)
    {
        cropWidth = engine->backbuffer.width - offsetDstX;
    }

    if(offsetDstY + cropHeight > engine->backbuffer.height)
    {
        cropHeight = engine->backbuffer.height - offsetDstY;
    }

    u08 *dstData = dst->data + offsetDstY * bufWidth  + offsetDstX * RV_BPP;
    u08 *srcData = src->data + offsetSrcY * copyWidth + offsetSrcX * RV_BPP;

    u08 *maxDst = dst->data + dst->width * dst->height * RV_BPP - 1;
    u08 *maxSrc = src->data + src->width * src->height * RV_BPP - 1;

    cropWidth *= RV_BPP;
    for(u32 y = 0; y < cropHeight; ++y)
    {
        for(u32 x = 0; x < cropWidth; x += RV_BPP)
        {
            u64 srcIndex = y * copyWidth + x;
            u64 dstIndex = y * bufWidth  + x;

            if(dstData + dstIndex > maxDst ||
               srcData + srcIndex > maxSrc
            ){
                break;
            }

            if(srcData[srcIndex + 3])
            {
                dstData[dstIndex]     = srcData[srcIndex];
                dstData[dstIndex + 1] = srcData[srcIndex + 1];
                dstData[dstIndex + 2] = srcData[srcIndex + 2];
                dstData[dstIndex + 3] = srcData[srcIndex + 3];
            }
        }
    }
}

void _bltBuffer
(
    EngineData *engine
){
    Dimensions dim = {0};
    dim = rvGetWindowSize(engine);

    StretchDIBits(engine->context, 0, 0, (int)dim.width, (int)dim.height, 0, 0,
                  (int)engine->backbuffer.width, (int)engine->backbuffer.height,
                  engine->backbuffer.data, &engine->backbuffer.info, DIB_RGB_COLORS,
                  SRCCOPY);
}

void _loadText
(
    EngineData *engine,
    RiverImage *image,
    StringView *sv,
    u08        font,
    u16        charsize,
    u32        spacing,
    u32        offsetX,
    u32        offsetY
){
    if(!engine->planes[font].data)
    {
        RV_ERROR("font not found. Check loaded planes.");
        return;
    }

    if(!image)
    {
        RV_ERROR("destination image is null.");
        return;
    }

    u32 minTextWidth = (charsize + spacing) * (u32)sv->size;

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
        RV_ERROR("offsetX too large.");
        return;
    }
    if(offsetY > image->height)
    {
        RV_ERROR("offsetY too large.");
        return;
    }

    u32 fontImgWidth = engine->planes[font].width;
    u32 imageChars = (image->width) / ((charsize + spacing));

    for(u32 i = 0; i < imageChars; ++i)
    {
        char character = 0x20;
        if(i < sv->size && sv->data[i])
        {
            character = sv->data[i];
        }

        u32 charBigX = (u32)(character - 0x21) * charsize % fontImgWidth;
        u32 charBigY = (u32)(character - 0x21) * charsize / fontImgWidth;

        if(character == 0x20)
        {
            charBigX = (u32)(0x5F) * charsize % fontImgWidth;
            charBigY = (u32)(0x5F) * charsize / fontImgWidth;
        }
        else if(character == RV_ASCII_CURSOR)
        {
            charBigX = (u32)(0x5E) * charsize % fontImgWidth;
            charBigY = (u32)(0x5E) * charsize / fontImgWidth;
        }
        else if(character < 0x20)
        {
            continue;
        }

        u64 trueSrcOffset  = (charBigY * charsize * fontImgWidth + charBigX) * RV_BPP;
        u64 trueDestOffset = (offsetY * image->width + offsetX + i *
                              (charsize + spacing)) * RV_BPP;

        u08* charloc = engine->planes[font].data + trueSrcOffset;
        u08* destloc = image->data + trueDestOffset;

        for(u32 j = 0; j < charsize; ++j)
        {
            u08* charlineLoc = charloc + j * fontImgWidth * RV_BPP;
            u08* destlineLoc = destloc + j * image->width * RV_BPP;

            memcpy(destlineLoc, charlineLoc, charsize * RV_BPP);
        }
    }
}
