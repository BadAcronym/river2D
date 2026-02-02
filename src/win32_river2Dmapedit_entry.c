#include "river2D_main.h"
#include "win32_river2D_platform.h"

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

    win32LoadXInput();

    global_running = true;

    //get size from project settings
    win32ResizeDIBSection(&global_backbuffer, 1280, 720);

    WNDCLASS wc = {0};

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = win32WindowCallback;
    wc.hInstance = instance;
    wc.lpszClassName = "CPongClass";

    if(!RegisterClassA(&wc))
    {
        printf("unable to register window class.");
        return GetLastError();
    };

    int x      = CW_USEDEFAULT;
    int y      = CW_USEDEFAULT;
    int width  = CW_USEDEFAULT;
    int height = CW_USEDEFAULT;

    HWND window = CreateWindowExA(0, wc.lpszClassName, "CPong",
                                  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                  x, y, width, height,
                                  0, 0, instance, 0);

    if(!window)
    {
        printf("unable to obtain window handle.");
        return GetLastError();
    }

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
