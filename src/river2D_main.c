//TODO: load renderer .so/.dll (software, openGL, Vulkan) dynamically
//same with input libs (like Xinput)

//TODO: main menu with loading files, map creator/tile editor

#include "river2D_main.h"
#include "imgsurf_load.h"
#include <stdio.h>
#include <stdlib.h>

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
        #if 1
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
    image->width  = 2560;
    image->height = 1440;

    for(uint32_t i = 0; i < 2560 * 1440 * RIVER2D_BPP; i += 4)
    {
        image->data[i]     = 0xC6;
        image->data[i + 1] = 0x4F;
        image->data[i + 2] = 0xAC;
        image->data[i + 3] = 0xFF;
    }
}

void river2D_loadImage
(
    const char*   path,
    River2D_Image *image,
    uint8_t       format,
    uint8_t       depth
){
    image->data = imgsurf_load(path, &image->width, &image->height, format, depth);

    if(!image->data)
    {
        fprintf(stderr, "Failed to load image from file: %s\n", path);
        writeMissingTexture(image);
    }
}
