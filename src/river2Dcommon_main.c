#include "river2D_main.h"
#include "imgsurf_main.h"

#include <stdlib.h>

#define BILLION 1000000000

f_internal void calcDelta
(
    const RiverTime *time1,
    const RiverTime *time2,
    int64_t      *deltaS,
    int64_t      *deltaNS
){
    if(time1->ns > BILLION)
    {
        fprintf(stderr, "\033[31mERROR: timestamp 0 is malformed.\033[0m\n");
    }
    if(time2->ns > BILLION)
    {
        fprintf(stderr, "\033[31mERROR: timestamp 1 is malformed.\033[0m\n");
    }

    // uninitialized times
    if(time1->s == INT64_MIN && time2->s > 0)
    {
        *deltaS = time2->s;
    }
    else
    {
        *deltaS = time2->s - time1->s;
    }

    if(time1->ns == INT64_MIN && time2->ns > 0)
    {
        *deltaNS = time2->ns;
    }
    else
    {
        *deltaNS = time2->ns - time1->ns;
    }

    if(*deltaNS < 0)
    {
        *deltaS  -= 1;
        *deltaNS += BILLION;
    }
}

RiverTime rvDeltaTime
(
    const RiverTime *time1,
    const RiverTime *time2
){
    if(!time1 || !time2)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        RiverTime time = {0, 0};
        return time;
    }

    int64_t deltaS  = 0;
    int64_t deltaNS = 0;
    calcDelta(time1, time2, &deltaS, &deltaNS);

    RiverTime delta = {deltaS, deltaNS};
    return delta;
}

float rvDeltaTime_ms
(
    const RiverTime *time1,
    const RiverTime *time2
){
    if(!time1 || !time2)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        return -1;
    }

    int64_t deltaS  = 0;
    int64_t deltaNS = 0;
    calcDelta(time1, time2, &deltaS, &deltaNS);

    return (float)((float)deltaS * 1e3f + (float)deltaNS / 1e6f);
}

int64_t rvDeltaTime_ns
(
    const RiverTime *time1,
    const RiverTime *time2
){
    if(!time1 || !time2)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        return -1;
    }

    int64_t deltaS  = 0;
    int64_t deltaNS = 0;
    calcDelta(time1, time2, &deltaS, &deltaNS);

    return (deltaS * BILLION + deltaNS);
}

RiverTime rvDeltaTime_now
(
    const RiverTime *time
){
    if(!time || !time->s)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        RiverTime result = {-1, -1};
        return result;
    }

    RiverTime current = rvQueryTime();
    int64_t   deltaS  = 0;
    int64_t   deltaNS = 0;

    if(time->s > current.s || (time->ns > current.ns && time->s == current.s))
    {
        fprintf(stderr, "\033[31mERROR: timestamp lies in the future.\033[0m\n");
        RiverTime result = {0, 0};
        return result;
    }

    calcDelta(time, &current, &deltaS, &deltaNS);

    RiverTime delta = {deltaS, deltaNS};
    return delta;
}

float rvDeltaTime_now_ms
(
    const RiverTime *time
){
    if(!time)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        return -1;
    }

    RiverTime current = rvQueryTime();
    int64_t   deltaS  = 0;
    int64_t   deltaNS = 0;

    if(time->s > current.s || (time->ns > current.ns && time->s == current.s))
    {
        fprintf(stderr, "\033[31mERROR: timestamp lies in the future.\033[0m\n");
        return -2;
    }

    calcDelta(time, &current, &deltaS, &deltaNS);

    return (float)((float)deltaS * 1e3f + (float)deltaNS / 1e6f);
}

uint64_t rvDeltaTime_now_ns
(
    const RiverTime *time
){
    if(!time)
    {
        fprintf(stderr, "\033[31mERROR: passed uninitialized timestamp.\033[0m\n");
        return BILLION + 1;
    }

    RiverTime current = rvQueryTime();
    int64_t   deltaS  = 0;
    int64_t   deltaNS = 0;

    if(time->s > current.s || (time->ns > current.ns && time->s == current.s))
    {
        fprintf(stderr, "\033[31mERROR: timestamp lies in the future.\033[0m\n");
        return BILLION + 2;
    }

    calcDelta(time, &current, &deltaS, &deltaNS);

    return(uint64_t)(deltaS * BILLION + deltaNS);
}

void rvAppendImage
(
    EngineData *engine,
    RiverImage *src,
    RiverImage *dst,
    uint8_t    direction
){
    uint32_t og_width  = dst->width;
    uint32_t og_height = dst->height;

    uint32_t width  = dst->width;
    uint32_t height = dst->height;

    if(direction == RV_HORIZONTAL)
    {
        width = dst->width + src->width;

        if(dst->height < src->height)
        {
            height = src->height;
        }
    }
    else if(direction == RV_VERTICAL)
    {
        height = dst->height + src->height;

        if(dst->width < src->width)
        {
            width = src->width;
        }
    }
    else
    {
        fprintf(stderr, "\033[31m\nERROR: unkown direction, cannot append.\n\033[0m");
        return;
    }

    RiverImage tmp = {0};
    rvCreateImage(engine, &tmp, width, height);

    rvCompositeSettings comp = {0};
    comp.dst                 = &tmp;
    comp.pictop              = RV_PICTOP_OVER;

    if(dst->data)
    {
        comp.src        = dst;
        comp.cropWidth  = dst->width;
        comp.cropHeight = dst->height;

        rvCompositeImage(engine, &comp);

        rvDestroyImage(dst);
    }

    comp.src        = src;
    comp.offsetDstX = direction == RV_HORIZONTAL ? og_width  : 0;
    comp.offsetDstY = direction == RV_VERTICAL   ? og_height : 0;
    comp.cropHeight = src->height;
    comp.cropWidth  = src->width;

    rvCompositeImage(engine, &comp);

    dst->path     = tmp.path;
    dst->data     = tmp.data;
    dst->width    = tmp.width;
    dst->height   = tmp.height;
    dst->channels = tmp.channels;
    #ifdef BUILD_LINUX
    dst->picture  = tmp.picture;
    dst->pixmap   = tmp.pixmap;
    #endif
}

