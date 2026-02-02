//TODO: load renderer .so/.dll (software, openGL, Vulkan) dynamically?
//same with input libs (like Xinput)

//TODO: main menu with loading files, map creator/tile editor

#include "river2D_main.h"
#include "imgsurf_load.h"
#include <stdio.h>
// #include <stdlib.h>

void river2D_processControls
(
    bool               isDown,
    int32_t            key,
    River2D_ControlMap *controls
){
    switch(key)
    {
        case RIVER2D_KEY_UP:
        {
            if(isDown)
            {
                controls->keymap |= RIVER2D_BIT_UP;
            }
            else
            {
                controls->keymap &= ~RIVER2D_BIT_UP;
            }
            break;
        }
        case RIVER2D_KEY_DOWN:
        {
            if(isDown)
            {
                controls->keymap |= RIVER2D_BIT_DOWN;
            }
            else
            {
                controls->keymap &= ~RIVER2D_BIT_DOWN;
            }
            break;
        }
        case RIVER2D_KEY_LEFT:
        {
            if(isDown)
            {
                controls->keymap |= RIVER2D_BIT_LEFT;
            }
            else
            {
                controls->keymap &= ~RIVER2D_BIT_LEFT;
            }
            break;
        }
        case RIVER2D_KEY_RIGHT:
        {
            if(isDown)
            {
                controls->keymap |= RIVER2D_BIT_RIGHT;
            }
            else
            {
                controls->keymap &= ~RIVER2D_BIT_RIGHT;
            }
            break;
        }
        case RIVER2D_KEY_TAB:
        {
            if(isDown)
            {
                controls->keymap |= RIVER2D_BIT_TAB;
            }
            else
            {
                controls->keymap &= ~RIVER2D_BIT_TAB;
            }
            break;
        }
        case RIVER2D_KEY_ESCAPE:
        {
            if(isDown)
            {
                controls->keymap |= RIVER2D_BIT_ESCAPE;
            }
            else
            {
                controls->keymap &= ~RIVER2D_BIT_ESCAPE;
            }
            break;
        }
        //test keycodes
        #if 0
        default:
        {
            printf("keycode: %u\n", key);
        }
        #endif
    }

    //TODO: tab and other hotkeys to navigate & control the editor
}

void river2D_updateEditor()
{
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

        //TODAY: parse config data and pass to *config

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
