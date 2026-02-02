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
    engine->backbuffer.info.bmiHeader.biHeight = (long)engine->backbuffer.height;
    engine->backbuffer.info.bmiHeader.biPlanes = 1;
    engine->backbuffer.info.bmiHeader.biBitCount = 32;
    engine->backbuffer.info.bmiHeader.biCompression = BI_RGB;

    engine->backbuffer.data = VirtualAlloc(0, width * height * RIVER2D_BPP, MEM_COMMIT, PAGE_READWRITE);
    if(!engine->backbuffer.data)
    {
        fprintf(stderr, "\033[31;1;7mERROR: failed to resize backbuffer\033[0m");
    }

    engine->width  = width;
    engine->height = height;
}

void river2D_init
(
    EngineData         *engine,
    River2D_Image      *planes
){
    river2D_loadConfig(&engine->config);

    engine->width  = engine->config.width;
    engine->height = engine->config.height;

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
    //TODO: does this not have to happen every update call?
    // ReleaseDC(engine->window, engine->context);

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

    StretchDIBits(engine->context, 0, 0, width, height, 0, 0, (int)engine->width, (int)engine->height,
                  engine->backbuffer.data, &engine->backbuffer.info, DIB_RGB_COLORS, SRCCOPY);

    river2D_queryTime(&engine->lastFrametime);
}

//TODAY: add graceful handling of buffer size
void river2D_compositeImage
(
    EngineData    *engine,
    River2D_Image *image,
    uint8_t       pictop
){
    //TODO: deal with alpha and actual compositing instead of just overlaying/copying
    if(pictop != RIVER2D_PICTOP_OVER)
    {
        fprintf(stderr, "\033[33;1;7mSORRY: only RIVER2D_PICTOP_OVER implemented for now. :/\033[0m\n");
        return;
    }

    if(!image)
    {
        fprintf(stderr, "\033[31;1;7mERROR: no image to composite with.\033[0m\n");
        return;
    }
    if(!image->data)
    {
        fprintf(stderr, "\033[31;1;7mERROR: image->data is nullptr.\033[0m\n");
        return;
    }

    if(!engine->backbuffer.data)
    {
        fprintf(stderr, "\033[31;1;7mERROR: no image to composite onto.\033[0m\n");
        return;
    }

    //TODO: verify that both images are actually RGBA (in other words, that there's enough space)

    uint8_t *dest = (uint8_t*)engine->backbuffer.data;
    uint64_t copyWidth = image->width * RIVER2D_BPP;
    uint64_t bufWidth  = engine->width * RIVER2D_BPP;

    for(uint32_t y = 0; y < image->height; ++y)
    {
        for(uint32_t x = 0; x < copyWidth; x += RIVER2D_BPP)
        {
            uint64_t srcIndex  = y * copyWidth + x;
            uint64_t dstIndex  = ((image->height - y - 1) * bufWidth) + x;
            if(image->data[srcIndex + 3])
            {
                dest[dstIndex]     = image->data[srcIndex];
                dest[dstIndex + 1] = image->data[srcIndex + 1];
                dest[dstIndex + 2] = image->data[srcIndex + 2];
                dest[dstIndex + 3] = image->data[srcIndex + 3];
            }
        }
    }
}
