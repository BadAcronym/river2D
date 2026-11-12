#ifndef RV_HEADER
#define RV_HEADER

#include "pd_path.h"
#include "pd_string_view.h"

#include <stdio.h>

#define bool  _Bool
#define true  1
#define false 0

#define v_persistent  static
#define s_global      static
#define f_internal    static

#ifdef BUILD_LINUX
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <X11/XKBlib.h>
    #include <X11/Xcursor/Xcursor.h>
    #include <X11/extensions/Xrender.h>

    #ifndef  __USE_POSIX199309
        #define __USE_POSIX199309
    #endif
    #include <time.h>

    #include <pthread.h>
    #define  RV_SCANLINE 32
    #define  RV_CONFIG_PATH "./.river2Dconf"

    #define RV_MOUSE1 Button1
    #define RV_MOUSE2 Button3
    #define RV_MOUSE3 Button2
    #define RV_MOUSE4 Button4
    #define RV_MOUSE5 Button5
#endif

#ifdef BUILD_WINDOWS
    #include <Windows.h>
    #define  RV_CONFIG_PATH "./river2D.ini"

    #define RV_MOUSE1 0x01
    #define RV_MOUSE2 0x02
    #define RV_MOUSE3 0x10
#endif

#define RV_ERROR_NONE             0
#define RV_ERROR_LOADIMAGE_PTR    1
#define RV_ERROR_LOADIMAGE_FILE   2
#define RV_ERROR_INVALID_HEADER   3
#define RV_ERROR_INVALID_METADATA 4
#define RV_ERROR_INVALID_INDICES  5
#define RV_ERROR_WRITE_METADATA   6
#define RV_ERROR_WRITE_INDICES    7
#define RV_ERROR_NULLPTR          8

#define RV_TILE_BIT_ANIMATED      0x01
#define RV_TILE_BIT_COLLISION     0x02

#define RV_VERTICAL   1
#define RV_HORIZONTAL 2

#define RV_UP    1
#define RV_DOWN  2
#define RV_LEFT  3
#define RV_RIGHT 4

#define RV_BPP                      4
#define RV_PIXDEPTH                 32
#define RV_MAX_PLANES               64
#define RV_MAX_THREADS              8

#define RV_RENDERER_SOFTWARE        0
#define RV_RENDERER_OPENGL          1
#define RV_RENDERER_VULKAN          2
#define RV_RENDERER_DIRECTX         3

#define RV_ASCII_CURSOR             0x01
#define RV_ASCII_UP                 0x02
#define RV_ASCII_DOWN               0x03
#define RV_ASCII_LEFT               0x04
#define RV_ASCII_RIGHT              0x05
#define RV_ASCII_BACKSPACE          0x08
#define RV_ASCII_TAB                0x09
#define RV_ASCII_ENTER              0x0A
#define RV_ASCII_LSHIFT             0x0E
#define RV_ASCII_RSHIFT             0x0F
#define RV_ASCII_LCTRL              0x11
#define RV_ASCII_RCTRL              0x12
#define RV_ASCII_LALT               0x13
#define RV_ASCII_ALTGR              0x14
#define RV_ASCII_ESCAPE             0x1B
#define RV_ASCII_DELETE             0x7F

#define RV_WIN32_UP                 0x26
#define RV_WIN32_DOWN               0x28
#define RV_WIN32_LEFT               0x25
#define RV_WIN32_RIGHT              0x26
#define RV_WIN32_TAB
#define RV_WIN32_ENTER              0x0D
#define RV_WIN32_LSHIFT             0x10
#define RV_WIN32_RSHIFT
#define RV_WIN32_LCTRL
#define RV_WIN32_RCTRL
#define RV_WIN32_LALT
#define RV_WIN32_ALTGR
#define RV_WIN32_DELETE             0x2E
#define RV_WIN32_COLON              0xBA
#define RV_WIN32_COMMA              0xBC
#define RV_WIN32_DOT                0xBE
#define RV_WIN32_SLASH              0xBF
#define RV_WIN32_BACKSLASH          0xDC
#define RV_WIN32_EQUALS             0xBB
#define RV_WIN32_MINUS              0xBD

#define RV_CHANNELS_RGBA            0
#define RV_CHANNELS_BGRA            1
#define RV_CHANNELS_RGB             2
#define RV_CHANNELS_BGR             3
#define RV_CHANNELS_MAX             3

#define RV_FONT_DEFAULT             0
#define RV_FONT_MAX                 0

