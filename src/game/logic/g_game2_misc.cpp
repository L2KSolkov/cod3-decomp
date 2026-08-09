// ============================================================================
// g_game2_misc.cpp - game2.o misc functions (default pak, mission stats,
// multiplayer rank helpers, spline util)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <new>
#include <stdio.h>
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
extern bool gTotalResetOfLevel;         // ?gTotalResetOfLevel (game2.o)
extern const char* notSet;              // ?notSet (game2.o, "Not Set")
extern int bg_iNumWeapons;              // ?bg_iNumWeapons (game.o)

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

extern _xmission_data gXMissionData[];  // ?gXMissionData (game2.o)
extern _xmission_data* gMissionData;      // ?gMissionData (game2.o)
extern _xmission_data gTempMissionData;   // ?gTempMissionData (game2.o)

// ============================================================================
// Mission stats enums/types (stat_support.cpp)
// ============================================================================
enum eMissionStats {
    eInvalid = -1,
    eTotalKills = 0,
    eTotalDeaths = 1,
    eGrenadeKillsInterval = 0x10,
    eNumStats = 0x11,
};

enum eWeaponCategory {
    eRiffle = 0,
    eNonGerman = 5,
};

struct _weapon_name {
    char name[0x20];  // +0x00
};

struct _weapon_category {
    _weapon_name* weapons;  // +0x00
};

extern _weapon_category gWeaponCategories[];  // ?gWeaponCategories (game2.o)

#define STAT_ASSERT(which)                                                  \
    do {                                                                    \
        if ((int)(which) <= (int)eInvalid || (int)(which) >= (int)eNumStats) \
        {                                                                   \
            if (!AeAssert::IsIgnored()                                      \
                && AeAssert::Assert("invalid range access detected!"))      \
                __debugbreak();                                             \
        }                                                                   \
    } while (0)

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

    ScriptEventHandler();               // ea: 0x4F9860
    ~ScriptEventHandler();              // ea: 0x4F59D0
    bool AddEvent(HashString h, HashString callback);  // ea: 0x4F98D0
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
// stat_LoadMissionStatsFrom - load mission stats from an xmission data array
// ea: 0x4F65C0
// ============================================================================
void stat_LoadMissionStatsFrom(_xmission_data* data, int num_missions)
{
    _xmission_data* table = gXMissionData;
    while (num_missions > 0)
    {
        if (_stricmp(table->name, data->name) == 0)
        {
            table->missionTime = data->missionTime;
            table->missionLastTick = data->missionLastTick;
            memcpy(table->missionStat, data->missionStat, 0x44u);
            table->weaponsUsed[0] = data->weaponsUsed[0];
            table->weaponsUsed[1] = data->weaponsUsed[1];
            table->weaponsUsed[2] = data->weaponsUsed[2];
        }
        ++table;
        ++data;
        --num_missions;
    }
}

// ea: 0x4F6640
void stat_ClearMissionStats()
{
    gTotalResetOfLevel = true;
}

// ea: 0x4F6650
_xmission_data* stat_GetLevelName()
{
    if (gMissionData == nullptr)
        return (_xmission_data*)notSet;
    return gMissionData;
}

// ea: 0x4F6660
int stat_GetStat(eMissionStats which, bool allLevels, bool andCommited)
{
    STAT_ASSERT(which);
    if (!gMissionDataInitialized)
        return 0;
    int result = 0;
    if (gMissionData != nullptr)
    {
        if (allLevels)
        {
            _xmission_data* v4 = gXMissionData;
            for (;;)
            {
                result += v4->missionStat[which];
                if (v4 == gMissionData)
                    break;
                ++v4;
                if (v4 == nullptr)
                    return result;
            }
            result += gTempMissionData.missionStat[which];
        }
        else
        {
            result = gTempMissionData.missionStat[which];
            if (andCommited)
                result += gMissionData->missionStat[which];
        }
    }
    return result;
}

// ea: 0x4F6710
void stat_SetStat(eMissionStats which, int stat, _xmission_data* xd)
{
    STAT_ASSERT(which);
    if (xd != nullptr)
        xd->missionStat[which] = stat;
    else
        gTempMissionData.missionStat[which] = stat;
}

// ea: 0x4F6790
void stat_IncStat(eMissionStats which, _xmission_data* xd)
{
    STAT_ASSERT(which);
    if (xd != nullptr)
        ++xd->missionStat[which];
    else
        ++gTempMissionData.missionStat[which];
}

