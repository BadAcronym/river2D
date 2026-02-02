#pragma once

#include <stdbool.h>
#include <stdint.h>

#define persistent  static
#define global      static
#define internal    static

#ifdef BUILD_LINUX
    #include "X11/Xlib.h"
    #include "X11/extensions/Xrender.h"
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

#define RIVER2D_BPP        4
#define RIVER2D_PIXDEPTH   32
#define RIVER2D_MAX_PLANES 64

//X keycodes
//TODO: read from config file and translate to X, Win32 or whatever
//
//find a way to define at runtime and just load a set of keycodes
#define RIVER2D_KEY_UP     25
#define RIVER2D_KEY_LEFT   27
#define RIVER2D_KEY_RIGHT  28
#define RIVER2D_KEY_DOWN   39
#define RIVER2D_KEY_TAB    23
#define RIVER2D_KEY_ESCAPE 99

//thought: could we be wasting cache here? is 1 << n somehow more efficient?
#define RIVER2D_BIT_UP     0b000001
#define RIVER2D_BIT_DOWN   0b000010
#define RIVER2D_BIT_LEFT   0b000100
#define RIVER2D_BIT_RIGHT  0b001000
#define RIVER2D_BIT_TAB    0b010000
#define RIVER2D_BIT_ESCAPE 0b100000

#define RIVER2D_CHANNELS_RGBA 0
#define RIVER2D_CHANNELS_BGRA 1
#define RIVER2D_CHANNELS_RGB  2
#define RIVER2D_CHANNELS_BGR  3
#define RIVER2D_CHANNELS_MAX  3

#define RIVER2D_FONT_DEFAULT 0
#define RIVER2D_FONT_MAX     0

#define RIVER2D_TYPE_FILE      0
#define RIVER2D_TYPE_DIRECTORY 1
#define RIVER2D_TYPE_ERROR     2
#define RIVER2D_TYPE_OTHER     3
#define RIVER2D_TYPE_MAX       3

//probably only needed for Xrender
#define RIVER2D_PICTOP_CLEAR		0
#define RIVER2D_PICTOP_SRC			1
#define RIVER2D_PICTOP_DST			2
#define RIVER2D_PICTOP_OVER			3
#define RIVER2D_PICTOP_OVERREVERSE	4
#define RIVER2D_PICTOP_IN			5
#define RIVER2D_PICTOP_INREVERSE	6
#define RIVER2D_PICTOP_OUT			7
#define RIVER2D_PICTOP_OUTREVERSE	8
#define RIVER2D_PICTOP_ATOP			9
#define RIVER2D_PICTOP_ATOPREVERSE	10
#define RIVER2D_PICTOP_XOR			11
#define RIVER2D_PICTOP_ADD			12
#define RIVER2D_PICTOP_SATURATE		13
#define RIVER2D_PICTOP_MAXIMUM		13

typedef struct PerformanceCounter
{
    uint64_t time;
    uint64_t freq;
}
PerformanceCounter;

#define RIVER2D_CHOICE_SHOW_FPS_BIT      0b00000000000000000000000000000001
#define RIVER2D_CHOICE_STATIC_CANVAS_BIT 0b00000000000000000000000000000010
#define RIVER2D_CHOICE_BACKGROUNDS_BYTE  0b11111111000000000000000000000000

typedef struct River2D_Config
{
    uint32_t choices;
    uint8_t  renderer;
    uint8_t  backgrounds;
    uint32_t width;
    uint32_t height;
}
River2D_Config;

//TODO: give images some sort of parallax option
typedef struct River2D_Image
{
    uint8_t     *data;
    uint8_t     channels;
    uint32_t    width;
    uint32_t    height;
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
    //others, if more than 64 bits are needed
    //
    //later add velocities for gamepads
}
River2D_ControlMap;

typedef struct EngineData
{
    uint32_t           width;
    uint32_t           height;
    const char         *windowName;
    River2D_ControlMap controls;
    River2D_Config     config;
    River2D_Image      *planes;
    River2D_Time       lastFrametime;
    River2D_Time       lastFPStime;
    uint16_t           runningFrames;

#ifdef BUILD_LINUX
    Display            *display;
    Screen             *screen;
    XRenderPictFormat  *format;
    Visual             *visual;
    Window             window;
    GC                 context;
    Pixmap             backbuffer;
    Pixmap             compBuffer;
#endif

#ifdef BUILD_WINDOWS
    HINSTANCE          instance;
    HWND               window;
    HDC                context;
#endif
}
EngineData;

extern void river2D_loadImage
(
    const char    *path,
    River2D_Image *image,
    uint8_t       format,
    uint8_t       depth
);

extern void river2D_destroyImage
(
    River2D_Image *image
);

extern void river2D_compositeImage
(
    EngineData    *engine,
    River2D_Image *image,
    uint8_t       pictop
);

extern void river2D_loadText
(
    EngineData    *engine,
    River2D_Image *image,
    const char    *text,
    uint8_t       font,
    uint16_t      charsize,
    uint32_t      spacing,
    uint32_t      offsetY,
    uint32_t      offsetX
);

extern void river2D_queryTime
(
    River2D_Time *time
);

extern uint8_t river2D_verifyPath
(
    const char *path
);

extern void river2D_loadConfig
(
    River2D_Config *config
);

extern void river2D_init
(
    EngineData         *engine,
    River2D_Image      *planes
);

extern int32_t river2D_shutdown
(
    EngineData *engine
);

extern const char* river2D_contains
(
    const char *bigStr,
    const char *smallStr
);
