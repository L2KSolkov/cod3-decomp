// ============================================================================
// cg_weapon.cpp - weapon selection, firing FX, recoil, hold breath (cg.o)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/game_types.h"
#include "game/core/core_types.h"

#include <math.h>
#include <stdio.h>
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
struct sentient_s {
    int lastShotTime;  // +0x00
};
extern Entity* Entity_GetOwner(Entity* ent);
extern void AnimationPlayer_Play(void* player, void* anim, bool forceRestart,
                                 float fade_in, float callback_time,
                                 void* callback, float speed,
                                 float time_in_seconds_to_start);
extern void sWeaponAnimCallback();
extern void* cg_weapons;          // weaponInfo_s[]
extern int dword_F6A2A0[4 * 802];
extern int dword_F6A2A4[4 * 802];
extern int dword_F6A2A8[4 * 802];
extern void* EntityManager_mPlayers[16];
extern void* sADSMetaAnimPlayer;
extern void DObjFree(void* obj, int bClearTree);
extern void* DObj_GetTree(void* obj);
extern void* DObj_New(unsigned int size);
extern void DObj_Ctor(void* obj, int pakId);
extern void DObj_Dtor(void* obj);
extern void DObj_OpDelete(void* obj);
extern void DObjCreate(DObjModel* models, int numModels, void* tree,
                       void* dobj, int gameId);
extern void DObjCreateAnimationPlayer(void* obj, int modelIndex);
extern void Q_strncpyz(char* dest, const char* src, int destsize);
extern void* RE_RegisterModel(void* result, const char* name, int pakId,
                              int imagetype);
extern void ValidatePakId(int pakId);
extern int CurPakId();
extern int PAK_ID_MIN;
extern void* AnimBankManager_GetBank(void* mgr, int pakId);
extern void* AnimBankManager_sInst;
extern void* AnimBank_GetAnimTree(void* bank, const char* name);
extern void* cdGetAnim(unsigned int hash);
extern void* XAnimCreateTree(void* ent, void* anims);
extern void* XAnimIsLooped(void* anims, unsigned int animIndex);
extern void* XAnimGetLength(void* anims, unsigned int animIndex);
extern void Com_Error(int code, const char* fmt, ...);
extern void Com_Printf(const char* fmt, ...);
extern void* GetTextureData(const char* name, int image_type,
                            const char* fromPak);
extern char* va(const char* fmt, ...);
extern const char* SEH_StringEd_GetString(const char* pszReference);
extern void* bg_itemlist;
extern void CG_RegisterItemVisuals(int itemNum);
extern bool CG_SetupViewModelDObj(void* dobj, int weaponNum);
extern void CG_WeaponRunXModelAnims(PlayerState* ps, weaponInfo_s* weapon);
extern bool CG_GetWeapReticleZoom(float* pfZoom);
extern int InteractionController_CanRunWeaponAnims(void* self);
extern void* InteractionController_Inst(int instance);
extern void* PlayerAnimMgr_sInst;
extern void* Entity_GetRefEntity(Entity* ent);
extern void CG_AddPlayerWeapon(void* parent, PlayerState* ps, Entity* entity,
                               int bDrawGun);
extern void AddLeanToPosition(float* const vPosition, float fViewYaw,
                              float fLeanFrac, float fViewRoll,
                              float fLeanDist);
extern void DObjAdvanceAnimationPlayer(void* d, float deltaT);
extern void DObjInitServerTime(void* d, float dtime);
extern bool DObjUpdateServerInfo(void* obj, float dtime, bool bNotify,
                                 int animindex);
extern void DObjCalcAnim(void* obj, int iPhase);
extern void j_nullsub_82(void* obj, int* partBits);
extern void CG_UpdateViewModelPosAndOrientation(void* hand);
extern void RE_AddViewModelToScene(void* ent);
extern void RE_SetViewModelInfoIndex(int index);
extern void AxisCopy(const float (*in)[3], float (*out)[3]);
extern void DObjSkel2MatrixMultiply43(const DObjSkelMat* in1,
                                      const float (*in2)[3],
                                      DObjSkelMat* out);
extern int DObjGetBoneIndex(const DObj* obj, unsigned int boneNameHash);
extern DObjSkelMat* DObjGetMatrixArray(const DObj* obj, int modelIndex);
extern void* DObj_GetMat(void* obj, int boneIndex);
extern int GamePause_IsGamePaused(int client);
extern int CG_HoldBreathUpdate();
extern void CG_WeaponUpdateLoopingSound(Entity* entity);
extern void CG_WeaponIKAddToFireQueue(Entity* attacker, int weapon);
extern int CG_WeaponFireRecoil();
extern void CG_WeaponFlash(Entity* entity, int weaponNum,
                           const math::Position3* origin, int bViewFlash);
extern void CG_EjectWeaponBrass(Entity* entity, int event);
extern void PostEffectEventWeapon(const Entity* ent, const char* weaponType,
                                  int weaponAction);
extern void PostEffectEventPointLightFlash(const Entity* ent,
                                           const char* weaponType,
                                           int weaponAction);
extern void PostEffectEventWeaponFire1st(const Entity* ent,
                                         const char* weaponType,
                                         int weaponAction, int cacheSound,
                                         int barrel);
extern void PostEffectEventWeaponFire3rd(const Entity* ent,
                                         const char* weaponType,
                                         int weaponAction, int cacheSound);
extern int MultiplayerMgr_IsLocalPlayer(void* mgr, const Entity* player);
extern void* MultiplayerMgr_sInst;
extern int dword_F62960[4 * 1580];
extern int dword_F6403C[4 * 1580];
extern int dword_F64040[4 * 1580];
extern int dword_F64044[4 * 1580];
extern int dword_F64048[4 * 1580];
extern int dword_F6404C[4 * 1580];
extern int dword_F6355C[4 * 1580];
extern int dword_F640A4[4 * 1580];
extern int cg_drawGun;
extern float cg_gun_x;
extern float cg_gun_y;
extern float cg_gun_z;
extern float angle[4 * 395];
extern float dword_F63CB4[4 * 1580];
extern float dword_F63C80[4 * 1580];
extern float dword_F63C84[4 * 1580];
extern float dword_F63C88[4 * 1580];
extern float dword_F63C8C[4 * 1580];
extern float dword_F63C90[4 * 1580];
extern float dword_F63C94[4 * 1580];
extern float dword_F63C98[4 * 1580];
extern float dword_F63C9C[4 * 1580];
extern float dword_F63CA0[4 * 1580];
extern float dword_F64074[4 * 1580];
extern float dword_F64078[4 * 1580];
extern float dword_F6407C[4 * 1580];
extern float unk_F64080[4 * 6320];
extern int s_barrelTags[4];
extern float* ejectBrassCasingOrigin;
extern int dword_F5E6BC;
extern int dword_F5E6C0;
extern unsigned int tagHash;
extern unsigned int tag_brass_hash;
extern unsigned int HashString_CalcHash(const char* str);
extern void CG_ChangeViewmodelDobj(int client, const char* handModel);
extern void CG_RegisterWeapon(int weaponNum);
extern struct ServerTime_t { float mTickDelta; } ServerTime_sInst;
extern float tr_viewModelInfo_mWeaponScale[4];
extern int tr_viewModelInfo_mWeaponOrigin_used;
extern void* tr_viewModelInfo_mWeaponOrigin;
extern unsigned int tagHashInit;
extern void InteractionController_PostPhysicsUpdate(void* self, float deltaT);

