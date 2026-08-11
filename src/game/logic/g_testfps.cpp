// ============================================================================
// g_testfps.cpp - FPS test harness methods (game2.o)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Renderer world / BSP views (render.o; verified against IDA)
struct BspTreeLocal {
    char _pad0[0x18];
    InplaceVector<unsigned int> mCells;  // +0x18 InplaceVector<BspCell>
};
struct world_tLocal {
    char _pad0[0x100];
    BspTreeLocal* bspTree;  // +0x100
};
struct trGlobals_t {
    char _pad0[0x290];
    world_tLocal* world;  // +0x290
};
trGlobals_t tr;  // ?tr@@3UtrGlobals_t@@A (render.o @ 0x13642D0)

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
// TestFPS::NextPosition - ea: 0x501A60
// Sweep the current cell in a raster pattern, dropping to the floor each
// step. Advances to the next cell when the sweep completes.
// ============================================================================
extern const void* StreamZoneManager_GetCellZone(void* self,
                                                 int cellIndex);  // ?GetCellZone@StreamZoneManager@@QAEPBVStreamZone@@H@Z

void TestFPS::NextPosition()
{
    int numCells = tr.world->bspTree->mCells.mSize;
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    mPlayerHandle.mVal = Player->mHandle.mHandle.mVal;
    const StreamZoneLocal* zone = (const StreamZoneLocal*)
        StreamZoneManager_GetCellZone(StreamZoneManager::sInst, mCellIndex);
    if (zone == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\TestFPS.cpp";
        AeAssert::gCurrentLine = 427;
        AeAssert::gCurrentExpr = "zone";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    BoundingBoxLocal bbox = GetCellBBox(mCellIndex, zone);
    int infoNode = *(int*)((char*)zone + 36);
    if (infoNode == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\TestFPS.cpp";
        AeAssert::gCurrentLine = 432;
        AeAssert::gCurrentExpr = "infoNode";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    bool IsLoaded =
        PakManager::sInst->IsLoaded((TPakId)*(unsigned int*)(infoNode + 180));
    math::Position3 pos;
    pos.v.m128_f32[0] = mCurrentPosition.x;
    pos.v.m128_f32[1] = mCurrentPosition.y;
    pos.v.m128_f32[2] = mCurrentPosition.z;
    if (!IsLoaded)
    {
        float cx = (bbox.vmax.v.m128_f32[0] + bbox.vmin.v.m128_f32[0]) * 0.5f;
        float cy = (bbox.vmax.v.m128_f32[1] + bbox.vmin.v.m128_f32[1]) * 0.5f;
        if (fabsf(mCurrentPosition.x - cx) > 0.000001f
            || fabsf(mCurrentPosition.y - cy) > 0.000001f)
        {
            mCurrentPosition.x = cx;
            mCurrentPosition.y = cy;
            return;
        }
    }
    if (mCellIndex < numCells)
    {
        trace_t trace;
        for (;;)
        {
            math::Position3 probe = pos;
            probe.v.m128_f32[2] = pos.v.m128_f32[2] - 25.0f;
            if (mCellX >= 0
                && CheckForFloor(&probe, &trace,
                                 bbox.vmin.v.m128_f32[2])
                && R_CellForPoint(&trace.endpos) == mCellIndex)
            {
                mCurrentPosition.x = trace.endpos.v.m128_f32[0];
                mCurrentPosition.y = trace.endpos.v.m128_f32[1];
                mCurrentPosition.z = trace.endpos.v.m128_f32[2];
                ++mCurrentPositionIndex;
                return;
            }
            int cellW = (int)((bbox.vmax.v.m128_f32[0] - bbox.vmin.v.m128_f32[0])
                              * mDeltaInverse);
            int cellH = (int)((bbox.vmax.v.m128_f32[1] - bbox.vmin.v.m128_f32[1])
                              * mDeltaInverse);
            int nx = mCellX + mCellXDelta;
            mCellX = nx;
            if (nx >= cellW || nx < 0)
            {
                mCellX = mCellXDelta <= 0 ? 0 : cellW - 1;
                mCellXDelta = -mCellXDelta;
                ++mCellY;
                if (mCellY >= cellH)
                    break;
            }
            float px = bbox.vmin.v.m128_f32[0]
                + (mCellX * mDelta)
                + ((bbox.vmax.v.m128_f32[0] - bbox.vmin.v.m128_f32[0]) * 0.5f)
                - (mCellX * mDelta);
            float py = bbox.vmin.v.m128_f32[1]
                + (mCellY * mDelta)
                + ((bbox.vmax.v.m128_f32[1] - bbox.vmin.v.m128_f32[1]) * 0.5f)
                - (mCellY * mDelta);
            math::Position3 p2;
            p2.v.m128_f32[0] = px;
            p2.v.m128_f32[1] = py;
            p2.v.m128_f32[2] = bbox.vmin.v.m128_f32[2];
            if (CheckForFloor(&p2, &trace, bbox.vmin.v.m128_f32[2])
                && R_CellForPoint(&trace.endpos) == mCellIndex)
            {
                mCurrentPosition.x = trace.endpos.v.m128_f32[0];
                mCurrentPosition.y = trace.endpos.v.m128_f32[1];
                mCurrentPosition.z = trace.endpos.v.m128_f32[2];
                ++mCurrentPositionIndex;
                return;
            }
            if (mCellIndex >= numCells)
                return;
            pos = p2;
        }
        mCellY = 0;
        OutputStats();
        ++mCellIndex;
        while (mCellIndex < numCells
               && !((bool*)&mCells_size)[mCellIndex - 100])
            ++mCellIndex;
        if (mCellIndex < numCells)
        {
            const StreamZoneLocal* CellZone = (const StreamZoneLocal*)
                StreamZoneManager_GetCellZone(StreamZoneManager::sInst,
                                              mCellIndex);
            if (CellZone == nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\TestFPS.cpp";
                AeAssert::gCurrentLine = 527;
                AeAssert::gCurrentExpr = "zone";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            BoundingBoxLocal nb = GetCellBBox(mCellIndex, CellZone);
            mCurrentPosition.x = (nb.vmin.v.m128_f32[0]
                                  + nb.vmax.v.m128_f32[0]) * 0.5f;
            mCurrentPosition.y = (nb.vmin.v.m128_f32[1]
                                  + nb.vmax.v.m128_f32[1]) * 0.5f;
            mCurrentPosition.z = (nb.vmin.v.m128_f32[2]
                                  + nb.vmax.v.m128_f32[2]) * 0.5f;
        }
    }
}

// ============================================================================
// TestFPS::Test - ea: 0x509620
// Start the FPS sweep: freeze the player, parse the cell list, init sweep
// state, and take the first step.
// ============================================================================
extern vmCvar_t g_performanceTestDelta;      // ?g_performanceTestDelta@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t g_performanceTestDeltaAngle; // ?g_performanceTestDeltaAngle@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t g_performanceTestCell;       // ?g_performanceTestCell@@3UvmCvar_t@@A (game2.o)
extern int FS_CreatePath(char* path);  // ?FS_CreatePath@@YAHPAD@Z
extern int gStartTime;                       // ?gStartTime@@3HA (game2.o)
extern char* strtok(char* str, const char* delim);
extern int sscanf(const char* s, const char* fmt, ...);

void TestFPS::Test()
{
    if (mTesting)
        return;
    if (mFile != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\TestFPS.cpp";
        AeAssert::gCurrentLine = 79;
        AeAssert::gCurrentExpr = "!mFile";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    mTesting = true;
    mCurrentPositionIndex = 0;
    mCellIndex = 0;
    mCellX = 0;
    mCellY = 0;
    mCellXDelta = 1;
    mCurrentAngle = 0;
    mLastFile[0] = 0;
    mBlock = 0;
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    Player->client->noclip = 1;
    Player->client->bFrozen = 1;
    mDeltaAngle = g_performanceTestDeltaAngle.integer;
    mDelta = (float)g_performanceTestDelta.integer;
    mDeltaInverse = 1.0f / mDelta;
    int mSize = tr.world->bspTree->mCells.mSize;
    if (mSize >= 100)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\TestFPS.cpp";
        AeAssert::gCurrentLine = 101;
        AeAssert::gCurrentExpr = "100 > numCells";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    mCurrentPosition.x = Player->r.currentOrigin.v.m128_f32[0];
    mCurrentPosition.y = Player->r.currentOrigin.v.m128_f32[1];
    mCurrentPosition.z = Player->r.currentOrigin.v.m128_f32[2];
    char path[256];
    sprintf(path, "c:\\cod\\assets\\levels\\%s\\stats\\", sv_mapname->string);
    if (FS_CreatePath(path) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\TestFPS.cpp";
        AeAssert::gCurrentLine = 110;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("Could not create path %s.", path))
            __debugbreak();
    }
    Cvar_Set("g_performanceTest", "1");
    bool* cells = (bool*)&mCells_size;
    if (g_performanceTestCell.integer >= 0)
    {
        for (int i = 0; i < mSize; ++i)
            cells[i] = false;
        char tokenBuf[128];
        strcpy(tokenBuf, g_performanceTestCell.string);
        char* token = strtok(tokenBuf, ",");
        if (token == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\TestFPS.cpp";
            AeAssert::gCurrentLine = 155;
            AeAssert::gCurrentExpr = "token";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        while (token != nullptr)
        {
            char name[64], range[64];
            int start = 0;
            if (sscanf(token, "%s %s", name, range) == 2)
                start = atoi(range);
            int end = atoi(name);
            if (start != 0)
            {
                if (start >= end)
                {
                    for (int i = 0; i <= start - end; ++i)
                    {
                        if (i >= mSize)
                            break;
                        cells[end + i] = true;
                    }
                }
                else
                {
                    G_Printf("Invalid cell range %i %i.\n", end, start);
                }
            }
            else if (end < mSize)
            {
                cells[end] = true;
            }
            else
            {
                G_Printf("Invalid cell index %i.\n", end);
            }
            token = strtok(nullptr, ",");
        }
        for (int i = 0; i < mSize; ++i)
        {
            if (cells[i])
            {
                mCellIndex = i;
                break;
            }
        }
    }
    else if (mSize > 0)
    {
        for (int i = 0; i < mSize; ++i)
            cells[i] = true;
    }
    mCurrentAngle -= mDeltaAngle;
    mCellX = -1;
    NextPosition();
    gStartTime = Sys_Milliseconds();
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
