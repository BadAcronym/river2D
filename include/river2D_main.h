#pragma once

#include <stdbool.h>
#include <stdint.h>

#define persistent  static
#define global      static
#define internal    static

#define clang_ignore_unused\
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Wunused-parameter\"") \

#define clang_ignore_functype_mismatch\
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Wcast-function-type-mismatch\"") \

#define clang_diagnostic_pop\
    _Pragma("clang diagnostic pop")\

#define RIVER_BPP 4

//X keycodes
//TODO: read from config file and translate to X, Win32 or whatever
//find a way to define at runtime and just load a set of keycodes
#define River2D_KEY_UP    41
#define River2D_KEY_LEFT  27
#define River2D_KEY_RIGHT 28
#define River2D_KEY_DOWN  39

#define River2D_KEY_TAB   23

#define River2D_DIR_UP    0b0001
#define River2D_DIR_DOWN  0b0010
#define River2D_DIR_LEFT  0b0100
#define River2D_DIR_RIGHT 0b1000

typedef struct Dimensions
{
    uint32_t width;
    uint32_t height;
}
Dimensions;

typedef struct PerformanceCounter
{
    uint64_t time;
    uint64_t freq;
}
PerformanceCounter;

//the reason we're even using a uint8_t is so that we can have diagonal
//movement in the editor even though you might not want it in the game.
typedef struct River2DControlMap
{
    bool    mouseClick_left;
    bool    mouseClick_right;
    uint8_t direction;
    bool    tab;
}
River2DControlMap;

void river2D_processControls
(
    bool              isDown,
    int32_t           key,
    River2DControlMap *controls
);