extern Entity* EntityManager_GetPlayer(void* mgr, int idx);
extern void* EntityManager_sInst;
extern void* EntityManager_mPlayers[16];
extern int Com_BitCheck(const int* const array, int bitNum);
extern void* BG_GetInfoForWeapon(int weapon);
extern int BG_GetWeaponIndexForName(const char* pszName);
extern int BG_GetWeaponSlotForName(const char* pszSlotName);
extern int BG_SelectWeaponIndex(int iWeaponIndex, int client);
extern int BG_GetNumWeapons();
extern int BG_WeaponAmmo(const PlayerState* pPS, int iWeapon);
extern int BG_ClipForWeapon(int iWeapon);
extern bool BG_AllowPlayerWeaponAtVehiclePos(int vehType, int vehPos);
enum weapSlot_t : int;
extern weapSlot_t BG_IsPlayerWeaponInSlot(const PlayerState* pPS,
                                          int iWeaponIndex, int bAnyMode);
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

// ea: 0x00699380
void CG_StartWeaponAnim(int weaponNum, DObj* dobj, int animIndex,
                        float fadeInTime, float startTimeInSec,
                        int forceRestart)
{
    weaponInfo_s* weaponInfo = &((weaponInfo_s*)cg_weapons)[weaponNum];
    if (animIndex <= 0 || animIndex >= 25)
    {
        CG_ASSERT(
            "( animIndex > WEAP_ANIM_VIEWMODEL_START ) && ( animIndex < "
            "WEAP_ANIM_VIEWMODEL_END )",
            "c:\\cod\\code\\game\\cg_weapons.cpp", 348);
    }
    int j = 0;
    if (dobj->numModels != 0)
    {
        do
        {
            int v7 = (int)dobj->tree[j];
            void** animPlayers = dobj->animPlayers;
            if (v7 != 0 && animPlayers[j] != nullptr)
            {
                XAnimEntry* entries = (XAnimEntry*)(*(int*)(v7 + 8) + 4);
                XAnimEntry* v8 = &entries[animIndex];
                if (v8->anim == nullptr)
                    v8->anim = cdGetAnim(v8->hash);
                void* anim = v8->anim;
                if (anim != nullptr)
                {
                    AnimationPlayer_Play(
                        animPlayers[j], anim, forceRestart != 0, fadeInTime,
                        0.0f, j != 0 ? nullptr : &sWeaponAnimCallback,
                        weaponInfo->viewModelAnimRates[animIndex],
                        startTimeInSec);
                }
            }
            ++j;
        } while (j < dobj->numModels);
    }
}

// ea: 0x006A9BE0
void CG_CreateWeaponDObjsForClient(int client, int weapon)
{
    void* v2 = (void*)dword_F6A2A0[802 * client];
    if (v2 != nullptr)
    {
        if (weapon == dword_F6A2A4[802 * client])
            return;
        DObjFree(v2, 1);
        void* v3 = (void*)dword_F6A2A0[802 * client];
        if (v3 != nullptr)
        {
            DObj_Dtor(v3);
            DObj_OpDelete(v3);
        }
        dword_F6A2A0[802 * client] = 0;
    }
    int v4 = CurPakId();
    void* v5 = DObj_New(0xE8);
    void* v6;
    if (v5 != nullptr)
    {
        DObj_Ctor(v5, v4);
        v6 = v5;
    }
    else
    {
        v6 = nullptr;
    }
    dword_F6A2A0[802 * client] = (int)v6;
    if (CG_SetupViewModelDObj(v6, weapon))
        dword_F6A2A4[802 * client] = weapon;
}

// ea: 0x006AA8A0
void CG_UpdateHandViewmodels(const char* handModel)
{
    Entity* player = EntityManager_GetPlayer(EntityManager_sInst, currCl);
    int weapon = player->client->ps.weapon;
    if (strcmp(((weaponInfo_s*)cg_weapons)[weapon].handModel, handModel) != 0)
        CG_ChangeViewmodelDobj(currCl, handModel);
}

// ea: 0x006AA5B0
void CG_ChangeViewmodelDobj(int client, const char* handModel)
{
    Entity* player = EntityManager_GetPlayer(EntityManager_sInst, client);
    int weapon = player->client->ps.weapon;
    if (weapon != 0)
    {
        if (weapon < 1)
            CG_ASSERT("weaponNum >= 1", "c:\\cod\\code\\game\\cg_weapons.cpp",
                      1514);
        if (weapon > BG_GetNumWeapons())
            CG_ASSERT("weaponNum <= BG_GetNumWeapons()",
                      "c:\\cod\\code\\game\\cg_weapons.cpp", 1515);
        weaponInfo_s* v3 = &((weaponInfo_s*)cg_weapons)[weapon];
        int* v4 = &dword_F6A2A0[802 * client];
        weaponFileInfo_t* InfoForWeapon =
            (weaponFileInfo_t*)BG_GetInfoForWeapon(weapon);
        const char* szGunXModel = (const char*)&((char*)InfoForWeapon)[0x140];
        const char* szHandXModel = (const char*)&((char*)InfoForWeapon)[0x100];
        if (*v4 != 0 && szGunXModel[0] != 0)
        {
            void* pAnimTree = DObj_GetTree((void*)*v4);
            if (pAnimTree == nullptr)
                CG_ASSERT("pAnimTree", "c:\\cod\\code\\game\\cg_weapons.cpp",
                          1527);
            DObjFree((void*)*v4, 0);
            DObjModel dobjModels[2];
            memset(dobjModels, 0, sizeof(dobjModels));
            int v6 = CurPakId();
            void* result;
            dobjModels[0].model.mValue =
                RE_RegisterModel(&result, szHandXModel, v6, 6);
            dobjModels[0].model.mPakId = *(int*)((char*)&result + 4);
            dobjModels[0].boneName = Broc::string();
            dobjModels[0].ignoreCollision = 0;
            ValidatePakId(dobjModels[0].model.mPakId);
            if (dobjModels[0].model.mValue == nullptr)
                CG_ASSERT("dobjModels[0].model",
                          "c:\\cod\\code\\game\\cg_weapons.cpp", 1536);
            int v7 = CurPakId();
            void* result2;
            void* v8 = RE_RegisterModel(&result2, szGunXModel,
                                        v7, 6);
            dobjModels[1].model.mValue = *(void**)v8;
            dobjModels[1].model.mPakId = *(int*)((char*)v8 + 4);
            dobjModels[1].boneName = Broc::string("tag_weapon");
            dobjModels[1].ignoreCollision = 0;
            ValidatePakId(dobjModels[1].model.mPakId);
            if (dobjModels[1].model.mValue == nullptr)
                CG_ASSERT("dobjModels[1].model",
                          "c:\\cod\\code\\game\\cg_weapons.cpp", 1543);
            DObjCreate(dobjModels, 2, pAnimTree, (void*)*v4, 0);
            Q_strncpyz(v3->handModel, handModel, 24);
        }
    }
}

