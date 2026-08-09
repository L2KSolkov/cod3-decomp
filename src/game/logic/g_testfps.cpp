// ============================================================================
// g_testfps.cpp - FPS test harness methods (game2.o)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <stdio.h>

extern cvar_t* sv_mapname;  // ?sv_mapname@@3PAUcvar_t@@A

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
