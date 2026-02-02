#include "river2D_main.h"

River2D_Time river2D_deltaTime
(
    const River2D_Time *time
){
    if(!time || !time->s)
    {
        fprintf(stderr, "\033[33mWARNING: passed uninitialized timestamp.\033[0m\n");
    }

    River2D_Time current = river2D_queryTime();
    River2D_Time delta   = {0};

    delta.s = current.s - time->ns;

    if(current.ns >= time->ns)
    {
        delta.ns = current.ns - time->ns;
    }
    else
    {
        delta.s -= 1;
        delta.ns = (current.ns + 1e9) - time->ns;
    }

    delta.s  = current.s  - time->s;
    delta.ns = current.ns - time->ns;

    return delta;
}
