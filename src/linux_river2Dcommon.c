#include "river2D_main.h"

#define __USE_POSIX199309
#include <time.h>

#include <sys/stat.h>

void river2D_queryTime
(
    River2D_Time *time
){
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);

    time->s  = (uint64_t)spec.tv_sec;
    time->ns = (uint64_t)spec.tv_nsec;
}

uint8_t river2D_verifyPath
(
    const char *path
){
    struct stat pathInfo;

    if(stat(path, &pathInfo))
    {
        return RIVER2D_TYPE_ERROR;
    }

    if(S_ISDIR(pathInfo.st_mode))
    {
        return RIVER2D_TYPE_DIRECTORY;
    }

    if(S_ISREG(pathInfo.st_mode))
    {
        return RIVER2D_TYPE_FILE;
    }

    return RIVER2D_TYPE_OTHER;
}

uint8_t river2D_interpretCharAsKey
(
    char inp
){
    const uint8_t alphabetic_table[26] =
    {
        38, 56, 54, 40, 26, 41, 42, 43, 31, 44,
        45, 46, 58, 57, 32, 33, 24, 27, 39, 28,
        30, 55, 25, 53, 29, 52,
    };

    if(inp < 123 && inp > 96)
    {
        return alphabetic_table[inp - 97];
    }
    return 0;
}
