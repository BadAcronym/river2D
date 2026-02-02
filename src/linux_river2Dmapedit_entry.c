#include "river2D_main.h"
#include "linux_river2Dsoftware_platform.h"

//TODO: future renderers
//river2D_vulkan.dll / libriver2D_vulkan.so
//river2D_d3d.dll    / libriver2D_d3d.so
//river2D_openGL.dll / libriver2D_opengl.so
//
//move the functions to their appropriate paths
//
//TODO: read tile size, tiles and animations, save them to some format which can then be read by the game

int main()
{
    EngineData engine = {0};
    river2D_init(&engine);

    while(engine.running)
    {
        while(XPending(engine.display) > 0)
        {
            XEvent event = {0};
            XNextEvent(engine.display, &event);
            switch(event.type)
            {
                case KeyPress:
                {
                    river2D_processControls(true, event.xkey.keycode, &engine.controls);
                    break;
                }
                case KeyRelease:
                {
                    river2D_processControls(false, event.xkey.keycode, &engine.controls);
                    break;
                }
                case ConfigureNotify:
                {
                    //move to config file that's read on init
                    if(!engine.config.static_canvas_enable)
                    {
                        river2D_resizeBackbuffer(&engine, event.xconfigure.width, event.xconfigure.height);
                    }
                    break;
                }
                case ClientMessage:
                {
                    engine.running = false;
                    break;
                }
                //TODO: handle ColormapNotify?
            }
        }
        river2D_updateEditor();
        river2D_drawFrame(&engine);
    }

    return river2D_shutdown(&engine);
}
