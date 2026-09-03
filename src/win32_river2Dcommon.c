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

void rvResolveRenderer
(
    EngineData *engine,
    StringView libpath,
    uint8_t    renderer
){
    char libpath_cstr[libpath.size + 1];
    sv_cstr(libpath, libpath_cstr);

    SetDllDirectoryA(libpath_cstr);

    if(renderer == RV_RENDERER_SOFTWARE)
    {
        HMODULE software = LoadLibraryA("river2Dsoftware.dll");
        if(!software)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: Unable to load software renderer!\n");
            fprintf(stderr, "Tried to load from library path: '"PRI_SV"'",
                    ARG_SV(libpath));
            fprintf(stderr, "\033[0m\n");
            return;
        }

        resolveFunction((void**)&engine->init,           software, "_init");
        resolveFunction((void**)&engine->shutdown,       software, "_shutdown");
        resolveFunction((void**)&engine->loadText,       software, "_loadText");
        resolveFunction((void**)&engine->bltBuffer,      software, "_bltBuffer");
        resolveFunction((void**)&engine->compositeImage, software, "_compositeImage");
    }
    else if(renderer == RV_RENDERER_OPENGL)
    {
        fprintf(stderr, "\033[33m\nWARNING: OpenGL renderer not built yet for "
                "river2D.\033[0m");
    }
    else if(renderer == RV_RENDERER_VULKAN)
    {
        fprintf(stderr, "\033[33m\nWARNING: Vulkan renderer not built yet for "
                "river2D.\033[0m");
    }
    else if(renderer == RV_RENDERER_DIRECTX)
    {
        fprintf(stderr, "\033[33m\nWARNING: DirectX renderer not built yet for "
                "river2D.\033[0m");
    }
    else
    {
        fprintf(stderr, "\033[31m\nERROR: invalid renderer specified in "
                "rvResolveRenderer.\033[0m");
    }
}

f_internal void writeMissingTexture
(
    RiverImage *image
){
    uint64_t imgsize = image->width * image->height;

    for(uint64_t i = 0; i < imgsize; ++i)
    {
        ((uint32_t*)image->data)[i] = 0xC64FACFF;
    }
}

void rvLoadImage_file
(
    EngineData *engine,
    StringView path,
    RiverImage *image,
    uint8_t    format,
    uint8_t    bitdepth
){
    (void)engine;
    char path_buf[path.size + 1];
    sv_cstr(path, path_buf);

    image->data = imLoadFile(path_buf, &image->width, &image->height, format, bitdepth);

    if(!image->data)
    {
        fprintf(stderr, "Failed to load image from file: '%s'\n", path_buf);
        writeMissingTexture(image);
    }
}

void rvLoadImage_ptr
(
    EngineData *engine,
    void       *file,
    RiverImage *image,
    uint8_t    channels,
    uint8_t    bitdepth
){
    image->data = imLoadPtr(file, IM_FILE_QOI, &image->width, &image->height,
                            channels, bitdepth);
    if(!image->data)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to load image to pointer.\n\033[0m");
    }

    image->path = cstr_sv("rvLoadImage_ptr");
}

void rvCreateImage
(
    EngineData *engine,
    RiverImage *image,
    uint32_t   width,
    uint32_t   height
){
    image->path   = cstr_sv("rvCreateImage");
    image->data   = calloc(width * height * RV_BPP, 1);
    image->width  = width;
    image->height = height;

    image->info.bmiHeader.biSize        = sizeof(image->info.bmiHeader);
    image->info.bmiHeader.biWidth       =  (long)width;
    image->info.bmiHeader.biHeight      = -(long)height;
    image->info.bmiHeader.biPlanes      = 1;
    image->info.bmiHeader.biBitCount    = 32;
    image->info.bmiHeader.biCompression = BI_RGB;
}

RiverTime rvQueryTime
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

    RiverTime time;
    time.s  = seconds;
    time.ns = nanoseconds;

    return time;
}

uint8_t rvCharToKey
(
    char inp
){
    if(inp >= 'a' && inp <= 'z')
    {
        return inp - 0x20;
    }

    if(inp == RV_ASCII_LSHIFT)
    {
        return 0x10;
    }
    else if(inp == '-')
    {
        return 0xC0;
    }
    else if(inp == '=')
    {
        return 0xBB;
    }

    return inp;
}

Dimensions rvGetWindowSize
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

void rvChangeCursor
(
    EngineData *engine,
    RiverImage *image
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

// to simplify access to renderer-agnostic function calls:
void rvInit
(
    EngineData *engine,
    RiverImage *planes
){
    engine->init(engine, planes);
}

int32_t rvShutdown
(
    EngineData *engine
){
    return engine->shutdown(engine);
}

void rvBltBuffer
(
    EngineData *engine
){
    engine->bltBuffer(engine);
}

void rvLoadText
(
    EngineData         *engine,
    rvLoadTextSettings *settings
){
    engine->loadText(engine,             settings->image,
                     settings->sv,       settings->font,
                     settings->charsize, settings->spacing,
                     settings->offsetX,  settings->offsetY);
}

void rvCompositeImage
(
    EngineData          *engine,
    rvCompositeSettings *settings
){
    engine->compositeImage(engine,               settings->src,
                           settings->dst,        settings->pictop,
                           settings->offsetSrcX, settings->offsetSrcY,
                           settings->offsetDstX, settings->offsetDstY,
                           settings->cropWidth,  settings->cropHeight);
}

void rvSyncImage
(
    EngineData *engine,
    RiverImage *image,
    bool       CPU_to_GPU
){
    return;
}
