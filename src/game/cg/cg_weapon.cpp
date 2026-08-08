// ============================================================================
// cg_weapon.cpp - weapon selection, firing FX, recoil, hold breath (cg.o)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/game_types.h"
#include "game/core/core_types.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

extern int currCl;
extern int level_time;
extern float* dword_F63B8C[4 * 1580];
extern int dword_F63B34[4 * 1580];
extern int dword_F62960[4 * 1580];
extern int dword_F64024[4 * 1580];
extern int dword_F64028[4 * 1580];
extern int dword_F6402C[4 * 1580];
extern int dword_F6405C[4 * 1580];
extern int dword_F64060[4 * 1580];
extern void* sADSMetaAnimPlayer;
extern int dword_F69BF4;
extern float FOCUS_DISTANCE;
extern float angle[4 * 395];
extern float dword_F63C70[4 * 1580];
extern float dword_F63C74[4 * 1580];
extern float dword_F63C78[4 * 1580];
extern float dword_F63C80[4 * 1580];
extern float dword_F63C84[4 * 1580];
extern float dword_F63C88[4 * 1580];
extern int cg_aWeaponSelectTime[4];
extern int cg_weaponCycleDelay;
extern int cgGlobal_frametime;
extern struct level_locals_t { int time; } level;

extern Entity* EntityManager_GetPlayer(void* mgr, int idx);
extern void* EntityManager_sInst;
extern void* EntityManager_mPlayers[16];
extern int Com_BitCheck(const int* array, int bitNum);
extern void* BG_GetInfoForWeapon(int weapon);
extern int BG_GetWeaponIndexForName(const char* pszName);
extern int BG_GetWeaponSlotForName(const char* pszSlotName);
extern int BG_SelectWeaponIndex(int iWeaponIndex, int client);
extern int BG_GetNumWeapons();
extern int BG_WeaponAmmo(const PlayerState* pPS, int iWeapon);
extern int BG_ClipForWeapon(int iWeapon);
extern bool BG_AllowPlayerWeaponAtVehiclePos(int vehType, int vehPos);
extern int BG_IsPlayerWeaponInSlot(const PlayerState* pPS, int iWeaponIndex,
                                   int bAnyMode);
extern PlayerState* GetPlayerState(int idx);
extern Entity* GetPlayer(int idx);
extern bool TestSpecialWeapon(Entity* player, int weapon);
extern void CG_CycleWeap(int bNext, int bIgnoreEmpty);
extern void CG_Error(const char* msg, ...);
extern char* CG_Argv(int arg);
extern void Cmd_ArgvBuffer(int arg, char* buffer, int bufferLength);
extern int CG_DObjGetViewModelTagMatrix(void* obj, unsigned int tag_name_hash,
                                        void* tagMat);
extern int CG_DObjGetWorldTagMatrix(Entity* entity, void* obj,
                                    unsigned int tag_name_hash,
                                    void* tagMat);
extern void BG_EvaluateTrajectory(const void* tr, int atTime,
                                  math::Position3* result);
extern float DiffTrack(float tgt, float cur, float rate, float deltaTime);
extern void PostEffectEventScriptCall(const Entity* ent, const char* scriptId,
                                      bool queue, int pakid, bool important);
extern float player_breath_hold_time;
extern float player_breath_snd_delay;
extern float player_breath_snd_lerp;
extern int cgGlobal_time;
extern int dword_F641E8[4 * 1580];
extern int dword_F641E0[4 * 1580];
extern int dword_F641E4[4 * 1580];
extern int dword_F641EC[4 * 1580];
extern int dword_F6A2A0[4 * 802];
extern unsigned int tag_flash_hash;
extern unsigned int HashString_CalcHash(const char* str);
extern void DObjFree(void* obj, int bClearTree);
extern void mem_heap_free(void* ptr);
extern void DObj_Dtor(void* obj);
extern void DObj_OpDelete(void* obj);
extern void PostEffectEventWeapon(const Entity* ent, const char* weaponType,
                                  int weaponAction);
