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
extern int dword_F641E0[2];
extern int dword_F641E4;
extern int dword_F641E8;
extern int dword_F641EC;
extern int dword_F6A2A0;
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
    if (dword_F6A2A0 != 0)
    {
        DObjFree((void*)dword_F6A2A0, 1);
        if (dword_F6A2A0 != 0)
        {
            DObj_Dtor((void*)dword_F6A2A0);
            DObj_OpDelete((void*)dword_F6A2A0);
        }
        dword_F6A2A0 = 0;
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
    dword_F641E4 = 0;
    dword_F641E8 = 0;
    dword_F641EC = 0;
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
