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
        StringView sv_file = cstr_sv("/libriver2Dsoftware.so");
        const char *so     = sv_concat(libpath, sv_file);

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

void river2D_createImage
(
    EngineData    *engine,
    River2D_Image *image,
    uint32_t      width,
    uint32_t      height
){
    image->path   = cstr_sv("river2D_createImage");
    image->data   = calloc(width * height * RIVER2D_BPP, 1);
    image->width  = width;
    image->height = height;

    image->pixmap = XCreatePixmap(engine->display, XDefaultRootWindow(engine->display),
                                  image->width, image->height, 32);
    XImage *img   = XCreateImage(engine->display, engine->visual, 32, ZPixmap, 0,
                                 (char*)image->data, image->width, image->height,
                                 32, 0);

    XPutImage(engine->display, image->pixmap, engine->context, img, 0, 0, 0,
    0, image->width, image->height);

    img->data = NULL;
    XDestroyImage(img);

    image->picture = XRenderCreatePicture(engine->display, image->pixmap,
                                          engine->format, 0, 0);
    if(!image->picture)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XRenderPicture.\n\033[0m");
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
    StringView    path,
    River2D_Image *image,
    uint8_t       channels,
    uint8_t       bitdepth
){
    const char *path_cstr = sv_cstr(path);

    image->data = imgsurf_load_file(path_cstr, &image->width, &image->height,
                                    channels, bitdepth);
    image->path = path;

    if(!image->data)
    {
        fprintf(stderr, "Failed to load image from file: %s\n", path_cstr);
        writeMissingTexture(image);
        return;
    }

    image->pixmap = XCreatePixmap(engine->display, XDefaultRootWindow(engine->display),
                                  image->width, image->height, 32);

    river2D_syncImage(engine, image, true);

    image->picture = XRenderCreatePicture(engine->display, image->pixmap,
                                          engine->format, 0, 0);
    if(!image->picture)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XRenderPicture from file: "
                "%s!.\n\033[0m", path_cstr);
    }

    free((void*)path_cstr);
}

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
    image->path = cstr_sv("river2D_loadImage_ptr");

    if(!image->data)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to load image to pointer.\n\033[0m");
        writeMissingTexture(image);
        return;
    }

    image->path   = cstr_sv("river2D_loadImage_ptr");
    image->pixmap = XCreatePixmap(engine->display, XDefaultRootWindow(engine->display),
                                  image->width, image->height, 32);

    river2D_syncImage(engine, image, true);

    image->picture = XRenderCreatePicture(engine->display, image->pixmap,
                                          engine->format, 0, 0);
    if(!image->picture)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XRenderPicture from pointer."
                "\n\033[0m");
    }
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
    const char *path_cstr = sv_cstr(path);

    if(stat(path_cstr, &pathInfo))
    {
        free((void*)path_cstr);
        return RIVER2D_TYPE_ERROR;
    }

    free((void*)path_cstr);

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

StringView river2D_listFiles
(
    StringView directory
){
    DIR           *dir;
    struct dirent *ent;
    uint32_t      listSize = 0;

    const char *path = sv_cstr(directory);

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
        fprintf(stderr, "\n\033[31;1;7mERROR: failed to open directory.\033[0m\n");
        return(StringView){0};
    }

    String list = {0};
    list.data   = (char*)malloc(listSize + 1);
    list.size   = 0;
    uint32_t offset = 0;

    free(dir);
    dir = opendir(path);

    while((ent = readdir(dir)))
    {
        uint8_t curr_length = 0;
        for(; curr_length >= 0 && ent->d_name[curr_length] != '\0'; ++curr_length)
        {
            list.data[offset + curr_length] = ent->d_name[curr_length];
        }
        list.data[offset + curr_length] = ';';
        list.size += curr_length + 1;
        offset    += curr_length + 1;
    }

    list.data[offset] = '\0';

    StringView result =
    {
        .data = list.data,
        .size = list.size
    };

    const char *sorted = sv_sort_by_delim(result, ';');
    free((void*)result.data);
    result.data = sorted;

    free(dir);
    free((void*)path);

    return result;
}

