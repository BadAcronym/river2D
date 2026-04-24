#include "river2D_main.h"
#include "imgsurf_main.h"

#define STRING_VIEW_IMPL
#include "string_view.h"

#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>

f_internal void resolveFunction
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
    else
    {
        fprintf(stderr, "Loaded symbol: %s at %p\n", name, *fptr);
    }
    #endif
}

void river2D_resolveRenderer
(
    EngineData *engine,
    StringView libpath,
    uint8_t    renderer
){
    if(renderer == RIVER2D_RENDERER_SOFTWARE)
    {
        char so[256] = {'\0'};
        sprintf(so, PRI_SV"/libriver2Dsoftware.so", ARG_SV(libpath));

        char *error = 0;
        void *software = dlopen(so, RTLD_NOW);
        if(!software)
        {
            fprintf(stderr, "\033[31;1;7mERROR: Software renderer could not be loaded "
                    "from specified folder: "PRI_SV"\n", ARG_SV(libpath));
            fputs(dlerror(), stderr);
            fprintf(stderr, "\033[0m\n");
        }

        resolveFunction((void**)&engine->init,      software, "init",      &error);
        resolveFunction((void**)&engine->shutdown,  software, "shutdown",  &error);
        resolveFunction((void**)&engine->loadText,  software, "loadText",  &error);
        resolveFunction((void**)&engine->bltBuffer, software, "bltBuffer", &error);
        resolveFunction((void**)&engine->compositeImage,
                        software, "compositeImage", &error);
    }
    else if(renderer == RIVER2D_RENDERER_OPENGL)
    {
        fprintf(stderr, "\033[33m\nWARNING: OpenGL renderer not built yet for river2D."
                "\033[0m");
    }
    else if(renderer == RIVER2D_RENDERER_VULKAN)
    {
        fprintf(stderr, "\033[33m\nWARNING: Vulkan renderer not built yet for river2D."
                "\033[0m");
    }
    else if(renderer == RIVER2D_RENDERER_DIRECTX)
    {
        fprintf(stderr, "\033[33m\nWARNING: DirectX renderer not built yet for river2D."
                "\033[0m");
    }
    else
    {
        fprintf(stderr, "\033[31m\nERROR: invalid renderer specified "
                "in river2D_resolveRenderer.\033[0m");
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
    uint8_t       channels,
    uint8_t       bitdepth
){
    image->data = imgsurf_load_file(path, &image->width, &image->height,
                                    channels, bitdepth);
    image->path = path;

    if(!image->data)
    {
        fprintf(stderr, "Failed to load image from file: %s\n", path);
        writeMissingTexture(image);
    }

    image->pixmap = XCreatePixmap(engine->display, XDefaultRootWindow(engine->display),
                                  image->width, image->height, 32);
    XImage *img   = XCreateImage(engine->display, engine->visual, 32, ZPixmap, 0,
                                 (char*)image->data, image->width, image->height,
                                 32, 0);
    if(!img)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XImage from file: %s!."
                "\n\033[0m", path);
    }

    XPutImage(engine->display, image->pixmap, engine->context, img, 0, 0, 0, 0,
              image->width, image->height);

    img->data = NULL;
    XDestroyImage(img);

    image->picture = XRenderCreatePicture(engine->display, image->pixmap,
                                          engine->format, 0, 0);
    if(!image->picture)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XRenderPicture from file: "
                "%s!.\n\033[0m", path);
    }
}

