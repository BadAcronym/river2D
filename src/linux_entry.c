#include "river2D_main.h"
#include "linux_platform.h"

#include <stdio.h>

int main()
{
    Display *display = XOpenDisplay(0);
    if(!display)
    {
        fprintf(stderr, "Failed to open default Display!\n");
        return -1;
    }

    Screen *screen = DefaultScreenOfDisplay(display);
    if(!screen)
    {
        fprintf(stderr, "Failed to get default Display!\n");
        return -2;
    }

    Dimensions dimensions = {};
    dimensions.width = WidthOfScreen(screen);
    dimensions.height = HeightOfScreen(screen);

    Window window = river2D_openWindow(display, dimensions, "river2D Editor");

    if(!window)
    {
        fprintf(stderr, "Failed to create window!\n");
        return -3;
    }

    GC gc = XCreateGC(display, window, 0, 0);
    if(!gc)
    {
        fprintf(stderr, "Failed to create Graphics Context!\n");
        return -4;
    }

    Backbuffer buf = {};
    river2D_resizeBackbuffer(&buf, dimensions);

    River2DControlMap controls = {};

    bool running = true;
    while(running)
    {
        while(XPending(display) > 0)
        {
            XEvent event = {};
            XNextEvent(display, &event);
            switch(event.type)
            {
                case KeyPress:
                {
                    river2D_processControls(true, event.xkey.keycode, &controls);
                    break;
                }
                case KeyRelease:
                {
                    river2D_processControls(false, event.xkey.keycode, &controls);
                    break;
                }
                case ConfigureNotify:
                {
                    dimensions.width  = event.xconfigure.width;
                    dimensions.height = event.xconfigure.height;
                    river2D_resizeBackbuffer(&buf, dimensions);
                    break;
                }
                case ClientMessage:
                {
                    running = false;
                    break;
                }
            }
        }
        river2D_updateEditor();
        river2D_drawFrame(display, dimensions, window, gc);
    }

    return river2D_shutdown(display, window, gc);
}
