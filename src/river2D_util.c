#include <stdint.h>
#include <string.h>

#include "river2D_main.h"
#include "imgsurf_load.h"
#include <stdio.h>
// #include <stdlib.h>

const char* river2D_contains
(
    const char *bigStr,
    const char *smallStr
){
    size_t subsize = strlen(smallStr);
    size_t size = strlen(bigStr) - subsize;

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

//TODO: allow for hot reloading via menu if necessary, apply config
//TODO: fuzz config file, make sure it can't crash the engine

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

    uint8_t code = river2D_verifyPath(RIVER2D_CONFIG_PATH);

    if(code == RIVER2D_TYPE_FILE)
    {
        FILE *file = fopen(RIVER2D_CONFIG_PATH, "r");
        if(!file)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: Could not open file %s\033[0m\n", RIVER2D_CONFIG_PATH);
            return;
        }

        bool parsedWidth = false;
        bool parsedHeight = false;

        char buf[bufsize];
        while(fgets(buf, bufsize, file))
        {
            const char* fpsloc = river2D_contains(buf, "showFPS");
            if(fpsloc && (fpsloc - buf) < bufsize)
            {
                bool parsedShowFps = *(fpsloc + 9) == '1' || *(fpsloc + 9) == 't';
                if(!parsedShowFps)
                {
                    continue;
                }
                config->choices |= RIVER2D_CHOICE_SHOW_FPS_BIT;
            }

            //TODAY: parse width, height, etc
        }

        if(!parsedWidth)
        {
            config->width = 1280;
        }
        if(!parsedHeight)
        {
            config->height = 720;
        }

        fclose(file);
        return;
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

    config->choices = 0;
    config->choices |= RIVER2D_CHOICE_SHOW_FPS_BIT;
    config->width   = 1280;
    config->height  = 720;
}
