// ============================================================================
// screenshot.cpp - render.o screenshot + GL debug helpers (tr_main.cpp)
// ============================================================================

#include "game/logic/g_local.h"

#include <stdio.h>

extern void Com_sprintf(char* dest, int size, const char* fmt, ...);  // core.o

// ea: 0x006C0400
void GL_CheckErrors(const char* str)
{
    (void)str;
}

// ea: 0x006C0430
void R_ScreenshotFilename(int lastNumber, char* fileName)
{
    if (lastNumber >= 0x2710)
        Com_sprintf(fileName, 128, "screenshots/shot9999.tga");
    else
        Com_sprintf(fileName, 128, "screenshots/shot%04i.tga", lastNumber);
}

// ea: 0x006C0480
void R_ScreenshotFilenameJPEG(int lastNumber, char* fileName)
{
    if (lastNumber >= 0x2710)
        Com_sprintf(fileName, 128, "screenshots/shot9999.jpg");
    else
        Com_sprintf(fileName, 128, "screenshots/shot%04i.jpg", lastNumber);
}

// ea: 0x006C04D0
void R_LevelShot()
{
}

// ea: 0x006C04E0
void R_SaveGameShot(const char* name)
{
    (void)name;
}
