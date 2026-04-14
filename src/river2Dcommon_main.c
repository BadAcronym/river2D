#include "river2D_main.h"
#include <stdlib.h>

#define BILLION 1000000000

internal void calcDelta
(
    const River2D_Time *time1,
    const River2D_Time *time2,
    int64_t      *deltaS,
    int64_t      *deltaNS
){
    if(time1->ns > BILLION)
    {
        fprintf(stderr, "\033[31mERROR: timestamp 0 is malformed.\033[0m\n");
    }
    if(time2->ns > BILLION)
    {
        fprintf(stderr, "\033[31mERROR: timestamp 1 is malformed.\033[0m\n");
    }

    *deltaS  = time2->s  - time1->s;
    *deltaNS = time2->ns - time1->ns;

    if(*deltaNS < 0)
    {
        *deltaS  -= 1;
        *deltaNS += BILLION;
    }
}

River2D_Time river2D_deltaTime
(
    const River2D_Time *time1,
    const River2D_Time *time2
){
    if(!time1 || !time2)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        River2D_Time time = {0, 0};
        return time;
    }

    int64_t deltaS  = 0;
    int64_t deltaNS = 0;
    calcDelta(time1, time2, &deltaS, &deltaNS);

    River2D_Time delta = {deltaS, deltaNS};
    return delta;
}

float river2D_deltaTime_ms
(
    const River2D_Time *time1,
    const River2D_Time *time2
){
    if(!time1 || !time2)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        return -1;
    }

    int64_t deltaS  = 0;
    int64_t deltaNS = 0;
    calcDelta(time1, time2, &deltaS, &deltaNS);

    return (float)((float)deltaS * 1e3f + (float)deltaNS / 1e6f);
}

extern int64_t river2D_deltaTime_ns
(
    const River2D_Time *time1,
    const River2D_Time *time2
){
    if(!time1 || !time2)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        return -1;
    }

    int64_t deltaS  = 0;
    int64_t deltaNS = 0;
    calcDelta(time1, time2, &deltaS, &deltaNS);

    return (deltaS * 1e9f + deltaNS);
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
        fprintf(stderr, "\033[31mERROR: timestamp lies in the future.\033[0m\n");
        River2D_Time time = {0, 0};
        return time;
    }

    calcDelta(time, &current, &deltaS, &deltaNS);

    River2D_Time delta = {deltaS, deltaNS};
    return delta;
}

float river2D_deltaTime_now_ms
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
        fprintf(stderr, "\033[31mERROR: timestamp lies in the future.\033[0m\n");
        return -2;
    }

    calcDelta(time, &current, &deltaS, &deltaNS);

    return (float)((float)deltaS * 1e3f + (float)deltaNS / 1e6f);
}

extern uint64_t river2D_deltaTime_now_ns
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
        fprintf(stderr, "\033[31mERROR: timestamp lies in the future.\033[0m\n");
        return -2;
    }

    calcDelta(time, &current, &deltaS, &deltaNS);

    return (deltaS * 1e9f + deltaNS);
}

void river2D_destroyImage
(
    River2D_Image *image
){
    if(!image)
    {
        fprintf(stderr, "No image to be freed.\n");
        return;
    }

    if(image->data)
    {
        free(image->data);
        image->data = 0;
    }
}