// ea: 0x4F6800
void stat_DecStat(eMissionStats which, _xmission_data* xd)
{
    STAT_ASSERT(which);
    if (xd != nullptr)
    {
        --xd->missionStat[which];
        if (xd->missionStat[which] < 0)
            xd->missionStat[which] = 0;
    }
    else
    {
        --gTempMissionData.missionStat[which];
        if (gTempMissionData.missionStat[which] < 0)
            gTempMissionData.missionStat[which] = 0;
    }
}

// ea: 0x4F6890
double stat_GetAvgStat(unsigned int which)
{
    STAT_ASSERT(which);
    if (!gMissionDataInitialized || gMissionData == nullptr)
        return 0.0;
    int v2 = 0;
    _xmission_data* v3 = gXMissionData;
    int v4 = 0;
    for (;;)
    {
        ++v4;
        v2 += gMissionData->missionStat[which];
        if (v3 == gMissionData)
            break;
        ++v3;
        if (v3 == nullptr)
            return 0.0;
    }
    return (double)v2 / (double)v4;
}

// ea: 0x4F6950
void stat_GetWeaponMasks(int* m1, int* m2, int* m3)
{
    if (m1 != nullptr && m2 != nullptr && m3 != nullptr)
    {
        *m3 = 0;
        *m2 = 0;
        *m1 = 0;
        if (gMissionDataInitialized && gMissionData != nullptr)
        {
            *m1 = gTempMissionData.weaponsUsed[0] | gMissionData->weaponsUsed[0];
            *m2 = gTempMissionData.weaponsUsed[1] | gMissionData->weaponsUsed[1];
            *m3 = gTempMissionData.weaponsUsed[2] | gMissionData->weaponsUsed[2];
        }
    }
}

// ea: 0x4F69D0
int stat_GetMissionCompletionTime()
{
    if (gMissionDataInitialized && gMissionData != nullptr)
        return gMissionData->missionTime;
    return 0;
}

// ea: 0x4F69F0
char stat_UpdateMissionCompletionTime()
{
    if (gMissionDataInitialized && gMissionData != nullptr)
    {
        unsigned long long v1 = __rdtsc();
        int v2 = (int)(v1 - gMissionData->missionLastTick);
        gMissionData->missionLastTick = (int)__rdtsc();
        gMissionData->missionTime += v2;
        gMissionData->missionTime = gMissionData->missionTime / 1000;
        return (char)gMissionData->missionTime;
    }
    return 0;
}

// ea: 0x4F6A70
char stat_WasPlayerWeaponUsed(int weaponHash, int* bitSetArray)
{
    if (!gMissionDataInitialized || gMissionData == nullptr)
        return 0;
    int v3 = 0;
    if (bg_iNumWeapons <= 0)
        return 0;
    for (;;)
    {
        weaponFileInfo_t* info = BG_GetInfoForWeapon(v3);
        if (info != nullptr && info->internalNameHash == (unsigned int)weaponHash)
            break;
        ++v3;
        if (v3 >= bg_iNumWeapons)
            return 0;
    }
    if (v3 == -1)
        return 0;
    int v5 = v3 >> 5;
    int v6 = 1 << (v3 % 32);
    if ((v6 & gMissionData->weaponsUsed[v3 >> 5]) == 0
        && (v6 & gTempMissionData.weaponsUsed[v5]) == 0)
        return 0;
    if (bitSetArray != nullptr)
        bitSetArray[v5] |= v6;
    return 1;
}

// ea: 0x4F6B10
void stat_FillMask(eWeaponCategory category, int* i1, int* i2, int* i3)
{
    *i1 = 0;
    *i2 = 0;
    *i3 = 0;
    int bitSet = 0;
    int v10 = 0;
    int v11 = 0;
    if (category <= eNonGerman)
    {
        _weapon_name* weapons = gWeaponCategories[category].weapons;
        if (weapons->name[0] != 0)
        {
            do
            {
                stat_WasPlayerWeaponUsed((int)HashString::CalcHash(weapons->name),
                                         &bitSet);
                char v6 = weapons[1].name[0];
                ++weapons;
                if (v6 == 0)
                    break;
            } while (true);
        }
        *i1 = bitSet;
        *i2 = v10;
        *i3 = v11;
    }
}