// ea: 0x006AB0B0
void CG_AddViewWeapon(PlayerState* ps)
{
    int v2 = 1;
    if (ps != nullptr
        && ((ps->pm_flags & 0x100000) != 0 || ps->pm_type < 4)
        && dword_F6355C[1580 * currCl] == 0)
    {
        float zoom = 0.0f;
        if (cg_drawGun != 2
            && (cg_drawGun == 0 || CG_GetWeapReticleZoom(&zoom) != 0))
            v2 = 0;
        if ((ps->eFlags & 0x6000) == 0)
        {
            int weapon = ps->weapon;
            if (weapon <= 0)
            {
                int v8 = 1580 * currCl;
                dword_F6403C[v8] = *(int*)&angle[1580 * currCl];
                dword_F64040[v8] = *(int*)&dword_F63CB4[v8];
                dword_F64044[v8] = 0;
                dword_F64048[v8] = 0;
                dword_F6404C[v8] = 0;
            }
            else
            {
                CG_RegisterWeapon(weapon);
                CG_CreateWeaponDObjsForClient(currCl, ps->weapon);
                weaponInfo_s* v4 = &((weaponInfo_s*)cg_weapons)[ps->weapon];
                void* v5 = InteractionController_Inst(currCl);
                if (InteractionController_CanRunWeaponAnims(v5) == 0
                    || PlayerAnimMgr_sInst != nullptr)
                    dword_F6A2A8[802 * currCl] = -1;
                else
                    CG_WeaponRunXModelAnims(ps, v4);
                Entity* Player = GetPlayer(currCl);
                refEntity_t* RefEntity =
                    (refEntity_t*)Entity_GetRefEntity(Player);
                RefEntity->renderfx = 12;
                CG_AddPlayerWeapon(RefEntity, ps, Player, v2);
            }
        }
    }
}

// ea: 0x006AB230
void CG_FireWeapon(Entity* attacker, EntityState* attackerState, int event,
                   unsigned int barrel)
{
    if (barrel >= 4)
    {
        CG_ASSERT("( barrel >= 0 ) && ( barrel < 4 )",
                  "c:\\cod\\code\\game\\cg_weapons.cpp", 4251);
    }
    if (!dword_F62960[1580 * currCl])
        return;
    Client* client = attacker->client;
    const char* weapInfo = (const char*)s_barrelTags[barrel];
    Entity* v6 = attacker;
    EntityState* ent;
    if (client && (client->ps.eFlags & 0x100000) != 0
        && client->ps.vehPos <= 1)
    {
        v6 = Entity_GetOwner(attacker);
        if (client->ps.vehPos)
        {
            ent = (EntityState*)(v6->scr_vehicle ? v6->scr_vehicle->gunnerWeapon : 0);
            weapInfo = "tag_gunner_flash";
        }
        else
        {
            if (event != 187)
            {
                ent = (EntityState*)attacker->s.weapon;
                goto LABEL_18;
            }
            ent = (EntityState*)(v6->scr_vehicle ? v6->scr_vehicle->altWeapon : 0);
            weapInfo = "tag_guncoax";
        }
    }
    else
    {
        ent = (EntityState*)attacker->s.weapon;
        if (attacker->scr_vehicle && attacker->scr_vehicle->shooter == 1)
        {
            ent = (EntityState*)(attacker->scr_vehicle ? attacker->scr_vehicle->gunnerWeapon : 0);
        }
    }
LABEL_18:
    if ((int)ent > 0)
    {
        if ((int)ent > BG_GetNumWeapons())
        {
            CG_Error("CG_FireWeapon: weapon > BG_GetNumWeapons()");
            return;
        }
        int weapon = (int)ent;
        weaponFileInfo_t* InfoForWeapon =
            (weaponFileInfo_t*)BG_GetInfoForWeapon(weapon);
        if (attacker->client)
        {
            if (attacker->sentient)
                attacker->sentient->lastShotTime = level.time;
            CG_WeaponIKAddToFireQueue(attacker, weapon);
        }
        int lc = 0;
        if (v6 == EntityManager_GetPlayer(EntityManager_sInst, currCl)
            && (GetPlayerState(currCl)->pm_flags & 0x180000) != 0)
        {
            CG_WeaponFireRecoil();
        }
        Entity* Player = EntityManager_GetPlayer(EntityManager_sInst, currCl);
        if (v6 == Player
            || (Player->tagInfo != nullptr && Player->tagInfo->parent == v6))
            lc = 1;
        if (level.time < v6->invulnerability_timeout)
        {
            v6->invulnerability_timeout = 0;
            MultiplayerMgr_IsLocalPlayer(MultiplayerMgr_sInst, v6);
        }
        if ((int)attackerState < 0x12)
        {
            if (v6->s.eType == 14)
            {
                PostEffectEventWeapon(v6, InfoForWeapon->szInternalName,
                                      2 /* kActionWEAPON_WORLD_FLASH */);
                weaponFileInfo_t* v18 =
                    (weaponFileInfo_t*)BG_GetInfoForWeapon(weapon);
                PostEffectEventPointLightFlash(v6, v18->szInternalName, 2);
            }
            else
            {
                math::Position3 origin;
                CG_WeaponFlash(v6, weapon, &origin, lc);
            }
            if (lc)
            {
                PostEffectEventWeapon(v6, InfoForWeapon->szInternalName,
                                      7 /* kActionEI_MELEE_ENEMY_WINNING */);
            }
            if (event == 189)
            {
                if (lc)
                    PostEffectEventWeaponFire1st(
                        v6, InfoForWeapon->szInternalName,
                        8 /* kActionWEAPON_LAST_SHOT_1ST */, -1, (int)barrel);
                else
                    PostEffectEventWeaponFire3rd(
                        v6, InfoForWeapon->szInternalName,
                        9 /* kActionWEAPON_LAST_SHOT_3RD */, -1);
            }
            else if (lc)
            {
                PostEffectEventWeaponFire1st(
                    v6, InfoForWeapon->szInternalName,
                    10 /* kActionWEAPON_FIRE_1ST */, 0, (int)barrel);
            }
            else
            {
                PostEffectEventWeaponFire3rd(
                    v6, InfoForWeapon->szInternalName,
                    11 /* kActionWEAPON_FIRE_3RD */, 1);
            }
        }
        if (!((weaponFileInfoFull*)InfoForWeapon)->bBoltAction)
            CG_EjectWeaponBrass(v6, event);
    }
}

