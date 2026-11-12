#include "river2D_main.h"
#include "rv_print_macros.h"

#include "imgsurf_main.h"

#include <stdlib.h>

#define BILLION 1000000000

f_internal void calcDelta
(
    const RiverTime *time1,
    const RiverTime *time2,
    i64   *deltaS,
    i64   *deltaNS
){
    if(time1->ns > BILLION)
    {
        RV_WARN("time1 is malformed.");
    }
    if(time2->ns > BILLION)
    {
        RV_WARN("time2 is malformed.");
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
        RV_WARN("passed uninitialized timestamp.");
        RiverTime time = {0, 0};
        return time;
    }

    i64 deltaS  = 0;
    i64 deltaNS = 0;
    calcDelta(time1, time2, &deltaS, &deltaNS);

    RiverTime delta = {deltaS, deltaNS};
    return delta;
}

f32 rvDeltaTime_ms
(
    const RiverTime *time1,
    const RiverTime *time2
){
    if(!time1 || !time2)
    {
        RV_WARN("passed uninitialized timestamp.");
        return 0;
    }

    i64 deltaS  = 0;
    i64 deltaNS = 0;
    calcDelta(time1, time2, &deltaS, &deltaNS);

    return (f32)((f32)deltaS * 1e3f + (f32)deltaNS / 1e6f);
}

i64 rvDeltaTime_ns
(
    const RiverTime *time1,
    const RiverTime *time2
){
    if(!time1 || !time2)
    {
        RV_WARN("passed uninitialized timestamp.");
        return 0;
    }

    i64 deltaS  = 0;
    i64 deltaNS = 0;
    calcDelta(time1, time2, &deltaS, &deltaNS);

    return (deltaS * BILLION + deltaNS);
}

RiverTime rvDeltaTime_now
(
    const RiverTime *time
){
    if(!time || !time->s)
    {
        RV_WARN("passed uninitialized timestamp.");
        RiverTime result = {0, 0};
        return result;
    }

    RiverTime current = rvQueryTime();
    i64       deltaS  = 0;
    i64       deltaNS = 0;

    if(time->s > current.s || (time->ns > current.ns && time->s == current.s))
    {
        RV_ERROR("timestamp lies in the future.");
        RiverTime result = {0, 0};
        return result;
    }

    calcDelta(time, &current, &deltaS, &deltaNS);

    RiverTime delta = {deltaS, deltaNS};
    return delta;
}

f32 rvDeltaTime_now_ms
(
    const RiverTime *time
){
    if(!time)
    {
        RV_ERROR("passed uninitialized timestamp.");
        return 0;
    }

    RiverTime current = rvQueryTime();
    i64       deltaS  = 0;
    i64       deltaNS = 0;

    if(time->s > current.s || (time->ns > current.ns && time->s == current.s))
    {
        RV_ERROR("timestamp lies in the future.");
        return 0;
    }

    calcDelta(time, &current, &deltaS, &deltaNS);

    return (f32)((f32)deltaS * 1e3f + (f32)deltaNS / 1e6f);
}

u64 rvDeltaTime_now_ns
(
    const RiverTime *time
){
    if(!time)
    {
        RV_ERROR("passed uninitialized timestamp.");
        return 0;
    }

    RiverTime current = rvQueryTime();
    i64       deltaS  = 0;
    i64       deltaNS = 0;

    if(time->s > current.s || (time->ns > current.ns && time->s == current.s))
    {
        RV_ERROR("timestamp lies in the future.");
        return 0;
    }

    calcDelta(time, &current, &deltaS, &deltaNS);

    return(u64)(deltaS * BILLION + deltaNS);
}

void rvAppendImage
(
    EngineData *engine,
    RiverImage *src,
    RiverImage *dst,
    u08        direction
){
    u32 ogWidth  = dst->width;
    u32 ogHeight = dst->height;

    u32 width  = dst->width;
    u32 height = dst->height;

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
        RV_ERROR("unknown direction, cannot append.");
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
    comp.offsetDstX = direction == RV_HORIZONTAL ? ogWidth  : 0;
    comp.offsetDstY = direction == RV_VERTICAL   ? ogHeight : 0;
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
        RV_WARN("No image to free.");
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
    if(!set)
    {
        RV_ERROR("set cannot be null.");
        set->errorcode = RV_ERROR_NULLPTR;
        return (TileMap){0};
    }
    if(!set->file)
    {
        RV_ERROR("file is not open. cannot read from nullptr.");
        set->errorcode = RV_ERROR_NULLPTR;
        return (TileMap){0};
    }

    const char header[9] = "r2Dtiles";
    int byte = 0;

    for(u08 i = 0; i < 8 && ((byte = fgetc(set->file)) != EOF); ++i)
    {
        if(byte != header[i])
        {
            set->errorcode = RV_ERROR_INVALID_HEADER;
            return (TileMap){0};
        }
    }

    size elements = 0;

    if(((elements = fread(set->tilesize,  2, 1, set->file)) != 1) ||
       ((elements = fread(set->mapWidth,  4, 1, set->file)) != 1) ||
       ((elements = fread(set->mapHeight, 4, 1, set->file)) != 1) ||
       ((elements = fread(set->mapLayers, 1, 1, set->file)) != 1)
    ){
        RV_ERROR("could not validate .rte header.");
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
        RV_WARN("failed to load tilesheet from pointer: %p", (void*)set->file);
        set->errorcode = RV_ERROR_LOADIMAGE_PTR;
        return (TileMap){0};
    }
    set->tilesheet->path = pdCstrSV("rvLoadTilemap");

    u64 sheetW = set->tilesheet->width  / *set->tilesize;
    u64 sheetH = set->tilesheet->height / *set->tilesize;

    u64 maxdatabyte  = sheetW * sheetH * sizeof(TileMetadata);
    u64 maxindexbyte = *set->mapLayers * *set->mapWidth * *set->mapHeight *
                           sizeof(TileIndex);

    TileMap map = {0};

    map.metadata = malloc(maxdatabyte);
    map.indices  = malloc(maxindexbyte);

    for(u64 i = 0; i < maxdatabyte && ((byte = fgetc(set->file)) != EOF); ++i)
    {
        ((u08*)map.metadata)[i] = (u08)byte;
    }

    for(u64 i = 0; i < maxindexbyte && ((byte = fgetc(set->file)) != EOF); ++i)
    {
        ((u08*)map.indices)[i] = (u08)byte;
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

    size elems = 0;

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

    u64 dataC = set->tilesheet->width  / set->tilesize *
                set->tilesheet->height / set->tilesize;
    if((elems = fwrite(set->metadata, sizeof(TileMetadata), dataC, set->file) != dataC))
    {
        set->errorcode = RV_ERROR_WRITE_METADATA;
        return;
    }

    u64 indexC = set->mapLayers * set->mapHeight * set->mapWidth;
    if((elems = fwrite(set->indices, sizeof(TileIndex), indexC, set->file) != indexC))
    {
        set->errorcode = RV_ERROR_WRITE_INDICES;
        return;
    }
}
