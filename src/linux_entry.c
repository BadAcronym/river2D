#include "river2D_main.h"
#include "linux_platform.h"

#include <stdio.h>

int main()
{
    EngineData engine = {};

    initRiver2D(&engine);

    while(engine.running)
    {
        while(XPending(engine.display) > 0)
        {
            XEvent event = {};
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
                    river2D_resizeBackbuffer(&engine, event.xconfigure.width, event.xconfigure.height);
                    break;
                }
                case ClientMessage:
                {
                    engine.running = false;
                    break;
                }
            }
        }
        river2D_updateEditor();
        river2D_drawFrame(&engine);
    }

    return shutdownRiver2D(&engine);
}
