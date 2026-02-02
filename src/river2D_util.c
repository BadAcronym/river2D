#include <stdint.h>
#include <string.h>

#include "river2D_main.h"
#include "imgsurf_load.h"
#include <stdio.h>

const char* river2D_contains
(
    const char *bigStr,
    const char *smallStr
){
    size_t subsize = strlen(smallStr);
    size_t size = strlen(bigStr) - subsize;

    if(size > subsize)
    {
        return 0;
    }

    for(size_t i = 0, j = 0; i < size; ++i)
    {
        if(bigStr[i] == smallStr[j])
        {
            for(; j < subsize; ++j)
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

        //NOTE: we are reading the whole file twice. why..?
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
                fprintf(stderr, "parsed result: true\n");
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

                fprintf(stderr, "parsed result: %u\n", parsedWidth_canvas);

                config->canvas_width = parsedWidth_canvas;
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

                fprintf(stderr, "parsed result: %u\n", parsedHeight_canvas);

                config->canvas_height = parsedHeight_canvas;
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

                fprintf(stderr, "parsed result: %u\n", parsedWidth_window);

                config->window_width = parsedWidth_window;
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

                fprintf(stderr, "parsed result: %u\n", parsedHeight_window);

                config->window_height = parsedHeight_window;
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
