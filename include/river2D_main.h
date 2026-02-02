#pragma once

#include <stdbool.h>
#include <stdint.h>

#define persistent  static
#define global      static
#define internal    static

#ifdef BUILD_LINUX
    #include "X11/Xlib.h"
    #include "X11/extensions/Xrender.h"
    #define RIVER2D_SCANLINE  8
#endif

#ifdef BUILD_WINDOWS
    #include "Windows.h"
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

//probably only needed for Xrender
#define RIVER2D_PICTOP_CLEAR			0
#define RIVER2D_PICTOP_SRC			    1
#define RIVER2D_PICTOP_DST			    2
#define RIVER2D_PICTOP_OVER			    3
#define RIVER2D_PICTOP_OVERREVERSE		4
#define RIVER2D_PICTOP_IN			    5
#define RIVER2D_PICTOP_INREVERSE		6
#define RIVER2D_PICTOP_OUT			    7
#define RIVER2D_PICTOP_OUTREVERSE		8
#define RIVER2D_PICTOP_ATOP			    9
#define RIVER2D_PICTOP_ATOPREVERSE		10
#define RIVER2D_PICTOP_XOR			    11
#define RIVER2D_PICTOP_ADD			    12
#define RIVER2D_PICTOP_SATURATE			13
#define RIVER2D_PICTOP_MAXIMUM			13

typedef struct PerformanceCounter
{
    uint64_t time;
    uint64_t freq;
}
PerformanceCounter;

typedef struct River2D_Config
{
    uint32_t width;
    uint32_t height;
    bool     static_canvas_enable;
    // uint32_t static_canvas_width;
    // uint32_t static_canvas_height;

    uint8_t  backgrounds;

    //choose renderer here
}
River2D_Config;

typedef struct River2D_Image
{
    uint8_t*    data;
    //TODO: make format enum or ID
    char*       format;
    uint32_t    width;
    uint32_t    height;
}
River2D_Image;

typedef struct River2D_Time
{
    uint64_t s;
    uint64_t ms;
    uint64_t us;
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
    bool               running;
    River2D_ControlMap controls;
    uint32_t           width;
    uint32_t           height;
    const char*        windowName;
    River2D_Config     config;
    River2D_Image      planes[RIVER2D_MAX_PLANES];
    River2D_Time       lastFrametime;
    River2D_Time       lastFPStime;
    uint32_t           runningFrames;

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
    ///
#endif
}
EngineData;

extern void river2D_processControls
(
    bool              isDown,
    int32_t           key,
    River2D_ControlMap *controls
);

extern void river2D_updateEditor
(
);

extern void river2D_loadImage
(
    const char*   path,
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

extern River2D_Time river2D_queryTime
(
    void
);