extern void PostEffectEventPointLightFlash(const Entity* ent,
                                           const char* weaponType,
                                           int weaponAction);
extern void AngleVectors(const math::Position3* angles, float* forward,
                         float* right, float* up);

struct weaponInfo_s;
struct DObj;

struct weaponFileInfoFull : weaponFileInfo_t {
    int  weapClass;   // +0x84
    int  slot;        // +0x88
    int  bOffHand;    // +0x8C
    int  bSlotStackable;  // +0x90
};

static char buffer_0[256];

// ea: 0x00687F40
bool CG_GetWeapReticleZoom(float* pfZoom)
{
    Entity* player = EntityManager_GetPlayer(EntityManager_sInst, currCl);
    float fWeaponPosFrac = player->client->ps.fWeaponPosFrac;
    *pfZoom = 0.0f;
    float* v2 = dword_F63B8C[1580 * currCl];
    if ((v2[1616] == 0.0f && v2[1620] == 0.0f) || fWeaponPosFrac == 0.0f)
        return false;
    float v4;
    if (dword_F63B34[1580 * currCl] != 0)
    {
        float v3 = fWeaponPosFrac - (1.0f - v2[1608]);
        *pfZoom = v3;
        if (v3 <= 0.0f)
            return *pfZoom > 0.0099999998f;
        v4 = v3 / v2[1608];
    }
    else
    {
        float v5 = fWeaponPosFrac - (1.0f - v2[1612]);
        *pfZoom = v5;
        if (v5 <= 0.0f)
            return *pfZoom > 0.0099999998f;
        v4 = v5 / v2[1612];
    }
    *pfZoom = v4;
    return *pfZoom > 0.0099999998f;
}

// ea: 0x0068F520
void CG_SaveViewModelAnimTrees()
{
}

// ea: 0x0068F530
void CG_LoadViewModelAnimTrees()
{
}

// ea: 0x0068F540
void CG_SaveWeaponInfo()
{
}

// ea: 0x0068F550
void CG_LoadWeaponInfo()
{
}

// ea: 0x0068F560
void CG_FreeWeapons()
{
    if (dword_F6A2A0[0] != 0)
    {
        DObjFree((void*)dword_F6A2A0[0], 1);
        if (dword_F6A2A0[0] != 0)
        {
            DObj_Dtor((void*)dword_F6A2A0[0]);
            DObj_OpDelete((void*)dword_F6A2A0[0]);
        }
        dword_F6A2A0[0] = 0;
        if (sADSMetaAnimPlayer != nullptr)
        {
            delete (void*)sADSMetaAnimPlayer;
            sADSMetaAnimPlayer = nullptr;
        }
        if (dword_F69BF4 != 0)
        {
            mem_heap_free((void*)dword_F69BF4);
            dword_F69BF4 = 0;
        }
    }
}

// ea: 0x0068FAB0
float CG_WeaponTrackValue(float tgt, float cur, float rate)
{
    float v3 = (float)fabs((double)(tgt - cur));
    float step = ((cgGlobal_frametime * 0.001f) * (tgt - cur)) * rate;
    if (v3 <= 0.0049999999f || (float)fabs((double)step) > v3)
        return tgt;
    return step + cur;
}

// ea: 0x006921F0
void CG_WeaponFlash(Entity* entity, int weaponNum,
                    const math::Position3* origin, int bViewFlash)
{
    if (bViewFlash != 0)
    {
        weaponFileInfo_t* info =
            (weaponFileInfo_t*)BG_GetInfoForWeapon(weaponNum);
        PostEffectEventWeapon(entity, info->szInternalName,
                              1 /* kActionWEAPON_VIEW_FLASH */);
    }
    else
    {
        weaponFileInfo_t* info =
            (weaponFileInfo_t*)BG_GetInfoForWeapon(weaponNum);
        PostEffectEventWeapon(entity, info->szInternalName,
                              2 /* kActionWEAPON_WORLD_FLASH */);
    }
    weaponFileInfo_t* InfoForWeapon =
        (weaponFileInfo_t*)BG_GetInfoForWeapon(weaponNum);
    PostEffectEventPointLightFlash(entity, InfoForWeapon->szInternalName,
                                   2);
}

