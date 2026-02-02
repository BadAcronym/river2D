#include <stdint.h>
#include <string.h>

#include "river2D_main.h"
#include "imgsurf_main.h"
#include <stdio.h>

const char* river2D_contains
(
    const char *bigStr,
    const char *smallStr
){
    size_t smallsize = strlen(smallStr);
    size_t bigsize   = strlen(bigStr);

    for(size_t i = 0, j = 0; i < bigsize - smallsize; ++i)
    {
        if(bigStr[i] == smallStr[j])
        {
            for(; j < smallsize; ++j)
            {
                if(bigStr[i + j] != smallStr[j])
                {
                    goto retry;
                }
            }
            return &bigStr[i];
        }
retry:
        j = 0;
    }

    return 0;
}

internal void writeMissingTexture
(
    River2D_Image *image
){
    for(uint32_t y = 0; y < image->height; ++y)
    {
        for(uint32_t x = 0; x < image->width; ++x)
        {
            ((uint32_t*)image->data)[x * y * RIVER2D_BPP] = 0xC64FACFF;
        }
    }
}

void river2D_loadImage
(
    const char    *path,
    River2D_Image *image,
    uint8_t       format,
    uint8_t       bitdepth
){
    image->data = imgsurf_load(path, &image->width, &image->height, format, bitdepth);

    if(!image->data)
    {
        fprintf(stderr, "Failed to load image from file: %s\n", path);
        writeMissingTexture(image);
    }
}

// TODO: (river2D #8) allow for hot reloading via menu if necessary, apply config
//fuzz config file, make sure it can't crash the engine

#define bufsize 32

void river2D_loadConfig
(
    River2D_Config *config
){
    if(!config)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: *config is nullptr.\033[0m\n");
        return;
    }
    uint32_t parsedWidth_canvas  = 0;
    uint32_t parsedHeight_canvas = 0;
    uint32_t parsedWidth_window  = 0;
    uint32_t parsedHeight_window = 0;

    uint8_t code = river2D_verifyPath(RIVER2D_CONFIG_PATH);

    if(code == RIVER2D_TYPE_FILE)
    {
        FILE *file = fopen(RIVER2D_CONFIG_PATH, "r");
        if(!file)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: Could not open file %s\033[0m\n", RIVER2D_CONFIG_PATH);
            return;
        }

        char buf[bufsize];
        while(fgets(buf, bufsize, file))
        {
            const char* fpsloc = river2D_contains(buf, "showFPS");
            if(fpsloc && (fpsloc + 9 - buf) < bufsize)
            {
                bool foundShowFps = *(fpsloc + 9) == '1' || *(fpsloc + 9) == 't';
                if(!foundShowFps)
                {
                    continue;
                }
                config->choices |= RIVER2D_CHOICE_SHOW_FPS_BIT;
                #ifdef DEBUG
                fprintf(stderr, "parsed result: true\n");
                #endif
                continue;
            }

            const char* cwidthloc = river2D_contains(buf, "canvas_width");
            if(cwidthloc && (cwidthloc + 14 - buf) < bufsize)
            {
                for(uint32_t i = 0; i < bufsize; ++i)
                {
                    if(cwidthloc + 14 + i - buf > bufsize)
                    {
                        break;
                    }

                    char digit = *(cwidthloc + 14 + i);
                    if(digit < 0x30 || digit > 0x39)
                    {
                        break;
                    }
                    parsedWidth_canvas *= 10;
                    parsedWidth_canvas += (digit - 0x30);
                }

                #ifdef DEBUG
                fprintf(stderr, "parsed result: %u\n", parsedWidth_canvas);
                #endif

                config->canvas_width = parsedWidth_canvas;
                continue;
            }

            const char* cheightloc = river2D_contains(buf, "canvas_height");
            if(cheightloc && (cheightloc + 15 - buf) < bufsize)
            {
                for(uint32_t i = 0; i < bufsize; ++i)
                {
                    if(cheightloc + 15 + i - buf > bufsize)
                    {
                        break;
                    }

                    char digit = *(cheightloc + 15 + i);
                    if(digit < 0x30 || digit > 0x39)
                    {
                        break;
                    }
                    parsedHeight_canvas *= 10;
                    parsedHeight_canvas += (digit - 0x30);
                }

                #ifdef DEBUG
                fprintf(stderr, "parsed result: %u\n", parsedHeight_canvas);
                #endif

                config->canvas_height = parsedHeight_canvas;
                continue;
            }

            const char* wwidthloc = river2D_contains(buf, "window_width");
            if(wwidthloc && (wwidthloc + 14 - buf) < bufsize)
            {
                for(uint32_t i = 0; i < bufsize; ++i)
                {
                    if(wwidthloc + 14 + i - buf > bufsize)
                    {
                        break;
                    }

                    char digit = *(wwidthloc + 14 + i);
                    if(digit < 0x30 || digit > 0x39)
                    {
                        break;
                    }
                    parsedWidth_window *= 10;
                    parsedWidth_window += (digit - 0x30);
                }

                #ifdef DEBUG
                fprintf(stderr, "parsed result: %u\n", parsedWidth_window);
                #endif

                config->window_width = parsedWidth_window;
                continue;
            }

            const char* wheightloc = river2D_contains(buf, "window_height");
            if(wheightloc && (wheightloc + 15 - buf) < bufsize)
            {
                for(uint32_t i = 0; i < bufsize; ++i)
                {
                    if(wheightloc + 15 + i - buf > bufsize)
                    {
                        break;
                    }

                    char digit = *(wheightloc + 15 + i);
                    if(digit < 0x30 || digit > 0x39)
                    {
                        break;
                    }
                    parsedHeight_window *= 10;
                    parsedHeight_window += (digit - 0x30);
                }

                #ifdef DEBUG
                fprintf(stderr, "parsed result: %u\n", parsedHeight_window);
                #endif

                config->window_height = parsedHeight_window;
                continue;
            }
        }

        fclose(file);
    }
    else if(code == RIVER2D_TYPE_ERROR)
    {
        fprintf(stderr, "\nCan't find the file '%s', default config loaded.\n\n", RIVER2D_CONFIG_PATH);
    }
    else if(code == RIVER2D_TYPE_DIRECTORY)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: '%s' is a Directory! Default config loaded.\033[0m\n",
                RIVER2D_CONFIG_PATH);
    }
    else if(code == RIVER2D_TYPE_OTHER)
    {
        fprintf(stderr, "\nUnknown filetype for '%s', default config loaded.\n\n", RIVER2D_CONFIG_PATH);
    }

    if(!parsedWidth_canvas)
    {
        config->canvas_width  = 1280;
    }
    if(!parsedHeight_canvas)
    {
        config->canvas_height = 720;
    }
    if(!parsedWidth_window)
    {
        config->window_width  = 2560;
    }
    if(!parsedHeight_window)
    {
        config->window_height = 1440;
    }
}

