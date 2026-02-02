//TODO: load renderer .so/.dll (software, openGL, Vulkan) dynamically
//same with input libs (like Xinput)

//TODO: main menu with loading files, map creator/tile editor

#include "river2D_main.h"
#include <stdio.h>

void river2D_processControls
(
    bool              isDown,
    int32_t           key,
    River2DControlMap *controls
){
    switch(key)
    {
        case River2D_KEY_UP:
        {
            if(isDown)
            {
                controls->keymap |= River2D_BIT_UP;
            }
            else
            {
                controls->keymap &= ~River2D_BIT_UP;
            }
            break;
        }
        case River2D_KEY_DOWN:
        {
            if(isDown)
            {
                controls->keymap |= River2D_BIT_DOWN;
            }
            else
            {
                controls->keymap &= ~River2D_BIT_DOWN;
            }
            break;
        }
        case River2D_KEY_LEFT:
        {
            if(isDown)
            {
                controls->keymap |= River2D_BIT_LEFT;
            }
            else
            {
                controls->keymap &= ~River2D_BIT_LEFT;
            }
            break;
        }
        case River2D_KEY_RIGHT:
        {
            if(isDown)
            {
                controls->keymap |= River2D_BIT_RIGHT;
            }
            else
            {
                controls->keymap &= ~River2D_BIT_RIGHT;
            }
            break;
        }
        case River2D_KEY_TAB:
        {
            if(isDown)
            {
                controls->keymap |= River2D_BIT_TAB;
            }
            else
            {
                controls->keymap &= ~River2D_BIT_TAB;
            }
            break;
        }
        case River2D_KEY_ESCAPE:
        {
            if(isDown)
            {
                controls->keymap |= River2D_BIT_ESCAPE;
            }
            else
            {
                controls->keymap &= ~River2D_BIT_ESCAPE;
            }
            break;
        }
        #if 0
        default:
        {
            printf("%u", key);
            printf("%c", '\n');
        }
        #endif
    }

    //TODO: tab and other hotkeys
}

void river2D_updateEditor()
{
}
