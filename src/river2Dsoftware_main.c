#include "river2D_main.h"

#ifdef BUILD_WINDOWS
    #include "win32_river2Dsoftware_platform.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//TODAY: multi-thread.
//1 pixel at a time for each thread, if possible.
//this is by far the biggest bottleneck.
//pthread_t seems to be real easy to work with.
//I wonder if windows has a good equivalent...
void river2D_compositeImage
(
    EngineData    *engine,
    River2D_Image *image,
    uint8_t       pictop,
    uint32_t      offsetX,
    uint32_t      offsetY,
    uint32_t      cropX,
    uint32_t      cropY
){
    //TODO: deal with alpha and actual compositing instead of just overlaying/copying
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

    //TODO: verify that both images are actually RGBA
    //(in other words, that there's enough space)

    //TODO: validate that offset doesn't exceed buffer destination image

    uint8_t *dest = (uint8_t*)engine->backbuffer.data;
    uint64_t copyWidth = image->width * RIVER2D_BPP;
    uint64_t bufWidth  = engine->backbuffer.width * RIVER2D_BPP;

    for(uint32_t y = 0; y < image->height; ++y)
    {
        for(uint32_t x = 0; x < copyWidth; x += RIVER2D_BPP)
        {
            uint64_t srcIndex = y * copyWidth + x;
            uint64_t dstIndex = y * bufWidth + x;
            if(image->data[srcIndex + 3])
            {
                dest[dstIndex]     = image->data[srcIndex];
                dest[dstIndex + 1] = image->data[srcIndex + 1];
                dest[dstIndex + 2] = image->data[srcIndex + 2];
                dest[dstIndex + 3] = image->data[srcIndex + 3];
            }
        }
    }
}

//TODO: allow for other font colours?
//maybe load image as some stencil boolean, then operate on the pixels (with desired colour)
//based on that stencil
//this could also be its own river2D_compositeText function
void river2D_loadText
(
    EngineData    *engine,
    River2D_Image *image,
    const char    *text,
    uint8_t       font,
    uint16_t      charsize,
    uint32_t      spacing,
    uint32_t      offsetY,
    uint32_t      offsetX
){
    if(!engine->planes[font].data)
    {
        fprintf(stderr, "Font not found. Check loaded planes.\n");
        return;
    }

    if(!image)
    {
        fprintf(stderr, "Destination image is null.\n");
        return;
    }

    uint32_t minTextWidth = charsize * (uint32_t)(strlen(text) + 1);

    if(image->width != minTextWidth || image->height != charsize)
    {
        free(image->data);
        image->data = 0;
    }

    if(!image->data)
    {
        image->height = charsize;
        image->width  = minTextWidth;

        image->data = calloc(image->height * image->width * RIVER2D_BPP, 1);
    }

    if(offsetY > image->height)
    {
        fprintf(stderr, "offsetY too large.\n");
        return;
    }
    if(offsetX > image->width)
    {
        fprintf(stderr, "offsetX too large.\n");
        return;
    }
    uint32_t fontImgWidth = engine->planes[font].width;

    for(uint32_t i = 0; text[i] != '\0'; ++i)
    {
        if(text[i] < 33 || text[i] > 127)
        {
            continue;
        }
        uint32_t  charBigY = (uint32_t)(text[i] - 33) * charsize / fontImgWidth;
        uint32_t  charBigX = (uint32_t)(text[i] - 33) * charsize % fontImgWidth;

        uint64_t trueSrcOffset = (charBigY * charsize * fontImgWidth + charBigX) * RIVER2D_BPP;
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
}
