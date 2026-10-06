#include "river2D_main.h"
#include "imgsurf_main.h"
#include "pd_print_macros.h"

#include "pd_string_view.h"

#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>

f_internal void resolve
(
    void       **fptr,
    void       *libPtr,
    const char *name,
    char       **error
){
    *fptr = dlsym(libPtr, name);
    if((*error = dlerror()))
    {
        PD_ERROR("could not load symbol: '%s': %s", name, *error);
    }
    else
    {
        PD_DEBUG("Loaded symbol: %s from %p at %p", name, libPtr, *fptr);
    }
}

void rvResolveFunctions
(
    EngineData *engine,
    StringView libpath,
    uint8_t    renderer
){
    char *error = 0;

    if(renderer == RV_RENDERER_SOFTWARE)
    {
        char so[4096] = {0};

        StringView sv_file = pdCstrSV("/libriver2Dsoftware.so");
        pdSVConcat(libpath, sv_file, so);

        void *software = dlopen(so, RTLD_NOW);
        if(!software)
        {
            PD_ERROR("Software renderer could not be loaded from specified folder: '"
                     PRI_SV"'. dlerror: %s", ARG_SV(libpath), dlerror());
            return;
        }

        resolve((void**)&engine->init,           software, "init",           &error);
        resolve((void**)&engine->shutdown,       software, "shutdown",       &error);
        resolve((void**)&engine->loadText,       software, "loadText",       &error);
        resolve((void**)&engine->bltBuffer,      software, "bltBuffer",      &error);
        resolve((void**)&engine->compositeImage, software, "compositeImage", &error);
    }
    else if(renderer == RV_RENDERER_OPENGL)
    {
        PD_WARN("OpenGL renderer not built yet for river2D.");
    }
    else if(renderer == RV_RENDERER_VULKAN)
    {
        PD_WARN("Vulkan renderer not built yet for river2D.");
    }
    else if(renderer == RV_RENDERER_DIRECTX)
    {
        PD_ERROR("DirectX renderer not supported on linux.");
    }
    else
    {
        PD_ERROR("unknown renderer specified in rvResolveFunctions.");
    }

    const char *x11Path = "/usr/lib/libX11.so";
    void *x11 = dlopen(x11Path, RTLD_NOW);
    if(!x11)
    {
        PD_ERROR("X11 library could not be loaded from path: '/usr/lib/libX11.so'."
                 "dlerror: %s", dlerror());
        return;
    }

    resolve((void**)&engine->xPending,        x11, "XPending",              &error);
    resolve((void**)&engine->xNextEvent,      x11, "XNextEvent",            &error);
    resolve((void**)&engine->xOpenDisplay,    x11, "XOpenDisplay",          &error);
    resolve((void**)&engine->xFree,           x11, "XFree",                 &error);
    resolve((void**)&engine->xInternAtom,     x11, "XInternAtom",           &error);
    resolve((void**)&engine->xGetWinAttr,     x11, "XGetWindowAttributes",  &error);
    resolve((void**)&engine->xSetWMProtocols, x11, "XSetWMProtocols",       &error);
    resolve((void**)&engine->xCreateColormap, x11, "XCreateColormap",       &error);
    resolve((void**)&engine->xGetVisualInfo,  x11, "XGetVisualInfo",        &error);
    resolve((void**)&engine->xKeySymToString, x11, "XKeysymToString",       &error);
    resolve((void**)&engine->xCreateImage,    x11, "XCreateImage",          &error);
    resolve((void**)&engine->xGetImage,       x11, "XGetImage",             &error);
    resolve((void**)&engine->xPutImage,       x11, "XPutImage",             &error);
    resolve((void**)&engine->xCreatePixmap,   x11, "XCreatePixmap",         &error);
    resolve((void**)&engine->xFreePixmap,     x11, "XFreePixmap",           &error);
    resolve((void**)&engine->xDefRootWindow,  x11, "XDefaultRootWindow",    &error);
    resolve((void**)&engine->xkbcodeToKeysym, x11, "XkbKeycodeToKeysym",    &error);
    resolve((void**)&engine->xCreateWindow,   x11, "XCreateWindow",         &error);
    resolve((void**)&engine->xStoreName,      x11, "XStoreName",            &error);
    resolve((void**)&engine->xSelectInput,    x11, "XSelectInput",          &error);
    resolve((void**)&engine->xMapWindow,      x11, "XMapWindow",            &error);
    resolve((void**)&engine->xCreateGC,       x11, "XCreateGC",             &error);
    resolve((void**)&engine->xFreeGC,         x11, "XFreeGC",               &error);
    resolve((void**)&engine->xDestroyWindow,  x11, "XDestroyWindow",        &error);
    resolve((void**)&engine->xCloseDisplay,   x11, "XCloseDisplay",         &error);

    const char *xcursorPath = "/usr/lib/libXcursor.so";
    void *xcur = dlopen(xcursorPath, RTLD_NOW);
    if(!xcur)
    {
        PD_ERROR("Xcursor library could not be loaded from path: "
                 "'/usr/lib/libXcursor.so'. dlerror: %s", dlerror());
    }

    resolve((void**)&engine->xDefineCursor,  xcur, "XDefineCursor",          &error);
    resolve((void**)&engine->xCursorImgLoad, xcur, "XcursorImageLoadCursor", &error);

    const char *xrenderPath = "/usr/lib/libXrender.so";
    void *xrender = dlopen(xrenderPath, RTLD_NOW);
    if(!xrender)
    {
        PD_ERROR("Xrender library could not be loaded from path: "
                 "'/usr/lib/libXrender.so'. dlerror: %s", dlerror());
        return;
    }

    resolve((void**)&engine->xRenderCreatePicture, xrender,
            "XRenderCreatePicture", &error);
    resolve((void**)&engine->xRenderFindStFormat, xrender,
            "XRenderFindStandardFormat", &error);
    resolve((void**)&engine->xRenderComp, xrender,
            "XRenderComposite", &error);
    resolve((void**)&engine->xRenderSetPicTrans, xrender,
            "XRenderSetPictureTransform", &error);
    resolve((void**)&engine->xRenderSetPicFilter, xrender,
            "XRenderSetPictureFilter", &error);
}

