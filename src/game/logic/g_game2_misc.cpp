// ============================================================================
// g_game2_misc.cpp - game2.o misc functions (default pak, mission stats,
// multiplayer rank helpers, spline util)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <string.h>

#include "filesystem/apk.h"

// Cross-object externs
extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern void tlMemFree(void* ptr);
extern int default_apk_size;    // ?default_apk_size (game2.o)
extern unsigned char default_apk[];  // ?default_apk (game2.o)
extern unsigned char* default_pak_buf;  // ?default_pak_buf (game2.o)
extern bool gMissionDataInitialized;    // ?gMissionDataInitialized (game2.o)
extern void BrocAddEntityThread(Entity* ent, unsigned int fcnHash,
                                void* params);  // ?BrocAddEntityThread (scr.o)

// ============================================================================
// ScriptEventHandler - script event dispatch list (0x44, IDA verified)
// ============================================================================
struct ScriptEvent {
    HashString notify;    // +0x00
    HashString callback;  // +0x04
};

struct ScriptEventHandler {
    unsigned char m_dlist_node[8];      // +0x00
    ScriptEvent mEvents[7];             // +0x08
    ScriptEventHandler* mNext;          // +0x40

    ~ScriptEventHandler();              // ea: 0x4F59D0
    bool RemoveEvent(HashString h, HashString callback);  // ea: 0x4F59F0
    bool ExecEvents(Entity* ent, HashString h, void* params);  // ea: 0x4F5A50
};
static_assert(sizeof(ScriptEventHandler) == 0x44,
              "ScriptEventHandler size mismatch");

// ============================================================================
// ScriptEventHandler::~ScriptEventHandler - ea: 0x4F59D0
// ============================================================================
ScriptEventHandler::~ScriptEventHandler()
{
    ScriptEventHandler* mNext = this->mNext;
    if (mNext != nullptr)
    {
        mNext->~ScriptEventHandler();
        operator delete(mNext);
    }
}

// ============================================================================
// ScriptEventHandler::RemoveEvent - ea: 0x4F59F0
// ============================================================================
bool ScriptEventHandler::RemoveEvent(HashString h, HashString callback)
{
    ScriptEventHandler* cur = this;
    for (;;)
    {
        int v3 = 0;
        do
        {
            if (cur->mEvents[v3].notify.mHash == h.mHash
                && cur->mEvents[v3].callback.mHash == callback.mHash)
            {
                cur->mEvents[v3].callback.mHash = 0;
                cur->mEvents[v3].notify.mHash = 0;
                return true;
            }
            ++v3;
        } while (v3 < 7);
        if (cur->mNext == nullptr)
            break;
        cur = cur->mNext;
    }
    return false;
}

// ============================================================================
// ScriptEventHandler::ExecEvents - ea: 0x4F5A50
// ============================================================================
bool ScriptEventHandler::ExecEvents(Entity* ent, HashString h, void* params)
{
    bool v4 = false;
    for (int i = 7; i != 0; --i)
    {
        if (mEvents[7 - i].notify.mHash == h.mHash)
        {
            BrocAddEntityThread(ent, mEvents[7 - i].callback.mHash, params);
            v4 = true;
        }
    }
    ScriptEventHandler* mNext = this->mNext;
    if (mNext != nullptr)
        return mNext->ExecEvents(ent, h, nullptr) || v4;
    return v4;
}

