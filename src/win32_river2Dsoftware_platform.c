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

void river2D_resizeBackbuffer
(
    EngineData *engine,
    uint32_t   width,
    uint32_t   height
){
    if(engine->backbuffer.data)
    {
        VirtualFree(engine->backbuffer.data, 0, MEM_RELEASE);
    }

    engine->backbuffer.width  = width;
    engine->backbuffer.height = height;

    engine->backbuffer.info.bmiHeader.biSize   = sizeof(engine->backbuffer.info.bmiHeader);
    engine->backbuffer.info.bmiHeader.biWidth  = (long)engine->backbuffer.width;
    engine->backbuffer.info.bmiHeader.biHeight = -(long)engine->backbuffer.height;
    engine->backbuffer.info.bmiHeader.biPlanes = 1;
    engine->backbuffer.info.bmiHeader.biBitCount = 32;
    engine->backbuffer.info.bmiHeader.biCompression = BI_RGB;

    engine->backbuffer.data = VirtualAlloc(0, width * height * RIVER2D_BPP, MEM_COMMIT, PAGE_READWRITE);
    if(!engine->backbuffer.data)
    {
        fprintf(stderr, "\033[31;1;7mERROR: failed to resize backbuffer\033[0m");
    }
}

void river2D_init
(
    EngineData         *engine,
    River2D_Image      *planes
){
    river2D_loadConfig(&engine->config);

    engine->planes = planes;

    engine->windowName = "unnamed river2D application";

    if(!(engine->config.choices & RIVER2D_CHOICE_STATIC_CANVAS_BIT))
    {
        river2D_resizeBackbuffer(engine, engine->config.width, engine->config.height);
    }

    River2D_Time time;
    river2D_queryTime(&time);
    engine->lastFrametime = time;
    engine->lastFPStime = time;
}

int32_t river2D_shutdown
(
    EngineData *engine
){
    //stuff
    DestroyWindow(engine->window);

    return 0;
}

void river2D_bltBuffer
(
    EngineData *engine
){
    RECT clientRect;
    GetClientRect(engine->window, &clientRect);
    int width  = clientRect.right  - clientRect.left;
    int height = clientRect.bottom - clientRect.top;

    StretchDIBits(engine->context, 0, 0, width, height, 0, 0,
                  (int)engine->backbuffer.width, (int)engine->backbuffer.height,
                  engine->backbuffer.data, &engine->backbuffer.info, DIB_RGB_COLORS, SRCCOPY);

    river2D_queryTime(&engine->lastFrametime);
}