// ea: 0x4F6B90 / 0x4F6C10
bool stat_WasPlayerWeaponCategoryUsed(eWeaponCategory category, bool exclusive)
{
    if (!gMissionDataInitialized || gMissionData == nullptr)
        return false;
    int mask1, mask2, mask3;
    stat_FillMask(category, &mask1, &mask2, &mask3);
    if ((mask3 | mask2 | mask1) == 0)
        return false;
    if (!exclusive)
        return true;
    if (category < 4)
    {
        for (int v3 = eRiffle; v3 != 4; ++v3)
        {
            if (v3 != (int)category)
            {
                stat_FillMask((eWeaponCategory)v3, &mask1, &mask2, &mask3);
                if ((mask3 | mask2 | mask1) != 0)
                    return false;
            }
        }
        return true;
    }
    if (category == 4)
    {
        stat_FillMask(eNonGerman, &mask3, &mask2, &mask1);
        return (mask1 | mask2 | mask3) == 0;
    }
    if (category == eNonGerman)
    {
        stat_FillMask((eWeaponCategory)4, &mask3, &mask2, &mask1);
        return (mask1 | mask2 | mask3) == 0;
    }
    return false;
}

// ea: 0x4F6C80
void stat_SetPlayerWeaponUsed(int weapon)
{
    if (gMissionDataInitialized && gMissionData != nullptr)
        gTempMissionData.weaponsUsed[weapon >> 5] |= 1 << (weapon % 32);
}

// ea: 0x4FE870
int stat_support_Initialize()
{
    if (gXMissionData[0].name[0] != 0)
    {
        _xmission_data* table = gXMissionData;
        do
        {
            table->missionTime = 0;
            table->missionLastTick = 0;
            memset(table->missionStat, 0, 0x44u);
            table->weaponsUsed[0] = 0;
            table->weaponsUsed[1] = 0;
            table->weaponsUsed[2] = 0;
            ++table;
        } while (table->name[0] != 0);
    }
    memset(&gTempMissionData, 0, sizeof(gTempMissionData));
    gTotalResetOfLevel = false;
    gMissionDataInitialized = true;
    return 0;
}

// ea: 0x4FE8D0
char stat_SetMissionToTrack(const char* mission_name)
{
    gMissionData = nullptr;
    _xmission_data* v1 = gXMissionData;
    if (mission_name == nullptr)
    {
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("mission name must be set"))
            __debugbreak();
    }
    if (!gMissionDataInitialized || gXMissionData[0].name[0] == 0)
        return 0;
    while (_stricmp(v1->name, mission_name) != 0)
    {
        ++v1;
        if (v1->name[0] == 0)
            return 0;
    }
    gMissionData = v1;
    stat_ResetMissionStats(gTotalResetOfLevel);
    gTotalResetOfLevel = false;
    if (v1 != nullptr)
        ++v1->missionStat[0];
    else
        ++gTempMissionData.missionStat[0];
    return 1;
}

// ============================================================================
// ButtonEntry - key binding entry (0xC, IDA verified)
// ============================================================================
struct BaseCmdFuncInfo;

struct ButtonEntry {
    unsigned char mKeyInfoIndex;         // +0x00
    unsigned char _pad1[3];
    const BaseCmdFuncInfo* mBoundCmdPress;   // +0x04
    const BaseCmdFuncInfo* mBoundCmdRelease; // +0x08

    void SetCmdBinding();                    // ea: 0x4F6CC0
    const BaseCmdFuncInfo* GetBoundCmdPress();  // ea: 0x4F6D50
    const BaseCmdFuncInfo* GetBoundCmdRelease();  // ea: 0x4F6D80
};
static_assert(sizeof(ButtonEntry) == 0xC, "ButtonEntry size mismatch");

struct KeyInfoEntry {
    int mState;              // +0x00 (bitfields mDown/mRepeats)
    char* mBoundCmdName;     // +0x04
};
static_assert(sizeof(KeyInfoEntry) == 8, "KeyInfoEntry size mismatch");

extern const BaseCmdFuncInfo* GetCmd(const char* cmdName);  // ?GetCmd (core.o)
extern void Com_sprintf(char* dest, int size, const char* fmt, ...);

// KeyInfo::mKeys - per-client array of 256 KeyInfoEntry
extern KeyInfoEntry gKeyInfoMKeys[1][256];  // ?mKeys@KeyInfo (game2.o)

