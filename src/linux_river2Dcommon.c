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

    river2D_refreshImage(engine, image);

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

    image->pixmap = XCreatePixmap(engine->display, XDefaultRootWindow(engine->display),
                                  image->width, image->height, 32);

    river2D_refreshImage(engine, image);

    image->picture = XRenderCreatePicture(engine->display, image->pixmap,
                                          engine->format, 0, 0);
    if(!image->picture)
    {
        fprintf(stderr, "\033[31m\nERROR: failed to create XRenderPicture from pointer."
                "\n\033[0m");
    }
}

void river2D_refreshImage
(
    EngineData    *engine,
    River2D_Image *image
){
    XImage *ximg = XCreateImage(engine->display, engine->visual, 32, ZPixmap, 0,
                               (char*)image->data, image->width, image->height, 32, 0);
    XPutImage(engine->display, image->pixmap, engine->context, ximg, 0, 0, 0, 0,
              image->width, image->height);

    ximg->data = NULL;
    XDestroyImage(ximg);
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

extern AsciiKey xkeyToAscii
(
    EngineData *engine,
    XEvent     *event
){
    KeySym sym_shifted = XkbKeycodeToKeysym(engine->display,
                                            (KeyCode)event->xkey.keycode,
                                            0, event->xkey.state & ShiftMask);

    KeySym sym_unshifted = XkbKeycodeToKeysym(engine->display,
                                              (KeyCode)event->xkey.keycode,
                                              0, 0);

    char *codeString_shifted   = XKeysymToString(sym_shifted);
    char *codeString_unshifted = XKeysymToString(sym_unshifted);

    StringView sv_shifted   = cstr_sv(codeString_shifted);
    StringView sv_unshifted = cstr_sv(codeString_unshifted);

    #ifdef DEBUG
    fprintf(stderr, "sv_shifted:   "PRI_SV"\n", ARG_SV(sv_shifted));
    fprintf(stderr, "sv_unshifted: "PRI_SV"\n", ARG_SV(sv_unshifted));
    #endif

    if(sv_unshifted.size == 0)
    {
        return (AsciiKey){0};
    }

    if(sv_shifted.size == 1 && sv_unshifted.size == 1)
    {
        return(AsciiKey)
        {
            .unshifted = (uint8_t)sv_unshifted.data[0],
            .shifted   = (uint8_t)sv_shifted.data[0]
        };
    }

    StringView lalt = cstr_sv("Alt_L");
    if(sv_same(lalt, sv_shifted))
    {
        return(AsciiKey)
        {
            .unshifted = RIVER2D_ASCII_LALT,
            .shifted   = RIVER2D_ASCII_LALT
        };
    }
    StringView ralt = cstr_sv("ISO_Level3_S");
    if(sv_find(ralt, sv_shifted) == sv_shifted.data)
    {
        return(AsciiKey)
        {
            .unshifted = RIVER2D_ASCII_ALTGR,
            .shifted   = RIVER2D_ASCII_ALTGR
        };
    }

    StringView backspace = cstr_sv("B");
    if(sv_find(backspace, sv_shifted) == sv_shifted.data)
    {
        return(AsciiKey)
        {
            .unshifted = RIVER2D_ASCII_BACKSPACE,
            .shifted   = RIVER2D_ASCII_BACKSPACE
        };
    }

    StringView lctrl = cstr_sv("Control_L");
    if(sv_same(lctrl, sv_shifted))
    {
        return(AsciiKey)
        {
            .unshifted = RIVER2D_ASCII_LCTRL,
            .shifted   = RIVER2D_ASCII_LCTRL
        };
    }
    StringView rctrl = cstr_sv("Control_R");
    if(sv_same(rctrl, sv_shifted))
    {
        return(AsciiKey)
        {
            .unshifted = RIVER2D_ASCII_RCTRL,
            .shifted   = RIVER2D_ASCII_RCTRL
        };
    }

    StringView delete = cstr_sv("D");
    if(sv_find(delete, sv_shifted) == sv_shifted.data)
    {
        return(AsciiKey)
        {
            .unshifted = RIVER2D_ASCII_DELETE,
            .shifted   = RIVER2D_ASCII_DELETE
        };
    }

    StringView escape = cstr_sv("E");
    if(sv_find(escape, sv_shifted) == sv_shifted.data)
    {
        return(AsciiKey)
        {
            .unshifted = RIVER2D_ASCII_ESCAPE,
            .shifted   = RIVER2D_ASCII_ESCAPE
        };
    }

    StringView enter = cstr_sv("R");
    if(sv_find(enter, sv_shifted) == sv_shifted.data)
    {
        return(AsciiKey)
        {
            .unshifted = RIVER2D_ASCII_ENTER,
            .shifted   = RIVER2D_ASCII_ENTER
        };
    }

    StringView lshift = cstr_sv("Shift_L");
    if(sv_same(lshift, sv_shifted))
    {
        return(AsciiKey)
        {
            .unshifted = RIVER2D_ASCII_LSHIFT,
            .shifted   = RIVER2D_ASCII_LSHIFT
        };
    }
    StringView rshift = cstr_sv("Shift_R");
    if(sv_same(rshift, sv_shifted))
    {
        return(AsciiKey)
        {
            .unshifted = RIVER2D_ASCII_RSHIFT,
            .shifted   = RIVER2D_ASCII_RSHIFT
        };
    }

    StringView tab = cstr_sv("T");
    if(sv_find(tab, sv_shifted) == sv_shifted.data)
    {
        return(AsciiKey)
        {
            .unshifted = RIVER2D_ASCII_TAB,
            .shifted   = RIVER2D_ASCII_TAB
        };
    }

    AsciiKey result = {0};

    if(sv_shifted.size == 1)
    {
        result.shifted = (uint8_t)sv_shifted.data[0];
    }
    if(sv_unshifted.size == 1)
    {
        result.unshifted = (uint8_t)sv_unshifted.data[0];
    }

    StringView ampersand = cstr_sv("am");
    if(sv_find(ampersand, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '&';
    }
    else if(sv_find(ampersand, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '&';
    }

    StringView apostrophe = cstr_sv("ap");
    if(sv_find(apostrophe, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '\'';
    }
    else if(sv_find(apostrophe, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '\'';
    }

    StringView circum = cstr_sv("asciic");
    if(sv_find(circum, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '^';
    }
    else if(sv_find(circum, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '^';
    }

    StringView asterisk = cstr_sv("ast");
    if(sv_find(asterisk, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '*';
    }
    else if(sv_find(asterisk, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '*';
    }

    StringView at = cstr_sv("at");
    if(sv_same(at, sv_shifted))
    {
        result.shifted = '@';
    }
    else if(sv_same(at, sv_unshifted))
    {
        result.unshifted = '@';
    }

    StringView backslash = cstr_sv("bac");
    if(sv_find(backslash, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '\\';
    }
    else if(sv_find(backslash, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '\\';
    }

    StringView verticalbar = cstr_sv("bar");
    if(sv_same(verticalbar, sv_shifted))
    {
        result.shifted = '|';
    }
    else if(sv_same(verticalbar, sv_unshifted))
    {
        result.unshifted = '|';
    }

    StringView braceleft = cstr_sv("bracel");
    if(sv_find(braceleft, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '{';
    }
    else if(sv_find(braceleft, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '{';
    }
    StringView braceright = cstr_sv("bracer");
    if(sv_find(braceright, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '}';
    }
    else if(sv_find(braceright, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '}';
    }

    StringView bracketleft = cstr_sv("bracketl");
    if(sv_find(bracketleft, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '[';
    }
    else if(sv_find(bracketleft, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '[';
    }
    StringView bracketright = cstr_sv("bracketr");
    if(sv_find(bracketright, sv_shifted) == sv_shifted.data)
    {
        result.shifted = ']';
    }
    else if(sv_find(bracketright, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = ']';
    }

    StringView colon = cstr_sv("col");
    if(sv_find(colon, sv_shifted) == sv_shifted.data)
    {
        result.shifted = ':';
    }
    else if(sv_find(colon, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = ':';
    }

    StringView comma = cstr_sv("com");
    if(sv_find(comma, sv_shifted) == sv_shifted.data)
    {
        result.shifted = ',';
    }
    else if(sv_find(comma, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = ',';
    }

    StringView dollar = cstr_sv("do");
    if(sv_find(dollar, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '$';
    }
    else if(sv_find(dollar, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '$';
    }

    StringView equal = cstr_sv("eq");
    if(sv_find(equal, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '=';
    }
    else if(sv_find(equal, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '=';
    }

    StringView exclam = cstr_sv("ex");
    if(sv_find(exclam, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '!';
    }
    else if(sv_find(exclam, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '!';
    }

    StringView greater = cstr_sv("gr");
    if(sv_find(greater, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '>';
    }
    else if(sv_find(greater, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '>';
    }

    StringView less = cstr_sv("le");
    if(sv_find(less, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '<';
    }
    else if(sv_find(less, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '<';
    }

    StringView minus = cstr_sv("mi");
    if(sv_find(minus, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '-';
    }
    else if(sv_find(minus, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '-';
    }

    StringView num = cstr_sv("nu");
    if(sv_find(num, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '#';
    }
    else if(sv_find(num, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '#';
    }

    StringView parenleft = cstr_sv("parenl");
    if(sv_find(parenleft, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '(';
    }
    else if(sv_find(parenleft, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '(';
    }
    StringView parenright = cstr_sv("parenr");
    if(sv_find(parenright, sv_shifted) == sv_shifted.data)
    {
        result.shifted = ')';
    }
    else if(sv_find(parenright, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = ')';
    }

    StringView percent = cstr_sv("perc");
    if(sv_find(percent, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '.';
    }
    else if(sv_find(percent, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '.';
    }

    StringView period = cstr_sv("peri");
    if(sv_find(period, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '.';
    }
    else if(sv_find(period, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '.';
    }

    StringView question = cstr_sv("que");
    if(sv_find(question, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '?';
    }
    else if(sv_find(question, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '?';
    }

    StringView quote = cstr_sv("quo");
    if(sv_find(quote, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '\"';
    }
    else if(sv_find(quote, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '\"';
    }

    StringView semicolon = cstr_sv("se");
    if(sv_find(semicolon, sv_shifted) == sv_shifted.data)
    {
        result.shifted = ';';
    }
    else if(sv_find(semicolon, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = ';';
    }

    StringView slash = cstr_sv("sl");
    if(sv_find(slash, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '/';
    }
    else if(sv_find(slash, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '/';
    }

    StringView space = cstr_sv("sp");
    if(sv_find(space, sv_shifted) == sv_shifted.data)
    {
        result.shifted = ' ';
    }
    else if(sv_find(space, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = ' ';
    }

    StringView underscore = cstr_sv("un");
    if(sv_find(underscore, sv_shifted) == sv_shifted.data)
    {
        result.shifted = '_';
    }
    else if(sv_find(underscore, sv_unshifted) == sv_unshifted.data)
    {
        result.unshifted = '_';
    }

#ifdef DEBUG
    if(!result.shifted && !result.unshifted)
    {
        fprintf(stderr, "unshifted: "PRI_SV"\n", ARG_SV(sv_unshifted));
        fprintf(stderr, "shifted: "PRI_SV"\n",   ARG_SV(sv_shifted));
    }
#endif
    return result;
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
