#include "river2D_main.h"
#include "imgsurf_main.h"

#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>

internal void resolveFunction
(
    void       **fptr,
    void       *renderer,
    const char *name,
    char       **error
){
    *fptr = dlsym(renderer, name);
    if((*error = dlerror()))
    {
        fprintf(stderr, "\033[31;1;7mERROR: Error while loading symbol %s.\n", name);
        fputs(*error, stderr);
        fprintf(stderr, "\033[0m\n");
    }

    #ifdef DEBUG
    fprintf(stderr, "Loaded symbol: %s\n", name);
    #endif
}

void river2D_resolveRenderer
(
    EngineData *engine,
    const char *libpath,
    uint8_t    renderer
){
    if(renderer == RIVER2D_RENDERER_SOFTWARE)
    {
        char so[256] = {'\0'};
        sprintf(so, "%s/libriver2Dsoftware.so", libpath);

        char *error = 0;
        void *software = dlopen(so, RTLD_NOW);
        if(!software)
        {
            fprintf(stderr, "\033[31;1;7mERROR: Software renderer could not be loaded from specified folder: %s\n", libpath);
            fputs(dlerror(), stderr);
            fprintf(stderr, "\033[0m\n");
        }

        resolveFunction((void**)&engine->init,           software, "river2D_init",           &error);
        resolveFunction((void**)&engine->shutdown,       software, "river2D_shutdown",       &error);
        resolveFunction((void**)&engine->loadText,       software, "river2D_loadText",       &error);
        resolveFunction((void**)&engine->bltBuffer,      software, "river2D_bltBuffer",      &error);
        resolveFunction((void**)&engine->compositeImage, software, "river2D_compositeImage", &error);
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

internal void writeMissingTexture
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
    uint8_t       channels,
    uint8_t       bitdepth
){
    image->data = imgsurf_load_file(path, &image->width, &image->height, channels, bitdepth);
    image->path = path;

    if(!image->data)
    {
        fprintf(stderr, "Failed to load image from file: %s\n", path);
        writeMissingTexture(image);
    }

    image->pixmap = XCreatePixmap(engine->display, XDefaultRootWindow(engine->display), image->width, image->height, 32);
    XImage *img   = XCreateImage(engine->display, engine->visual, 32, ZPixmap, 0, (char*)image->data, image->width, image->height, 32, 0);
    if(!img)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XImage from file: %s!.\n\033[0m", path);
    }

    XPutImage(engine->display, image->pixmap, engine->context, img, 0, 0, 0, 0, image->width, image->height);

    img->data = NULL;
    XDestroyImage(img);

    image->picture = XRenderCreatePicture(engine->display, image->pixmap, engine->format, 0, 0);
    if(!image->picture)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XRenderPicture from file: %s!.\n\033[0m", path);
    }
}

// TODO: error checking
void river2D_loadImage_ptr
(
    EngineData    *engine,
    FILE          *file,
    River2D_Image *image,
    uint8_t       channels,
    uint8_t       bitdepth
){
    image->data = imgsurf_load_ptr(file, IMGSURF_FILE_QOI, &image->width, &image->height, channels, bitdepth);
    image->path = "river2D_loadImage_ptr";

    image->pixmap = XCreatePixmap(engine->display, XDefaultRootWindow(engine->display), image->width, image->height, 32);
    XImage *img   = XCreateImage(engine->display, engine->visual, 32, ZPixmap, 0, (char*)image->data, image->width, image->height, 32, 0);
    if(!img)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XImage from pointer.\n\033[0m");
    }

    XPutImage(engine->display, image->pixmap, engine->context, img, 0, 0, 0, 0, image->width, image->height);

    img->data = NULL;
    XDestroyImage(img);

    image->picture = XRenderCreatePicture(engine->display, image->pixmap, engine->format, 0, 0);
    if(!image->picture)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XRenderPicture from pointer.\n\033[0m");
    }
}

void river2D_createImage
(
    EngineData    *engine,
    River2D_Image *image,
    uint32_t      width,
    uint32_t      height
){
    image->path   = "river2D_createImage";
    image->data   = calloc(width * height * RIVER2D_BPP, 1);
    image->width  = width;
    image->height = height;

    image->pixmap = XCreatePixmap(engine->display, XDefaultRootWindow(engine->display), image->width, image->height, 32);
    XImage *img   = XCreateImage(engine->display, engine->visual, 32, ZPixmap, 0, (char*)image->data, image->width, image->height, 32, 0);

    // I don't think we need this here, but still I'm gonna leave it for posterity's sake, maybe it fixes a bug down the line?
    // XPutImage(engine->display, image->pixmap, engine->context, img, 0, 0, 0, 0, image->width, image->height);

    img->data = NULL;
    XDestroyImage(img);

    image->picture = XRenderCreatePicture(engine->display, image->pixmap, engine->format, 0, 0);
    if(!image->picture)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XRenderPicture.\n\033[0m");
    }
}

void river2D_refreshImage
(
    EngineData    *engine,
    River2D_Image *image
){
    XImage *img = XCreateImage(engine->display, engine->visual, 32, ZPixmap, 0, (char*)image->data, image->width, image->height, 32, 0);
    XPutImage(engine->display, image->pixmap, engine->context, img, 0, 0, 0, 0, image->width, image->height);

    img->data = NULL;
    XDestroyImage(img);
}

void river2D_clearImage
(
    EngineData    *engine,
    River2D_Image *image
){
    for(uint64_t i = 0; i < image->width * image->height; ++i)
    {
        image->data[i] = 0;
    }

    XImage *img = XCreateImage(engine->display, engine->visual, 32, ZPixmap, 0, (char*)image->data, image->width, image->height, 32, 0);
    XPutImage(engine->display, image->pixmap, engine->context, img, 0, 0, 0, 0, image->width, image->height);

    img->data = NULL;
    XDestroyImage(img);
}

void river2D_destroyImage
(
    River2D_Image *image
){
    if(!image)
    {
        fprintf(stderr, "No image to be freed.\n");
        return;
    }

    if(image->data)
    {
        free(image->data);
        image->data = 0;
    }
}

River2D_Time river2D_queryTime
(
    void
){
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);

    River2D_Time time =
    {
        .s  = (uint64_t)spec.tv_sec,
        .ns = (uint64_t)spec.tv_nsec
    };

    return time;
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

const char* river2D_listFiles
(
    const char *path
){
    DIR           *dir;
    struct dirent *ent;
    uint32_t      listSize = 0;

    if((dir = opendir(path)))
    {
        while((ent = readdir(dir)))
        {
            uint8_t length = 0;
            for(; length < 255 && ent->d_name[length] != '\0'; ++length)
            {
            }
            listSize += length + 1;
        }
    }
    else
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: failed to open cwd.\033[0m\n");
        return 0;
    }

    char     *list  = (char*)malloc(listSize + 1);
    uint32_t offset = 0;

    free(dir);
    dir = opendir(path);

    while((ent = readdir(dir)))
    {
        uint8_t length = 0;
        for(; length < 255 && ent->d_name[length] != '\0'; ++length)
        {
            list[offset + length] = ent->d_name[length];
        }
        list[offset + length] = ';';
        offset += length + 1;
    }

    list[offset] = '\0';

    free(dir);
    return list;
}

uint8_t river2D_interpretCharAsKey
(
    char inp
){
    const uint8_t numeric_table[10] =
    {
        0x13, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12
    };

    const uint8_t alphabetic_table[26] =
    {
        38, 56, 54, 40, 26, 41, 42, 43, 31, 44,
        45, 46, 58, 57, 32, 33, 24, 27, 39, 28,
        30, 55, 25, 53, 29, 52,
    };

    if(inp > 0x2F && inp < 0x3A)
    {
        return numeric_table[inp - 0x30];
    }

    if(inp > 0x60 && inp < 0x7A)
    {
        return alphabetic_table[inp - 0x61];
    }

    // BACKLOG: translate not only escape (0x1b) but also the rest of the ascii keyboard codes here
    // maybe just interpret ascii codes in general? might not be best.
    if(inp == 0x1B)
    {
        return 0x09;
    }
    else if(inp == 0x2D)
    {
        return 0x14;
    }
    else if(inp == 0x3D)
    {
        return 0x15;
    }

    return 0;
}

Dimensions river2D_getWindowSize
(
    EngineData *engine
){
    XWindowAttributes attr;
    XGetWindowAttributes(engine->display, engine->window, &attr);

    Dimensions dim = {attr.width, attr.height};

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

    XcursorImage ximg = {0};
    ximg.pixels       = (uint32_t*)image->data;
    ximg.width        = image->width;
    ximg.height       = image->height;

    Cursor cursor = XcursorImageLoadCursor(engine->display, &ximg);

    XDefineCursor(engine->display, engine->window, cursor);
    engine->currentCursor = image;
}