#define RV_PICTOP_MINIMUM           0
#define RV_PICTOP_CLEAR             0
#define RV_PICTOP_SRC               1
#define RV_PICTOP_DST               2
#define RV_PICTOP_OVER              3
#define RV_PICTOP_OVERREVERSE       4
#define RV_PICTOP_IN                5
#define RV_PICTOP_INREVERSE         6
#define RV_PICTOP_OUT               7
#define RV_PICTOP_OUTREVERSE        8
#define RV_PICTOP_ATOP              9
#define RV_PICTOP_ATOPREVERSE       10
#define RV_PICTOP_XOR               11
#define RV_PICTOP_ADD               12
#define RV_PICTOP_SATURATE          13
#define RV_PICTOP_MAXIMUM           13

#define RV_CHOICE_SHOW_FPS_BIT      1
#define RV_CHOICE_STATIC_CANVAS_BIT 2
#define RV_CHOICE_BACKGROUNDS_BYTE  0xFF000000

#define RV_BIT_HOVER                0x01

#define RV_ALIGN_TOPLEFT            1
#define RV_ALIGN_TOPCENTER          2
#define RV_ALIGN_TOPRIGHT           3
#define RV_ALIGN_CENTERLEFT         4
#define RV_ALIGN_CENTERRIGHT        5
#define RV_ALIGN_BOTTOMLEFT         6
#define RV_ALIGN_BOTTOMCENTER       7
#define RV_ALIGN_BOTTOMRIGHT        8

typedef struct PerformanceCounter
{
    u64 time;
    u64 freq;
}
PerformanceCounter;

typedef struct RiverConfig
{
    u08 renderer;
    u08 backgrounds;
    u32 windowWidth;
    u32 windowHeight;
    u32 canvasWidth;
    u32 canvasHeight;
    u32 choices;
}
RiverConfig;

typedef struct RiverImage
{
    StringView path;
    u08        *data;
    u08        channels;
    u32        width;
    u32        height;

    #ifdef BUILD_LINUX
    Pixmap     pixmap;
    Picture    picture;
    #endif

    #ifdef BUILD_WINDOWS
    BITMAPINFO info;
    #endif
}
RiverImage;

typedef struct RiverTime
{
    i64 s;
    i64 ns;
}
RiverTime;

typedef struct AsciiKey
{
    u08 key;
    u08 raw;
}
AsciiKey;

typedef struct Coordinates
{
    f32 x;
    f32 y;
}
Coordinates;

typedef struct TileCoords
{
    u16 x;
    u16 y;
}
TileCoords;

typedef struct Area
{
    Coordinates upLeft;
    Coordinates upRight;
    Coordinates lowLeft;
    Coordinates lowRight;
}
Area;

typedef struct Rect
{
    Coordinates upLeft;
    Coordinates lowRight;
}
Rect;

typedef struct Button
{
    StringView name;
    Rect       area;
    u08        status;
}
Button;

typedef struct Dimensions
{
    u32 width;
    u32 height;
}
Dimensions;

typedef struct TileMetadata
{
    u08 fps;
    u08 flags;
    i16 next;
}
TileMetadata;

typedef struct TileIndex
{
    u16 x;
    u16 y;
}
TileIndex;

typedef struct TileMap
{
    TileMetadata *metadata;
    TileIndex    *indices;
}
TileMap;

typedef struct RiverControls
{
    u64         keymap;
    u64         buttonmap;
    Coordinates pointer;
    RiverTime   lastScrollTime;
    u64         rumble;
    u08         keycodes[128];
    u08         buttoncodes[64];
    char        ascii;
}
RiverControls;