// TODO: error checking
void river2D_loadImage_ptr
(
    EngineData    *engine,
    void          *file,
    River2D_Image *image,
    uint8_t       channels,
    uint8_t       bitdepth
){
    image->data = imgsurf_load_ptr(file, IMGSURF_FILE_QOI,
                                   &image->width, &image->height, channels, bitdepth);
    image->path = "river2D_loadImage_ptr";

    image->pixmap = XCreatePixmap(engine->display, XDefaultRootWindow(engine->display),
                                  image->width, image->height, 32);
    XImage *img   = XCreateImage(engine->display, engine->visual, 32, ZPixmap, 0,
                                 (char*)image->data, image->width, image->height,
                                 32, 0);
    if(!img)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XImage from pointer."
                "\n\033[0m");
    }

    XPutImage(engine->display, image->pixmap, engine->context, img, 0, 0, 0, 0,
              image->width, image->height);

    img->data = NULL;
    XDestroyImage(img);

    image->picture = XRenderCreatePicture(engine->display, image->pixmap,
                                          engine->format, 0, 0);
    if(!image->picture)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XRenderPicture from pointer."
                "\n\033[0m");
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

    image->pixmap = XCreatePixmap(engine->display, XDefaultRootWindow(engine->display),
                                  image->width, image->height, 32);
    XImage *img   = XCreateImage(engine->display, engine->visual, 32, ZPixmap, 0,
                                 (char*)image->data, image->width, image->height,
                                 32, 0);

    // I don't think we need this here, but still I'm gonna leave it for
    // posterity's sake, maybe it fixes a bug down the line?
    // XPutImage(engine->display, image->pixmap, engine->context, img, 0, 0, 0,
    // 0, image->width, image->height);

    img->data = NULL;
    XDestroyImage(img);

    image->picture = XRenderCreatePicture(engine->display, image->pixmap,
                                          engine->format, 0, 0);

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
    XImage *img = XCreateImage(engine->display, engine->visual, 32, ZPixmap, 0,
                               (char*)image->data, image->width, image->height, 32, 0);
    XPutImage(engine->display, image->pixmap, engine->context, img, 0, 0, 0, 0,
              image->width, image->height);

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

    XImage *img = XCreateImage(engine->display, engine->visual, 32, ZPixmap, 0,
                               (char*)image->data, image->width, image->height, 32, 0);
    XPutImage(engine->display, image->pixmap, engine->context, img, 0, 0, 0, 0,
              image->width, image->height);

    img->data = NULL;
    XDestroyImage(img);
}

River2D_Time river2D_queryTime
(
    void
){
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);

    River2D_Time time =
    {
        .s  = (int64_t)spec.tv_sec,
        .ns = (int64_t)spec.tv_nsec
    };

    return time;
}

