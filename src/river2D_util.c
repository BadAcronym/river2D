#include "river2D_main.h"

#include <stdint.h>
#include <stdio.h>

#define bufsize 32

void rvLoadConfig
(
    RiverConfig *config
){
    if(!config)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: *config is nullptr.\033[0m\n");
        return;
    }
    uint32_t parsedWidth_canvas  = 0;
    uint32_t parsedHeight_canvas = 0;
    uint32_t parsedWidth_window  = 0;
    uint32_t parsedHeight_window = 0;

    StringView codePath = cstr_sv(RV_CONFIG_PATH);
    uint8_t    code     = pdVerifyPath(codePath);

    if(code == PD_TYPE_FILE)
    {
        FILE *file = fopen(RV_CONFIG_PATH, "r");
        if(!file)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: Could not open file %s\033[0m\n",
                    RV_CONFIG_PATH);
            return;
        }

        char buf[bufsize];
        while(fgets(buf, bufsize, file))
        {
            StringView buffer;
            buffer.data = buf;
            buffer.size = bufsize;

            StringView FPS_sv  = cstr_sv("showFPS");
            const char* fpsloc = sv_find(FPS_sv, buffer);
            if(fpsloc)
            {
                bool foundShowFps = *(fpsloc + 9) == '1' || *(fpsloc + 9) == 't';
                if(!foundShowFps)
                {
                    continue;
                }
                config->choices |= RV_CHOICE_SHOW_FPS_BIT;
                #ifdef DEBUG
                fprintf(stderr, "parsed result: true\n");
                #endif
                continue;
            }

            StringView cwidth_sv  = cstr_sv("canvas_width");
            const char* cwidthloc = sv_find(cwidth_sv, buffer);
            if(cwidthloc)
            {
                for(uint32_t i = 0; i < bufsize; ++i)
                {
                    if(cwidthloc + 14 + i - buf > bufsize)
                    {
                        break;
                    }

                    char digit = *(cwidthloc + 14 + i);
                    if(digit < 0x30 || digit > 0x39)
                    {
                        break;
                    }
                    parsedWidth_canvas *= 10;
                    parsedWidth_canvas += (uint32_t)(digit - 0x30);
                }

                #ifdef DEBUG
                fprintf(stderr, "parsed result: %u\n", parsedWidth_canvas);
                #endif

                config->canvas_width = parsedWidth_canvas;
                continue;
            }

            StringView cheight_sv  = cstr_sv("canvas_height");
            const char* cheightloc = sv_find(cheight_sv, buffer);
            if(cheightloc)
            {
                for(uint32_t i = 0; i < bufsize; ++i)
                {
                    if(cheightloc + 15 + i - buf > bufsize)
                    {
                        break;
                    }

                    char digit = *(cheightloc + 15 + i);
                    if(digit < 0x30 || digit > 0x39)
                    {
                        break;
                    }
                    parsedHeight_canvas *= 10;
                    parsedHeight_canvas += (uint32_t)(digit - 0x30);
                }

                #ifdef DEBUG
                fprintf(stderr, "parsed result: %u\n", parsedHeight_canvas);
                #endif

                config->canvas_height = parsedHeight_canvas;
                continue;
            }

            StringView wwidth_sv  = cstr_sv("window_width");
            const char* wwidthloc = sv_find(wwidth_sv, buffer);
            if(wwidthloc && (wwidthloc + 14 - buf) < bufsize)
            {
                for(uint32_t i = 0; i < bufsize; ++i)
                {
                    if(wwidthloc + 14 + i - buf > bufsize)
                    {
                        break;
                    }

                    char digit = *(wwidthloc + 14 + i);
                    if(digit < 0x30 || digit > 0x39)
                    {
                        break;
                    }
                    parsedWidth_window *= 10;
                    parsedWidth_window += (uint32_t)(digit - 0x30);
                }

                #ifdef DEBUG
                fprintf(stderr, "parsed result: %u\n", parsedWidth_window);
                #endif

                config->window_width = parsedWidth_window;
                continue;
            }

            StringView wheight_sv  = cstr_sv("window_height");
            const char* wheightloc = sv_find(wheight_sv, buffer);
            if(wheightloc && (wheightloc + 15 - buf) < bufsize)
            {
                for(uint32_t i = 0; i < bufsize; ++i)
                {
                    if(wheightloc + 15 + i - buf > bufsize)
                    {
                        break;
                    }

                    char digit = *(wheightloc + 15 + i);
                    if(digit < 0x30 || digit > 0x39)
                    {
                        break;
                    }
                    parsedHeight_window *= 10;
                    parsedHeight_window += (uint32_t)(digit - 0x30);
                }

                #ifdef DEBUG
                fprintf(stderr, "parsed result: %u\n", parsedHeight_window);
                #endif

                config->window_height = parsedHeight_window;
                continue;
            }
        }

        fclose(file);
    }
    else if(code == PD_TYPE_ERROR)
    {
        fprintf(stderr, "\nCan't find the file '%s', default config loaded.\n\n",
                RV_CONFIG_PATH);
    }
    else if(code == PD_TYPE_DIRECTORY)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: '%s' is a Directory! "
                "Default config loaded.\033[0m\n", RV_CONFIG_PATH);
    }
    else if(code == PD_TYPE_OTHER)
    {
        fprintf(stderr, "\nUnknown filetype for '%s', default config loaded.\n\n",
                RV_CONFIG_PATH);
    }

    if(!parsedWidth_canvas)
    {
        config->canvas_width  = 1280;
    }
    if(!parsedHeight_canvas)
    {
        config->canvas_height = 720;
    }
    if(!parsedWidth_window)
    {
        config->window_width  = 2560;
    }
    if(!parsedHeight_window)
    {
        config->window_height = 1440;
    }
}

