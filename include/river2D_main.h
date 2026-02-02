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

    #define __USE_POSIX199309
    #include <time.h>

    #include "pthread.h"
    #define  RIVER2D_SCANLINE  8
    #define  RIVER2D_CONFIG_PATH "./.river2Dconf"
#endif

#ifdef BUILD_WINDOWS
    #include "Windows.h"
    #define  RIVER2D_CONFIG_PATH "./river2D.ini"
#endif

#define clang_ignore_unused\
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Wunused-parameter\"") \

#define clang_ignore_functype_mismatch\
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Wcast-function-type-mismatch\"") \

#define clang_diagnostic_pop\
    _Pragma("clang diagnostic pop")\

#define RIVER2D_BPP         4
#define RIVER2D_PIXDEPTH    32
#define RIVER2D_MAX_PLANES  64
#define RIVER2D_MAX_THREADS 8

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

#define RIVER2D_PICTOP_OVER    0
#define RIVER2D_PICTOP_REPLACE 1
#define RIVER2D_PICTOP_MAXIMUM 2

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
    uint32_t width;
    uint32_t height;
}
River2D_Config;

typedef struct River2D_Image
{
    uint8_t  *data;
    uint8_t  channels;
    uint32_t width;
    uint32_t height;
}
River2D_Image;

typedef struct River2D_Time
{
    uint64_t s;
    uint64_t ns;
}
River2D_Time;

typedef struct River2D_ControlMap
{
    uint64_t keymap;
    uint64_t rumble;
    uint8_t  keycodes[64];
    //other general use stuff
}
River2D_ControlMap;

typedef struct PictopData
{
    uint32_t threadHeight;
    uint64_t srcCutoffX;
    uint64_t copyWidth;
    uint64_t bufWidth;
    uint8_t  *src;
    uint8_t  *dest;
}
PictopData;

typedef struct ThreadData
{
    PictopData *data;
    uint32_t   y;
}
ThreadData;

//TODAY: (river2D #3) revise in favour of floats 0.0f - 1.0f
//I think that's less prone to errors than absolute screen coords...
typedef struct Coordinates
{
    uint32_t x;
    uint32_t y;
}
Coordinates;

#ifdef BUILD_LINUX
typedef struct X11Backbuffer
{
    uint8_t  *data;
    uint32_t width;
    uint32_t height;
}
X11Backbuffer;

typedef struct PosixThreadpool
{
    pthread_t  threads[RIVER2D_MAX_THREADS];
    ThreadData *threadData[RIVER2D_MAX_THREADS];
}
PosixThreadpool;
#elif defined(BUILD_WINDOWS)
typedef struct Win32Backbuffer
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
    River2D_Time       lastFrametime;
    River2D_Time       lastFPStime;
    uint16_t           runningFrames;
    Coordinates        playerSheetOffset;
    Coordinates        playerPosition;

#ifdef BUILD_LINUX
    Display            *display;
    Screen             *screen;
    Visual             *visual;
    Window             window;
    GC                 context;
    X11Backbuffer      backbuffer;
    PosixThreadpool    pool;
#endif

#ifdef BUILD_WINDOWS
    HINSTANCE          instance;
    HWND               window;
    HDC                context;
    Win32Backbuffer    backbuffer;
#endif
}
EngineData;

extern void river2D_loadImage
(
    const char    *path,
    River2D_Image *image,
    uint8_t       format,
    uint8_t       bitdepth
);

extern void river2D_destroyImage
(
    River2D_Image *image
);

extern void river2D_queryTime
(
    River2D_Time *time
);

extern uint8_t river2D_verifyPath
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
