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

uint8_t river2D_interpretCharAsKey
(
    char inp
){
    if(inp > 0x60 && inp < 0x7A)
    {
        return inp - 32;
    }

    return inp;
}

Dimensions river2D_getWindowSize
(
    EngineData *engine
){
    Dimensions dim  = {0};
    RECT       rect = {0};

    GetWindowRect(engine->window, &rect);

    dim.width  = rect.right  - rect.left;
    dim.height = rect.bottom - rect.top;

    return dim;
}

// TODO: do hCursor shenanigans
void river2D_changeCursor
(
    EngineData *engine,
    River2D_Image *image
){
}
