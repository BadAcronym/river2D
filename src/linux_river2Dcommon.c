#include "river2D_main.h"
#include "imgsurf_main.h"

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

void rvResolveRenderer
(
    EngineData *engine,
    StringView libpath,
    uint8_t    renderer
){
    if(renderer == RV_RENDERER_SOFTWARE)
    {
        char so[4096] = {0};
        char *error   = 0;

        StringView sv_file = cstr_sv("/libriver2Dsoftware.so");
        sv_concat(libpath, sv_file, so);

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
    else if(renderer == RV_RENDERER_OPENGL)
    {
        fprintf(stderr, "\033[33m\nWARNING: OpenGL renderer not built yet for river2D."
                "\033[0m");
    }
    else if(renderer == RV_RENDERER_VULKAN)
    {
        fprintf(stderr, "\033[33m\nWARNING: Vulkan renderer not built yet for river2D."
                "\033[0m");
    }
    else if(renderer == RV_RENDERER_DIRECTX)
    {
        fprintf(stderr, "\033[33m\nWARNING: DirectX renderer not built yet for river2D."
                "\033[0m");
    }
    else
    {
        fprintf(stderr, "\033[31m\nERROR: invalid renderer specified "
                "in rvResolveRenderer.\033[0m");
    }
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
    uint8_t    channels,
    uint8_t    bitdepth
){
    char path_cstr[4096] = {0};
    sv_cstr(path, path_cstr);

    image->data = imLoadFile(path_cstr, &image->width, &image->height,
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

    rvSyncImage(engine, image, true);

    image->picture = XRenderCreatePicture(engine->display, image->pixmap,
                                          engine->format, 0, 0);
    if(!image->picture)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XRenderPicture from file: "
                "%s!.\n\033[0m", path_cstr);
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
    image->path = cstr_sv("rvLoadImage_ptr");

    if(!image->data)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to load image to pointer.\n\033[0m");
        writeMissingTexture(image);
        return;
    }

    image->path   = cstr_sv("rvLoadImage_ptr");
    image->pixmap = XCreatePixmap(engine->display, XDefaultRootWindow(engine->display),
                                  image->width, image->height, 32);

    rvSyncImage(engine, image, true);

    image->picture = XRenderCreatePicture(engine->display, image->pixmap,
                                          engine->format, 0, 0);
    if(!image->picture)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XRenderPicture from pointer."
                "\n\033[0m");
    }
}

void rvClearImage
(
    EngineData *engine,
    RiverImage *image
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

RiverTime rvQueryTime
(
    void
){
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);

    RiverTime time =
    {
        .s  = (int64_t)spec.tv_sec,
        .ns = (int64_t)spec.tv_nsec
    };

    return time;
}

f_internal uint8_t xkeyToAscii
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
        return RV_ASCII_LALT;
    }
    StringView ralt = cstr_sv("ISO_Level3_S");
    if(sv_find(ralt, sv) == sv.data)
    {
        return RV_ASCII_ALTGR;
    }

    StringView backspace = cstr_sv("B");
    if(sv_find(backspace, sv) == sv.data)
    {
        return RV_ASCII_BACKSPACE;
    }

    StringView lctrl = cstr_sv("Control_L");
    if(sv_same(lctrl, sv))
    {
        return RV_ASCII_LCTRL;
    }
    StringView rctrl = cstr_sv("Control_R");
    if(sv_same(rctrl, sv))
    {
        return RV_ASCII_RCTRL;
    }

    StringView delete = cstr_sv("De");
    if(sv_find(delete, sv) == sv.data)
    {
        return RV_ASCII_DELETE;
    }

    StringView down = cstr_sv("Do");
    if(sv_find(down, sv) == sv.data)
    {
        return RV_ASCII_DOWN;
    }

    StringView escape = cstr_sv("E");
    if(sv_find(escape, sv) == sv.data)
    {
        return RV_ASCII_ESCAPE;
    }

    StringView left = cstr_sv("L");
    if(sv_find(left, sv) == sv.data)
    {
        return RV_ASCII_LEFT;
    }

    StringView enter = cstr_sv("Re");
    if(sv_find(enter, sv) == sv.data)
    {
        return RV_ASCII_ENTER;
    }

    StringView right = cstr_sv("Ri");
    if(sv_find(right, sv) == sv.data)
    {
        return RV_ASCII_RIGHT;
    }

    StringView lshift = cstr_sv("Shift_L");
    if(sv_same(lshift, sv))
    {
        return RV_ASCII_LSHIFT;
    }
    StringView rshift = cstr_sv("Shift_R");
    if(sv_same(rshift, sv))
    {
        return RV_ASCII_RSHIFT;
    }

    StringView tab = cstr_sv("T");
    if(sv_find(tab, sv) == sv.data)
    {
        return RV_ASCII_TAB;
    }

    StringView up = cstr_sv("U");
    if(sv_find(up, sv) == sv.data)
    {
        return RV_ASCII_UP;
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

AsciiKey rvProcessXKey
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

Dimensions rvGetWindowSize
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

void rvChangeCursor
(
    EngineData *engine,
    RiverImage *image
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
    if(CPU_to_GPU)
    {
        XImage *ximg = XCreateImage(engine->display, engine->visual, 32, ZPixmap, 0,
                                   (char*)image->data, image->width, image->height,
                                    32, 0);
        XPutImage(engine->display, image->pixmap, engine->context, ximg, 0, 0, 0, 0,
                  image->width, image->height);

        ximg->data = NULL;
        XDestroyImage(ximg);
        return;
    }

    XImage *ximg = XGetImage(engine->display, image->pixmap, 0, 0, image->width,
                             image->height, AllPlanes, ZPixmap);

    if(!ximg)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: failed to sync ximg.\033[0m\n");
    }

    if(image->data)
    {
        free(image->data);
    }
    image->data = (uint8_t*)ximg->data;

    ximg->data = NULL;
    XDestroyImage(ximg);
}