uint8_t xkeyToAscii
(
    KeySym sym
){
    char *codeString = XKeysymToString(sym);
    StringView sv    = cstr_sv(codeString);

    #ifdef DEBUG
    fprintf(stderr, "codeString: "PRI_SV"\n", ARG_SV(sv));
    #endif

    if(sv.size == 0)
    {
        return 0;
    }

    if(sv.size == 1)
    {
        return (uint8_t)sv.data[0];
    }

    StringView lalt = cstr_sv("Alt_L");
    if(sv_same(lalt, sv))
    {
        return RIVER2D_ASCII_LALT;
    }
    StringView ralt = cstr_sv("ISO_Level3_S");
    if(sv_find(ralt, sv) == sv.data)
    {
        return RIVER2D_ASCII_ALTGR;
    }

    StringView backspace = cstr_sv("B");
    if(sv_find(backspace, sv) == sv.data)
    {
        return RIVER2D_ASCII_BACKSPACE;
    }

    StringView lctrl = cstr_sv("Control_L");
    if(sv_same(lctrl, sv))
    {
        return RIVER2D_ASCII_LCTRL;
    }
    StringView rctrl = cstr_sv("Control_R");
    if(sv_same(rctrl, sv))
    {
        return RIVER2D_ASCII_RCTRL;
    }

    StringView delete = cstr_sv("De");
    if(sv_find(delete, sv) == sv.data)
    {
        return RIVER2D_ASCII_DELETE;
    }

    StringView down = cstr_sv("Do");
    if(sv_find(down, sv) == sv.data)
    {
        return RIVER2D_ASCII_DOWN;
    }

    StringView escape = cstr_sv("E");
    if(sv_find(escape, sv) == sv.data)
    {
        return RIVER2D_ASCII_ESCAPE;
    }

    StringView left = cstr_sv("L");
    if(sv_find(left, sv) == sv.data)
    {
        return RIVER2D_ASCII_LEFT;
    }

    StringView enter = cstr_sv("Re");
    if(sv_find(enter, sv) == sv.data)
    {
        return RIVER2D_ASCII_ENTER;
    }

    StringView right = cstr_sv("Ri");
    if(sv_find(right, sv) == sv.data)
    {
        return RIVER2D_ASCII_RIGHT;
    }

    StringView lshift = cstr_sv("Shift_L");
    if(sv_same(lshift, sv))
    {
        return RIVER2D_ASCII_LSHIFT;
    }
    StringView rshift = cstr_sv("Shift_R");
    if(sv_same(rshift, sv))
    {
        return RIVER2D_ASCII_RSHIFT;
    }

    StringView tab = cstr_sv("T");
    if(sv_find(tab, sv) == sv.data)
    {
        return RIVER2D_ASCII_TAB;
    }

    StringView up = cstr_sv("U");
    if(sv_find(up, sv) == sv.data)
    {
        return RIVER2D_ASCII_UP;
    }

    StringView ampersand = cstr_sv("am");
    if(sv_find(ampersand, sv) == sv.data)
    {
        return '&';
    }

    StringView apostrophe = cstr_sv("ap");
    if(sv_find(apostrophe, sv) == sv.data)
    {
        return '\'';
    }

    StringView circum = cstr_sv("asciic");
    if(sv_find(circum, sv) == sv.data)
    {
        return '^';
    }

    StringView asterisk = cstr_sv("ast");
    if(sv_find(asterisk, sv) == sv.data)
    {
        return '*';
    }

    StringView at = cstr_sv("at");
    if(sv_same(at, sv))
    {
        return '@';
    }

    StringView backslash = cstr_sv("bac");
    if(sv_find(backslash, sv) == sv.data)
    {
        return '\\';
    }

    StringView verticalbar = cstr_sv("bar");
    if(sv_same(verticalbar, sv))
    {
        return '|';
    }

    StringView braceleft = cstr_sv("bracel");
    if(sv_find(braceleft, sv) == sv.data)
    {
        return '{';
    }
    StringView braceright = cstr_sv("bracer");
    if(sv_find(braceright, sv) == sv.data)
    {
        return '}';
    }

    StringView bracketleft = cstr_sv("bracketl");
    if(sv_find(bracketleft, sv) == sv.data)
    {
        return '[';
    }
    StringView bracketright = cstr_sv("bracketr");
    if(sv_find(bracketright, sv) == sv.data)
    {
        return ']';
    }

    StringView colon = cstr_sv("col");
    if(sv_find(colon, sv) == sv.data)
    {
        return ':';
    }

    StringView comma = cstr_sv("com");
    if(sv_find(comma, sv) == sv.data)
    {
        return ',';
    }

    StringView dollar = cstr_sv("do");
    if(sv_find(dollar, sv) == sv.data)
    {
        return '$';
    }

    StringView equal = cstr_sv("eq");
    if(sv_find(equal, sv) == sv.data)
    {
        return '=';
    }

    StringView exclam = cstr_sv("ex");
    if(sv_find(exclam, sv) == sv.data)
    {
        return '!';
    }

    StringView greater = cstr_sv("gr");
    if(sv_find(greater, sv) == sv.data)
    {
        return '>';
    }

    StringView less = cstr_sv("le");
    if(sv_find(less, sv) == sv.data)
    {
        return '<';
    }

    StringView minus = cstr_sv("mi");
    if(sv_find(minus, sv) == sv.data)
    {
        return '-';
    }

    StringView num = cstr_sv("nu");
    if(sv_find(num, sv) == sv.data)
    {
        return '#';
    }

    StringView parenleft = cstr_sv("parenl");
    if(sv_find(parenleft, sv) == sv.data)
    {
        return '(';
    }
    StringView parenright = cstr_sv("parenr");
    if(sv_find(parenright, sv) == sv.data)
    {
        return ')';
    }

    StringView percent = cstr_sv("perc");
    if(sv_find(percent, sv) == sv.data)
    {
        return '.';
    }

    StringView period = cstr_sv("peri");
    if(sv_find(period, sv) == sv.data)
    {
        return '.';
    }

    StringView question = cstr_sv("que");
    if(sv_find(question, sv) == sv.data)
    {
        return '?';
    }

    StringView quote = cstr_sv("quo");
    if(sv_find(quote, sv) == sv.data)
    {
        return '\"';
    }

    StringView semicolon = cstr_sv("se");
    if(sv_find(semicolon, sv) == sv.data)
    {
        return ';';
    }

    StringView slash = cstr_sv("sl");
    if(sv_find(slash, sv) == sv.data)
    {
        return '/';
    }

    StringView space = cstr_sv("sp");
    if(sv_find(space, sv) == sv.data)
    {
        return ' ';
    }

    StringView underscore = cstr_sv("un");
    if(sv_find(underscore, sv) == sv.data)
    {
        return '_';
    }

#ifdef DEBUG
    fprintf(stderr, "Key not evaluated: "PRI_SV"\n", ARG_SV(sv));
#endif
    return 0;
}

