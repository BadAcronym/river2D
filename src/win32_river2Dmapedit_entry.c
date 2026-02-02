#include "river2D_main.h"
#include "river2Dmapedit_main.h"

#include <stdio.h>

global bool global_running = true;
global EngineData *global_engine;

clang_ignore_unused

#define RIVER2D_INIT(name) void name(EngineData *engine, River2D_Image *planes)
typedef RIVER2D_INIT(river2D_init_);
RIVER2D_INIT(River2D_init_Stub)
{
    return;
}
global river2D_init_ *_river2D_init_ = River2D_init_Stub;
#define river2D_init _river2D_init_

#define RIVER2D_SHUT(name) int32_t name(EngineData *engine)
typedef RIVER2D_SHUT(river2D_shut_);
RIVER2D_SHUT(River2D_shut_Stub)
{
    return -1;
}
global river2D_shut_ *_river2D_shut_ = River2D_shut_Stub;
#define river2D_shutdown _river2D_shut_

#define RIVER2D_BLT(name) void name(EngineData *engine)
typedef RIVER2D_BLT(river2D_blt_);
RIVER2D_BLT(River2D_blt_Stub)
{
    return;
}
global river2D_blt_ *_river2D_blt_ = River2D_blt_Stub;
#define river2D_bltBuffer _river2D_blt_

clang_diagnostic_pop

#ifdef DEBUG
    #define LIBPATH "./bin/debug/"
#else
    #define LIBPATH "./bin/release/"
#endif

internal void loadRenderer_software
(
    void
){
    SetDllDirectoryA(LIBPATH);
    HMODULE software = LoadLibraryA("river2Dsoftware.dll");
    if(!software)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Unable to load software renderer!\n");
        fprintf(stderr, "Tried to load from library path:" LIBPATH);
        fprintf(stderr, "\033[0m\n");
        return;
    }

    river2D_init      = (river2D_init_*)GetProcAddress(software, "river2D_init");
    if(!river2D_init)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Unable to load symbol river2D_init!\033[0m\n");
        return;
    }
    river2D_shutdown  = (river2D_shut_*)GetProcAddress(software, "river2D_shutdown");
    if(!river2D_shutdown)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Unable to load symbol river2D_shutdown!\033[0m\n");
        return;
    }
    river2D_bltBuffer =  (river2D_blt_*)GetProcAddress(software, "river2D_bltBuffer");
    if(!river2D_bltBuffer)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Unable to load symbol river2D_bltBuffer!\033[0m\n");
        return;
    }
}

LRESULT CALLBACK win32WindowCallback
(
    HWND   window,
    UINT   message,
    WPARAM wParam,
    LPARAM lParam
){
    switch(message)
    {
        case WM_DESTROY:
        {
            printf("WM_DESTROY\n");
            break;
        }
        case WM_CLOSE:
        {
            printf("WM_CLOSE\n");
            global_running = false;
            break;
        }
        case WM_ACTIVATEAPP:
        {
            printf("WM_ACTIVATEAPP\n");
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT paintStruct;
            HDC context = BeginPaint(window, &paintStruct);

            river2D_bltBuffer(global_engine);

            EndPaint(window, &paintStruct);
            break;
        }
        case WM_KEYDOWN:
        {
        }
        case WM_KEYUP:
        {
            bool wasKeyDown = (lParam & (1 << 30)) != 0;
            bool isKeyDown  = (lParam & (1 << 31)) == 0;

            if(wasKeyDown == isKeyDown)
            {
                break;
            }

            //TODO: lata
            // if(wParam == PLAYER1_UP)
            // {
            //     global_keyMap.player1_up = isKeyDown;
            // }
            // else if(wParam == PLAYER1_DOWN)
            // {
            //     global_keyMap.player1_down = isKeyDown;
            // }
            // else if(wParam == PLAYER2_UP)
            // {
            //     global_keyMap.player2_up = isKeyDown;
            // }
            // else if(wParam == PLAYER2_DOWN)
            // {
            //     global_keyMap.player2_down = isKeyDown;
            // }
        }
        default:
        {
            return DefWindowProcA(window, message, wParam, lParam);
        }
    }

    return 0;
}

int CALLBACK WinMain
(
    HINSTANCE instance,
    HINSTANCE prevInstance,
    LPSTR     cmdline,
    int       cmdShow
){
    //silence MSVC, I just plain don't need these params but
    //HAVE to specify them in order for WinMain to be called
    (void)cmdShow;
    (void)cmdline;
    (void)prevInstance;

    ShowCursor(false);

    //TODO: fixup when actually loading input
    // win32LoadXInput();

    EngineData    engine = {0};
    global_engine = &engine;
    River2D_Image planes[RIVER2D_MAX_PLANES] = {0};

    loadRenderer_software();

    engine.instance = instance;

    river2D_init(&engine, planes);

    WNDCLASSA wc = {0};
    LPCSTR className = "River2DClass";

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = win32WindowCallback;
    wc.hInstance = engine.instance;
    wc.lpszClassName = className;

    if(!RegisterClassA(&wc))
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Unable to register window class!\033[0m\n");
    };

    int x      = CW_USEDEFAULT;
    int y      = CW_USEDEFAULT;
    int width  = CW_USEDEFAULT;
    int height = CW_USEDEFAULT;

    engine.window = CreateWindowExA(0, wc.lpszClassName, "River2D",
                                    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                    x, y, width, height,
                                    0, 0, instance, 0);

    while(global_running)
    {
        MSG message;

        while(PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
        {
            if(message.message == WM_QUIT)
            {
                global_running = false;
                break;
            }

            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        mapedit_update();
        river2D_bltBuffer(&engine);
    }

    return river2D_shutdown(&engine);
}

#ifdef DEBUG
int main
(
    void
){
    return WinMain(GetModuleHandleA(0), 0, GetCommandLineA(), 0);
}
#endif
