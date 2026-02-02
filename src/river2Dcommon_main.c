#include "river2D_main.h"

River2D_Time river2D_deltaTime
(
    const River2D_Time *time
){
    River2D_Time current = river2D_queryTime();
    River2D_Time delta = {0};

    if(current.ns < time->ns)
    {
        delta.s = current.s - time->s - 1;
        delta.ns = (1000000000 + current.ns) - time->ns;
    }
    else
    {
        delta.s  = current.s  - time->s;
        delta.ns = current.ns - time->ns;
    }

    return delta;
}
