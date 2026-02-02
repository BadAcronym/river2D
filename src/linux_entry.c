#include "river2D_main.h"
#include "linux_platform.h"

#include <stdio.h>

int main()
{
    Dimensions dimensions = {0};
    dimensions.width = EDITOR_WIDTH;
    dimensions.height = EDITOR_HEIGHT;

    Display *display = XOpenDisplay(NULL);

    Window window = X11openWindow(display, dimensions, "river2D Editor");

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

        //TODO: update & editor work
    }

    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
