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
                controls->direction |= River2D_DIR_UP;
            }
            else
            {
                controls->direction &= ~River2D_DIR_UP;
            }
            break;
        }
        case River2D_KEY_DOWN:
        {
            if(isDown)
            {
                controls->direction |= River2D_DIR_DOWN;
            }
            else
            {
                controls->direction &= ~River2D_DIR_DOWN;
            }
            break;
        }
        case River2D_KEY_LEFT:
        {
            if(isDown)
            {
                controls->direction |= River2D_DIR_LEFT;
            }
            else
            {
                controls->direction &= ~River2D_DIR_LEFT;
            }
            break;
        }
        case River2D_KEY_RIGHT:
        {
            if(isDown)
            {
                controls->direction |= River2D_DIR_RIGHT;
            }
            else
            {
                controls->direction &= ~River2D_DIR_RIGHT;
            }
            break;
        }
        case River2D_KEY_TAB:
        {
            controls->tab = isDown;
            break;
        }
    }

    //TODO: tab and other hotkeys
}