// ea: 0x4F6CC0
void ButtonEntry::SetCmdBinding()
{
    int mKeyInfoIndex = this->mKeyInfoIndex;
    mBoundCmdPress = nullptr;
    mBoundCmdRelease = nullptr;
    KeyInfoEntry* v4 = &gKeyInfoMKeys[currCl][mKeyInfoIndex];
    char* mBoundCmdName = v4->mBoundCmdName;
    if (mBoundCmdName != nullptr)
    {
        mBoundCmdPress = GetCmd(v4->mBoundCmdName);
        if (*mBoundCmdName == '+')
        {
            char releaseName[512];
            Com_sprintf(releaseName, 512, "-%s", mBoundCmdName + 1);
            mBoundCmdRelease = GetCmd(releaseName);
        }
    }
}

// ea: 0x4F6D50
const BaseCmdFuncInfo* ButtonEntry::GetBoundCmdPress()
{
    if (mBoundCmdPress == nullptr)
    {
        if (mKeyInfoIndex == 0xFF)
            return mBoundCmdPress;
    }
    else if (mKeyInfoIndex != 0xFF)
    {
        return mBoundCmdPress;
    }
    SetCmdBinding();
    return mBoundCmdPress;
}

// ea: 0x4F6D80
const BaseCmdFuncInfo* ButtonEntry::GetBoundCmdRelease()
{
    if (mBoundCmdRelease == nullptr)
    {
        if (mKeyInfoIndex == 0xFF)
            return mBoundCmdRelease;
    }
    else if (mKeyInfoIndex != 0xFF)
    {
        return mBoundCmdRelease;
    }
    SetCmdBinding();
    return mBoundCmdRelease;
}

// ============================================================================
// ScriptEventHandler ctor / AddEvent (extend the existing struct)
// ============================================================================
extern void* ScriptEventHandler_sAllocator;  // ?sAllocator@ScriptEventHandler
extern void* PoolAllocator_Allocate(void* self, unsigned int s,
                                    bool forceHeapAlloc);

// ea: 0x4F9860
ScriptEventHandler::ScriptEventHandler()
{
    memset(m_dlist_node, 0, sizeof(m_dlist_node));
    for (int i = 0; i < 7; ++i)
    {
        mEvents[i].notify.mHash = 0;
        mEvents[i].callback.mHash = 0;
    }
    mNext = nullptr;
}

// ea: 0x4F98D0
bool ScriptEventHandler::AddEvent(HashString h, HashString callback)
{
    ScriptEventHandler* cur = this;
    for (;;)
    {
        int v4 = 0;
        do
        {
            if (cur->mEvents[v4].callback.mHash == 0)
            {
                cur->mEvents[v4].notify = h;
                cur->mEvents[v4].callback = callback;
                return true;
            }
            ++v4;
        } while (v4 < 7);
        if (cur->mNext != nullptr)
        {
            cur = cur->mNext;
            continue;
        }
        void* v6 = PoolAllocator_Allocate(ScriptEventHandler_sAllocator, 0x44u,
                                          false);
        ScriptEventHandler* v7 = nullptr;
        if (v6 != nullptr)
            v7 = new (v6) ScriptEventHandler;
        cur->mNext = v7;
        if (v7 == nullptr)
            break;
        cur = v7;
    }
    return false;
}

// ============================================================================
// FnReverseOptions - flip all effect-sound toggles
// ea: 0x4F4510
// ============================================================================
struct SoundOptions {
    int mFxDontPlayFootSteps;     // +0x00
    int mFxDontPlayGearRattle;    // +0x04
    int mFxDontPlayLanding;       // +0x08
    int mFxDontPlayScriptCall;    // +0x0C
    int mFxDontPlayScriptCall_Dir;// +0x10
    int mFxDontPlayWeapon;        // +0x14
    int mFxDontPlayBulletHit;     // +0x18
    int mFxDontPlayGrenadeBounce; // +0x1C
    int mFxDontPlayProjExplode;   // +0x20
    int mFxDontPlayVehicle;       // +0x24
    int mFxDontPlayTurret;        // +0x28
    int mFxDontPlayVehicleWheel;  // +0x2C
    int mFxDontPlayLightFlash;    // +0x30
    int mFxDontPlayMusic;         // +0x34
};
static_assert(sizeof(SoundOptions) == 0x38, "SoundOptions size mismatch");

extern SoundOptions gSoundOptions;  // ?gSoundOptions (game2.o)