// ea: 0x006A9CC0
void CG_RegisterWeapon(int weaponNum)
{
    CurPakId();
    if (!weaponNum)
        return;
    if (weaponNum < 1)
        CG_ASSERT("weaponNum >= 1", "c:\\cod\\code\\game\\cg_weapons.cpp",
                  1163);
    if (weaponNum > BG_GetNumWeapons())
        CG_ASSERT("weaponNum <= BG_GetNumWeapons()",
                  "c:\\cod\\code\\game\\cg_weapons.cpp", 1164);
    weaponInfo_s* v1 = &((weaponInfo_s*)cg_weapons)[weaponNum];
    weaponFileInfo_t* InfoForWeapon =
        (weaponFileInfo_t*)BG_GetInfoForWeapon(weaponNum);
    if (!v1->registered && InfoForWeapon)
    {
        memset(v1, 0, sizeof(weaponInfo_s));
        v1->registered = 1;
        v1->item = &((char*)bg_itemlist)[52 * weaponNum];
        CG_RegisterItemVisuals(weaponNum);
        const char* szGunXModel = (const char*)&((char*)InfoForWeapon)[0x140];
        const char* szHandXModel = (const char*)&((char*)InfoForWeapon)[0x100];
        const char* szInternalName = (const char*)&((char*)InfoForWeapon)[0x40];
        const char* szDisplayName = (const char*)&((char*)InfoForWeapon)[0xC0];
        const char* szWorldModel = (const char*)&((char*)InfoForWeapon)[0x180];
        const char* szPickupModel = (const char*)&((char*)InfoForWeapon)[0x1C0];
        if (szGunXModel[0] != 0)
        {
            if (!szHandXModel[0])
                Com_Error(2 /* ERR_DROP */, "%s", szDisplayName);
            if (!szInternalName[0])
                Com_Error(2, "%s", szDisplayName);
            void* Bank = AnimBankManager_GetBank(AnimBankManager_sInst,
                                                 PAK_ID_MIN);
            void* AnimTree =
                AnimBank_GetAnimTree(Bank, szInternalName);
            if (!AnimTree)
            {
                CG_ASSERT("pAnims",
                          "c:\\cod\\code\\game\\cg_weapons.cpp", 1266);
                goto LABEL_25;
            }
            XAnimEntry* entries = (XAnimEntry*)((char*)AnimTree + 4);
            int entryCount = *(int*)((char*)AnimTree + 0x38);
            for (int ai = 0; ai < entryCount; ++ai)
            {
                XAnimEntry* v7 = &entries[ai];
                unsigned int hash = v7->hash;
                v7->lastAttempt = 0;
                void* Anim = cdGetAnim(hash);
                v7->anim = Anim;
            }
            for (int i = 0; i < 25; ++i)
                v1->viewModelAnimRates[i] = 1.0f;
            v1->viewModelAnimRates[5] = 0.0f;
            v1->viewModelAnimRates[8] = 0.0f;
            v1->viewModelAnimRates[9] = 0.0f;
            v1->viewModelAnimRates[10] = 0.0f;
            v1->viewModelAnimRates[11] = 0.0f;
            v1->viewModelAnimRates[12] = 0.0f;
            v1->viewModelAnimRates[13] = 0.0f;
            v1->viewModelAnimRates[14] = 0.0f;
            v1->viewModelAnimRates[15] = 0.0f;
            v1->viewModelAnimRates[16] = 0.0f;
            XAnimIsLooped(AnimTree, 0x17);
            XAnimIsLooped(AnimTree, 0x18);
        }
        if (szWorldModel[0])
        {
            int v20 = CurPakId();
            void* result;
            v1->iWorldSurfIndex.mValue =
                RE_RegisterModel(&result, szWorldModel, v20, 7);
            v1->iWorldSurfIndex.mPakId = *(int*)((char*)&result + 4);
            ValidatePakId(v1->iWorldSurfIndex.mPakId);
            if (!v1->iWorldSurfIndex.mValue)
                Com_Printf("WARNING: Weapon %s could not load world model\n",
                           szWorldModel);
        }
        if (szPickupModel[0])
        {
            int v21 = CurPakId();
            void* result;
            v1->iPickupSurfIndex.mValue =
                RE_RegisterModel(&result, szPickupModel, v21, 7);
            v1->iPickupSurfIndex.mPakId = *(int*)((char*)&result + 4);
        }
        else
        {
            v1->iPickupSurfIndex = v1->iWorldSurfIndex;
        }
        v1->weaponIcon[0] = GetTextureData(
            *(const char**)((char*)bg_itemlist + 52 * weaponNum + 20), 0,
            "mp_frontEnd");
        v1->weaponIcon[1] = GetTextureData(
            va("%s_select",
               *(const char**)((char*)bg_itemlist + 52 * weaponNum + 20)),
            0, "mp_frontEnd");
        v1->ammoIcon = GetTextureData(
            *(const char**)((char*)bg_itemlist + 52 * weaponNum + 24), 0,
            "mp_frontEnd");
        const char* szReticleCenter =
            (const char*)&((char*)InfoForWeapon)[0x2A0];
        if (szReticleCenter[0])
            v1->hReticleCenter = GetTextureData(szReticleCenter, 0,
                                                "mp_frontEnd");
        const char* szReticleSide =
            (const char*)&((char*)InfoForWeapon)[0x2E0];
        if (szReticleSide[0])
            v1->hReticleSide = GetTextureData(szReticleSide, 0, "mp_frontEnd");
        const char* szOverlayShader =
            (const char*)&((char*)InfoForWeapon)[0x320];
        if (szOverlayShader[0])
            v1->hADSOverlay = GetTextureData(szOverlayShader, 0,
                                             "mp_frontEnd");
        const char* szProjectileModel =
            (const char*)&((char*)InfoForWeapon)[0x200];
        if (szProjectileModel[0])
        {
            int v28 = CurPakId();
            void* v39;
            void* v29 = RE_RegisterModel(&v39, szProjectileModel, v28, 7);
            v1->iMissileSurfIndex.mValue = *(void**)v29;
            v1->iMissileSurfIndex.mPakId = *(int*)((char*)v29 + 4);
            ValidatePakId(v1->iMissileSurfIndex.mPakId);
            if (!v1->iMissileSurfIndex.mValue)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\cg_weapons.cpp";
                AeAssert::gCurrentLine = 1434;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored()
                    && AeAssert::Warning(
                        "Weapon %s does not specify a valid projectile "
                        "model (%s)\n",
                        szInternalName, szWorldModel))
                    __debugbreak();
            }
        }
        const char* szHudIcon = (const char*)&((char*)InfoForWeapon)[0x3A0];
        if (szHudIcon[0])
            v1->hHudIcon = GetTextureData(szHudIcon, 0, "mp_frontEnd");
        const char* szAmmoIcon = (const char*)&((char*)InfoForWeapon)[0x3E0];
        if (szAmmoIcon[0])
            v1->hAmmoIcon = GetTextureData(szAmmoIcon, 0, "mp_frontEnd");
        v1->pszTranslatedDisplayName =
            SEH_StringEd_GetString(szDisplayName);
        if (!v1->pszTranslatedDisplayName)
            v1->pszTranslatedDisplayName = szDisplayName;
    }
