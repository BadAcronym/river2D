#include "river2D_main.h"

River2D_Time river2D_deltaTime
(
    const River2D_Time *smaller,
    const River2D_Time *bigger
){
    if(!smaller || !bigger)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        River2D_Time time = {-1, -1};
        return time;
    }

    int64_t      deltaS  = 0;
    int64_t      deltaNS = 0;

    if(smaller->s > bigger->s || (smaller->ns > bigger->ns && smaller->s == bigger->s))
    {
        fprintf(stderr, "\033[31mERROR: delta time lies in the future.\033[0m\n");
        River2D_Time time = {-2, -2};
        return time;
    }

    deltaS  = bigger->s  - smaller->s;
    deltaNS = bigger->ns - smaller->ns;

    if(deltaNS < 0)
    {
        deltaS  -= 1;
        deltaNS += 1e9L;
    }

    River2D_Time delta = {deltaS, deltaNS};
    return delta;
}

uint64_t river2D_deltaTime_ms
(
    const River2D_Time *smaller,
    const River2D_Time *bigger
){
    if(!smaller || !bigger)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        return -1;
    }

    int64_t      deltaS  = 0;
    int64_t      deltaNS = 0;

    if(smaller->s > bigger->s || (smaller->ns > bigger->ns && smaller->s == bigger->s))
    {
        fprintf(stderr, "\033[31mERROR: delta time lies in the future.\033[0m\n");
        River2D_Time time;
        return -2;
    }

    deltaS  = bigger->s  - smaller->s;
    deltaNS = bigger->ns - smaller->ns;

    if(deltaNS < 0)
    {
        deltaS  -= 1;
        deltaNS += 1e9L;
    }

    return ((double)deltaS * 1e3f + (double)deltaNS / 1e6f);
}

River2D_Time river2D_deltaTime_now
(
    const River2D_Time *time
){
    if(!time || !time->s)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        River2D_Time time = {-1, -1};
        return time;
    }

    River2D_Time current = river2D_queryTime();
    int64_t      deltaS  = 0;
    int64_t      deltaNS = 0;

    if(time->s > current.s || (time->ns > current.ns && time->s == current.s))
    {
        fprintf(stderr, "\033[31mERROR: delta time lies in the future.\033[0m\n");
        River2D_Time time = {-2, -2};
        return time;
    }

    deltaS  = current.s  - time->s;
    deltaNS = current.ns - time->ns;

    if(deltaNS < 0)
    {
        deltaS  -= 1;
        deltaNS += 1e9L;
    }

    River2D_Time delta = {deltaS, deltaNS};
    return delta;
}

uint64_t river2D_deltaTime_now_ms
(
    const River2D_Time *time
){
    if(!time)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        return -1;
    }

    River2D_Time current = river2D_queryTime();
    int64_t      deltaS  = 0;
    int64_t      deltaNS = 0;

    if(time->s > current.s || (time->ns > current.ns && time->s == current.s))
    {
        fprintf(stderr, "\033[31mERROR: delta time lies in the future.\033[0m\n");
        return -2;
    }

    deltaS  = current.s  - time->s;
    deltaNS = current.ns - time->ns;

    if(deltaNS < 0)
    {
        deltaS  -= 1;
        deltaNS += 1e9L;
    }

    return ((double)deltaS * 1e3f + (double)deltaNS / 1e6f);
}