void rvCreateImage
(
    EngineData *engine,
    RiverImage *image,
    uint32_t   width,
    uint32_t   height
){
    image->path   = pdCstrSV("rvCreateImage");
    image->data   = calloc(width * height * RV_BPP, 1);
    image->width  = width;
    image->height = height;

    image->pixmap = engine->xCreatePixmap(engine->display,
                                          engine->xDefRootWindow(engine->display),
                                          image->width, image->height, 32);
    XImage *img   = engine->xCreateImage(engine->display, engine->visual,
                                         32, ZPixmap, 0, (char*)image->data,
                                         image->width, image->height, 32, 0);

    engine->xPutImage(engine->display, image->pixmap, engine->context, img, 0, 0, 0, 0,
                      image->width, image->height);

    img->data = NULL;
    XDestroyImage(img);

    image->picture = engine->xRenderCreatePicture(engine->display, image->pixmap,
                                                  engine->format, 0, 0);
    if(!image->picture)
    {
        PD_ERROR("failed to create XRenderPicture.");
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
    pdSVCstr(path, path_cstr);

    image->data = imLoadFile(path_cstr, &image->width, &image->height,
                             channels, bitdepth);
    image->path = path;

    if(!image->data)
    {
        PD_WARN("failed to load image from file: '%s'.", path_cstr);
        writeMissingTexture(image);
        return;
    }

    image->pixmap = engine->xCreatePixmap(engine->display,
                                          engine->xDefRootWindow(engine->display),
                                          image->width, image->height, 32);

    rvSyncImage(engine, image, true);

    image->picture = engine->xRenderCreatePicture(engine->display, image->pixmap,
                                                  engine->format, 0, 0);
    if(!image->picture)
    {
        PD_WARN("failed to create XRenderPicture from file: '%s'.", path_cstr);
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
    image->path = pdCstrSV("rvLoadImage_ptr");

    if(!image->data)
    {
        PD_WARN("failed to load image from pointer: %p.", file);
        writeMissingTexture(image);
        return;
    }

    image->path   = pdCstrSV("rvLoadImage_ptr");
    image->pixmap = engine->xCreatePixmap(engine->display,
                                          engine->xDefRootWindow(engine->display),
                                          image->width, image->height, 32);

    rvSyncImage(engine, image, true);

    image->picture = engine->xRenderCreatePicture(engine->display, image->pixmap,
                                                  engine->format, 0, 0);
    if(!image->picture)
    {
        PD_WARN("failed to create XRenderPicture from pointer: %p.", file);
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

    XImage *img = engine->xCreateImage(engine->display, engine->visual, 32,
                                       ZPixmap, 0, (char*)image->data,
                                       image->width, image->height, 32, 0);
    engine->xPutImage(engine->display, image->pixmap, engine->context, img, 0, 0, 0, 0,
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
    EngineData *engine,
    KeySym     sym
){
    char *codeString = engine->xKeySymToString(sym);
    StringView sv    = pdCstrSV(codeString);

    PD_TRACE("codeString: '"PRI_SV"'", ARG_SV(sv));

    if(sv.size == 0)
    {
        return 0;
    }

    if(sv.size == 1)
    {
        return (uint8_t)sv.data[0];
    }

    StringView lalt = pdCstrSV("Alt_L");
    if(pdSVSame(lalt, sv))
    {
        return RV_ASCII_LALT;
    }
    StringView ralt = pdCstrSV("ISO_Level3_S");
    if(pdSVFind(ralt, sv) == sv.data)
    {
        return RV_ASCII_ALTGR;
    }

    StringView backspace = pdCstrSV("B");
    if(pdSVFind(backspace, sv) == sv.data)
    {
        return RV_ASCII_BACKSPACE;
    }

    StringView lctrl = pdCstrSV("Control_L");
    if(pdSVSame(lctrl, sv))
    {
        return RV_ASCII_LCTRL;
    }
    StringView rctrl = pdCstrSV("Control_R");
    if(pdSVSame(rctrl, sv))
    {
        return RV_ASCII_RCTRL;
    }

    StringView delete = pdCstrSV("De");
    if(pdSVFind(delete, sv) == sv.data)
    {
        return RV_ASCII_DELETE;
    }

    StringView down = pdCstrSV("Do");
    if(pdSVFind(down, sv) == sv.data)
    {
        return RV_ASCII_DOWN;
    }

    StringView escape = pdCstrSV("E");
    if(pdSVFind(escape, sv) == sv.data)
    {
        return RV_ASCII_ESCAPE;
    }

    StringView left = pdCstrSV("L");
    if(pdSVFind(left, sv) == sv.data)
    {
        return RV_ASCII_LEFT;
    }

    StringView enter = pdCstrSV("Re");
    if(pdSVFind(enter, sv) == sv.data)
    {
        return RV_ASCII_ENTER;
    }

    StringView right = pdCstrSV("Ri");
    if(pdSVFind(right, sv) == sv.data)
    {
        return RV_ASCII_RIGHT;
    }

    StringView lshift = pdCstrSV("Shift_L");
    if(pdSVSame(lshift, sv))
    {
        return RV_ASCII_LSHIFT;
    }
    StringView rshift = pdCstrSV("Shift_R");
    if(pdSVSame(rshift, sv))
    {
        return RV_ASCII_RSHIFT;
    }

    StringView tab = pdCstrSV("T");
    if(pdSVFind(tab, sv) == sv.data)
    {
        return RV_ASCII_TAB;
    }

    StringView up = pdCstrSV("U");
    if(pdSVFind(up, sv) == sv.data)
    {
        return RV_ASCII_UP;
    }

    StringView ampersand = pdCstrSV("am");
    if(pdSVFind(ampersand, sv) == sv.data)
    {
        return '&';
    }

    StringView apostrophe = pdCstrSV("ap");
    if(pdSVFind(apostrophe, sv) == sv.data)
    {
        return '\'';
    }

    StringView circum = pdCstrSV("asciic");
    if(pdSVFind(circum, sv) == sv.data)
    {
        return '^';
    }

    StringView asterisk = pdCstrSV("ast");
    if(pdSVFind(asterisk, sv) == sv.data)
    {
        return '*';
    }

    StringView at = pdCstrSV("at");
    if(pdSVSame(at, sv))
    {
        return '@';
    }

    StringView backslash = pdCstrSV("bac");
    if(pdSVFind(backslash, sv) == sv.data)
    {
        return '\\';
    }

    StringView verticalbar = pdCstrSV("bar");
    if(pdSVSame(verticalbar, sv))
    {
        return '|';
    }

    StringView braceleft = pdCstrSV("bracel");
    if(pdSVFind(braceleft, sv) == sv.data)
    {
        return '{';
    }
    StringView braceright = pdCstrSV("bracer");
    if(pdSVFind(braceright, sv) == sv.data)
    {
        return '}';
    }

    StringView bracketleft = pdCstrSV("bracketl");
    if(pdSVFind(bracketleft, sv) == sv.data)
    {
        return '[';
    }
    StringView bracketright = pdCstrSV("bracketr");
    if(pdSVFind(bracketright, sv) == sv.data)
    {
        return ']';
    }

    StringView colon = pdCstrSV("col");
    if(pdSVFind(colon, sv) == sv.data)
    {
        return ':';
    }

    StringView comma = pdCstrSV("com");
    if(pdSVFind(comma, sv) == sv.data)
    {
        return ',';
    }

    StringView dollar = pdCstrSV("do");
    if(pdSVFind(dollar, sv) == sv.data)
    {
        return '$';
    }

    StringView equal = pdCstrSV("eq");
    if(pdSVFind(equal, sv) == sv.data)
    {
        return '=';
    }

    StringView exclam = pdCstrSV("ex");
    if(pdSVFind(exclam, sv) == sv.data)
    {
        return '!';
    }

    StringView greater = pdCstrSV("gr");
    if(pdSVFind(greater, sv) == sv.data)
    {
        return '>';
    }

    StringView less = pdCstrSV("le");
    if(pdSVFind(less, sv) == sv.data)
    {
        return '<';
    }

    StringView minus = pdCstrSV("mi");
    if(pdSVFind(minus, sv) == sv.data)
    {
        return '-';
    }

    StringView num = pdCstrSV("nu");
    if(pdSVFind(num, sv) == sv.data)
    {
        return '#';
    }

    StringView parenleft = pdCstrSV("parenl");
    if(pdSVFind(parenleft, sv) == sv.data)
    {
        return '(';
    }
    StringView parenright = pdCstrSV("parenr");
    if(pdSVFind(parenright, sv) == sv.data)
    {
        return ')';
    }

    StringView percent = pdCstrSV("perc");
    if(pdSVFind(percent, sv) == sv.data)
    {
        return '.';
    }

    StringView period = pdCstrSV("peri");
    if(pdSVFind(period, sv) == sv.data)
    {
        return '.';
    }

    StringView question = pdCstrSV("que");
    if(pdSVFind(question, sv) == sv.data)
    {
        return '?';
    }

    StringView quote = pdCstrSV("quo");
    if(pdSVFind(quote, sv) == sv.data)
    {
        return '\"';
    }

    StringView semicolon = pdCstrSV("se");
    if(pdSVFind(semicolon, sv) == sv.data)
    {
        return ';';
    }

    StringView slash = pdCstrSV("sl");
    if(pdSVFind(slash, sv) == sv.data)
    {
        return '/';
    }

    StringView space = pdCstrSV("sp");
    if(pdSVFind(space, sv) == sv.data)
    {
        return ' ';
    }

    StringView underscore = pdCstrSV("un");
    if(pdSVFind(underscore, sv) == sv.data)
    {
        return '_';
    }

    PD_DEBUG("Key not evaluated: '"PRI_SV"'", ARG_SV(sv));
    return 0;
}

AsciiKey rvProcessXKey
(
    EngineData *engine,
    XEvent     *event
){
    KeySym sym_key = engine->xkbcodeToKeysym(engine->display,
                                             (KeyCode)event->xkey.keycode, 0, 0);

    KeySym sym_raw = engine->xkbcodeToKeysym(engine->display,
                                             (KeyCode)event->xkey.keycode, 0,
                                             event->xkey.state & ShiftMask);

    return(AsciiKey)
    {
        .key = xkeyToAscii(engine, sym_key),
        .raw = xkeyToAscii(engine, sym_raw)
    };
}

Dimensions rvGetWindowSize
(
    EngineData *engine
){
    XWindowAttributes attr;
    engine->xGetWinAttr(engine->display, engine->window, &attr);

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

    Cursor cursor = engine->xCursorImgLoad(engine->display, &ximg);

    engine->xDefineCursor(engine->display, engine->window, cursor);
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
        XImage *ximg = engine->xCreateImage(engine->display, engine->visual, 32,
                                            ZPixmap, 0, (char*)image->data,
                                            image->width, image->height, 32, 0);
        engine->xPutImage(engine->display, image->pixmap, engine->context, ximg,
                          0, 0, 0, 0, image->width, image->height);

        ximg->data = NULL;
        XDestroyImage(ximg);
        return;
    }

    XImage *ximg = engine->xGetImage(engine->display, image->pixmap, 0, 0,
                                     image->width, image->height, AllPlanes, ZPixmap);

    if(!ximg)
    {
        PD_WARN("failed to sync ximg.");
    }

    if(image->data)
    {
        free(image->data);
    }
    image->data = (uint8_t*)ximg->data;

    ximg->data = NULL;
    XDestroyImage(ximg);
}
