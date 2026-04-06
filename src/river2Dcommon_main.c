#include "river2D_main.h"

River2D_Time river2D_deltaTime
(
    const River2D_Time *smaller,
    const River2D_Time *bigger
){
    if(!smaller || !smaller->s || !bigger || !bigger->s)
    {
        fprintf(stderr, "\033[33mWARNING: passed uninitialized timestamp.\033[0m\n");
    }

    int64_t      deltaS  = 0;
    int64_t      deltaNS = 0;

    if(smaller->s > bigger->s || (smaller->ns > bigger->ns && smaller->s == bigger->s))
    {
        fprintf(stderr, "\033[33mWARNING: delta time lies in the future.\033[0m\n");
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
    if(!smaller || !smaller->s || !bigger || !bigger->s)
    {
        fprintf(stderr, "\033[33mWARNING: passed uninitialized timestamp.\033[0m\n");
    }

    int64_t      deltaS  = 0;
    int64_t      deltaNS = 0;

    if(smaller->s > bigger->s || (smaller->ns > bigger->ns && smaller->s == bigger->s))
    {
        fprintf(stderr, "\033[33mWARNING: delta time lies in the future.\033[0m\n");
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
        fprintf(stderr, "\033[33mWARNING: passed uninitialized timestamp.\033[0m\n");
    }

    River2D_Time current = river2D_queryTime();
    int64_t      deltaS  = 0;
    int64_t      deltaNS = 0;

    if(time->s > current.s || (time->ns > current.ns && time->s == current.s))
    {
        fprintf(stderr, "\033[33mWARNING: delta time lies in the future.\033[0m\n");
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
    if(!time || !time->s)
    {
        fprintf(stderr, "\033[33mWARNING: passed uninitialized timestamp.\033[0m\n");
    }

    River2D_Time current = river2D_queryTime();
    int64_t      deltaS  = 0;
    int64_t      deltaNS = 0;

    if(time->s > current.s || (time->ns > current.ns && time->s == current.s))
    {
        fprintf(stderr, "\033[33mWARNING: delta time lies in the future.\033[0m\n");
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
