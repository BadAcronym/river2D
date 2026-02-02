#include "river2D_main.h"
#include "win32_river2Dsoftware_platform.h"

#include <stdio.h>

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

    EngineData    engine = {0};
    River2D_Image planes[RIVER2D_MAX_PLANES] = {0};

    engine->instance = instance;

    river2D_init(&engine, planes);

    bool running = true;

    if(!engine->window)
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

        // resetRumble(&global_paddles);

        // for(DWORD controlIndex = 0; controlIndex < XUSER_MAX_COUNT; ++controlIndex)
        // {
        //     XINPUT_STATE controlState;
        //     if(XInputGetState(controlIndex, &controlState) == ERROR_SUCCESS)
        //     {
        //         XINPUT_GAMEPAD *pad = &controlState.Gamepad;
        //         global_controllerMap.player1_up   = pad->wButtons & XINPUT_GAMEPAD_DPAD_UP ||
        //                                             pad->sThumbLY > CPONG_DEADZONE;
        //
        //         global_controllerMap.player1_down = pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN ||
        //                                             pad->sThumbLY < -CPONG_DEADZONE;
        //
        //         global_controllerMap.player2_up   = pad->wButtons & XINPUT_GAMEPAD_Y ||
        //                                             pad->sThumbRY > CPONG_DEADZONE;
        //
        //         global_controllerMap.player2_down = pad->wButtons & XINPUT_GAMEPAD_A ||
        //                                             pad->sThumbRY < -CPONG_DEADZONE;
        //
        //     }
        // }

        mapedit_update();

        river2D_bltBuffer();
    }

    return river2D_shutdown(&engine);
}
clang_diagnostic_pop