LABEL_25:
    return;
}

extern void* DObjGetTree(void* obj);
extern int ADSMetaAnimPlayer_Update(void* self, void* pAnimTree,
                                    weaponInfo_s* weaponInfo);
extern void Camera_StartAnimating(void* cam, float minTweenTime);
extern void Camera_StopAnimating(void* cam, float minTweenTime);
extern void* gCamera;
extern int CanInterrupt(void* pAnimTree, void* client_cgs);
extern void* cgs;
extern int fireSide;
extern void GetADSLerpTimeRemaining(PlayerState* ps, weaponFileInfo_t* info);
extern int CG_StartAnimBlend(int weaponNum, DObj* dobj, int toAnimIndex,
                             int fromAnimIndex, float blendTime);
extern void PM_KillQueuedReloadSound(PlayerState* ps);

// ea: 0x0069ED70
void CG_WeaponRunXModelAnims(PlayerState* ps, weaponInfo_s* weapon)
{
    void* v3 = (void*)dword_F6A2A0[802 * currCl];
    if (v3 == nullptr)
    {
        CG_ASSERT("0", "c:\\cod\\code\\game\\cg_weapons.cpp", 456);
        return;
    }
    void* Tree = DObjGetTree(v3);
    DObj* v5 = (DObj*)dword_F6A2A0[802 * currCl];
    weaponFileInfo_t* InfoForWeapon =
        (weaponFileInfo_t*)BG_GetInfoForWeapon(ps->weapon);
    weaponFileInfoFull* pWeap = (weaponFileInfoFull*)InfoForWeapon;
    int weaponstate = ps->weaponstate;
    int bInAds = 0;
    float a1 = 0.1f;
    if (weaponstate == 5
            && ps->weaponTime - *(int*)((char*)InfoForWeapon + 0x3C4) > 0
        || weaponstate == 14
        || EntityManager_GetPlayer(EntityManager_sInst, currCl)
                   ->client->ps.fWeaponPosFrac
               < 0.1f)
    {
        bInAds = 0;
    }
    else if ((ps->pm_flags & 0x20) != 0)
    {
        bInAds = 1;
    }
    int playingADSAnim = 0;
    if (((weaponFileInfoFull*)InfoForWeapon)->weapClass != 10 /* WEAPCLASS_LMG */)
        playingADSAnim = ADSMetaAnimPlayer_Update(
            (char*)sADSMetaAnimPlayer + 8 * currCl, Tree, weapon);
    weaponFileInfoFull* v8 = (weaponFileInfoFull*)InfoForWeapon;
    unsigned int v9 = ps->weapAnim & 0xFFFFFDFF;
    if ((pWeap->bAnimateCamReload != 0
         && (v9 == 11 || v9 == 12 || v9 == 13 || v9 == 14))
        || (pWeap->bAnimateCamMelee != 0 && v9 == 8)
        || (pWeap->bAnimateCamFire != 0 && (v9 == 2 || v9 == 3)))
    {
        void* cam = (char*)gCamera + 0x1F0 * currCl;
        if ((*(unsigned short*)((char*)cam + 0x140) & 1) == 0)
            Camera_StartAnimating(cam, 0.1f);
    }
    else
    {
        void* cam = (char*)gCamera + 0x1F0 * currCl;
        if ((*(unsigned short*)((char*)cam + 0x140) & 1) != 0)
            Camera_StopAnimating(cam, 0.0f);
    }
    v8 = (weaponFileInfoFull*)InfoForWeapon;
    int v10 = dword_F6A2A8[802 * currCl];
    if (ps->weapAnim != v10)
    {
        switch (v9)
        {
        case 0u:
        case 0x17u:
            if (CanInterrupt(Tree, (char*)cgs + currCl) && playingADSAnim == 0)
            {
                unsigned int v11 = dword_F6A2A8[802 * currCl] & 0xFFFFFDFF;
                float fadeInTimea = 0.0f;
                if (v11 == 4 || v11 == 7)
                    fadeInTimea = 0.1f;
                int weapClass = ((weaponFileInfoFull*)InfoForWeapon)->weapClass;
                if ((weapClass == 10 /* LMG */
                     || weapClass == 17 /* SPOTTER */)
                    && (ps->pm_flags & 0x20) != 0)
                {
                    CG_StartWeaponAnim(ps->weapon, v5, 20, 0.0f, 0.0f, 1);
                }
                else if (bInAds != 0)
                {
                    CG_StartWeaponAnim(ps->weapon, v5, 3, fadeInTimea, 0.0f,
                                       1);
                }
                else if (ps->ammoclip[BG_ClipForWeapon(ps->weapon)] != 0)
                {
                    if (v11 == 2)
                        fadeInTimea = 0.1f;
                    CG_StartWeaponAnim(ps->weapon, v5, 1, fadeInTimea, 0.0f,
                                       1);
                }
                else
                {
                    CG_StartWeaponAnim(ps->weapon, v5, 2, fadeInTimea, 0.0f,
                                       1);
                }
                goto L177920;
            }
            return;
        case 2u:
            if (v8->bADSPositionInfo != 0 || v8->slot != 8 /* WEAPSLOT_PISTOL */)
            {
                CG_StartWeaponAnim(ps->weapon, v5, 4, 0.1f, 0.0f, 1);
            }
            else
            {
                if (fireSide)
                    CG_StartWeaponAnim(ps->weapon, v5, 17, 0.1f, 0.0f, 1);
                else
                    CG_StartWeaponAnim(ps->weapon, v5, 4, 0.1f, 0.0f, 1);
                fireSide = !fireSide;
            }
            goto L177920;
        case 3u:
            CG_StartWeaponAnim(ps->weapon, v5, 6, 0.0f, 0.0f, 1);
            goto L177920;
        case 4u:
            if ((v10 & 0xFFFFFDFF) != 7
                || (GetADSLerpTimeRemaining(ps, InfoForWeapon),
                    CG_StartAnimBlend(ps->weapon, v5, 7, 19,
                                      a1 - 0.050000001f)) == 0)
            {
                CG_StartWeaponAnim(ps->weapon, v5, 7, ps->fWeaponPosFrac * 0.5f,
                                   0.0f, 1);
            }
            goto L177920;
        case 5u:
            CG_StartWeaponAnim(ps->weapon, v5, 17, 0.0f, 0.0f, 1);
            goto L177920;
        case 6u:
            CG_StartWeaponAnim(ps->weapon, v5, 18, 0.0f, 0.0f, 1);
            goto L177920;
        case 7u:
            if ((v10 & 0xFFFFFDFF) != 4
                || (GetADSLerpTimeRemaining(ps, InfoForWeapon),
                    CG_StartAnimBlend(ps->weapon, v5, 19, 7,
                                      a1 - 0.050000001f)) == 0)
            {
                CG_StartWeaponAnim(ps->weapon, v5, 19,
                                   (1.0f - ps->fWeaponPosFrac) * 0.5f, 0.0f,
                                   1);
            }
            goto L177920;
        case 8u:
            CG_StartWeaponAnim(ps->weapon, v5, 8, 0.0f, 0.0f, 1);
            goto L177920;
        case 9u:
        {
            float v13 = 0.30000001f;
            if (ps->fWeaponPosFrac <= 0.0f)
                v13 = 0.1f;
            CG_StartWeaponAnim(ps->weapon, v5, 14, v13, 0.0f, 1);
            goto L177920;
        }
        case 0xAu:
            CG_StartWeaponAnim(ps->weapon, v5, 13, 0.0f, 0.0f, 1);
            goto L177920;
        case 0xBu:
        {
            float v14 = 0.30000001f;
            if (ps->fWeaponPosFrac <= 0.0f)
                v14 = 0.1f;
            CG_StartWeaponAnim(ps->weapon, v5, 9, v14, 0.0f, 1);
            goto LABEL_88;
        }
        case 0xCu:
        {
            float v15 = 0.30000001f;
            if (ps->fWeaponPosFrac <= 0.0f)
                v15 = 0.1f;
            CG_StartWeaponAnim(ps->weapon, v5, 10, v15, 0.0f, 1);
            goto LABEL_88;
        }
        case 0xDu:
        {
            float v16 = 0.30000001f;
            if (ps->fWeaponPosFrac <= 0.0f)
                v16 = 0.1f;
            CG_StartWeaponAnim(ps->weapon, v5, 11, v16, 0.0f, 1);
            goto LABEL_88;
        }
        case 0xEu:
            CG_StartWeaponAnim(ps->weapon, v5, 12, 0.0f, 0.0f, 1);
            goto LABEL_88;
        case 0xFu:
            CG_StartWeaponAnim(ps->weapon, v5, 16, 0.0f, 0.0f, 1);
            goto L177920;
        case 0x10u:
            CG_StartWeaponAnim(ps->weapon, v5, 15, 0.0f, 0.0f, 1);
            goto L177920;
        case 0x11u:
            CG_StartWeaponAnim(ps->weapon, v5, 5, 0.0f, 0.0f, 1);
            goto L177920;
        case 0x13u:
            CG_StartWeaponAnim(ps->weapon, v5, 21, 0.0f, 0.0f, 1);
            goto L177920;
        case 0x14u:
            CG_StartWeaponAnim(ps->weapon, v5, 22, 0.0f, 0.0f, 1);
            goto L177920;
        case 0x15u:
        case 0x16u:
            goto L177920;
        default:
            CG_StartWeaponAnim(ps->weapon, v5, 1, 0.0f, 0.0f, 1);
            Com_Printf("CG_WeaponRunXModelAnims: Unknown weapon animation %i\n",
                       ps->weapAnim & 0xFFFFFDFF);
        L177920:
            if (EntityManager_GetPlayer(EntityManager_sInst, currCl)
                    ->client->ps.queuedReloadSoundPlayStarted)
            {
                PM_KillQueuedReloadSound(GetPlayerState(currCl));
            }
        LABEL_88:
            dword_F6A2A8[802 * currCl] = ps->weapAnim;
            break;
        }
    }
}

