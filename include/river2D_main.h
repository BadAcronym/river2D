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
//the bit macros stay, though
#define River2D_KEY_UP     41
#define River2D_KEY_LEFT   27
#define River2D_KEY_RIGHT  28
#define River2D_KEY_DOWN   39
#define River2D_KEY_TAB    23
#define River2D_KEY_ESCAPE 99

#define River2D_BIT_UP     0b000001
#define River2D_BIT_DOWN   0b000010
#define River2D_BIT_LEFT   0b000100
#define River2D_BIT_RIGHT  0b001000
#define River2D_BIT_TAB    0b010000
#define River2D_BIT_ESCAPE 0b100000

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

typedef struct River2DControlMap
{
    uint64_t keymap;
    //others, if more than 64 bits are needed
}
River2DControlMap;

void river2D_processControls
(
    bool              isDown,
    int32_t           key,
    River2DControlMap *controls
);
