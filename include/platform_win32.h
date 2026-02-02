#pragma once

#include <Windows.h>
#include <Xinput.h>

#include <stdint.h>

typedef struct Win32WindowDimensions
{
    uint32_t width;
    uint32_t height;
}
Win32WindowDimensions;

typedef struct Win32OffscreenBuffer
{
    BITMAPINFO info;
    void       *memory;
    uint32_t   width;
    uint32_t   height;
}
Win32OffscreenBuffer;

typedef struct
{
    uint64_t time;
    uint64_t freq;
}Time;

extern void win32LoadXInput(void);
