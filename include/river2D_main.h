#pragma once

#include <stdint.h>

#define bool  _Bool
#define true  1
#define false 0

#define persistent  static
#define global      static
#define internal    static

#ifdef BUILD_LINUX
    #include "X11/Xlib.h"
    #include "X11/Xutil.h"
    #include "X11/Xcursor/Xcursor.h"
    #include "X11/extensions/Xrender.h"

    #define __USE_POSIX199309
    #include <time.h>

    #include "pthread.h"
    #define  RIVER2D_SCANLINE 32
    #define  RIVER2D_CONFIG_PATH "./.river2Dconf"

    #define RIVER2D_MOUSE1 Button1
    #define RIVER2D_MOUSE2 Button2
    #define RIVER2D_MOUSE3 Button3
    #define RIVER2D_MOUSE4 Button4
    #define RIVER2D_MOUSE5 Button5

#endif

#ifdef BUILD_WINDOWS
    #include "Windows.h"
    #define  RIVER2D_CONFIG_PATH "./river2D.ini"

    #define RIVER2D_MOUSE1 0x01
    #define RIVER2D_MOUSE2 0x02
    #define RIVER2D_MOUSE3 0x10
#endif

#define RIVER2D_BPP         4
#define RIVER2D_PIXDEPTH    32
#define RIVER2D_MAX_PLANES  64
// BACKLOG: (river2D #14) move max threads to some sort of detection function that polls the amount of cores
#define RIVER2D_MAX_THREADS 8

#define RIVER2D_RENDERER_SOFTWARE 0
#define RIVER2D_RENDERER_OPENGL   1
#define RIVER2D_RENDERER_VULKAN   2
#define RIVER2D_RENDERER_DIRECTX  3

#define RIVER2D_KEY_UP         0
#define RIVER2D_KEY_LEFT       1
#define RIVER2D_KEY_RIGHT      2
#define RIVER2D_KEY_DOWN       3
#define RIVER2D_KEY_TAB        4
#define RIVER2D_KEY_ESCAPE     5

#define RIVER2D_BIT_UP         1
#define RIVER2D_BIT_DOWN       2
#define RIVER2D_BIT_LEFT       4
#define RIVER2D_BIT_RIGHT      8
#define RIVER2D_BIT_TAB        16
#define RIVER2D_BIT_ESCAPE     32

#define RIVER2D_CHANNELS_RGBA  0
#define RIVER2D_CHANNELS_BGRA  1
#define RIVER2D_CHANNELS_RGB   2
#define RIVER2D_CHANNELS_BGR   3
#define RIVER2D_CHANNELS_MAX   3

#define RIVER2D_FONT_DEFAULT   0
#define RIVER2D_FONT_MAX       0

#define RIVER2D_TYPE_FILE      0
#define RIVER2D_TYPE_DIRECTORY 1
#define RIVER2D_TYPE_ERROR     2
#define RIVER2D_TYPE_OTHER     3
#define RIVER2D_TYPE_MAX       3

#define RIVER2D_PICTOP_MINIMUM	   0
#define RIVER2D_PICTOP_CLEAR	   0
#define RIVER2D_PICTOP_SRC		   1
#define RIVER2D_PICTOP_DST		   2
#define RIVER2D_PICTOP_OVER		   3
#define RIVER2D_PICTOP_OVERREVERSE 4
#define RIVER2D_PICTOP_IN		   5
#define RIVER2D_PICTOP_INREVERSE   6
#define RIVER2D_PICTOP_OUT		   7
#define RIVER2D_PICTOP_OUTREVERSE  8
#define RIVER2D_PICTOP_ATOP		   9
#define RIVER2D_PICTOP_ATOPREVERSE 10
#define RIVER2D_PICTOP_XOR		   11
#define RIVER2D_PICTOP_ADD		   12
#define RIVER2D_PICTOP_SATURATE	   13
#define RIVER2D_PICTOP_MAXIMUM	   13

#define RIVER2D_CHOICE_SHOW_FPS_BIT      1
#define RIVER2D_CHOICE_STATIC_CANVAS_BIT 2
#define RIVER2D_CHOICE_BACKGROUNDS_BYTE  0xFF000000

typedef struct PerformanceCounter
{
    uint64_t time;
    uint64_t freq;
}
PerformanceCounter;

typedef struct River2D_Config
{
    uint32_t choices;
    uint8_t  renderer;
    uint8_t  backgrounds;
    uint32_t window_width;
    uint32_t window_height;
    uint32_t canvas_width;
    uint32_t canvas_height;
}
River2D_Config;

typedef struct River2D_Image
{
    char     *path;
    uint8_t  *data;
    uint8_t  channels;
    uint32_t width;
    uint32_t height;

    #ifdef BUILD_LINUX
    Pixmap   pixmap;
    Picture  picture;
    #endif
}
River2D_Image;

typedef struct River2D_Time
{
    uint64_t s;
    uint64_t ns;
}
River2D_Time;