// ea: 0x00692260
void CG_OffsetLMG(float i_XOffset)
{
    float focusPoint[3], right[3], up[3];
    AngleVectors((math::Position3*)&angle[395 * currCl], focusPoint, right, up);
    int v1 = 1580 * currCl;
    float v3 = (dword_F63C80[v1] * i_XOffset) + dword_F63C70[v1];
    float v4 = (dword_F63C84[v1] * i_XOffset) + dword_F63C74[v1];
    float v5 = (dword_F63C88[v1] * i_XOffset) + dword_F63C78[v1];
    dword_F63C70[v1] = v3;
    dword_F63C74[v1] = v4;
    dword_F63C78[v1] = v5;
    float v18 = (FOCUS_DISTANCE * right[1] + v4) - dword_F63C74[v1];
    float v7 = (FOCUS_DISTANCE * focusPoint[0] + v3) - dword_F63C70[v1];
    float v19 = (FOCUS_DISTANCE * up[2] + v5) - v5;
    float focusDist = (float)sqrt((double)(v18 * v18 + v7 * v7));
    if (focusDist < 1.0f)
        focusDist = 1.0f;
    float v21 = (float)fabs((double)focusDist);
    float v22 = (float)fabs((double)v19);
    if (v22 + v21 == 0.0f)
    {
        angle[v1 * 4] = 0.0f * -57.295776f;
        return;
    }
    float v20 =
        1.0f / (float)sqrt((double)(focusDist * focusDist + v19 * v19));
    float v13;
    if (v22 <= v21)
    {
        float v14 = v20 * v22;
        v22 = v14;
        if (v14 >= 0.5f)
        {
            v22 = (float)sqrt((double)fabs((1.0f - v22) * 0.5f));
            float q = v22 * v22;
            v13 = ((((((((q * v22) * q) * q) * -0.1079625f)
                      - ((q * q) * 0.15000001f))
                     - ((q * v22) * 0.33333331f))
                    - (v22 * 2.0f))
                   + 1.570796f);
        }
        else
        {
            float q = v14 * v14;
            v13 = (((((((q * v14) * q) * q) * 0.053981241f)
                      + ((q * q) * 0.075000003f))
                     + ((q * v14) * 0.1666667f))
                    + v14);
        }
    }
    else
    {
        v22 = v20 * v21;
        if (v22 >= 0.5f)
        {
            v22 = (float)sqrt((double)fabs((1.0f - v22) * 0.5f));
            float q = v22 * v22;
            v13 = ((((((((q * v22) * q) * q) * -0.1079625f)
                      - ((q * q) * 0.15000001f))
                     - ((q * v22) * 0.33333331f))
                    - (v22 * 2.0f))
                   + 1.570796f);
        }
        else
        {
            float v11 = v20 * v21;
            float q = v11 * v11;
            v13 = (((((((q * v11) * q) * q) * 0.053981241f)
                      + ((q * q) * 0.075000003f))
                     + ((q * v11) * 0.1666667f))
                    + v11);
        }
        v13 = 1.5707964f - v13;
    }
    if (focusDist < 0.0f)
        v13 = 3.1415927f - v13;
    if (v19 < 0.0f)
        v13 = 0.0f - v13;
    angle[v1 * 4] = v13 * -57.295776f;
}

// ea: 0x006925C0
bool CG_WeaponSelectable(int i)
{
    Entity* Player = EntityManager_GetPlayer(EntityManager_sInst, currCl);
    return Com_BitCheck(Player->client->ps.weapons, i) != 0;
}

