// ============================================================================
// screenshot.cpp - render.o screenshot + GL debug helpers (tr_main.cpp)
// ============================================================================

#include "game/logic/g_local.h"
#include "game/sv/sv_stubs.h"
#include "render/ShaderCommon.h"

#include <stdio.h>

extern void Com_sprintf(char* dest, int size, const char* fmt, ...);  // core.o
extern int currCl;                      // ?currCl@@3HA
extern int gTakeScreenshot;             // ?gTakeScreenshot@@3HA @ 0xF743E0
extern int gScreenshotInProgress;       // ?gScreenshotInProgress@@3HA @ 0xF743E4

// ea: 0x006C0400
void GL_CheckErrors(const char* str)
{
    (void)str;
}

// ea: 0x006C0410
void R_TakeScreenshot(int x, int y, int width, int height, char* name)
{
    (void)x; (void)y; (void)width; (void)height; (void)name;
}

// ea: 0x006C0420
void R_TakeScreenshotJPEG(int x, int y, int width, int height, char* name)
{
    (void)x; (void)y; (void)width; (void)height; (void)name;
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

// ea: 0x006C04F0
void RE_CubemapShot(const char* name, int width, int height, float x, float y)
{
    (void)name; (void)width; (void)height; (void)x; (void)y;
}

// ea: 0x006C0500
void RE_CubemapWaterShot(const char* name, int width, int height,
                         float* x, float* y)
{
    (void)name; (void)width; (void)height; (void)x; (void)y;
}

// ea: 0x006C0510
void R_ScreenShotHigh_f()
{
    if (gScreenshotInProgress == 0)
    {
        gTakeScreenshot = 1;
        GamePause::SetGamePaused(currCl, true);
        ShaderCommon::gGlowIntensity = 0.0f;
    }
}
