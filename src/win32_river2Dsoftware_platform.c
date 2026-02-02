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

void river2D_init
(
    EngineData         *engine,
    River2D_Image      *planes
){
    river2D_loadConfig(&engine->config);

    engine->width  = engine->config.width;
    engine->height = engine->config.height;

    engine->planes = planes;

    engine->windowName = "river2D editor";

    engine->context = GetDC(engine->window);
    if(!engine->context)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Could not get DC!\033[0m\n");
    }

    if(!(engine->config.choices & RIVER2D_CHOICE_STATIC_CANVAS_BIT))
    {
        //TODO: get from config, worry about scaling
        // river2D_resizeBackbuffer(engine, engine->config.width, engine->config.height);
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
    //TODO: does this not have to happen every update call?
    ReleaseDC(engine->window, engine->context);

    return 0;
}

// void win32ResizeDIBSection
// (
//     Win32OffscreenBuffer *buf,
//     uint32_t             width,
//     uint32_t             height
// ){
//     if(buf->memory)
//     {
//         VirtualFree(buf->memory, 0, MEM_RELEASE);
//     }
//
//     buf->width  = width;
//     buf->height = height;
//
//     buf->info.bmiHeader.biSize        = sizeof(buf->info.bmiHeader);
//     buf->info.bmiHeader.biWidth       = buf->width;
//     buf->info.bmiHeader.biHeight      = buf->height;
//     buf->info.bmiHeader.biPlanes      = 1;
//     buf->info.bmiHeader.biBitCount    = 32;
//     buf->info.bmiHeader.biCompression = BI_RGB;
//
//     uint32_t bitmapMemorySize = buf->width * buf->height * RIVER_BPP;
//
//     buf->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
// }

void river2D_bltBuffer
(
    EngineData *engine
){
    Win32Backbuffer *buf = engine->backbuffer;
    //FIXME: fixup StretchDIBits call or use something else
    StretchDIBits(engine->context, 0, 0, (int)engine->width, (int)engine->height,
                  0, 0, (int)buf->width, (int)buf->height, buf->data, &buf->info, DIB_RGB_COLORS, SRCCOPY);
}
