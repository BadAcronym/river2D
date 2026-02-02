//TODO: load renderer .so/.dll (software, openGL, Vulkan) dynamically
//same with input libs (like Xinput)

//TODO: main menu with loading files, map creator/tile editor

#include "river2D_main.h"

void river2D_processControls
(
    bool              isDown,
    int32_t           key,
    River2DControlMap *controls
){
    if(key == River2D_UP)
    {
        if(isDown)
        {
            controls->direction |= River2D_DIR_UP;
        }
        else
        {
            controls->direction &= ~River2D_DIR_UP;
        }
    }
    else if(key == River2D_DOWN)
    {
        if(isDown)
        {
            controls->direction |= River2D_DIR_DOWN;
        }
        else
        {
            controls->direction &= ~River2D_DIR_DOWN;
        }
    }
    else if(key == River2D_LEFT)
    {
        if(isDown)
        {
            controls->direction |= River2D_DIR_LEFT;
        }
        else
        {
            controls->direction &= ~River2D_DIR_LEFT;
        }
    }
    else if(key == River2D_RIGHT)
    {
        if(isDown)
        {
            controls->direction |= River2D_DIR_RIGHT;
        }
        else
        {
            controls->direction &= ~River2D_DIR_RIGHT;
        }
    }

    //TODO: tab and other hotkeys
}