int FnReverseOptions()
{
    gSoundOptions.mFxDontPlayFootSteps =
        gSoundOptions.mFxDontPlayFootSteps == 0;
    gSoundOptions.mFxDontPlayGearRattle =
        gSoundOptions.mFxDontPlayGearRattle == 0;
    gSoundOptions.mFxDontPlayLanding =
        gSoundOptions.mFxDontPlayLanding == 0;
    gSoundOptions.mFxDontPlayScriptCall =
        gSoundOptions.mFxDontPlayScriptCall == 0;
    gSoundOptions.mFxDontPlayScriptCall_Dir =
        gSoundOptions.mFxDontPlayScriptCall_Dir == 0;
    gSoundOptions.mFxDontPlayWeapon =
        gSoundOptions.mFxDontPlayWeapon == 0;
    gSoundOptions.mFxDontPlayBulletHit =
        gSoundOptions.mFxDontPlayBulletHit == 0;
    gSoundOptions.mFxDontPlayGrenadeBounce =
        gSoundOptions.mFxDontPlayGrenadeBounce == 0;
    gSoundOptions.mFxDontPlayProjExplode =
        gSoundOptions.mFxDontPlayProjExplode == 0;
    gSoundOptions.mFxDontPlayVehicle =
        gSoundOptions.mFxDontPlayVehicle == 0;
    gSoundOptions.mFxDontPlayTurret =
        gSoundOptions.mFxDontPlayTurret == 0;
    gSoundOptions.mFxDontPlayVehicleWheel =
        gSoundOptions.mFxDontPlayVehicleWheel == 0;
    gSoundOptions.mFxDontPlayLightFlash =
        gSoundOptions.mFxDontPlayLightFlash == 0;
    int result = gSoundOptions.mFxDontPlayMusic;
    gSoundOptions.mFxDontPlayMusic =
        gSoundOptions.mFxDontPlayMusic == 0;
    return result;
}

// ============================================================================
// TestFPS::GetFilename - ea: 0x4F6F70
// ============================================================================
extern cvar_t* sv_mapname;  // ?sv_mapname@@3PAUcvar_t@@A

void TestFPS::GetFilename(char* filename)
{
    char path[256];
    sprintf(path, "c:\\cod\\assets\\levels\\%s\\stats\\", sv_mapname->string);
    sprintf(filename, "%sC%i.csv", path, mCellIndex);
}

// ============================================================================
// SplineMgr - spline asset bank (0x184, IDA verified)
// ============================================================================
struct SplineEntry {
    int pakId;   // +0x00 (PAK_ID_INVALID = -1)
    void* file;  // +0x04
    int pad8;    // +0x08
};

class SplineMgr {
public:
    unsigned char m_assetBase[4];      // +0x00 AssetBankSet
    SplineEntry mList[32];             // +0x04

    SplineEntry* GetUnusedEntry();     // ea: 0x4F9700
    void UnloadBank(int pakId);        // ea: 0x4F97C0
    static bool EndOfSpline(const float* p);  // ea: 0x4F59A0
};

extern bool AeAssert_Error(const char* fmt, ...);

// ea: 0x4F9700
SplineEntry* SplineMgr::GetUnusedEntry()
{
    for (int i = 0; i < 32; ++i)
    {
        if (mList[i].pakId == -1)
            return &mList[i];
    }
    if (!AeAssert::IsIgnored() && AeAssert_Error("Too many spline files loaded"))
        __debugbreak();
    return nullptr;
}

// ea: 0x4F97C0
void SplineMgr::UnloadBank(int pakId)
{
    int v4 = 0;
    for (;;)
    {
        if (v4 >= 32)
            return;
        if (mList[v4].pakId == pakId)
            break;
        ++v4;
    }
    mList[v4].pakId = -1;
    mList[v4].file = nullptr;
}

// ============================================================================
// SmokeGrenadeMgr visibility helpers
// ============================================================================
extern float sTime0, sTime1, sTime2, sTime3, sTime4;
extern float sOpacity2, sOpacity3;
extern bool g_drawSmokeGren_integer;  // vmCvar_t.integer
extern float ClampRange(const float* in, const float* beg, const float* end);

