#include "river2D_main.h"

#include <sys/stat.h>

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
