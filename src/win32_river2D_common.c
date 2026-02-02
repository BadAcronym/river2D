#include "river2D_main.h"

#include <sys/stat.h>

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

//TEST: verify if time outputs correctly
void river2D_queryTime
(
    River2D_Time *time
){
    LARGE_INTEGER t1;

    KeQuerySystemTimePrecise(&time);

    time->s = t1.QuadPart / 10000000;
    time->ns = t1.QuadPart * 100;
}