uint8_t river2D_verifyPath
(
    StringView path
){
    struct stat pathInfo;

    if(stat(puddle_sv_cstr(path), &pathInfo))
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
    StringView path
){
    DIR           *dir;
    struct dirent *ent;
    uint32_t      listSize = 0;

    const char *directory = puddle_sv_cstr(path);

    if((dir = opendir(directory)))
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
    dir = opendir(directory);

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

uint8_t xkeyToAscii
(
    EngineData *engine,
    XEvent     *event
){
    KeySym sym = XkbKeycodeToKeysym(engine->display, (KeyCode)event->xkey.keycode, 0,
                                    event->xkey.state & ShiftMask);
    char *codeString = XKeysymToString(sym);
    StringView sv;

    if(codeString)
    {
        sv = puddle_cstr_sv(codeString);
    }

    if(sv.size == 0)
    {
        return 0;
    }

    if(sv.size == 1)
    {
        // cast uppercase to lowercase
        if(sv.data[0] > 0x40 && sv.data[0] < 0x5B)
        {
            return (uint8_t)(sv.data[0] + 0x21);
        }

        return (uint8_t)sv.data[0];
    }

    StringView space = puddle_cstr_sv("s");
    if(puddle_sv_find(sv, space) == space.data)
    {
        return RIVER2D_ASCII_SPACE;
    }

    StringView backspace = puddle_cstr_sv("B");
    if(puddle_sv_find(sv, backspace) == sv.data)
    {
        return RIVER2D_ASCII_BACKSPACE;
    }

    StringView less = puddle_cstr_sv("l");
    if(puddle_sv_find(sv, less) == sv.data)
    {
        return '<';
    }
    StringView greater = puddle_cstr_sv("g");
    if(puddle_sv_find(sv, greater) == sv.data)
    {
        return '>';
    }

    StringView period = puddle_cstr_sv("p");
    if(puddle_sv_find(sv, period) == sv.data)
    {
        return '.';
    }
    StringView comma = puddle_cstr_sv("c");
    if(puddle_sv_find(sv, comma) == sv.data)
    {
        return ',';
    }

    StringView minus = puddle_cstr_sv("m");
    if(puddle_sv_find(sv, minus) == sv.data)
    {
        return '-';
    }
    StringView equal = puddle_cstr_sv("e");
    if(puddle_sv_find(sv, equal) == sv.data)
    {
        return '=';
    }

    StringView escape = puddle_cstr_sv("E");
    if(puddle_sv_find(sv, escape) == sv.data)
    {
        return RIVER2D_ASCII_ESCAPE;
    }

    StringView enter = puddle_cstr_sv("R");
    if(puddle_sv_find(sv, enter) == sv.data)
    {
        return RIVER2D_ASCII_ENTER;
    }

    StringView tab = puddle_cstr_sv("T");
    if(puddle_sv_find(sv, tab) == sv.data)
    {
        return RIVER2D_ASCII_TAB;
    }

    StringView lshift = puddle_cstr_sv("Shift_L");
    if(puddle_sv_same(sv, lshift))
    {
        return RIVER2D_ASCII_LSHIFT;
    }
    StringView rshift = puddle_cstr_sv("Shift_R");
    if(puddle_sv_same(sv, rshift))
    {
        return RIVER2D_ASCII_RSHIFT;
    }

    StringView lctrl = puddle_cstr_sv("Control_L");
    if(puddle_sv_same(sv, lctrl))
    {
        return RIVER2D_ASCII_LCTRL;
    }
    StringView rctrl = puddle_cstr_sv("Control_R");
    if(puddle_sv_same(sv, rctrl))
    {
        return RIVER2D_ASCII_RCTRL;
    }

    // StringView lalt = puddle_cstr_sv("Alt_L");
    // if(puddle_sv_find(sv, lalt) == sv.data)
    // {
    //     return RIVER2D_ASCII_LALT;
    // }
    // NOTE: ISO_Level3_Shift for ALT_GR, I think otherwise it'd just be ALT_L
    // StringView ralt = puddle_cstr_sv("ISO_Level3_Shift");
    // if(puddle_sv_find(sv, ralt) == sv.data)
    // {
    //     return RIVER2D_ASCII_RALT;
    // }

#ifdef DEBUG
    fprintf(stderr, PRI_SV"\n", ARG_SV(sv));
#endif
    return 0;
}

Dimensions river2D_getWindowSize
(
    EngineData *engine
){
    XWindowAttributes attr;
    XGetWindowAttributes(engine->display, engine->window, &attr);

    Dimensions dim =
    {
        (uint32_t)attr.width,
        (uint32_t)attr.height
    };

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

// to simplify access to renderer-agnostic function calls:
void river2D_init
(
    EngineData    *engine,
    River2D_Image *planes
){
    engine->init(engine, planes);
}

int32_t river2D_shutdown
(
    EngineData *engine
){
    return engine->shutdown(engine);
}

void river2D_bltBuffer
(
    EngineData *engine
){
    engine->bltBuffer(engine);
}

void river2D_loadText
(
    EngineData    *engine,
    River2D_Image *image,
    StringView    *sv,
    uint8_t       font,
    uint16_t      charsize,
    uint32_t      spacing,
    uint32_t      offsetX,
    uint32_t      offsetY
){
    engine->loadText(engine, image, sv, font, charsize, spacing, offsetX, offsetY);
}

void river2D_compositeImage
(
    EngineData    *engine,
    River2D_Image *src,
    River2D_Image *dst,
    uint8_t       pictop,
    uint32_t      offsetDstX,
    uint32_t      offsetDstY,
    uint32_t      offsetSrcX,
    uint32_t      offsetSrcY,
    uint32_t      cropWidth,
    uint32_t      cropHeight
){
    engine->compositeImage(engine, src, dst, pictop, offsetDstX, offsetDstY,
                           offsetSrcX, offsetSrcY, cropWidth, cropHeight);
}
