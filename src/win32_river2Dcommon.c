#include "river2D_main.h"
#include "imgsurf_main.h"

#include <sys/stat.h>
#include <stdio.h>

f_internal void resolveFunction
(
    void       **fptr,
    HMODULE    renderer,
    const char *name
){
    *fptr = GetProcAddress(renderer, name);
    if(!(*fptr))
    {
        fprintf(stderr, "\033[31;1;7mERROR: Unable to load symbol %s!\033[0m\n", name);
    }
    #ifdef DEBUG
    else
    {
        fprintf(stderr, "Loaded symbol: %s at 0x%x\n", name, *fptr);
    }
    #endif
}

void river2D_resolveRenderer
(
    EngineData *engine,
    StringView libpath,
    uint8_t    renderer
){
    SetDllDirectoryA(libpath);

    if(renderer == RIVER2D_RENDERER_SOFTWARE)
    {
        HMODULE software = LoadLibraryA("river2Dsoftware.dll");
        if(!software)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: Unable to load software renderer!\n");
            fprintf(stderr, "Tried to load from library path:%s ", libpath);
            fprintf(stderr, "\033[0m\n");
            return;
        }

        resolveFunction((void**)&engine->init,           software, "init");
        resolveFunction((void**)&engine->shutdown,       software, "shutdown");
        resolveFunction((void**)&engine->loadText,       software, "loadText");
        resolveFunction((void**)&engine->bltBuffer,      software, "bltBuffer");
        resolveFunction((void**)&engine->compositeImage, software, "compositeImage");
    }
    else if(renderer == RIVER2D_RENDERER_OPENGL)
    {
        fprintf(stderr, "\033[33m\nWARNING: OpenGL renderer not built yet for river2D.\033[0m");
    }
    else if(renderer == RIVER2D_RENDERER_VULKAN)
    {
        fprintf(stderr, "\033[33m\nWARNING: Vulkan renderer not built yet for river2D.\033[0m");
    }
    else if(renderer == RIVER2D_RENDERER_DIRECTX)
    {
        fprintf(stderr, "\033[33m\nWARNING: DirectX renderer not built yet for river2D.\033[0m");
    }
    else
    {
        fprintf(stderr, "\033[31m\nERROR: invalid renderer specified in river2D_resolveRenderer.\033[0m");
    }
}

f_internal void writeMissingTexture
(
    River2D_Image *image
){
    for(uint32_t y = 0; y < image->height; ++y)
    {
        for(uint32_t x = 0; x < image->width; ++x)
        {
            ((uint32_t*)image->data)[x * y * RIVER2D_BPP] = 0xC64FACFF;
        }
    }
}

void river2D_loadImage_file
(
    EngineData    *engine,
    char          *path,
    River2D_Image *image,
    uint8_t       format,
    uint8_t       bitdepth
){
    (void)engine;
    image->data = imgsurf_load_file(path, &image->width, &image->height, format, bitdepth);

    if(!image->data)
    {
        fprintf(stderr, "Failed to load image from file: %s\n", path);
        writeMissingTexture(image);
    }
}

void river2D_loadImage_ptr
(
    EngineData    *engine,
    void          *file,
    River2D_Image *image,
    uint8_t       channels,
    uint8_t       bitdepth
){
    image->data = imgsurf_load_ptr(file, IMGSURF_FILE_QOI, &image->width, &image->height, channels, bitdepth);
    image->path = puddle_cstr_sv("river2D_loadImage_ptr");
}

void river2D_createImage
(
    EngineData    *engine,
    River2D_Image *image,
    uint32_t      width,
    uint32_t      height
){
    image->path   = puddle_cstr_sv("river2D_createImage");
    image->data   = calloc(width * height * RIVER2D_BPP, 1);
    image->width  = width;
    image->height = height;

    image->info.bmiHeader.biSize        = sizeof(image->info.bmiHeader);
    image->info.bmiHeader.biWidth       =  (long)width;
    image->info.bmiHeader.biHeight      = -(long)height;
    image->info.bmiHeader.biPlanes      = 1;
    image->info.bmiHeader.biBitCount    = 32;
    image->info.bmiHeader.biCompression = BI_RGB;
}

River2D_Time river2D_queryTime
(
    void
){
    static LARGE_INTEGER freq;
    static int initialized = 0;

    if (!initialized)
    {
        QueryPerformanceFrequency(&freq);
        initialized = 1;
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    uint64_t seconds = counter.QuadPart / freq.QuadPart;
    uint64_t remainder = counter.QuadPart % freq.QuadPart;

    uint64_t nanoseconds = (remainder * 1000000000ULL) / freq.QuadPart;

    River2D_Time time;
    time.s  = seconds;
    time.ns = nanoseconds;

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

uint8_t river2D_charToKey
(
    char inp
){
    if(inp >= RIVER2D_ASCII_A && inp <= RIVER2D_ASCII_Z)
    {
        return inp - 0x20;
    }

    if(inp == RIVER2D_ASCII_LSHIFT)
    {
        return 0x10;
    }
    else if(inp == RIVER2D_ASCII_MINUS)
    {
        return 0xC0;
    }
    else if(inp == RIVER2D_ASCII_EQUALS)
    {
        return 0xBB;
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

void river2D_changeCursor
(
    EngineData    *engine,
    River2D_Image *image
){
    if(engine->currentCursor == image)
    {
        return;
    }

    DestroyCursor(engine->hCursor);
    DeleteObject(engine->cursorBitmap);
    DeleteObject(engine->cursorMask);

    BITMAPINFO bmi              = {0};
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = image->width;
    bmi.bmiHeader.biHeight      = -((LONG)image->height);
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biSizeImage   = 0;
    bmi.bmiHeader.biCompression = BI_RGB;

    uint32_t *data = 0;
    engine->cursorBitmap = CreateDIBSection(0, &bmi, DIB_RGB_COLORS, (void**)&data, 0, 0);

    bool null_cursor = true;

    for(uint32_t i = 0; i < image->height * image->width; ++i)
    {
        data[i] = ((uint32_t*)image->data)[i];
        if(data[i] & 0x000000FF)
        {
            null_cursor = false;
        }
    }

    engine->cursorMask = CreateBitmap(image->width, image->height, 1, 1, 0);

    ICONINFO iconInfo = {0};
    iconInfo.hbmColor = engine->cursorBitmap;
    iconInfo.hbmMask  = engine->cursorMask;

    engine->hCursor       = CreateIconIndirect(&iconInfo);
    engine->currentCursor = image;

    if(null_cursor)
    {
        SetCursor(0);
        return;
    }

    SetCursor(engine->hCursor);
}
