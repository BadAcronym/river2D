#pragma once

#include <Windows.h>
#include <Xinput.h>

#include <stdint.h>

#define openWindow         win32openWindow
#define drawFrame          win32drawFrame
#define allocateBackbuffer win32allocateBackbuffer
#define updateBackbuffer   win32updateBackbuffer
#define BltBuffer          win32bltBuffer
#define queryTime          win32queryTime

typedef struct Win32WindowDimensions
{
    uint32_t width;
    uint32_t height;
}
Win32WindowDimensions;

typedef struct Win32Backbuffer
{
    BITMAPINFO info;
    void       *memory;
    uint32_t   width;
    uint32_t   height;
}
Win32Backbuffer;

extern void win32LoadXInput(void);

extern Time win32QueryTime(void)
