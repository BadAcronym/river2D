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
}PerformanceCounter;
