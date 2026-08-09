// ============================================================================
// g_testfps.cpp - FPS test harness methods (game2.o)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <stdio.h>

extern cvar_t* sv_mapname;  // ?sv_mapname@@3PAUcvar_t@@A

// Rumble globals (game2.o data)
extern float gLowFreqDelay;
extern float gLowFreqRumbleIntensity;
extern float gLowFreqSteadyDuration;
extern float gLowFreqRampUpTime;
extern float gLowFreqRampDownTime;
extern float gHighFreqDelay;
extern float gHighFreqDuration;

// ============================================================================
// TakeCubeMapShot - capture a cube map screenshot (no-op on this target)
// ea: 0x4EC6D0
// ============================================================================
void TakeCubeMapShot()
{
}

// ============================================================================
// PlayRumble - trigger a rumble through the Broc API
// ea: 0x4EC6E0
// ============================================================================
int PlayRumble()
{
    return gpBrocAPI->mBrocExports.mRumble(
        gLowFreqDelay, gLowFreqRumbleIntensity, gLowFreqSteadyDuration,
        gLowFreqRampUpTime, gLowFreqRampDownTime, gHighFreqDelay,
        gHighFreqDuration, 0);
}

// ============================================================================
// TestFPS::~TestFPS - close the stats file if open
// ea: 0x4EBFF0
// ============================================================================
TestFPS::~TestFPS()
{
    if (mFile != nullptr)
    {
        fclose((FILE*)mFile);
        mFile = nullptr;
    }
}

// ============================================================================
// TestFPS::GetPath - build the level stats directory path
// ea: 0x4EC020
// ============================================================================
void TestFPS::GetPath(char* path)
{
    sprintf(path, "c:\\cod\\assets\\levels\\%s\\stats\\", sv_mapname->string);
}