AsciiKey processXKey
(
    EngineData *engine,
    XEvent     *event
){
    KeySym sym_key = XkbKeycodeToKeysym(engine->display, (KeyCode)event->xkey.keycode,
                                        0, 0);

    KeySym sym_raw = XkbKeycodeToKeysym(engine->display, (KeyCode)event->xkey.keycode,
                                        0, event->xkey.state & ShiftMask);

    return(AsciiKey)
    {
        .key = xkeyToAscii(sym_key),
        .raw = xkeyToAscii(sym_raw)
    };
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
    EngineData         *engine,
    rvLoadTextSettings *settings
){
    engine->loadText(engine,             settings->image,
                     settings->sv,       settings->font,
                     settings->charsize, settings->spacing,
                     settings->offsetX,  settings->offsetY);
}

void river2D_compositeImage
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

void river2D_syncImage
(
    EngineData    *engine,
    River2D_Image *image,
    bool          CPU_to_GPU
){
    if(CPU_to_GPU)
    {
        XImage *ximg = XCreateImage(engine->display, engine->visual, 32, ZPixmap, 0,
                                   (char*)image->data, image->width, image->height, 32, 0);
        XPutImage(engine->display, image->pixmap, engine->context, ximg, 0, 0, 0, 0,
                  image->width, image->height);

        ximg->data = NULL;
        XDestroyImage(ximg);
        return;
    }

    XImage *ximg = XGetImage(engine->display, image->pixmap, 0, 0,
                             image->width, image->height, AllPlanes, ZPixmap);

    if(!ximg)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: failed to sync ximg.\033[0m\n");
    }

    if(image->data)
    {
        free(image->data);
    }

    image->data = (uint8_t*)ximg->data;
}