// ============================================================================
// SegmentSphereIntersection - segment/sphere test
// ea: 0x4F5AC0
// ============================================================================
bool SegmentSphereIntersection(const float* startPoint, const float* endPoint,
                               const float* sphereOrigin, float sphereRadius)
{
    extern vmCvar_t g_drawSmokeGren;  // ?g_drawSmokeGren (g.o)
    struct DebugColor { float r, g, b, a; };
    extern void DebugRender_RenderLine(const math::Position3* pt1,
        const math::Position3* pt2, const DebugColor* col, float thickness);
    extern float VectorNormalize(float* v);  // ?VectorNormalize (g.o)

    if (g_drawSmokeGren.integer == 2)
    {
        DebugColor col = { 1.0f, 1.0f, 0.0f, 1.0f };
        math::Position3 p1;
        math::Position3 p2;
        p1.v.m128_f32[0] = startPoint[0];
        p1.v.m128_f32[1] = startPoint[1];
        p1.v.m128_f32[2] = startPoint[2];
        p2.v.m128_f32[0] = endPoint[0];
        p2.v.m128_f32[1] = endPoint[1];
        p2.v.m128_f32[2] = endPoint[2];
        DebugRender_RenderLine(&p1, &p2, &col, 5.0f);
    }
    float segDir[3];
    segDir[0] = endPoint[0] - startPoint[0];
    segDir[1] = endPoint[1] - startPoint[1];
    segDir[2] = endPoint[2] - startPoint[2];
    float rel[3] = {
        startPoint[0] - sphereOrigin[0],
        startPoint[1] - sphereOrigin[1],
        startPoint[2] - sphereOrigin[2],
    };
    float segLen = VectorNormalize(segDir);
    float v5 = (segDir[2] * segDir[2]) + (segDir[1] * segDir[1])
        + (segDir[0] * segDir[0]);
    float v6 = ((rel[2] * segDir[2]) + (rel[1] * segDir[1])
                + (rel[0] * segDir[0])) * 2.0f;
    float v7 = ((((rel[2] * rel[2]) + (rel[1] * rel[1])
                  + (rel[0] * rel[0])) - (sphereRadius * sphereRadius))
                * v5) * 4.0f;
    float v8 = (v6 * v6) - v7;
    if (v8 < 0.0f)
        return false;
    float inv = 1597463007.0f - ((float)((int)v8) * 0.5f);
    float t = ((0.0f - v6) - ((1.5f - (((v8 * 0.5f) * inv) * inv)) * inv))
        / (v5 * 2.0f);
    return t > 0.0f && segLen > t;
}

// ============================================================================
// nalMatrix4x4_to_Axis4 - copy rotation rows to axis array
// ea: 0x4F5ED0
// ============================================================================
struct nalMatrix4x4 {
    float x[4];  // rows
    float y[4];
    float z[4];
    float w[4];
};

void nalMatrix4x4_to_Axis4(nalMatrix4x4* mat, float (*axis)[3])
{
    for (int i = 0; i < 4; ++i)
    {
        axis[i][0] = mat->x[i];
        axis[i][1] = mat->y[i];
        axis[i][2] = mat->z[i];
    }
}

// ============================================================================
// AnimIK - IK state (0x7C, IDA verified; ctor/dtor only here)
// ============================================================================
struct AnimIK {
    unsigned char ikJoints[0x50];   // +0x00 AnimIKJointVars_t[4]
    int initialized;                // +0x50
    void* pose;                     // +0x54
    void* skeleton;                 // +0x58
    unsigned char _pad5C[0x7C - 0x5C];  // rest of AnimIK layout

    AnimIK();  // ea: 0x4F60A0
    ~AnimIK();  // ea: 0x4F60B0
};
static_assert(sizeof(AnimIK) == 0x7C, "AnimIK size mismatch");

// ea: 0x4F60A0
AnimIK::AnimIK()
{
    initialized = 0;
    pose = nullptr;
    skeleton = nullptr;
}

// ea: 0x4F60B0
AnimIK::~AnimIK()
{
}

// ============================================================================
// _xmission_data - 0x78 (IDA verified)
// ============================================================================
struct _xmission_data {
    char name[0x20];             // +0x00
    unsigned int weaponsUsed[3]; // +0x20
    int missionTime;             // +0x2C
    int missionLastTick;         // +0x30
    int missionStat[17];         // +0x34
};
static_assert(sizeof(_xmission_data) == 0x78, "_xmission_data size mismatch");

extern _xmission_data* gMissionData;      // ?gMissionData (game2.o)
extern _xmission_data gTempMissionData;   // ?gTempMissionData (game2.o)

// ============================================================================
// InitDefaultPak - load the embedded default pak archive
// ea: 0x4F6040
// ============================================================================
apk::apkFile* InitDefaultPak()
{
    unsigned char* v0 =
        (unsigned char*)tlMemAlloc(default_apk_size, 0x1000u, 0x10000);
    memcpy(v0, default_apk, default_apk_size);
    default_pak_buf = v0;
    apk::apkFile* result = apk::apkLoadFileInPlace(v0, true);
    __wbinvd();
    return result;
}

