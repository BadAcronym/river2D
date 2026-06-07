#pragma once

#include "string_view.h"

#include <stdint.h>
#include <stdio.h>

#define bool  _Bool
#define true  1
#define false 0

#define v_persistent  static
#define s_global      static
#define f_internal    static

#ifdef BUILD_LINUX
    #include "X11/Xlib.h"
    #include "X11/Xutil.h"
    #include "X11/XKBlib.h"
    #include "X11/Xcursor/Xcursor.h"
    #include "X11/extensions/Xrender.h"

    #define __USE_POSIX199309
    #include <time.h>

    #include "pthread.h"
    #define  RV_SCANLINE 32
    #define  RV_CONFIG_PATH "./.river2Dconf"

    #define RV_MOUSE1 Button1
    #define RV_MOUSE2 Button2
    #define RV_MOUSE3 Button3
    #define RV_MOUSE4 Button4
    #define RV_MOUSE5 Button5

#endif

#ifdef BUILD_WINDOWS
    #include "Windows.h"
    #define  RV_CONFIG_PATH "./river2D.ini"

    #define RV_MOUSE1 0x01
    #define RV_MOUSE2 0x02
    #define RV_MOUSE3 0x10
#endif

#define RV_ERROR_LOADIMAGE_PTR    1
#define RV_ERROR_LOADIMAGE_FILE   2
#define RV_ERROR_INVALID_HEADER   3
#define RV_ERROR_INVALID_METADATA 4
#define RV_ERROR_INVALID_INDICES  5
#define RV_ERROR_WRITE_METADATA   6
#define RV_ERROR_WRITE_INDICES    7
#define RV_SUCCESS                128

#define RV_TILE_BIT_ANIMATED      0x01
#define RV_TILE_BIT_COLLISION     0x02

#define RV_VERTICAL   1
#define RV_HORIZONTAL 2

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

#define RV_CHANNELS_RGBA            0
#define RV_CHANNELS_BGRA            1
#define RV_CHANNELS_RGB             2
#define RV_CHANNELS_BGR             3
#define RV_CHANNELS_MAX             3

#define RV_FONT_DEFAULT             0
#define RV_FONT_MAX                 0

#define RV_TYPE_FILE                0
#define RV_TYPE_DIRECTORY           1
#define RV_TYPE_ERROR               2
#define RV_TYPE_OTHER               3
#define RV_TYPE_MAX                 3

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
    uint64_t time;
    uint64_t freq;
}
PerformanceCounter;

typedef struct RiverConfig
{
    uint32_t choices;
    uint8_t  renderer;
    uint8_t  backgrounds;
    uint32_t window_width;
    uint32_t window_height;
    uint32_t canvas_width;
    uint32_t canvas_height;
}
RiverConfig;

typedef struct RiverImage
{
    StringView path;
    uint8_t    *data;
    uint8_t    channels;
    uint32_t   width;
    uint32_t   height;

    #ifdef BUILD_LINUX
    Pixmap   pixmap;
    Picture  picture;
    #endif

    #ifdef BUILD_WINDOWS
    BITMAPINFO info;
    #endif
}
RiverImage;

typedef struct RiverTime
{
    int64_t s;
    int64_t ns;
}
RiverTime;

typedef struct AsciiKey
{
    uint8_t key;
    uint8_t raw;
}
AsciiKey;

typedef struct Coordinates
{
    float x;
    float y;
}
Coordinates;

typedef struct TileCoords
{
    uint16_t x;
    uint16_t y;
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
    uint8_t    status;
}
Button;

typedef struct Dimensions
{
    uint32_t width;
    uint32_t height;
}
Dimensions;

typedef struct TileMetadata
{
    uint8_t  fps;
    uint8_t  flags;
    int16_t  next;
}
TileMetadata;