// ea: 0x00692710
int CG_Weapon_f()
{
    Entity* result = EntityManager_GetPlayer(EntityManager_sInst, currCl);
    if (result != 0 && *(int*)((char*)result + 596) != 0)
    {
        Client* client = EntityManager_GetPlayer(EntityManager_sInst, currCl)->client;
        if ((client->ps.pm_flags & 0x4000) == 0)
        {
            client = EntityManager_GetPlayer(EntityManager_sInst, currCl)->client;
            if ((client->ps.eFlags & 0x100000) == 0)
            {
                Cmd_ArgvBuffer(1, buffer_0, 256);
                int result2 = BG_GetWeaponIndexForName(buffer_0);
                if (result2 != 0)
                    return BG_SelectWeaponIndex(result2, currCl);
                const char* v1 = CG_Argv(1);
                result2 = atoi(v1);
                if (result2 != 0)
                    return BG_SelectWeaponIndex(result2, currCl);
            }
        }
    }
    return 0;
}

// ea: 0x00693730
int CG_WeaponFireRecoil()
{
    Entity* p = EntityManager_GetPlayer(EntityManager_sInst, currCl);
    float fPosLerp = p->client->ps.fWeaponPosFrac;
    float* info = dword_F63B8C[1580 * currCl];
    float fPitchKick, v2;
    if (fPosLerp == 1.0f)
    {
        fPitchKick = ((info[2040] - info[2036]) * (rand() * 0.000030517578f))
                     + info[2036];
        float v0 = rand() * 0.000030517578f;
        v2 = ((info[2048] - info[2044]) * v0) + info[2044];
    }
    else
    {
        fPitchKick = ((info[2112] - info[2108]) * (rand() * 0.000030517578f))
                     + info[2108];
        float v3 = rand() * 0.000030517578f;
        v2 = ((info[2120] - info[2116]) * v3) + info[2116];
    }
    int v1 = 1580 * currCl;
    float negPitch = -fPitchKick;
    float halfYaw = v2 * -0.5f;
    memcpy(&dword_F64024[v1], &negPitch, 4);
    memcpy(&dword_F64028[v1], &v2, 4);
    memcpy(&dword_F6402C[v1], &halfYaw, 4);
    float fPitchKicka, v6;
    if (fPosLerp <= 0.0f)
    {
        fPitchKicka = ((info[2080] - info[2076]) * (rand() * 0.000030517578f))
                      + info[2076];
        float v7 = rand() * 0.000030517578f;
        v6 = ((info[2088] - info[2084]) * v7) + info[2084];
    }
    else
    {
        fPitchKicka = ((info[2008] - info[2004]) * (rand() * 0.000030517578f))
                      + info[2004];
        float v4 = rand() * 0.000030517578f;
        v6 = ((info[2016] - info[2012]) * v4) + info[2012];
    }
    int result = 1580 * currCl;
    float pitchAccum = *(float*)&dword_F6405C[result] + fPitchKicka;
    float yawAccum = *(float*)&dword_F64060[result] + v6;
    memcpy(&dword_F6405C[result], &pitchAccum, 4);
    memcpy(&dword_F64060[result], &yawAccum, 4);
    return result * 4;
}

// ea: 0x006939F0
void CG_EjectWeaponBrass(Entity* entity, int event)
{
    if (entity->client != nullptr && entity->s.eType < 0x12u)
    {
        int weapon = entity->s.weapon;
        if (weapon != 0)
        {
            if (weapon <= BG_GetNumWeapons())
            {
                if (event == 189)
                {
                    weaponFileInfo_t* info =
                        (weaponFileInfo_t*)BG_GetInfoForWeapon(weapon);
                    PostEffectEventWeapon(entity, info->szInternalName,
                                          3 /* kActionWEAPON_LAST_SHOT_EJECT */);
                }
                else
                {
                    weaponFileInfo_t* info =
                        (weaponFileInfo_t*)BG_GetInfoForWeapon(weapon);
                    PostEffectEventWeapon(entity, info->szInternalName,
                                          4 /* kActionWEAPON_SHELL_EJECT */);
                }
            }
            else
            {
                CG_Error("CG_FireWeapon: ent->weapon > BG_GetNumWeapons()");
            }
        }
    }
}

