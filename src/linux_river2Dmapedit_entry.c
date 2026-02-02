#include "river2D_main.h"
#include "river2Dmapedit_main.h"

#include <dlfcn.h>
#include <stdio.h>

// TODO: (river2D #11) read tile size, tiles and animations,
// save them to some format which can then be read by the game

#ifdef DEBUG
    #define LIBPATH "./bin/debug/"
#else
    #define LIBPATH "./bin/release/"
#endif

int main
(
    void
){
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
    river2D_init = (void (*)(EngineData *engine, River2D_Image *planes))
                   dlsym(software, "river2D_init");
    if((error = dlerror()))
    {
        fprintf(stderr, "\033[31;1;7mERROR: Error while loading symbol river2D_init.\n");
        fputs(error, stderr);
        fprintf(stderr, "\033[0m\n");
        return -1;
    }

    int32_t (*river2D_shutdown)(EngineData *engine);
    river2D_shutdown = (int32_t (*)(EngineData *engine))
                       dlsym(software, "river2D_shutdown");
    if((error = dlerror()))
    {
        fprintf(stderr, "\033[31;1;7mERROR: Error while loading symbol river2D_shutdown.\n");
        fputs(error, stderr);
        fprintf(stderr, "\033[0m\n");
        return -1;
    }

    void (*river2D_bltBuffer)(EngineData *engine);
    river2D_bltBuffer = (void (*)(EngineData *engine))
                        dlsym(software, "river2D_bltBuffer");
    if((error = dlerror()))
    {
        fprintf(stderr, "\033[31;1;7mERROR: Error while loading symbol river2D_bltBuffer.\n");
        fputs(error, stderr);
        fprintf(stderr, "\033[0m\n");
        return -1;
    }
    ;
    EngineData    engine = {0};
    River2D_Image planes[RIVER2D_MAX_PLANES] = {0};

    river2D_loadConfig(&engine.config);
    river2D_init(&engine, planes);

    bool running = true;

    Atom WM_DELETE = XInternAtom(engine.display, "WM_DELETE_WINDOW", false);
    XSetWMProtocols(engine.display, engine.window, &WM_DELETE, 1);

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
                case ClientMessage:
                {
                    if(event.xclient.data.l[0] == (long)WM_DELETE)
                    {
                        running = false;
                    }
                    break;
                }
                case Expose:
                {
                    mapedit_update();
                    river2D_bltBuffer(&engine);
                    break;
                }
                case GraphicsExpose:
                {
                    mapedit_update();
                    river2D_bltBuffer(&engine);
                    break;
                }
                case ConfigureNotify:
                {
                    uint32_t newWidth  = event.xconfigure.width;
                    uint32_t newHeight = event.xconfigure.height;

                    if(newWidth != engine.config.window_width || newHeight != engine.config.window_height)
                    {
                        engine.config.window_width  = newWidth;
                        engine.config.window_height = newHeight;
                    }
                    break;
                }
                // TODO: (river2D #12) handle ColormapNotify?
            }
        }
        mapedit_update();
        river2D_bltBuffer(&engine);
    }

    return river2D_shutdown(&engine);
}
