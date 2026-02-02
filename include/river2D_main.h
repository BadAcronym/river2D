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

//TODO: declare all basic, abstracted functions
//and define them outside, built against the correct platform layer
extern void updateBitmap
(
);

extern uint8_t* getBitmapAddress
(
);