// ea: 0x4FA0E0
float SmokeGrenadeMgr::CalcOpacity(const SmokeGrenadeInfo& info) const
{
    float v2 = info.mTime;
    if (sTime0 > v2)
        return 0.0f;
    if (sTime1 > v2)
    {
        float in = (v2 - sTime0) / (sTime1 - sTime0);
        float beg = 0.0f;
        float end = 1.0f;
        return ClampRange(&in, &beg, &end);
    }
    if (sTime2 > v2)
        return 1.0f;
    if (sTime3 > v2)
    {
        float in = (v2 - sTime2) / (sTime3 - sTime2);
        float beg = 0.0f;
        float end = 1.0f;
        float v4 = ClampRange(&in, &beg, &end);
        return (1.0f - v4) * sOpacity2 + sOpacity3 * v4;
    }
    float beg = (v2 - sTime3) / (sTime4 - sTime3);
    float in = 0.0f;
    float end = 1.0f;
    return (1.0f - ClampRange(&beg, &in, &end)) * sOpacity3;
}

// ============================================================================
// SmokeGrenadeMgr visibility helpers (uses apsEffect bounds + opacity)
// ============================================================================
#include "aeps/apsEffect.h"
#include "aeps/apsGroup.h"

// ea: 0x4FA220
bool SmokeGrenadeMgr::PointCanSeePoint(const float* startPoint,
                                       const float* endPoint,
                                       float visThreshold)
{
    SmokeGrenadeInfo* mElements = mSmokeGrenadeInfoList.mElements;
    SmokeGrenadeInfo* end = &mElements[mSmokeGrenadeInfoList.mSize];
    if (mElements == end)
        return true;
    for (; mElements != end; ++mElements)
    {
        apsEffect* mEffect = (apsEffect*)mElements->mEffect;
        apsBounds bounds;
        mEffect->GetBounds(bounds);
        apsSphere sph = bounds.Sphere();
        bool hit = SegmentSphereIntersection(
            startPoint, endPoint, &sph.mSphere.v.m128_f32[0],
            sph.mSphere.v.m128_f32[3] * 0.33333334f);
        mElements->bHit[0] = hit;
        if (!hit)
            continue;
        int mSize = mEffect->mGroups.mSize;
        for (int i = 0; i < mSize; ++i)
        {
            apsGroup* group = mEffect->mGroups.mElements[i];
            if (group == nullptr)
                continue;
            apsBounds gb = group->mBounds;
            apsSphere gs = gb.Sphere();
            bool ghit = SegmentSphereIntersection(
                startPoint, endPoint, &gs.mSphere.v.m128_f32[0],
                gs.mSphere.v.m128_f32[3] * 0.25f);
            mElements->bHit[i + 1] = ghit;
            if (ghit)
            {
                if (CalcOpacity(*mElements) <= visThreshold)
                    break;
                return false;
            }
        }
    }
    return true;
}

// ea: 0x4FA400
bool SmokeGrenadeMgr::EntityCanSeePoint(const Entity* ent,
                                        const float* endPoint,
                                        float visThreshold)
{
    float startPoint[3];
    startPoint[0] = ent->r.currentOrigin.v.m128_f32[0];
    startPoint[1] = ent->r.currentOrigin.v.m128_f32[1];
    startPoint[2] = (ent->r.maxs.v.m128_f32[2] * 0.5f)
        + ent->r.currentOrigin.v.m128_f32[2];
    return PointCanSeePoint(startPoint, endPoint, visThreshold);
}

// ea: 0x4FA460
bool SmokeGrenadeMgr::EntityCanSeeEntity(const Entity* ent,
                                         const Entity* targEnt,
                                         float visThreshold)
{
    float startPoint[3];
    float endPoint[3];
    float v4 = ent->r.maxs.v.m128_f32[2];
    startPoint[0] = ent->r.currentOrigin.v.m128_f32[0];
    startPoint[1] = ent->r.currentOrigin.v.m128_f32[1];
    endPoint[0] = targEnt->r.currentOrigin.v.m128_f32[0];
    endPoint[1] = targEnt->r.currentOrigin.v.m128_f32[1];
    startPoint[2] = (v4 * 0.5f) + ent->r.currentOrigin.v.m128_f32[2];
    endPoint[2] = (targEnt->r.maxs.v.m128_f32[2] * 0.5f)
        + targEnt->r.currentOrigin.v.m128_f32[2];
    return PointCanSeePoint(startPoint, endPoint, visThreshold);
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
bool SplineMgr::EndOfSpline(const float* p)
{
    return *p == -1.0f && *(p + 1) == -1.0f && *(p + 2) == -1.0f;
}