// ea: 0x00693A80
void CG_WeaponIKAddToFireQueue(Entity* attacker, int weapon)
{
    weaponFileInfoFull* InfoForWeapon =
        (weaponFileInfoFull*)BG_GetInfoForWeapon(weapon);
    weaponFileInfoFull* weapInfo = InfoForWeapon;
    if (InfoForWeapon == nullptr)
    {
        CG_ASSERT("weapInfo", "c:\\cod\\code\\game\\cg_weapons.cpp", 4203);
    }
    int v3;
    for (v3 = 0; v3 < 15; ++v3)
    {
        if (InfoForWeapon->weapClass == 9 /* WEAPCLASS_PISTOL */)
        {
            if (v3 != 0)
            {
                attacker->client->AnimIKFireEvents[v3].fireTime = 0;
                attacker->client->AnimIKFireEvents[v3].fireWeapon = 0;
            }
            else
            {
                attacker->client->AnimIKFireEvents[0].fireTime =
                    level.time + 100;
                InfoForWeapon = weapInfo;
                attacker->client->AnimIKFireEvents[0].fireWeapon = weapon;
            }
            continue;
        }
        if (attacker->client->AnimIKFireEvents[v3].fireTime == 0)
            break;
    }
    if (v3 < 15)
    {
        attacker->client->AnimIKFireEvents[v3].fireTime = level.time + 100;
        attacker->client->AnimIKFireEvents[v3].fireWeapon = weapon;
    }
}

// ea: 0x00693B80
int CG_HoldBreathInit()
{
    dword_F641E0[0] = -1;
    dword_F641E4[0] = 0;
    dword_F641E8[0] = 0;
    dword_F641EC[0] = 0;
    return 0;
}

