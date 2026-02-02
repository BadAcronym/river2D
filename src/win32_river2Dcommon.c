#include "river2D_main.h"

#include <sys/stat.h>

River2D_Time river2D_queryTime
(
    void
){
    LARGE_INTEGER t1;
    LARGE_INTEGER freq;

    QueryPerformanceCounter(&t1);
    QueryPerformanceFrequency(&freq);

    River2D_Time time;
    time.s  = (uint64_t)(t1.QuadPart / freq.QuadPart);
    time.ns = (uint64_t)(1000000 * t1.QuadPart / freq.QuadPart) / 1000;

    return time;
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

// TODO: fixup windows control interpreting
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

Dimensions river2D_getWindowSize
(
    EngineData *engine
){
    //TODO: query win32 for window size
    Dimensions dim = {0};
    return dim;
}

// TODO: do hCursor shenanigans
void river2D_changeCursor
(
    EngineData *engine,
    River2D_Image *image
){
}