typedef struct TileIndex
{
    uint16_t x;
    uint16_t y;
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
    uint64_t     keymap;
    uint64_t     buttonmap;
    Coordinates  pointer;
    RiverTime lastScrollTime;
    uint64_t     rumble;
    uint8_t      keycodes[128];
    uint8_t      buttoncodes[64];
    char         ascii;
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

    void    (*init)           (struct EngineData *engine,    RiverImage *planes);
    int32_t (*shutdown)       (struct EngineData *engine);
    void    (*bltBuffer)      (struct EngineData *engine);

    void    (*loadText)       (struct EngineData *engine,    RiverImage *image,
                               StringView        *sv,        uint8_t    font,
                               uint16_t          charsize,   uint32_t   spacing,
                               uint32_t          offsetX,    uint32_t   offsetY);

    void    (*compositeImage) (struct EngineData *engine,    RiverImage *src,
                               RiverImage        *dst,       uint8_t    pictop,
                               uint32_t          offsetSrcX, uint32_t   offsetSrcY,
                               uint32_t          offsetDstX, uint32_t   offsetDstY,
                               uint32_t          cropWidth,  uint32_t   cropHeight);
}
EngineData;

typedef struct rvButtonSettings
{
    RiverImage  *img;
    Coordinates point;
    StringView  *name;
    Button      *button;
    uint8_t     alignment;
    uint8_t     font;
    uint16_t    charsize;
    uint32_t    spacing;
}
rvButtonSettings;

typedef struct rvLoadMapSettings
{
    FILE       *file;
    uint16_t   *tilesize;
    uint32_t   *mapWidth;
    uint32_t   *mapHeight;
    uint8_t    *mapLayers;
    RiverImage *tilesheet;
    uint8_t    errorcode;
}
rvLoadMapSettings;

typedef struct rvSaveMapSettings
{
    FILE         *file;
    uint16_t     tilesize;
    uint32_t     mapWidth;
    uint32_t     mapHeight;
    uint8_t      mapLayers;
    RiverImage   *tilesheet;
    uint8_t      errorcode;
    TileMetadata *metadata;
    TileIndex    *indices;
}
rvSaveMapSettings;

extern void rvLoadImage_file
(
    EngineData *engine,
    StringView path,
    RiverImage *image,
    uint8_t    channels,
    uint8_t    bitdepth
);

extern void rvLoadImage_ptr
(
    EngineData *engine,
    void       *file,
    RiverImage *image,
    uint8_t    channels,
    uint8_t    bitdepth
);

extern void rvCreateImage
(
    EngineData *engine,
    RiverImage *image,
    uint32_t   width,
    uint32_t   height
);

extern void rvAppendImage
(
    EngineData *engine,
    RiverImage *src,
    RiverImage *dst,
    uint8_t    direction
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
extern float rvDeltaTime_ms
(
    const RiverTime *time1,
    const RiverTime *time2
);

// delta is taken from time2 - time1.
extern int64_t rvDeltaTime_ns
(
    const RiverTime *time1,
    const RiverTime *time2
);

extern RiverTime rvDeltaTime_now
(
    const RiverTime *time
);

extern float rvDeltaTime_now_ms
(
    const RiverTime *time
);

extern uint64_t rvDeltaTime_now_ns
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

extern void rvResolveRenderer
(
    EngineData *engine,
    StringView libpath,
    uint8_t    renderer
);

extern uint8_t rvVerifyPath
(
    StringView path
);

// lists all files in a directory and packs them into a StringView, separated by ';'.
// makes you responsible for freeing the returned StringView's data. Not recursive.
StringView rvListFiles
(
    StringView directory
);

#ifdef BUILD_LINUX
extern AsciiKey rvProcessXKey
(
    EngineData *engine,
    XEvent     *event
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
extern int32_t rvShutdown
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
    uint8_t    font;
    uint16_t   charsize;
    uint32_t   spacing;
    uint32_t   offsetX;
    uint32_t   offsetY;
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
    uint8_t    pictop;
    uint32_t   offsetSrcX;
    uint32_t   offsetSrcY;
    uint32_t   offsetDstX;
    uint32_t   offsetDstY;
    uint32_t   cropWidth;
    uint32_t   cropHeight;
}
rvCompositeSettings;

// Takes `src` at offsets `offsetSrcX` and `offsetSrcY`.
// Composites `src` onto `dst`, at `offsetDstX`, `offsetDstY`, given `pictop`.
extern void rvCompositeImage
(
    EngineData          *engine,
    rvCompositeSettings *settings
);