// ea: 0x00692B10
Client* CG_WeaponSlot_f()
{
    if (dword_F62960[1580 * currCl] != 0)
    {
        Client* client =
            EntityManager_GetPlayer(EntityManager_sInst, currCl)->client;
        if ((client->ps.pm_flags & 0x4000) == 0)
        {
            client = EntityManager_GetPlayer(EntityManager_sInst, currCl)->client;
            if ((client->ps.eFlags & 0x100000) == 0)
            {
                client = EntityManager_GetPlayer(EntityManager_sInst, currCl)->client;
                if ((client->ps.pm_flags & 0x80000) != 0)
                {
                    if (cgGlobal_frametime - cg_aWeaponSelectTime[currCl]
                        >= cg_weaponCycleDelay)
                    {
                        cg_aWeaponSelectTime[currCl] = cgGlobal_frametime;
                        const char* v1 = CG_Argv(1);
                        int v2 = BG_GetWeaponSlotForName(v1);
                        if (v2 == 0)
                        {
                            const char* v4 = CG_Argv(1);
                            v2 = atoi(v4);
                        }
                        if (v2 > 0 && v2 < 10)
                        {
                            PlayerState* ps = GetPlayerState(currCl);
                            int v5 = ps->weaponslots[v2];
                            if (v5 != 0 && CG_WeaponSelectable(v5))
                                return (Client*)BG_SelectWeaponIndex(v5, currCl);
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}

// ea: 0x00692D50
bool CG_WeaponSlot_f(int iSlot)
{
    if (dword_F62960[1580 * currCl] == 0
        || (EntityManager_GetPlayer(EntityManager_sInst, currCl)->client->ps
                .pm_flags
            & 0x4000) != 0)
    {
        return false;
    }
    Client* client =
        EntityManager_GetPlayer(EntityManager_sInst, currCl)->client;
    Entity* p = EntityManager_GetPlayer(EntityManager_sInst, currCl);
    bool result = BG_AllowPlayerWeaponAtVehiclePos(p->client->ps.vehType,
                                                   client->ps.vehPos);
    if ((client->ps.eFlags & 0x106000) == 0 || result)
    {
        Entity* Player =
            EntityManager_GetPlayer(EntityManager_sInst, currCl);
        weaponFileInfoFull* InfoForWeapon = (weaponFileInfoFull*)BG_GetInfoForWeapon(
            Player->client->ps.weapon);
        weaponFileInfoFull* v6 = InfoForWeapon;
        weaponFileInfoFull* pWeap = InfoForWeapon;
        if (InfoForWeapon != nullptr
            && ((InfoForWeapon->weapClass == 10 /* WEAPCLASS_LMG */
                 && (GetPlayerState(currCl)->pm_flags & 0x20) != 0)
                || v6->weapClass == 16))
        {
            return false;
        }
        if (cgGlobal_time - cg_aWeaponSelectTime[currCl]
            < cg_weaponCycleDelay)
            return false;
        cg_aWeaponSelectTime[currCl] = cgGlobal_time;
        if (iSlot <= 0 || iSlot >= 10)
            return false;
        PlayerState* ps = GetPlayerState(currCl);
        int v9 = ps->weaponslots[iSlot];
        if (ps->weaponslots[iSlot] == 0
            || (ps->pm_flags & 0x10000) != 0)
            return false;
        weaponFileInfoFull* v10 =
            (weaponFileInfoFull*)BG_GetInfoForWeapon(v9);
        if (v10 != nullptr && v10->bOffHand != 0)
        {
            int v11 = BG_ClipForWeapon(v9);
            if (GetPlayerState(currCl)->ammoclip[v11] <= 0)
                return false;
            v6 = pWeap;
        }
        if (iSlot == 9)
        {
            if (GetPlayerState(currCl)->weapon == v9)
            {
                PlayerState* v12 = GetPlayerState(currCl);
                return BG_SelectWeaponIndex(v12->lastWeapon, currCl) != 0;
            }
            Entity* v13 = GetPlayer(currCl);
            if (!TestSpecialWeapon(v13, v9))
                return false;
        }
        else if (iSlot == 7 && v6 != nullptr
                 && v6->slot == 13 /* WEAPSLOT_BINOCS */)
        {
            PlayerState* v12 = GetPlayerState(currCl);
            return BG_SelectWeaponIndex(v12->lastWeapon, currCl) != 0;
        }
        if (CG_WeaponSelectable(v9) != 0)
            return BG_SelectWeaponIndex(v9, currCl) != 0;
        return false;
    }
    return result;
}

// ea: 0x0069A660
void CG_OutOfAmmoChange()
{
    if (dword_F62960[1580 * currCl] != 0)
    {
        float* info = dword_F63B8C[1580 * currCl];
        if (info[184] != 0.0f && BG_GetNumWeapons() >= 1)
        {
            int v0 = 1;
            while (true)
            {
                Entity* Player =
                    EntityManager_GetPlayer(EntityManager_sInst, currCl);
                if (Com_BitCheck(Player->client->ps.weapons, v0) != 0)
                {
                    weaponFileInfoFull* w =
                        (weaponFileInfoFull*)BG_GetInfoForWeapon(v0);
                    if (w->bSlotStackable != 0
                        && w->weapClass != 11 /* WEAPCLASS_GRENADE */
                        && w->slot == (int)info[180])
                    {
                        Entity* v2 =
                            EntityManager_GetPlayer(EntityManager_sInst,
                                                    currCl);
                        if (BG_WeaponAmmo(&v2->client->ps, v0) != 0)
                            break;
                    }
                }
                if (++v0 > BG_GetNumWeapons())
                    goto LABEL_10;
            }
            BG_SelectWeaponIndex(v0, currCl);
            return;
        }
    LABEL_10:
        {
            Entity* v3 = EntityManager_GetPlayer(EntityManager_sInst, currCl);
            if (BG_IsPlayerWeaponInSlot(&v3->client->ps,
                                        (int)*dword_F63B8C[1580 * currCl],
                                        1) != 0)
            {
                int iSlotPreferenceOrder[4] = {1, 2, 3, 4};
                int iNewSlot = 0;
                int v6 = 0;
                while (true)
                {
                    v6 = iSlotPreferenceOrder[iNewSlot];
                    if (EntityManager_mPlayers[currCl] != nullptr
                        && ((Entity*)EntityManager_mPlayers[currCl])
                                   ->client->ps.weaponslots[v6]
                               != 0
                        && (v6 == 1 || v6 == 2))
                    {
                        Client* client =
                            EntityManager_GetPlayer(EntityManager_sInst,
                                                    currCl)->client;
                        Entity* v9 =
                            EntityManager_GetPlayer(EntityManager_sInst,
                                                    currCl);
                        if (BG_WeaponAmmo(&v9->client->ps,
                                          client->ps.weaponslots[v6]) != 0)
                            break;
                    }
                    if (++iNewSlot >= 4)
                        goto LABEL_21;
                }
                Entity* v23 =
                    EntityManager_GetPlayer(EntityManager_sInst, currCl);
                BG_SelectWeaponIndex(v23->client->ps.weaponslots[v6], currCl);
            }
            else
            {
            LABEL_21:
                {
                    Entity* v12 = (Entity*)EntityManager_mPlayers[currCl];
                    Client* v14 = v12->client;
                    if (BG_IsPlayerWeaponInSlot(
                            &((Entity*)EntityManager_mPlayers[currCl])
                                 ->client->ps,
                            v14->ps.lastWeapon, 1) != 0)
                    {
                        Client* v19 =
                            ((Entity*)EntityManager_mPlayers[currCl])->client;
                        if (BG_WeaponAmmo(
                                &((Entity*)EntityManager_mPlayers[currCl])
                                     ->client->ps,
                                v19->ps.lastWeapon) != 0)
                        {
                            Entity* v22 = EntityManager_GetPlayer(
                                EntityManager_sInst, currCl);
                            BG_SelectWeaponIndex(
                                v22->client->ps.lastWeapon, currCl);
                        }
                        else
                        {
                            CG_CycleWeap(1, 1);
                        }
                    }
                    else
                    {
                        CG_CycleWeap(1, 1);
                    }
                }
            }
        }
    }
}

// ea: 0x0069AE40
int CG_HoldBreathUpdate()
{
    int v2 = currCl;
    if (dword_F641E8[1580 * currCl] > 0)
        dword_F641E8[1580 * currCl] -= cgGlobal_frametime;
    Entity* player = EntityManager_GetPlayer(EntityManager_sInst, currCl);
    bool holding = (player->client->ps.mFlags & 2) != 0;
    int result;
    if (!holding)
    {
        int v3 = 1580 * currCl;
        float cur = *(float*)&dword_F641EC[1580 * currCl];
        float tracked = DiffTrack(1.0f, cur, player_breath_snd_lerp,
                                  cgGlobal_frametime * 0.001f);
        memcpy(&dword_F641EC[1580 * currCl], &tracked, 4);
        int v4 = dword_F641E0[v3];
        if (v4 >= 0)
        {
            if (v4 > dword_F641E4[v3])
            {
                Entity* v8 =
                    EntityManager_GetPlayer(EntityManager_sInst, currCl);
                PostEffectEventScriptCall(v8, "BREATH_HOLD_HEART_BEAT", false,
                                          -1, false);
            }
        }
        else
        {
            dword_F641E0[v3] = 0;
            if (dword_F641E8[v3] > 0)
            {
                dword_F641E4[v3] = 0;
            }
            else
            {
                Entity* v5 =
                    EntityManager_GetPlayer(EntityManager_sInst, currCl);
                PostEffectEventScriptCall(v5, "BREATH_HOLD_BREATH_IN", false,
                                          -1, false);
                float v6 = player_breath_snd_delay * 1000.0f;
                int v7 = 1580 * currCl;
                dword_F641E4[v7] = 2;
                dword_F641E8[v7] = (int)v6;
            }
        }
        result = dword_F641E0[1580 * currCl];
        result += cgGlobal_frametime;
        dword_F641E0[1580 * currCl] = result;
    }
    else
    {
        if (dword_F641E0[1580 * currCl] >= 0)
        {
            float v10 = player_breath_hold_time * 1000.0f;
            int v11 = cgGlobal_frametime + dword_F641E0[1580 * currCl];
            dword_F641E0[1580 * currCl] = v11;
            if (v11 <= (int)v10)
            {
                if (dword_F641E8[1580 * v2] <= 0)
                {
                    Entity* Player =
                        EntityManager_GetPlayer(EntityManager_sInst, v2);
                    PostEffectEventScriptCall(Player,
                                              "BREATH_HOLD_BREATH_OUT", false,
                                              -1, false);
                    v2 = currCl;
                    dword_F641E8[1580 * currCl] =
                        (int)(player_breath_snd_delay * 1000.0f);
                }
            }
            else
            {
                Entity* v12 =
                    EntityManager_GetPlayer(EntityManager_sInst, v2);
                PostEffectEventScriptCall(v12, "BREATH_HOLD_GASP", false, -1,
                                          false);
                v2 = currCl;
            }
        }
        result = 6320 * v2;
        dword_F641E0[result] = -1;
        dword_F641E4[result] = 0;
        dword_F641EC[result] = 0;
    }
    return result;
}

static unsigned int sTagFlashInit = 0;

// ea: 0x0069ACD0
void CG_WeaponUpdateLoopingSound(Entity* entity)
{
    if ((sTagFlashInit & 1) == 0)
    {
        sTagFlashInit |= 1u;
        tag_flash_hash = HashString_CalcHash("tag_flash");
    }
    int fireSndDelay = entity->fireSndDelay;
    if (fireSndDelay > 0)
    {
        entity->fireSndDelay = fireSndDelay - cgGlobal_frametime;
        DObjSkelMat tagMtx;
        math::Position3 origin;
        int ViewModelTagMatrix;
        if (entity
            == EntityManager_GetPlayer(EntityManager_sInst, currCl))
        {
            void* v4 = (void*)dword_F6A2A0[802 * currCl];
            if (v4 == nullptr)
                goto LABEL_11;
            ViewModelTagMatrix =
                CG_DObjGetViewModelTagMatrix(v4, tag_flash_hash, &tagMtx);
        }
        else
        {
            void* mDObj = entity->mDObj;
            if (mDObj == nullptr)
                goto LABEL_11;
            ViewModelTagMatrix =
                CG_DObjGetWorldTagMatrix(entity, mDObj, tag_flash_hash,
                                         &tagMtx);
        }
        if (ViewModelTagMatrix != 0)
        {
            tagMtx.origin[1] = tagMtx.axis[2][1];
            tagMtx.origin[3] = tagMtx.axis[2][3];
            goto LABEL_12;
        }
    LABEL_11:
        BG_EvaluateTrajectory(&entity->s.pos, cgGlobal_time,
                              (math::Position3*)&tagMtx.origin[1]);
    LABEL_12:
        int weapon = entity->s.weapon;
        if (entity->fireSndDelay <= 0)
        {
            weaponFileInfo_t* InfoForWeapon =
                (weaponFileInfo_t*)BG_GetInfoForWeapon(weapon);
            PostEffectEventWeapon(entity, InfoForWeapon->szInternalName,
                                  5 /* kActionWEAPON_STOP_FIRE */);
        }
        else
        {
            weaponFileInfo_t* v8 =
                (weaponFileInfo_t*)BG_GetInfoForWeapon(weapon);
            PostEffectEventWeapon(entity, v8->szInternalName,
                                  6 /* kActionWEAPON_LOOP_FIRE */);
        }
    }
}