void rvDestroyImage
(
    RiverImage *image
){
    if(!image)
    {
        fprintf(stderr, "No image to be freed.\n");
        return;
    }

    if(image->data)
    {
        free(image->data);
        image->data = 0;
    }
}

TileMap rvLoadTilemap
(
    EngineData        *engine,
    rvLoadMapSettings *set
){
    const char header[9] = "r2Dtiles";
    int byte;

    for(uint8_t i = 0; i < 8 && ((byte = fgetc(set->file)) != EOF); ++i)
    {
        if(byte != header[i])
        {
            set->errorcode = RV_ERROR_INVALID_HEADER;
            return (TileMap){0};
        }
    }

    size_t elements = 0;

    if(((elements = fread(set->tilesize,  2, 1, set->file)) != 1) ||
       ((elements = fread(set->mapWidth,  4, 1, set->file)) != 1) ||
       ((elements = fread(set->mapHeight, 4, 1, set->file)) != 1) ||
       ((elements = fread(set->mapLayers, 1, 1, set->file)) != 1)
    ){
        set->errorcode = RV_ERROR_INVALID_HEADER;
        return (TileMap){0};
    }

    if(set->tilesheet->data)
    {
        rvDestroyImage(set->tilesheet);
    }
    rvLoadImage_ptr(engine, set->file, set->tilesheet, RV_CHANNELS_BGRA, 8);

    if(!set->tilesheet->data)
    {
        fprintf(stderr, "\033[31;1;7mERROR: failed to load tilesheet into ptr."
                "\033[0m\n");
        set->errorcode = RV_ERROR_LOADIMAGE_PTR;
        return (TileMap){0};
    }
    set->tilesheet->path = cstr_sv("rvLoadTilemap");

    uint64_t sheetW = set->tilesheet->width  / *set->tilesize;
    uint64_t sheetH = set->tilesheet->height / *set->tilesize;

    uint64_t maxdatabyte  = sheetW * sheetH * sizeof(TileMetadata);
    uint64_t maxindexbyte = *set->mapLayers * *set->mapWidth * *set->mapHeight *
                           sizeof(TileIndex);

    TileMap map = {0};

    map.metadata = malloc(maxdatabyte);
    map.indices  = malloc(maxindexbyte);

    for(uint64_t i = 0; i < maxdatabyte && ((byte = fgetc(set->file)) != EOF); ++i)
    {
        ((uint8_t*)map.metadata)[i] = (uint8_t)byte;
    }

    for(uint64_t i = 0; i < maxindexbyte && ((byte = fgetc(set->file)) != EOF); ++i)
    {
        ((uint8_t*)map.indices)[i] = (uint8_t)byte;
    }

    return map;
}

void rvSaveTilemap
(
    EngineData        *engine,
    rvSaveMapSettings *set
){
    const char header[9] = "r2Dtiles";
    fwrite(header, sizeof(header) - 1, 1, set->file);

    size_t elems = 0;

    if(((elems = fwrite(&set->tilesize,  2, 1, set->file)) != 1) ||
       ((elems = fwrite(&set->mapWidth,  4, 1, set->file)) != 1) ||
       ((elems = fwrite(&set->mapHeight, 4, 1, set->file)) != 1) ||
       ((elems = fwrite(&set->mapLayers, 1, 1, set->file)) != 1)
    ){
        set->errorcode = RV_ERROR_INVALID_HEADER;
        return;
    }

    rvSyncImage(engine, set->tilesheet, false);

    imWritePtr(set->file, set->tilesheet->data, IM_FILE_QOI,
               set->tilesheet->width, set->tilesheet->height, IM_CHANNELS_BGRA, 8);

    if(!set->metadata)
    {
        set->errorcode = RV_ERROR_INVALID_METADATA;
        return;
    }
    else if(!set->indices)
    {
        set->errorcode = RV_ERROR_INVALID_INDICES;
        return;
    }

    uint64_t dataC = set->tilesheet->width  / set->tilesize *
                     set->tilesheet->height / set->tilesize;
    if((elems = fwrite(set->metadata, sizeof(TileMetadata), dataC, set->file) != dataC))
    {
        set->errorcode = RV_ERROR_WRITE_METADATA;
        return;
    }

    uint64_t indexC = set->mapLayers * set->mapHeight * set->mapWidth;
    if((elems = fwrite(set->indices, sizeof(TileIndex), indexC, set->file) != indexC))
    {
        set->errorcode = RV_ERROR_WRITE_INDICES;
        return;
    }
}
