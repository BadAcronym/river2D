#include "river2D_main.h"

#include <sys/stat.h>
#include <stdio.h>

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

const char* river2D_listFiles
(
    const char *path
){
    WIN32_FIND_DATAA fileData;
    HANDLE           foundHandle;

    uint16_t stringLength = strlen(path);
    char *winPath         = malloc(stringLength + 3);
    for(uint16_t i = 0; i < stringLength; ++i)
    {
        winPath[i] = path[i];
    }
    winPath[stringLength]     = '\\';
    winPath[stringLength + 1] = '*';
    winPath[stringLength + 2] = '\0';

    foundHandle = FindFirstFileA(winPath, &fileData);
    if(foundHandle == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: failed to open cwd.\033[0m\n");
        return 0;
    }

    uint32_t listSize = 0;

    while(FindNextFileA(foundHandle, &fileData))
    {
        uint8_t length = 0;
        for(; length < 255 && fileData.cFileName[length] != '\0'; ++length)
        {
        }
        listSize += length + 1;
    }

    FindClose(foundHandle);
    foundHandle = FindFirstFileA(winPath, &fileData);

    char     *list  = (char*)malloc(listSize + 1);
    uint32_t offset = 0;

    while(FindNextFileA(foundHandle, &fileData))
    {
        uint8_t length = 0;
        for(; length < 255 && fileData.cFileName[length] != '\0'; ++length)
        {
            list[offset + length] = fileData.cFileName[length];
        }
        list[offset + length] = ';';
        offset += length + 1;
    }

    list[offset] = '\0';

    FindClose(foundHandle);
    return list;
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