// ea: 0x006AA920
void CG_AddPlayerWeapon(refEntity_t* parent, PlayerState* ps, Entity* entity,
                        int bDrawGun)
{
    int weapon = ps->weapon;
    bool bViewModel = ps != nullptr;
    if (ps == nullptr || dword_F640A4[1580 * currCl] == 0)
    {
        CG_RegisterWeapon(weapon);
        BG_GetInfoForWeapon(weapon);
        CG_WeaponUpdateLoopingSound(entity);
        refEntity_t* RefEntity = (refEntity_t*)Entity_GetRefEntity(entity);
        RefEntity->lightingOrigin[0] = parent->lightingOrigin[0];
        RefEntity->lightingOrigin[1] = parent->lightingOrigin[1];
        RefEntity->lightingOrigin[2] = parent->lightingOrigin[2];
        RefEntity->renderfx = parent->renderfx;
        if (bViewModel && dword_F6A2A0[802 * currCl] != 0)
        {
            RefEntity->reType = 1;
            RefEntity->obj = (void*)dword_F6A2A0[802 * currCl];
            RefEntity->renderfx = 140;
            RefEntity->entity = entity;
            RefEntity->lightingOrigin[0] = ps->origin.v.m128_f32[0];
            RefEntity->lightingOrigin[1] = ps->origin.v.m128_f32[1];
            float v10 = ps->origin.v.m128_f32[2];
            RefEntity->lightingOrigin[2] =
                ps->viewHeightCurrent + v10;
            AddLeanToPosition(RefEntity->lightingOrigin, ps->viewangles[1],
                              ps->leanf, 16.0f, 20.0f);
            DObjAdvanceAnimationPlayer((void*)dword_F6A2A0[802 * currCl],
                                       cgGlobal_frametime * 0.001f);
            DObjInitServerTime((void*)dword_F6A2A0[802 * currCl],
                               cgGlobal_frametime * 0.001f);
            int deltaT;
            switch (dword_F6A2A8[802 * currCl])
            {
            case 0: deltaT = 1; break;
            case 4: deltaT = 7; break;
            case 8: deltaT = 8; break;
            case 9: deltaT = 14; break;
            case 0xA: deltaT = 13; break;
            case 0xB: deltaT = 9; break;
            case 0xC: deltaT = 10; break;
            case 0x17: deltaT = 3; break;
            default: deltaT = 0; break;
            }
            DObjUpdateServerInfo((void*)dword_F6A2A0[802 * currCl],
                                 cgGlobal_frametime * 0.001f, true, deltaT);
            int partBits[4];
            memset(partBits, 255, sizeof(partBits));
            DObjCalcAnim((void*)dword_F6A2A0[802 * currCl], -1);
            j_nullsub_82((void*)dword_F6A2A0[802 * currCl], partBits);
            if (!GamePause_IsGamePaused(currCl))
            {
                float deltaTa = ServerTime_sInst.mTickDelta;
                void* v12 = InteractionController_Inst(currCl);
                InteractionController_PostPhysicsUpdate(v12, deltaTa);
            }
            CG_UpdateViewModelPosAndOrientation(parent);
            RefEntity->origin[0] = parent->origin[0];
            RefEntity->origin[1] = parent->origin[1];
            RefEntity->origin[2] = parent->origin[2];
            AxisCopy(parent->axis, RefEntity->axis);
            float v13 = ((0 + cg_gun_x) * dword_F63C80[1580 * currCl])
                        + RefEntity->origin[0];
            RefEntity->origin[0] = v13;
            float v14 = (dword_F63C8C[1580 * currCl] * cg_gun_y) + v13;
            RefEntity->origin[0] = v14;
            RefEntity->origin[0] =
                (dword_F63C98[1580 * currCl] * cg_gun_z) + v14;
            float v15 = ((0 + cg_gun_x) * dword_F63C84[1580 * currCl])
                        + RefEntity->origin[1];
            RefEntity->origin[1] = v15;
            float v16 = (dword_F63C90[1580 * currCl] * cg_gun_y) + v15;
            RefEntity->origin[1] = v16;
            RefEntity->origin[1] =
                (dword_F63C9C[1580 * currCl] * cg_gun_z) + v16;
            float v17 = ((0 + cg_gun_x) * dword_F63C88[1580 * currCl])
                        + RefEntity->origin[2];
            RefEntity->origin[2] = v17;
            float v18 = (dword_F63C94[1580 * currCl] * cg_gun_y) + v17;
            RefEntity->origin[2] = v18;
            RefEntity->origin[2] =
                (dword_F63CA0[1580 * currCl] * cg_gun_z) + v18;
            if (bDrawGun != 0)
            {
                if (tr_viewModelInfo_mWeaponScale[currCl] != 1.0f)
                {
                    DObj* v19 = (DObj*)dword_F6A2A0[802 * currCl];
                    if (v19 != nullptr)
                    {
                        if ((tagHashInit & 1) == 0)
                        {
                            tagHashInit |= 1u;
                            tagHash = HashString_CalcHash("tag_weapon");
                        }
                        int BoneIndex = DObjGetBoneIndex(v19, tagHash);
                        if (BoneIndex != -1)
                            tr_viewModelInfo_mWeaponOrigin = DObj_GetMat(v19, BoneIndex);
                    }
                }
                RE_AddViewModelToScene(RefEntity);
                RE_SetViewModelInfoIndex(currCl);
            }
            int v21 = 1580 * currCl;
            dword_F64074[v21] = *(int*)&RefEntity->origin[0];
            dword_F64078[v21] = *(int*)&RefEntity->origin[1];
            dword_F6407C[v21] = *(int*)&RefEntity->origin[2];
            AxisCopy(RefEntity->axis, (float (*)[3])&unk_F64080[6320 * currCl]);
            if ((tagHashInit & 2) == 0)
            {
                tagHashInit |= 2u;
                tag_brass_hash = HashString_CalcHash("tag_brass");
            }
            int v22 = DObjGetBoneIndex((DObj*)dword_F6A2A0[802 * currCl],
                                       tag_brass_hash);
            if (v22 > -1)
            {
                DObjSkelMat* MatrixArray =
                    DObjGetMatrixArray((DObj*)dword_F6A2A0[802 * currCl], 0);
                if (MatrixArray == nullptr)
                    CG_ASSERT("pMtxArray",
                              "c:\\cod\\code\\game\\cg_weapons.cpp", 3042);
                DObjSkelMat* v24 = &MatrixArray[v22];
                if (v24 == nullptr)
                    CG_ASSERT("pMtxArray",
                              "c:\\cod\\code\\game\\cg_weapons.cpp", 3045);
                float mtxBrass[28];
                mtxBrass[16] = RefEntity->axis[0][0];
                mtxBrass[17] = RefEntity->axis[0][1];
                mtxBrass[18] = RefEntity->axis[0][2];
                mtxBrass[19] = RefEntity->axis[1][0];
                mtxBrass[20] = RefEntity->axis[1][1];
                mtxBrass[21] = RefEntity->axis[1][2];
                mtxBrass[22] = RefEntity->axis[2][0];
                mtxBrass[23] = RefEntity->axis[2][1];
                mtxBrass[24] = RefEntity->axis[2][2];
                mtxBrass[25] = RefEntity->origin[0];
                mtxBrass[26] = RefEntity->origin[1];
                mtxBrass[27] = RefEntity->origin[2];
                DObjSkel2MatrixMultiply43(v24,
                                          (const float (*)[3])&mtxBrass[16],
                                          (DObjSkelMat*)mtxBrass);
                ejectBrassCasingOrigin[0] = mtxBrass[12];
                dword_F5E6BC = *(int*)&mtxBrass[13];
                dword_F5E6C0 = *(int*)&mtxBrass[14];
            }
            CG_HoldBreathUpdate();
        }
    }
}