// handle non-parallel cases
// bool rvInsideArea
// (
//     const Coordinates *point,
//     const Area        *area
// ){
//     return(point->x > area->upLeft.x && point->x < area->upRight.x &&
//            point->y > area->upLeft.y && point->y < area->lowRight.y);
// }

bool rvInsideRect
(
    const Coordinates *point,
    const Rect        *rect
){
    return(point->x > rect->upLeft.x && point->x < rect->lowRight.x &&
           point->y > rect->upLeft.y && point->y < rect->lowRight.y);
}

void rvCreateButton
(
    EngineData       *engine,
    rvButtonSettings *settings
){
    float imgWidth  = (float)settings->img->width;
    float imgHeight = (float)settings->img->height;

    float floatWidth  = (float)settings->name->size *
                        (float)(settings->charsize + settings->spacing) / imgWidth;
    float floatHeight = (float)settings->charsize / imgHeight;

    float offsetX = (settings->point.x - floatWidth / 2.0f) *
                    (float)imgWidth;
    float offsetY = (settings->point.y - floatHeight / 2.0f) *
                    (float)imgHeight;

    if(settings->alignment == RV_ALIGN_TOPLEFT   ||
       settings->alignment == RV_ALIGN_TOPCENTER ||
       settings->alignment == RV_ALIGN_TOPRIGHT
    ){
        offsetY = settings->point.y * (float)imgHeight;
    }

    if(settings->alignment == RV_ALIGN_BOTTOMLEFT   ||
       settings->alignment == RV_ALIGN_BOTTOMCENTER ||
       settings->alignment == RV_ALIGN_BOTTOMRIGHT
    ){
        offsetY = (settings->point.y - floatHeight) * imgHeight;
    }

    if(settings->alignment == RV_ALIGN_TOPLEFT    ||
       settings->alignment == RV_ALIGN_CENTERLEFT ||
       settings->alignment == RV_ALIGN_BOTTOMLEFT
    ){
        offsetX = settings->point.x * imgWidth;
    }

    if(settings->alignment == RV_ALIGN_TOPRIGHT    ||
       settings->alignment == RV_ALIGN_CENTERRIGHT ||
       settings->alignment == RV_ALIGN_BOTTOMRIGHT
    ){
        offsetX = (settings->point.x - floatWidth) * imgWidth;
    }

    settings->button->area.upLeft.x   = offsetX / imgWidth;
    settings->button->area.upLeft.y   = offsetY / imgHeight;
    settings->button->area.lowRight.x = settings->button->area.upLeft.x + floatWidth;
    settings->button->area.lowRight.y = settings->button->area.upLeft.y + floatHeight;

    settings->button->name = *settings->name;

    rvLoadTextSettings set = {0};
    set.image    = settings->img;
    set.sv       = settings->name;
    set.font     = settings->font;
    set.spacing  = settings->spacing;
    set.charsize = settings->charsize;
    set.offsetX  = (uint32_t)offsetX;
    set.offsetY  = (uint32_t)offsetY;

    rvLoadText(engine, &set);
}
