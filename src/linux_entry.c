#include "river2D_main.h"
#include "linux_platform.h"

#include <stdio.h>

int main()
{
    Display *display = XOpenDisplay(0);
    if(!display)
    {
        printf("%s", "Failed to open default Display!");
    }

    Screen *screen = DefaultScreenOfDisplay(display);

    Dimensions dimensions = {};
    dimensions.width = WidthOfScreen(screen);
    dimensions.height = HeightOfScreen(screen);

    Window window = X11openWindow(display, dimensions, "river2D Editor");

    X11allocateBackbuffer(dimensions);

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
                case ClientMessage:
                {
                    running = false;
                    break;
                }
            }
        }

        //work
    }

    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