typedef struct EngineData
{
    const char    *windowName;
    RiverControls controls;
    RiverConfig   config;
    RiverImage    backbuffer;
    RiverImage    *planes;
    RiverImage    *currentCursor;
    bool          running;

#ifdef BUILD_LINUX
    Display           *display;
    Screen            *screen;
    XRenderPictFormat *format;
    Visual            *visual;
    Window            window;
    GC                context;
    Picture           blitDstPict;
#endif

#ifdef BUILD_WINDOWS
    HINSTANCE          instance;
    HWND               window;
    HDC                context;
    HBITMAP            cursorBitmap;
    HBITMAP            cursorMask;
    HCURSOR            hCursor;
#endif

    void    (*init)           (struct EngineData *engine, RiverImage *planes);
    i32     (*shutdown)       (struct EngineData *engine);
    void    (*bltBuffer)      (struct EngineData *engine);

    void    (*loadText)       (struct EngineData *engine,    RiverImage *image,
                               StringView        *sv,        u08        font,
                               u16          charsize,   u32        spacing,
                               u32               offsetX,    u32        offsetY);

    void    (*compositeImage) (struct EngineData *engine,    RiverImage *src,
                               RiverImage        *dst,       u08        pictop,
                               u32               offsetSrcX, u32        offsetSrcY,
                               u32               offsetDstX, u32        offsetDstY,
                               u32               cropWidth,  u32        cropHeight);

#ifdef BUILD_LINUX
    // X11
    int          (*xPending)           (Display *display);
    int          (*xNextEvent)         (Display *display, XEvent *event);

    Display*     (*xOpenDisplay)       (const char *name);

    int          (*xFree)              (void *data);

    Atom         (*xInternAtom)        (Display *display, const char *name,
                                        Bool    only);

    Status       (*xGetWinAttr)        (Display           *display, Window w,
                                        XWindowAttributes *wAttr);

    Status       (*xSetWMProtocols)    (Display *display, Window  w,
                                        Atom    *proto,   i32 count);

    Colormap     (*xCreateColormap)    (Display *display, Window  w,
                                        Visual  *visual,  i32 alloc);

    XVisualInfo* (*xGetVisualInfo)     (Display     *display, long infoMask,
                                        XVisualInfo *vInfo,   int  *nItems);

    char*        (*xKeySymToString)    (KeySym keysym);

    XImage*      (*xCreateImage)       (Display *display, Visual *visual,
                                        u32     depth,    i32    format,
                                        i32     offset,   char   *data,
                                        u32     width,    u32    height,
                                        i32     bitPad,   i32    bytesPL);

    XImage*      (*xGetImage)          (Display *display, Drawable d,
                                        i32     x,        i32      y,
                                        u32     width,    u32      height,
                                        u64     plane,    i32      format);

    int          (*xPutImage)          (Display *display, Drawable d,
                                        GC      gc,       XImage   *image,
                                        i32     srcX,     i32      srcY,
                                        i32     dstX,     i32      dstY,
                                        u32     width,    u32      height);

    Pixmap       (*xCreatePixmap)      (Display *display, Drawable d,
                                        u32     width,    u32      height,
                                        u32     depth);

    int          (*xFreePixmap)        (Display *display, Pixmap pixmap);

    Window       (*xDefRootWindow)     (Display *display);

    KeySym       (*xkbcodeToKeysym)    (Display *display, KeyCode kc,
                                        i32     group,    i32     level);

    Window       (*xCreateWindow)      (Display  *display, Window               parent,
                                        i32      x,        i32                  y,
                                        u32      width,    u32                  height,
                                        u32      borderW,  i32                  depth,
                                        u32      class,    Visual               *visual,
                                        u64      mask,     XSetWindowAttributes *attr);

    int          (*xStoreName)         (Display    *display, Window w,
                                        const char *name);

    int          (*xSelectInput)       (Display *display, Window w,
                                        i64     eventMask);

    int          (*xMapWindow)         (Display *display, Window w);

    GC           (*xCreateGC)          (Display  *display, Drawable  d,
                                        u64      mask,     XGCValues *values);

    int          (*xFreeGC)            (Display *display, GC gc);

    int          (*xDestroyWindow)     (Display *display, Window w);

    int          (*xCloseDisplay)      (Display *display);

    // Xcursor
    int    (*xDefineCursor)  (Display *display, Window w, Cursor cursor);

    Cursor (*xCursorImgLoad) (Display *display, XcursorImage *img);

    // XRender

    Picture (*xRenderCreatePicture) (Display                        *display,
                                     Drawable                       drawable,
                                     const XRenderPictFormat        *format,
                                     unsigned long                  valuemask,
                                     const XRenderPictureAttributes *attributes);

    XRenderPictFormat* (*xRenderFindStFormat) (Display *display, int format);

    void (*xRenderComp) (Display *display,
                         int     op,      Picture src,
                         Picture mask,    Picture dst,
                         i32     src_x,   i32     src_y,
                         i32     mask_x,  i32     mask_y,
                         i32     dst_x,   i32     dst_y,
                         u32     width,   u32     height);

    void (*xRenderSetPicTrans)  (Display    *display, Picture picture,
                                 XTransform *transform);

    void (*xRenderSetPicFilter) (Display    *display, Picture picture,
                                 const char *filter,  XFixed  *params,
                                 i32        nparams);
#endif
}
EngineData;

typedef struct rvButtonSettings
{
    RiverImage  *img;
    Coordinates point;
    StringView  *name;
    Button      *button;
    u08         alignment;
    u08         font;
    u16    charsize;
    u32         spacing;
}
rvButtonSettings;

typedef struct rvLoadMapSettings
{
    FILE       *file;
    u16   *tilesize;
    u32        *mapWidth;
    u32        *mapHeight;
    u08        *mapLayers;
    RiverImage *tilesheet;
    u08        errorcode;
}
rvLoadMapSettings;