bool river2D_insideArea
(
    Coordinates *point,
    Area        *area
){
    // TODO:
    // in the future, handle non parallel cases.
    // if(area->upperLeft.x == area->lowerLeft.x && area->upperRight.x && ...)

    return(point->x > area->upperLeft.x && point->x < area->upperRight.x &&
           point->y > area->upperLeft.y && point->y < area->lowerRight.y);
}

bool river2D_insideRect
(
    Coordinates *point,
    Rect        *rect
){
    return(point->x > rect->upperLeft.x && point->x < rect->lowerRight.x &&
           point->y > rect->upperLeft.y && point->y < rect->lowerRight.y);
}

void river2D_createButton
(
    EngineData    *engine,
    River2D_Image *img,
    const char    *text,
    uint8_t       font,
    uint16_t      charsize,
    uint32_t      spacing,
    Coordinates   point,
    Rect          *rect
){
    uint32_t length = 0;

    for(uint32_t i = 0; text[i] != '\0'; ++i)
    {
        ++length;
    }

    double floatWidth  = (double)length * (charsize + spacing) / (double)engine->backbuffer.width;
    double floatHeight = (double)charsize / (double)engine->backbuffer.height;

    uint32_t offsetX   = (point.x - floatWidth  / 2.0f) * engine->backbuffer.width;
    uint32_t offsetY   = (point.y - floatHeight / 2.0f) * engine->backbuffer.height;

    rect->upperLeft.x  = (double)offsetX / (double)engine->backbuffer.width;
    rect->upperLeft.y  = (double)offsetY / (double)engine->backbuffer.height;
    rect->lowerRight.x = rect->upperLeft.x + floatWidth;
    rect->lowerRight.y = rect->upperLeft.y + floatHeight;

    engine->river2D_loadText(engine, img, text, font, charsize, spacing, offsetX, offsetY);
}
