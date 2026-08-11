// ============================================================================
// g_inspector_fns.cpp - InspectorManager menu callback functions (game2.o)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_inspector.h"
#include "game/logic/g_local.h"

#include <string.h>

// Cross-object externs
extern void ApplyControllerButtonConfig(int buttonConfig);  // ?ApplyControllerButtonConfig (game2.o)
extern void ApplyControllerStickConfig(int stickConfig);    // ?ApplyControllerStickConfig (game2.o)
extern void ChangePlayersMaxHealth(int newMaxHealth);       // ?ChangePlayersMaxHealth
extern void SV_DifficultyEasy();                            // sv_ccmds.cpp
extern void SV_DifficultyMedium();
extern void SV_DifficultyHard();
extern void TogglePakRender();                              // ?TogglePakRender
extern void Cmd_God_f(Entity* ent);                         // g.o
extern void Cmd_Noclip_f(Entity* ent);                      // g.o
extern void G_Printf(const char* fmt, ...);                 // g.o
extern void CG_MotionBlur_Begin(float level, float plateauTime,
                                float fadeTime);            // ?Begin@CG_MotionBlur
extern unsigned int BrocSys_GetEnt(const Broc::string& value, int fieldnameHash,
                                   unsigned int* array, int capacity, int flags);
extern void BrocSys_ShellShock(unsigned int entityHandleVal,
                               const Broc::string& shock, float fVal);
extern Entity* _Return_MF_UnderCrossHair();                 // ?_Return_MF_UnderCrossHair
extern int g_requiredIndex;   // ?g_requiredIndex@@3HA (game2.o)

namespace AeAssert {
extern bool IsIgnored();
extern bool Assert(const char* fmt, ...);
}

// game2.o data globals
extern int gNewEasyMaxHealth;
extern int gNewMediumMaxHealth;
extern int gNewHardMaxHealth;
extern vmCvar_t g_drawEntBBoxes;  // ?g_drawEntBBoxes

// nglDebug struct (ngl_debug.o)
struct nglDebugStruct {
    unsigned char ShowPerfInfo;  // +0x00
};
extern nglDebugStruct nglDebug;  // ?nglDebug@@3UnglDebugStruct@@A

// ============================================================================
// Controller config callbacks
// ============================================================================

// ea: 0x4F41F0
void FN_ControlConfigA()
{
    ApplyControllerButtonConfig(0);
}

// ea: 0x4F4200
void FN_ControlConfigB()
{
    ApplyControllerButtonConfig(1);
}

// ea: 0x4F4210
void FN_ControlConfigC()
{
    ApplyControllerButtonConfig(2);
}

// ea: 0x4F4220
void FN_ControlConfigD()
{
    ApplyControllerButtonConfig(3);
}

// ea: 0x4F4230
bool FN_ControlInvertAim()
{
    bool result = !gSaveGameData[0].mStubData.mInvertAim;
    gSaveGameData[0].mStubData.mInvertAim = !gSaveGameData[0].mStubData.mInvertAim;
    return result;
}

// ea: 0x4F4240
void FN_ControlSticksDefault()
{
    ApplyControllerStickConfig(0);
}

// ea: 0x4F4250
void FN_ControlSticksSouthPaw()
{
    ApplyControllerStickConfig(1);
}

// ea: 0x4F4260
void FN_ControlSticksLegacy()
{
    ApplyControllerStickConfig(2);
}

// ea: 0x4F4270
void FN_ControlSticksLegacySouthPaw()
{
    ApplyControllerStickConfig(3);
}

// ea: 0x4F4280
void FN_SelectGodMode()
{
    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    Cmd_God_f(player);
}

// ea: 0x4F42A0
void FN_ChangeEasyMaxHealth()
{
    ChangePlayersMaxHealth(gNewEasyMaxHealth);
}

// ea: 0x4F42B0
void FN_ChangeMediumMaxHealth()
{
    ChangePlayersMaxHealth(gNewMediumMaxHealth);
}

// ea: 0x4F42C0
void FN_ChangeHardMaxHealth()
{
    ChangePlayersMaxHealth(gNewHardMaxHealth);
}

// ea: 0x4F42D0
void FN_ApplyEasyDifficultyChanges()
{
    SV_DifficultyEasy();
    ChangePlayersMaxHealth(gNewEasyMaxHealth);
}

// ea: 0x4F42F0
void FN_ApplyMediumDifficultyChanges()
{
    SV_DifficultyMedium();
    ChangePlayersMaxHealth(gNewMediumMaxHealth);
}

