#include "river2D_main.h"
#include "win32_river2Dsoftware_platform.h"

#ifdef DEBUG
int main()
{
    return WinMain(GetModuleHandleA(0), 0, GetCommandLineA(), 0);
}
#endif

clang_ignore_unused
int CALLBACK WinMain
(
    HINSTANCE instance,
    HINSTANCE prevInstance,
    LPSTR     cmdline,
    int       cmdShow
){
    ShowCursor(false);

    //TODO: fixup when actually loading input
    // win32LoadXInput();

    bool running = true;

    //TODAY: get size from project settings
    win32ResizeDIBSection(&global_backbuffer, 1280, 720);

    WNDCLASS wc = {0};

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = win32WindowCallback;
    wc.hInstance = instance;
    wc.lpszClassName = "River2DClass";

    if(!RegisterClassA(&wc))
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Unable to register window class!\033[0m\n");
        return GetLastError();
    };

    int x      = CW_USEDEFAULT;
    int y      = CW_USEDEFAULT;
    int width  = CW_USEDEFAULT;
    int height = CW_USEDEFAULT;

    HWND window = CreateWindowExA(0, wc.lpszClassName, "River2D",
                                  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                  x, y, width, height,
                                  0, 0, instance, 0);

    if(!window)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Unable to obtain window handle!\033[0m\n");
        return GetLastError();
    }

    while(running)
    {
        MSG message;

        while(PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
        {
            if(message.message == WM_QUIT)
            {
                running = false;
                break;
            }

            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        resetRumble(&global_paddles);

        for(DWORD controlIndex = 0; controlIndex < XUSER_MAX_COUNT; ++controlIndex)
        {
            XINPUT_STATE controlState;
            if(XInputGetState(controlIndex, &controlState) == ERROR_SUCCESS)
            {
                XINPUT_GAMEPAD *pad = &controlState.Gamepad;
                global_controllerMap.player1_up   = pad->wButtons & XINPUT_GAMEPAD_DPAD_UP ||
                                                    pad->sThumbLY > CPONG_DEADZONE;

                global_controllerMap.player1_down = pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN ||
                                                    pad->sThumbLY < -CPONG_DEADZONE;

                global_controllerMap.player2_up   = pad->wButtons & XINPUT_GAMEPAD_Y ||
                                                    pad->sThumbRY > CPONG_DEADZONE;

                global_controllerMap.player2_down = pad->wButtons & XINPUT_GAMEPAD_A ||
                                                    pad->sThumbRY < -CPONG_DEADZONE;

            }
        }

        updatePaddles(global_backbuffer.height, &global_paddles);
        updateBall(global_backbuffer.width, global_backbuffer.height, &global_paddles, &ball);

        updateBackbuffer(&global_backbuffer, &global_paddles, &ball);

        HDC context = GetDC(window);

        Win32WindowDimensions dim = win32GetWindowDimensions(window);

        win32BltBuf(&global_backbuffer, context, dim.width, dim.height);

        ReleaseDC(window, context);
    }

    return 0;
}
clang_diagnostic_pop
