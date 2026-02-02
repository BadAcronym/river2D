#include "river2D_main.h"

#define __USE_POSIX199309
#include <time.h>

void river2D_queryTime
(
    River2D_Time *time
){
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);

    time->s  = spec.tv_sec;
    time->ns = spec.tv_nsec;
}
