#include "river2D_main.h"
#include "river2Dmapedit_main.h"

#include <dlfcn.h>
#include <stdio.h>

//TODO: read tile size, tiles and animations, save them to some format which can then be read by the game

#ifdef DEBUG
    #define LIBPATH "./bin/river2Dsoftware_linux/debug/"
#else
    #define LIBPATH "./bin/river2Dsoftware_linux/release/"
#endif

int main()
{
    char *error = 0;
    void *software = dlopen(LIBPATH "libriver2Dsoftware.so", RTLD_NOW);
    if(!software)
    {
        fprintf(stderr, "\033[31;1;7mERROR: Software renderer could not be loaded.\n");
        fputs(dlerror(), stderr);
        fprintf(stderr, "\033[0m\n");
        return -1;
    }

    void (*river2D_init)(EngineData *engine, River2D_Image *planes);
    river2D_init = dlsym(software, "river2D_init");
    if((error = dlerror()))
    {
        fprintf(stderr, "\033[31;1;7mERROR: Error while loading symbol river2D_init.\n");
        fputs(error, stderr);
        fprintf(stderr, "\033[0m\n");
        return -1;
    }

    int32_t (*river2D_shutdown)(EngineData *engine);
    river2D_shutdown = dlsym(software, "river2D_shutdown");
    if((error = dlerror()))
    {
        fprintf(stderr, "\033[31;1;7mERROR: Error while loading symbol river2D_shutdown.\n");
        fputs(error, stderr);
        fprintf(stderr, "\033[0m\n");
        return -1;
    }

    void (*river2D_bltBuffer)(EngineData *engine);
    river2D_bltBuffer = dlsym(software, "river2D_bltBuffer");
    if((error = dlerror()))
    {
        fprintf(stderr, "\033[31;1;7mERROR: Error while loading symbol river2D_bltBuffer.\n");
        fputs(error, stderr);
        fprintf(stderr, "\033[0m\n");
        return -1;
    }

    void (*river2D_resizeBackbuffer)(EngineData *engine, uint32_t width, uint32_t height);
    river2D_resizeBackbuffer = dlsym(software, "river2D_resizeBackbuffer");
    if((error = dlerror()))
    {
        fprintf(stderr, "\033[31;1;7mERROR: Error while loading symbol river2D_resizeBackbuffer.\n");
        fputs(error, stderr);
        fprintf(stderr, "\033[0m\n");
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
                        river2D_resizeBackbuffer(&engine, (uint32_t)event.xconfigure.width,
                                                          (uint32_t)event.xconfigure.height);
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

    dlclose(software);
    return river2D_shutdown(&engine);
}