typedef struct Coordinates
{
    double x;
    double y;
}
Coordinates;

typedef struct Area
{
    Coordinates upperLeft;
    Coordinates upperRight;
    Coordinates lowerLeft;
    Coordinates lowerRight;
}
Area;

typedef struct Rect
{
    Coordinates upperLeft;
    Coordinates lowerRight;
}
Rect;

typedef struct Dimensions
{
    uint32_t width;
    uint32_t height;
}
Dimensions;

typedef struct River2D_ControlMap
{
    uint64_t     keymap;
    uint64_t     buttonmap;
    Coordinates  pointer;
    River2D_Time lastScrollTime;
    uint64_t     rumble;
    uint8_t      keycodes[64];
    uint8_t      buttoncodes[64];
    //other general use stuff
}
River2D_ControlMap;

#ifdef BUILD_LINUX
typedef struct linuxBackbuffer
{
    Pixmap   pixmap;
    Picture  picture;
    uint32_t width;
    uint32_t height;
}
LinuxBackbuffer;

typedef struct PosixThreadpool
{
    pthread_t  threads[RIVER2D_MAX_THREADS];
}
PosixThreadpool;
#elif defined(BUILD_WINDOWS)
typedef struct Buffer
{
    BITMAPINFO info;
    void       *data;
    uint32_t   width;
    uint32_t   height;
}
Win32Backbuffer;
#endif

typedef struct EngineData
{
    const char         *windowName;
    River2D_ControlMap controls;
    River2D_Config     config;
    River2D_Image      *planes;
    River2D_Image      *currentCursor;
    bool               running;

#ifdef BUILD_LINUX
    Display            *display;
    Screen             *screen;
    XRenderPictFormat  *format;
    Visual             *visual;
    Window             window;
    GC                 context;
    LinuxBackbuffer    backbuffer;
    Picture            blitDstPict;
    PosixThreadpool    pool;
#endif

#ifdef BUILD_WINDOWS
    HINSTANCE          instance;
    HWND               window;
    HDC                context;
    Win32Backbuffer    backbuffer;
    HBITMAP            cursorBitmap;
    HBITMAP            cursorMask;
    HCURSOR            hCursor;
#endif

    void    (*init)           (struct EngineData *engine,    River2D_Image *planes);
    int32_t (*shutdown)       (struct EngineData *engine);
    void    (*bltBuffer)      (struct EngineData *engine);

    void    (*loadText)       (struct EngineData *engine,    River2D_Image *image,
                               const  char       *text,      uint8_t       font,
                               uint16_t          charsize,   uint32_t      spacing,
                               uint32_t          offsetY,    uint32_t      offsetX);

    void    (*compositeImage) (struct EngineData *engine,
                               River2D_Image     *image,     uint8_t       pictop,
                               uint32_t          offsetDstX, uint32_t      offsetDstY,
                               uint32_t          offsetSrcX, uint32_t      offsetSrcY,
                               uint32_t          cropWidth,  uint32_t      cropHeight);
}
EngineData;

extern void river2D_loadImage_file
(
    EngineData    *engine,
    char          *path,
    River2D_Image *image,
    uint8_t       channels,
    uint8_t       bitdepth
);

extern void river2D_loadImage_ptr
(
    EngineData    *engine,
    FILE          *file,
    River2D_Image *image,
    uint8_t       channels,
    uint8_t       bitdepth
);

extern void river2D_createImage
(
    EngineData    *engine,
    River2D_Image *image,
    uint32_t      width,
    uint32_t      height
);

extern void river2D_refreshImage
(
    EngineData    *engine,
    River2D_Image *image
);

extern void river2D_clearImage
(
    EngineData    *engine,
    River2D_Image *image
);

extern void river2D_destroyImage
(
    River2D_Image *image
);

extern River2D_Time river2D_queryTime
(
    void
);

extern River2D_Time river2D_deltaTime
(
    const River2D_Time *time
);

extern void river2D_resolveRenderer
(
    EngineData *engine,
    const char *libpath,
    uint8_t    renderer
);

extern uint8_t river2D_verifyPath
(
    const char *path
);

extern const char* river2D_listFiles
(
    const char *path
);

extern uint8_t river2D_interpretCharAsKey
(
    char inp
);

extern void river2D_loadConfig
(
    River2D_Config *config
);

extern const char* river2D_contains
(
    const char *bigStr,
    const char *smallStr
);

extern Dimensions river2D_getWindowSize
(
    EngineData *engine
);

extern void river2D_changeCursor
(
    EngineData    *engine,
    River2D_Image *image
);

extern bool river2D_insideArea
(
    Coordinates *point,
    Area        *area
);

extern bool river2D_insideRect
(
    Coordinates *point,
    Rect        *rect
);

extern void river2D_createButton
(
    EngineData    *engine,
    River2D_Image *img,
    const char    *text,
    uint8_t       font,
    uint16_t      charsize,
    uint32_t      spacing,
    Coordinates   point,
    Rect          *rect
);
