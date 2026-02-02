#include "river2D_main.h"

#include <sys/stat.h>

void river2D_queryTime
(
    River2D_Time *time
){
    LARGE_INTEGER t1;
    LARGE_INTEGER freq;

    QueryPerformanceCounter(&t1);
    QueryPerformanceFrequency(&freq);

    time->s  = (uint64_t)(t1.QuadPart * 1000 / freq.QuadPart);
    time->ns = (uint64_t)(t1.QuadPart * 1000000000000 / freq.QuadPart);
}

uint8_t river2D_verifyPath
(
    const char *path
){
    struct _stat pathInfo;

    if(_stat(path, &pathInfo))
    {
        return RIVER2D_TYPE_ERROR;
    }

    if(_S_IFDIR & pathInfo.st_mode)
    {
        return RIVER2D_TYPE_DIRECTORY;
    }

    if(_S_IFREG & pathInfo.st_mode)
    {
        return RIVER2D_TYPE_FILE;
    }

    return RIVER2D_TYPE_OTHER;
}
