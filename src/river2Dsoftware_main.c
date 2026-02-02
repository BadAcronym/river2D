#include "river2D_main.h"

#ifdef BUILD_WINDOWS
    #include "win32_river2Dsoftware_platform.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: (river2D #7) allow for other font colours
// TODO: (river2D #17) allow for linebreaks and other escape codes
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
        fprintf(stderr, "Font not found. Check loaded planes.\n");
        return;
    }

    if(!image)
    {
        fprintf(stderr, "Destination image is null.\n");
        return;
    }

    uint32_t minTextWidth = charsize * (uint32_t)(strlen(text) + 1);

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

        uint32_t  charBigX = (uint32_t)(text[i] - 0x21) * charsize % fontImgWidth;
        uint32_t  charBigY = (uint32_t)(text[i] - 0x21) * charsize / fontImgWidth;

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