typedef struct rvSaveMapSettings
{
    FILE         *file;
    u16     tilesize;
    u32          mapWidth;
    u32          mapHeight;
    u08          mapLayers;
    RiverImage   *tilesheet;
    u08          errorcode;
    TileMetadata *metadata;
    TileIndex    *indices;
}
rvSaveMapSettings;

extern void rvLoadImage_file
(
    EngineData *engine,
    StringView path,
    RiverImage *image,
    u08        channels,
    u08        bitdepth
);

extern void rvLoadImage_ptr
(
    EngineData *engine,
    void       *file,
    RiverImage *image,
    u08        channels,
    u08        bitdepth
);

extern void rvCreateImage
(
    EngineData *engine,
    RiverImage *image,
    u32        width,
    u32        height
);

extern void rvAppendImage
(
    EngineData *engine,
    RiverImage *src,
    RiverImage *dst,
    u08        direction
);

extern void rvSyncImage
(
    EngineData *engine,
    RiverImage *image,
    bool       CPU_to_GPU
);

extern void rvClearImage
(
    EngineData *engine,
    RiverImage *image
);

extern void rvDestroyImage
(
    RiverImage *image
);

extern RiverTime rvQueryTime
(
    void
);

// delta is taken from time2 - time1.
extern RiverTime rvDeltaTime
(
    const RiverTime *time1,
    const RiverTime *time2
);

// delta is taken from time2 - time1.
extern f32 rvDeltaTime_ms
(
    const RiverTime *time1,
    const RiverTime *time2
);

// delta is taken from time2 - time1.
extern i64 rvDeltaTime_ns
(
    const RiverTime *time1,
    const RiverTime *time2
);

extern RiverTime rvDeltaTime_now
(
    const RiverTime *time
);

extern f32 rvDeltaTime_now_ms
(
    const RiverTime *time
);

extern u64 rvDeltaTime_now_ns
(
    const RiverTime *time
);

extern TileMap rvLoadTilemap
(
    EngineData        *engine,
    rvLoadMapSettings *set
);

extern void rvSaveTilemap
(
    EngineData        *engine,
    rvSaveMapSettings *set
);

extern void rvResolveFunctions
(
    EngineData *engine,
    StringView libpath,
    u08    renderer
);

#ifdef BUILD_LINUX
extern AsciiKey rvProcessXKey
(
    EngineData *engine,
    XEvent     *event
);
#endif

#ifdef BUILD_WINDOWS
extern AsciiKey rvProcessWin32Key
(
    WPARAM  wParam,
    u08 ascii
);
#endif

extern void rvLoadConfig
(
    RiverConfig *config
);

extern Dimensions rvGetWindowSize
(
    EngineData *engine
);

extern void rvChangeCursor
(
    EngineData *engine,
    RiverImage *image
);

extern bool rvInsideArea
(
    const Coordinates *point,
    const Area        *area
);

extern bool rvInsideRect
(
    const Coordinates *point,
    const Rect        *rect
);

extern void rvCreateButton
(
    EngineData       *engine,
    rvButtonSettings *settings
);

// initializes the engine and all needed resources.
extern void rvInit
(
    EngineData *engine,
    RiverImage *planes
);

// shuts down the engine and safely frees all used resources.
extern i32 rvShutdown
(
    EngineData *engine
);

// takes whatever is in `engine->backbuffer` and blts it to the window, performing
// scaling, if necessary.
extern void rvBltBuffer
(
    EngineData *engine
);

typedef struct rvLoadTextSettings
{
    RiverImage *image;
    StringView *sv;
    u08        font;
    u16        charsize;
    u32        spacing;
    u32        offsetX;
    u32        offsetY;
}
rvLoadTextSettings;

// reads the text from `sv`, creates an image with the wanted text,
// taking the font image from `engine->planes[font]`.
// Needs you to specify `charsize` and the `offsetX`, `offsetY`.
extern void rvLoadText
(
    EngineData         *engine,
    rvLoadTextSettings *settings
);

typedef struct rvCompositeSettings
{
    RiverImage *src;
    RiverImage *dst;
    u08        pictop;
    u32        offsetSrcX;
    u32        offsetSrcY;
    u32        offsetDstX;
    u32        offsetDstY;
    u32        cropWidth;
    u32        cropHeight;
}
rvCompositeSettings;

// Takes `src` at offsets `offsetSrcX` and `offsetSrcY`.
// Composites `src` onto `dst`, at `offsetDstX`, `offsetDstY`, given `pictop`.
extern void rvCompositeImage
(
    EngineData          *engine,
    rvCompositeSettings *settings
);

#endif
