#include "river2D_main.h"
#include "river2Dmapedit_main.h"
#include "linux_river2Dsoftware_platform.h"

#include <dlfcn.h>
#include <stdio.h>

//TODO: read tile size, tiles and animations, save them to some format which can then be read by the game

//TODAY: define stubs for loading renderer dlls later

clang_ignore_unused

#define RIVER2D_INIT(name) void name(EngineData *engine, River2D_Image *planes)
typedef RIVER2D_INIT(river2D_initialize);
RIVER2D_INIT(River2D_Init_Stub)
{
    return;
}
global river2D_initialize *river2D_init_ = River2D_Init_Stub;
#define river2D_init river2D_init_

#define RIVER2D_SHUT(name) int32_t name(EngineData *engine)
typedef RIVER2D_SHUT(river2D_shut);
RIVER2D_SHUT(River2D_Shutdown_Stub)
{
    return -1;
}
global river2D_shut *river2D_shut_ = River2D_Shutdown_Stub;
#define river2D_shutdown river2D_shut_

#define RIVER2D_BLT(name) void name(EngineData *engine)
typedef RIVER2D_BLT(river2D_blt);
RIVER2D_BLT(River2D_bltBuffer_Stub)
{
    return;
}
global river2D_blt *river2D_blt_ = River2D_bltBuffer_Stub;
#define river2D_bltBuffer river2D_blt_

#define RIVER2D_RESIZE(name) void name(EngineData *engine, uint32_t width, uint32_t height)
typedef RIVER2D_RESIZE(river2D_resize);
RIVER2D_RESIZE(River2D_Resize_Stub)
{
    return;
}
global river2D_resize *river2D_resize_ = River2D_Resize_Stub;
#define river2D_resizeBackbuffer river2D_resize_

clang_diagnostic_pop

int main()
{
    void *software = dlopen("libriver2Dsoftware.so", RTLD_NOW);
    if(!software)
    {
        fprintf(stderr, "\x1b[1;31mERROR: Software renderer could be found.\033[0m\n");
        return -1;
    }

    EngineData    engine = {0};
    River2D_Image planes[RIVER2D_MAX_PLANES] = {0};

    river2D_init(&engine, planes);

    bool running = true;

    while(running)
    {
        while(XPending(engine.display) > 0)
        {
            XEvent event = {0};
            XNextEvent(engine.display, &event);
            switch(event.type)
            {
                case KeyPress:
                {
                    mapedit_processControls(true, event.xkey.keycode, &engine.controls);
                    break;
                }
                case KeyRelease:
                {
                    mapedit_processControls(false, event.xkey.keycode, &engine.controls);
                    break;
                }
                case ConfigureNotify:
                {
                    if(!(engine.config.choices & RIVER2D_CHOICE_STATIC_CANVAS_BIT))
                    {
                        river2D_resizeBackbuffer(&engine, event.xconfigure.width, event.xconfigure.height);
                    }
                    break;
                }
                case ClientMessage:
                {
                    running = false;
                    break;
                }
                //TODO: handle ColormapNotify?
            }
        }
        mapedit_update();
        river2D_bltBuffer(&engine);
    }

    return river2D_shutdown(&engine);
}
