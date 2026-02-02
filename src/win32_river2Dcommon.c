#include "river2D_main.h"

#include <sys/stat.h>
#include <stdio.h>

River2D_Time river2D_queryTime
(
    void
){
    LARGE_INTEGER t1;
    LARGE_INTEGER freq;

    QueryPerformanceCounter(&t1);
    QueryPerformanceFrequency(&freq);

    River2D_Time time;
    time.s  = (uint64_t)(t1.QuadPart / freq.QuadPart);
    time.ns = (uint64_t)(1000000 * t1.QuadPart / freq.QuadPart) / 1000;

    return time;
}

uint8_t river2D_verifyPath
(
    const char *path
){
    struct _stat pathInfo;

    if(_stat(path, &pathInfo))
    {
        return RIVER2D_TYPE_ERROR;
    }

    if(_S_IFDIR & pathInfo.st_mode)
    {
        return RIVER2D_TYPE_DIRECTORY;
    }

    if(_S_IFREG & pathInfo.st_mode)
    {
        return RIVER2D_TYPE_FILE;
    }

    return RIVER2D_TYPE_OTHER;
}

const char* river2D_listFiles
(
    const char *path
){
    WIN32_FIND_DATAA fileData;
    HANDLE           foundHandle;

    uint16_t stringLength = strlen(path);
    char *winPath         = malloc(stringLength + 3);
    for(uint16_t i = 0; i < stringLength; ++i)
    {
        winPath[i] = path[i];
    }
    winPath[stringLength]     = '\\';
    winPath[stringLength + 1] = '*';
    winPath[stringLength + 2] = '\0';

    foundHandle = FindFirstFileA(winPath, &fileData);
    if(foundHandle == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: failed to open cwd.\033[0m\n");
        return 0;
    }

    uint32_t listSize = 0;

    while(FindNextFileA(foundHandle, &fileData))
    {
        uint8_t length = 0;
        for(; length < 255 && fileData.cFileName[length] != '\0'; ++length)
        {
        }
        listSize += length + 1;
    }

    FindClose(foundHandle);
    foundHandle = FindFirstFileA(winPath, &fileData);

    char     *list  = (char*)malloc(listSize + 1);
    uint32_t offset = 0;

    while(FindNextFileA(foundHandle, &fileData))
    {
        uint8_t length = 0;
        for(; length < 255 && fileData.cFileName[length] != '\0'; ++length)
        {
            list[offset + length] = fileData.cFileName[length];
        }
        list[offset + length] = ';';
        offset += length + 1;
    }

    list[offset] = '\0';

    FindClose(foundHandle);
    return list;
}

uint8_t river2D_interpretCharAsKey
(
    char inp
){
    if(inp > 0x60 && inp < 0x7A)
    {
        return inp - 32;
    }

    return inp;
}

Dimensions river2D_getWindowSize
(
    EngineData *engine
){
    Dimensions dim  = {0};
    RECT       rect = {0};

    GetWindowRect(engine->window, &rect);

    dim.width  = rect.right  - rect.left;
    dim.height = rect.bottom - rect.top;

    return dim;
}

// TODAY: do hCursor shenanigans
// 1. get cursor changing working
// 2. save hCursors to engineData, or at least, save everything but the colour and mask bitmaps.
// 3. from step 2 on, we can do the minimal work needed inside this function.
void river2D_changeCursor
(
    EngineData *engine,
    River2D_Image *image
){
    BITMAPV5HEADER header = {0};
	header.bV5Size        = sizeof(BITMAPV5HEADER);
	header.bV5Width       = (LONG)image->width;
	header.bV5Height      = -(LONG)image->height;
	header.bV5Planes      = 1;
	header.bV5BitCount    = 32;
	header.bV5Compression = BI_BITFIELDS;
	header.bV5RedMask     = 0x0000FF00;
	header.bV5GreenMask   = 0x00FF0000;
	header.bV5BlueMask    = 0xFF000000;
	header.bV5AlphaMask   = 0x000000FF;

	uint32_t *data = 0;
	HBITMAP colour = CreateDIBSection(engine->context, (BITMAPINFO*)&header, DIB_RGB_COLORS, (void**)&data, 0, 0);
	HBITMAP mask   = CreateBitmap(image->width, image->height, 1, 1, 0);

	ReleaseDC(0, engine->context);

	for(uint32_t y = 0; y < image->height; ++y)
	{
		for(uint32_t x = 0; x < image->width; ++x)
		{
			*data++ = image->data[y * image->width + x];
			// *data++ = 0xFFFFFF00;
            // TODO: set the corresponding bits inside mask
		}
	}

	ICONINFO iconInfo = {0};
	iconInfo.hbmMask  = mask;
	iconInfo.hbmColor = colour;

	HCURSOR cursor = CreateIconIndirect(&iconInfo);
	DeleteObject(colour);
	DeleteObject(mask);

    SetCursor(cursor);
}
