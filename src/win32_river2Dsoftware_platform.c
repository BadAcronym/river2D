#include "river2D_main.h"

#include "win32_river2Dsoftware_platform.h"

#include <stdio.h>

//TODO: move to win32 input
// clang_ignore_unused
//
// #define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
// typedef X_INPUT_GET_STATE(x_input_get_state);
// X_INPUT_GET_STATE(XInputGetState_Stub)
// {
//     return 0;
// }
// global x_input_get_state *XInputGetState_ = XInputGetState_Stub;
// #define XInputGetState XInputGetState_
//
// #define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
// typedef X_INPUT_SET_STATE(x_input_set_state);
// X_INPUT_SET_STATE(XInputSetState_Stub)
// {
//     return 0;
// }
// global x_input_set_state *XInputSetState_ = XInputSetState_Stub;
// #define XInputSetState XInputSetState_
//
// clang_diagnostic_pop
//
// void win32LoadXInput(void)
// {
//     HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
//     if(!XInputLibrary)
//     {
//         XInputLibrary = LoadLibraryA("xinput1_3.dll");
//     }
//
//     if(XInputLibrary)
//     {
//         clang_ignore_functype_mismatch
//
//         XInputGetState = (x_input_get_state*)GetProcAddress(XInputLibrary, "XInputGetState");
//         XInputSetState = (x_input_set_state*)GetProcAddress(XInputLibrary, "XInputSetState");
//
//         clang_diagnostic_pop
//     }
// }

void river2D_openWindow
(
    EngineData *engine
){
    WNDCLASS wc = {0};

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = win32WindowCallback;
    wc.hInstance = engine->instance;
    wc.lpszClassName = "River2DClass";

    if(!RegisterClassA(&wc))
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Unable to register window class!\033[0m\n");
    };

    int x      = CW_USEDEFAULT;
    int y      = CW_USEDEFAULT;
    int width  = CW_USEDEFAULT;
    int height = CW_USEDEFAULT;

    engine->window = CreateWindowExA(0, wc.lpszClassName, "River2D",
                                     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                     x, y, width, height,
                                     0, 0, instance, 0);
}

void river2D_init
(
    EngineData         *engine,
    River2D_Image      *planes
){
    river2D_loadConfig(&engine->config);

    engine->width  = engine->config.width;
    engine->height = engine->config.height;

    engine->planes   = planes;

    engine->windowName = "river2D editor";

    engine->window = river2D_openWindow(engine);
    if(!engine->window)
    {
        fprintf(stderr, "Failed to create window!\n");
    }

    // engine->context = ;
    // if(!engine->context)
    // {
    //     fprintf(stderr, "Failed to create Graphics Context!\n");
    // }

    if(!(engine->config.choices & RIVER2D_CHOICE_STATIC_CANVAS_BIT))
    {
        //TODO: get from config, worry about scaling
        river2D_resizeBackbuffer(engine, engine->config.width, engine->config.height);
    }

    River2D_Time time;
    river2D_queryTime(&time);
    engine->lastFrametime = time;
    engine->lastFPStime = time;
}

void win32ResizeDIBSection
(
    Win32OffscreenBuffer *buf,
    uint32_t             width,
    uint32_t             height
){
    if(buf->memory)
    {
        VirtualFree(buf->memory, 0, MEM_RELEASE);
    }

    buf->width  = width;
    buf->height = height;

    buf->info.bmiHeader.biSize        = sizeof(buf->info.bmiHeader);
    buf->info.bmiHeader.biWidth       = buf->width;
    buf->info.bmiHeader.biHeight      = buf->height;
    buf->info.bmiHeader.biPlanes      = 1;
    buf->info.bmiHeader.biBitCount    = 32;
    buf->info.bmiHeader.biCompression = BI_RGB;

    uint32_t bitmapMemorySize = buf->width * buf->height * RIVER_BPP;

    buf->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

void win32BltBuf
(
    Win32OffscreenBuffer *buf,
    HDC                  deviceContext,
    uint32_t             width,
    uint32_t             height
){
    StretchDIBits(deviceContext,
                  0, 0, width, height,
                  0, 0, buf->width, buf->height,
                  buf->memory, &buf->info,
                  DIB_RGB_COLORS, SRCCOPY);
}