// ============================================================================
// FiniDefaultPak - free the default pak buffer
// ea: 0x4F6090
// ============================================================================
void FiniDefaultPak()
{
    tlMemFree(default_pak_buf);
}

// ============================================================================
// stat_support_Shutdown - clear mission data init flag
// ea: 0x4F64D0
// ============================================================================
void stat_support_Shutdown()
{
    gMissionDataInitialized = false;
}

// ============================================================================
// stat_CommitStatsToLevel - merge temp mission stats into the mission data
// ea: 0x4F64E0
// ============================================================================
bool stat_CommitStatsToLevel()
{
    if (gMissionDataInitialized)
    {
        _xmission_data* v1 = gMissionData;
        if (gMissionData != nullptr)
        {
            int* missionStat = gTempMissionData.missionStat;
            int v3 = 52;
            for (;;)
            {
                *(int*)((char*)v1->name + v3) += *missionStat;
                *missionStat = 0;
                v3 += 4;
                ++missionStat;
                if (v3 >= 120)
                    break;
                v1 = gMissionData;
            }
            gMissionData->weaponsUsed[0] |= gTempMissionData.weaponsUsed[0];
            gMissionData->weaponsUsed[1] |= gTempMissionData.weaponsUsed[1];
            gMissionData->weaponsUsed[2] |= gTempMissionData.weaponsUsed[2];
            return gMissionData != nullptr;
        }
    }
    return gMissionDataInitialized;
}

// ============================================================================
// stat_ResetMissionStats - clear temp mission stats (optionally the totals)
// ea: 0x4F6560
// ============================================================================
void stat_ResetMissionStats(bool total)
{
    memset(&gTempMissionData, 0, sizeof(gTempMissionData));
    if (total && gMissionData != nullptr)
    {
        gMissionData->missionTime = 0;
        gMissionData->missionLastTick = 0;
        memset(gMissionData->missionStat, 0, sizeof(gMissionData->missionStat));
        gMissionData->weaponsUsed[0] = 0;
        gMissionData->weaponsUsed[1] = 0;
        gMissionData->weaponsUsed[2] = 0;
    }
}

// ============================================================================
// FN_Multiplayer_MapRestart - ea: 0x4F46D0
// ============================================================================
void FN_Multiplayer_MapRestart()
{
    MultiplayerMgr::sInst->MapRestart();
}

// ============================================================================
// FN_Multiplayer_Rank1 - reset persistent stats to rank 0
// ea: 0x4F46E0
// ============================================================================
Entity* FN_Multiplayer_Rank1()
{
    clientPersistent_t* p_pers =
        &EntityManager::sInst->GetPlayer(currCl)->client->pers;
    memset(p_pers, 0, 0x194u);
    p_pers->mStats[6][28] = 0;
    p_pers->mBaseScore = 0;
    Entity* result = EntityManager::sInst->GetPlayer(currCl);
    result->client->pers.rank = 0;
    return result;
}

// ============================================================================
// FN_Multiplayer_Rank2 - ea: 0x4F4740
// ============================================================================
Entity* FN_Multiplayer_Rank2()
{
    clientPersistent_t* p_pers =
        &EntityManager::sInst->GetPlayer(currCl)->client->pers;
    memset(p_pers, 0, 0x194u);
    p_pers->mStats[6][28] = 0;
    p_pers->mBaseScore = 0;
    EntityManager::sInst->GetPlayer(currCl)->client->pers.mStats[0][3] = 15;
    Entity* result = EntityManager::sInst->GetPlayer(currCl);
    result->client->pers.rank = 1;
    return result;
}

// ============================================================================
// FN_Multiplayer_Rank3 - ea: 0x4F47C0
// ============================================================================
Client* FN_Multiplayer_Rank3()
{
    EntityManager::sInst->GetPlayer(currCl)->client->pers.mStats[0][3] = 40;
    Client* result = EntityManager::sInst->GetPlayer(currCl)->client;
    result->pers.rank = 2;
    return result;
}

// ============================================================================
// SplineMgr::EndOfSpline - ea: 0x4F59A0
// ============================================================================
namespace SplineMgr {
bool EndOfSpline(const float* p)
{
    return *p == -1.0f && *(p + 1) == -1.0f && *(p + 2) == -1.0f;
}
}
