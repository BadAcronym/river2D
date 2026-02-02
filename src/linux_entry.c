#include "river2D_main.h"
#include "linux_platform.h"

#include <stdio.h>

int main()
{
    Dimensions dimensions = {0};
    dimensions.width = 640;
    dimensions.height = 360;

    Display *display = XOpenDisplay(NULL);

    Window window = X11openWindow(display, dimensions, "river2D");

    X11allocateBackbuffer(dimensions);

    bool running = true;
    while(running)
    {
        while(XPending(display) > 0)
        {
            XEvent event = {0};
            XNextEvent(display, &event);
            if(event.type == KeyPress)
            {
                //TODO: process keymap
                printf("%s", "KeyPress\n");
            }
            else if(event.type == KeyRelease)
            {
                //TODO: process keymap
                printf("%s", "KeyRelease\n");
            }
            else if(event.type == ClientMessage)
            {
                running = false;
                break;
            }
        }

        //TODO: work!
    }

    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