// ea: 0x4F4310
void FN_ApplyHardDifficultyChanges()
{
    SV_DifficultyHard();
    ChangePlayersMaxHealth(gNewHardMaxHealth);
}

// ea: 0x4F4330
void FN_NoClip()
{
    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    Cmd_Noclip_f(player);
}

// ea: 0x4F4350
void FN_PakRender()
{
    TogglePakRender();
}

// ea: 0x4F4360
bool FN_NGLStatDisplay()
{
    bool result = nglDebug.ShowPerfInfo != 1;
    nglDebug.ShowPerfInfo = nglDebug.ShowPerfInfo != 1;
    return result;
}

// ea: 0x4F4370
void FN_DefaultShellshockTestFunction()
{
    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    if (player != nullptr)
    {
        Broc::string shock("default");
        BrocSys_ShellShock(player->mHandle.mHandle.mVal, shock, 30.0f);
    }
}

// ea: 0x4F43F0
void FN_PainShellshockTestFunction()
{
    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    if (player != nullptr)
    {
        Broc::string shock("pain");
        BrocSys_ShellShock(player->mHandle.mHandle.mVal, shock, 30.0f);
    }
}

// ea: 0x4F4470
void FN_DeathShellshockTestFunction()
{
    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    if (player != nullptr)
    {
        Broc::string shock("death");
        BrocSys_ShellShock(player->mHandle.mHandle.mVal, shock, 30.0f);
    }
}

// ea: 0x4F44F0
void FN_CurgenMotionBlur()
{
    CG_MotionBlur_Begin(0.9f, 100.0f, 1.0f);
}

// ea: 0x4F46B0
void FN_DebugThread_Select_EntityHandle(DbLinkedHandle<EntityHandleDb, Entity> entityHandle)
{
    g_debugThread.m_entityHandle = entityHandle;
    g_debugThread.m_active = 1;
}

// ea: 0x4F4810
void FN_DumpThreadsForTarget()
{
}

// ea: 0x4F4820
void FN_DumpThreadsForAll()
{
}

// ea: 0x4F4830
int FN_DebugEntity_BBoxes()
{
    int result = ++g_drawEntBBoxes.integer;
    if (g_drawEntBBoxes.integer == 1)
    {
        g_drawEntBBoxes.integer = 2;
    }
    else if (result == 7)
    {
        g_drawEntBBoxes.integer = 0;
    }
    return result;
}

// ea: 0x4F8F20
Entity* FN_DebugThread_Select_Player()
{
    Entity* result = EntityManager::sInst->GetPlayer(currCl);
    g_debugThread.m_entityHandle.mHandle.mVal = result->mHandle.mHandle.mVal;
    g_debugThread.m_active = 1;
    return result;
}

// ea: 0x4F8F50
EntityManager* FN_DebugThread_Select_Level()
{
    g_debugThread.m_entityHandle.mHandle.mVal =
        EntityManager::sInst->mWorld->mHandle.mHandle.mVal;
    g_debugThread.m_active = 1;
    return EntityManager::sInst;
}

// ea: 0x503780
void FN_DebugThread_Select_EntityName(char* hashName, char* entityName)
{
    int v2 = (int)HashString::CalcHash(hashName);
    Broc::string value(entityName);
    unsigned int ent = BrocSys_GetEnt(value, v2, nullptr, 0, 0);
    unsigned int v4 = ent & 0xFFF;
    if (v4 < 0x540
        && ent >> 12 == EntityHandleDb::sInst.mElements[v4].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v4].mObject;
        if (mObject != nullptr)
        {
            g_debugThread.m_entityHandle.mHandle.mVal =
                mObject->mHandle.mHandle.mVal;
            g_debugThread.m_active = 1;
            G_Printf("^5Entity found\n");
            return;
        }
    }
    G_Printf("^5Cant find Entity\n");
}

// ea: 0x50B690
void FN_DebugThread_Select_Target()
{
    Entity* v0 = _Return_MF_UnderCrossHair();
    if (v0 != nullptr)
    {
        g_debugThread.m_entityHandle.mHandle.mVal = v0->mHandle.mHandle.mVal;
        g_debugThread.m_active = 1;
        Broc::entity ent{g_debugThread.m_entityHandle.mHandle.mVal};
        gpBrocAPI->mBrocExports.mAnimDebug(ent);
        G_Printf("^5Entity found\n");
    }
    else
    {
        g_debugThread.m_active = 0;
        G_Printf("^5Cant find Entity\n");
    }
}

