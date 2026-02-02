#include "river2Dsoftware_main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//TODO: allow for other font colours?
//maybe load image as some stencil boolean, then operate on the pixels (with desired colour)
//based on that stencil
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

    if(!image->data)
    {
        image->height = charsize;
        image->width  = (uint32_t)(image->height * charsize * strlen(text));

        image->data = malloc(image->height * image->width * RIVER2D_BPP);
        memset(image->data, 0, image->height * image->width * RIVER2D_BPP);
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

            memset(destlineLoc, 0, 2 * charsize * RIVER2D_BPP);
            memcpy(destlineLoc, charlineLoc, charsize * RIVER2D_BPP);
        }
    }
}