extern void FixupGunModelParts(void* xmp);
extern void* XModelParts_GetAnimDef(void* parts);
extern int XAnimEntry_Create(XAnimEntry* self);

// ea: 0x006A92E0
bool CG_SetupViewModelDObj(DObj* dobj, int weaponNum)
{
    CurPakId();
    if (weaponNum == 0)
        return false;
    if (weaponNum < 1)
        CG_ASSERT("weaponNum >= 1", "c:\\cod\\code\\game\\cg_weapons.cpp",
                  919);
    if (weaponNum > BG_GetNumWeapons())
        CG_ASSERT("weaponNum <= BG_GetNumWeapons()",
                  "c:\\cod\\code\\game\\cg_weapons.cpp", 920);
    weaponInfo_s* weaponInfo = &((weaponInfo_s*)cg_weapons)[weaponNum];
    weaponFileInfo_t* InfoForWeapon =
        (weaponFileInfo_t*)BG_GetInfoForWeapon(weaponNum);
    DObjModel dobjModels[7];
    memset(dobjModels, 0, sizeof(dobjModels));
    if (InfoForWeapon == nullptr
        || (((const char*)&((char*)InfoForWeapon)[0x140])[0] == 0))
        goto LABEL_90;
    const char* szHandXModel = (const char*)&((char*)InfoForWeapon)[0x100];
    const char* szGunXModel = (const char*)&((char*)InfoForWeapon)[0x140];
    const char* szInternalName = (const char*)&((char*)InfoForWeapon)[0x40];
    if (szHandXModel == nullptr || szHandXModel[0] == 0)
    {
        CG_ASSERT("0", "c:\\cod\\code\\game\\cg_weapons.cpp", 935);
        goto LABEL_90;
    }
    if (szInternalName[0] == 0)
    {
        CG_ASSERT("0", "c:\\cod\\code\\game\\cg_weapons.cpp", 941);
        goto LABEL_90;
    }
    int v7 = CurPakId();
    void* v55;
    dobjModels[0].model.mValue =
        RE_RegisterModel(&v55, szHandXModel, v7, 6);
    dobjModels[0].model.mPakId = *(int*)((char*)&v55 + 4);
    dobjModels[0].boneName = Broc::string();
    dobjModels[0].ignoreCollision = 0;
    if (!dobjModels[0].model.mValue)
        CG_ASSERT("dobjModels[0].model",
                  "c:\\cod\\code\\game\\cg_weapons.cpp", 952);
    int v8 = 1;
    bool v57 = false;
    if (szGunXModel[0] != 0)
    {
        v8 = 2;
        int v9 = CurPakId();
        void* v47;
        void* model = RE_RegisterModel(&v47, szGunXModel, v9, 6);
        dobjModels[1].model.mValue = *(void**)model;
        dobjModels[1].model.mPakId = *(int*)((char*)model + 4);
        dobjModels[1].boneName = Broc::string("tag_weapon");
        dobjModels[1].ignoreCollision = 0;
        if (dobjModels[1].model.mValue)
        {
            ValidatePakId(dobjModels[1].model.mPakId);
            void* parts = *(void**)((char*)dobjModels[1].model.mValue + 0x14);
            if (parts && XModelParts_GetAnimDef(parts))
            {
                ValidatePakId(dobjModels[1].model.mPakId);
                FixupGunModelParts(parts);
                v57 = true;
            }
        }
    }
    void* Bank = AnimBankManager_GetBank(AnimBankManager_sInst, PAK_ID_MIN);
    void* AnimTree = AnimBank_GetAnimTree(Bank, szInternalName);
    void* pAnims = AnimTree;
    if (AnimTree == nullptr)
    {
        CG_ASSERT("pAnims", "c:\\cod\\code\\game\\cg_weapons.cpp", 1010);
        goto LABEL_90;
    }
    XAnimEntry* entries = (XAnimEntry*)((char*)AnimTree + 4);
    int mSize = *(int*)((char*)AnimTree + 0x38);
    for (int i = 0; i < mSize; ++i)
    {
        XAnimEntry* v28 = &entries[i];
        v28->lastAttempt = 0;
        if (v28->hash == entries[1].hash && (i == 17 || i == 18 || i == 19))
            v28->hash = entries[3].hash;
        v28->anim = cdGetAnim(v28->hash);
    }
    void* Tree = nullptr;
    if (v57)
    {
        char treename[256];
        sprintf(treename, "%s%s", szInternalName, "_arms");
        void* v34 = AnimBankManager_GetBank(AnimBankManager_sInst,
                                            PAK_ID_MIN);
        void* v35 = AnimBank_GetAnimTree(v34, treename);
        void* pAnimsWeapon = v35;
        if (v35 == nullptr)
            CG_ASSERT("pAnimsWeapon",
                      "c:\\cod\\code\\game\\cg_weapons.cpp", 1038);
        XAnimEntry* entriesW = (XAnimEntry*)((char*)v35 + 4);
        int v36 = *(int*)((char*)v35 + 0x38);
        for (int i = 0; i < v36; ++i)
        {
            XAnimEntry* v37 = &entriesW[i];
            unsigned int hash = v37->hash;
            v37->lastAttempt = 0;
            v37->anim = cdGetAnim(hash);
        }
        Tree = XAnimCreateTree(nullptr, v35);
        if (Tree == nullptr)
            CG_ASSERT("pAnimTreeWeapon",
                      "c:\\cod\\code\\game\\cg_weapons.cpp", 1055);
    }
    void* mainTree = XAnimCreateTree(nullptr, pAnims);
    if (mainTree == nullptr)
        CG_ASSERT("pAnimTree", "c:\\cod\\code\\game\\cg_weapons.cpp", 1050);
    dobjModels[0].animTree = (XAnimTree*)mainTree;
    XAnimIsLooped(pAnims, 0x17);
    XAnimIsLooped(pAnims, 0x18);
    DObjCreate(dobjModels, v8, mainTree, dobj, 0);
    dword_F6A2A0[802 * currCl] = (int)dobj;
    if (strlen(szHandXModel) >= 0x18)
        CG_ASSERT("strlen(pWeap->szHandXModel) < ( 24 )",
                  "c:\\cod\\code\\game\\cg_weapons.cpp", 1080);
    Q_strncpyz(weaponInfo->handModel, szHandXModel, 24);
    for (int i = 0; i < v8; ++i)
        DObjCreateAnimationPlayer(dobj, i);
    return true;
LABEL_90:
    return false;
}
