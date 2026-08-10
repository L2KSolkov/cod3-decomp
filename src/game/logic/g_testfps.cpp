// ============================================================================
// g_testfps.cpp - FPS test harness methods (game2.o)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <float.h>
#include <stdio.h>

// ============================================================================
// StreamZone / ZoneCellDesc local views (streamer.o; TestFPS.cpp usage)
// ============================================================================
struct BoundingBoxLocal {
    math::Position3 vmin;  // +0x00
    math::Position3 vmax;  // +0x10
};
static_assert(sizeof(BoundingBoxLocal) == 0x20, "BoundingBoxLocal size mismatch");

struct ZoneCellDescLocal {
    BoundingBoxLocal mAabb;  // +0x00
    unsigned int mCellId;    // +0x20
};

struct StreamZoneLocal {
    InplaceVector<ZoneCellDescLocal*> mCells;  // +0x00
};

// ============================================================================
// GetCellBBox - ea: 0x4F6DB0
// ============================================================================
BoundingBoxLocal GetCellBBox(int cellNum, const StreamZoneLocal* zone)
{
    if (zone == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\TestFPS.cpp";
        AeAssert::gCurrentLine = 31;
        AeAssert::gCurrentExpr = "zone";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    BoundingBoxLocal result;
    result.vmin.v = _mm_set1_ps(FLT_MAX);
    result.vmax.v = _mm_set1_ps(-FLT_MAX);
    for (unsigned int v4 = 0; v4 < zone->mCells.mSize; ++v4)
    {
        ZoneCellDescLocal* desc = zone->mCells.mList[v4];
        if (desc->mCellId == (unsigned int)cellNum)
        {
            memcpy(&result, desc, sizeof(BoundingBoxLocal));
            return result;
        }
    }
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\TestFPS.cpp";
    AeAssert::gCurrentLine = 43;
    AeAssert::gCurrentExpr = "j < zone->GetNumCells()";
    if (!AeAssert::IsIgnored()
        && AeAssert::Assert("Could not locate zone for cell index %n", cellNum))
        __debugbreak();
    return result;
}

// ============================================================================
// TestFPS::CheckForFloor - ea: 0x4F6FC0
// Trace straight down from position to zMin; step down 25 units until a
// solid (non-decal) surface is hit.
// ============================================================================
bool TestFPS::CheckForFloor(const math::Position3* position, trace_t* trace,
                            float zMin)
{
    math::Position3 start = *position;
    math::Position3 end = *position;
    end.v.m128_f32[2] = zMin;
    math::Position3 zero;
    zero.v = _mm_setzero_ps();
    memset(trace, 0, sizeof(trace_t));
    DbLinkedHandle<EntityHandleDb, Entity> playerHandle;
    playerHandle.mHandle.mVal = mPlayerHandle.mVal;
    collision_context_t context(playerHandle, 17);
    for (;;)
    {
        g_Trace(trace, start, zero, zero, end, context);
        if ((trace->fraction >= 1.0f || (trace->surfaceFlags & 0x84) == 0)
            && trace->allsolid == 0)
            break;
        float z = start.v.m128_f32[2];
        if (trace->allsolid == 0)
            z = trace->endpos.v.m128_f32[2];
        start.v.m128_f32[2] = z - 25.0f;
        if (zMin > start.v.m128_f32[2])
            return false;
        memset(trace, 0, sizeof(trace_t));
    }
    return trace->fraction < 1.0f;
}

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
