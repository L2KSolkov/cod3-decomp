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