// ea: 0x503840
void FN_DebugThread_Select_Nearest()
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    float vPlayerPos[3];
    Sentient_GetOrigin(Player->sentient, vPlayerPos);
    Entity* pClosest = nullptr;
    float bestDist = 1000000000.0f;
    for (unsigned int idx = 0; idx < 0x540; ++idx)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[idx].mObject;
        if (mObject == nullptr)
            continue;
        if (mObject == Player || mObject->sentient == nullptr)
            continue;
        float dx = mObject->r.currentOrigin.v.m128_f32[0] - vPlayerPos[0];
        float dy = mObject->r.currentOrigin.v.m128_f32[1] - vPlayerPos[1];
        float dz = mObject->r.currentOrigin.v.m128_f32[2] - vPlayerPos[2];
        float distSq = dx * dx + dy * dy + dz * dz;
        if (bestDist > distSq)
        {
            pClosest = mObject;
            bestDist = distSq;
        }
    }
    if (pClosest != nullptr)
    {
        g_debugThread.m_entityHandle.mHandle.mVal =
            pClosest->mHandle.mHandle.mVal;
        g_debugThread.m_active = 1;
        Broc::entity ent{g_debugThread.m_entityHandle.mHandle.mVal};
        gpBrocAPI->mBrocExports.mAnimDebug(ent);
    }
}

// ea: 0x5039E0
void FN_DebugThread_Select_Nearest_Vehicle()
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    float vPlayerPos[3];
    Sentient_GetOrigin(Player->sentient, vPlayerPos);
    Entity* v2 = nullptr;
    float bestDist = 1000000000.0f;
    for (unsigned int idx = 0; idx < 0x540; ++idx)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[idx].mObject;
        if (mObject == nullptr)
            continue;
        if (mObject->s.eType != 14)
            continue;
        float dx = mObject->r.currentOrigin.v.m128_f32[0] - vPlayerPos[0];
        float dy = mObject->r.currentOrigin.v.m128_f32[1] - vPlayerPos[1];
        float dz = mObject->r.currentOrigin.v.m128_f32[2] - vPlayerPos[2];
        float distSq = dx * dx + dy * dy + dz * dz;
        if (bestDist > distSq)
        {
            v2 = mObject;
            bestDist = distSq;
        }
    }
    if (v2 != nullptr)
    {
        g_debugThread.m_entityHandle.mHandle.mVal = v2->mHandle.mHandle.mVal;
        g_debugThread.m_active = 1;
        Broc::entity ent{g_debugThread.m_entityHandle.mHandle.mVal};
        gpBrocAPI->mBrocExports.mAnimDebug(ent);
    }
}

// ea: 0x503B70
void FN_DebugThread_Select_Nearest_Trigger()
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    float vPlayerPos[3];
    Sentient_GetOrigin(Player->sentient, vPlayerPos);
    Entity* v2 = nullptr;
    float bestDist = 1000000000.0f;
    unsigned int triggerHash = HashString::CalcHash("trigger_multiple");
    for (unsigned int idx = 0; idx < 0x540; ++idx)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[idx].mObject;
        if (mObject == nullptr)
            continue;
        if (mObject->mClassNameHash.mHash != triggerHash)
            continue;
        float dx = mObject->r.currentOrigin.v.m128_f32[0] - vPlayerPos[0];
        float dy = mObject->r.currentOrigin.v.m128_f32[1] - vPlayerPos[1];
        float dz = mObject->r.currentOrigin.v.m128_f32[2] - vPlayerPos[2];
        float distSq = dx * dx + dy * dy + dz * dz;
        if (bestDist > distSq
            && g_debugThread.m_entityHandle.mHandle.mVal
                   != mObject->mHandle.mHandle.mVal)
        {
            v2 = mObject;
            bestDist = distSq;
        }
    }
    if (v2 != nullptr)
    {
        g_debugThread.m_entityHandle.mHandle.mVal = v2->mHandle.mHandle.mVal;
        g_debugThread.m_active = 1;
        Broc::entity ent{g_debugThread.m_entityHandle.mHandle.mVal};
        gpBrocAPI->mBrocExports.mAnimDebug(ent);
    }
}

// ea: 0x503D20
void FN_DebugThread_Select_UniqueIndex()
{
    for (unsigned int idx = 0; idx < 0x540; ++idx)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[idx].mObject;
        if (mObject == nullptr)
            continue;
        if (mObject->uniqueIndex != g_requiredIndex)
            continue;
        g_debugThread.m_entityHandle.mHandle.mVal =
            mObject->mHandle.mHandle.mVal;
        g_debugThread.m_active = 1;
        Broc::entity ent{g_debugThread.m_entityHandle.mHandle.mVal};
        gpBrocAPI->mBrocExports.mAnimDebug(ent);
        return;
    }
}
