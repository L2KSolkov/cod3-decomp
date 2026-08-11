// ============================================================================
// g_bg_pmove.cpp - game.o bg_pmove/bg_misc/bg_weapons helpers
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"
#include "core/tlFixedString.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// pml_t - pmove local state (0xC0, verified against IDA)
// ============================================================================
struct pml_t {
    float forward[3];        // +0x00
    float right[3];          // +0x0C
    float up[3];             // +0x18
    float frametime;         // +0x24
    int msec;                // +0x28
    int walking;             // +0x2C
    int groundPlane;         // +0x30
    int almostGroundPlane;   // +0x34
    trace_t groundTrace;     // +0x40 (0x50)
    float impactSpeed;       // +0x90
    float previous_origin[3];    // +0x94
    float previous_velocity[3];  // +0xA0
    int previous_waterlevel;     // +0xAC
    void* pWeap;                 // +0xB0 (weaponFileInfo_t*)
};
static_assert(sizeof(pml_t) == 0xC0, "pml_t size mismatch");

// Cross-object externs (game.o data)
extern pmove_t* pm;          // ?pm@@3PAUpmove_t@@A (game.o)
extern pml_t pml;            // ?pml@@3Upml_t@@A (game.o)
extern int dword_106000;     // ?dword_106000 (EF_* flags mask, BSS)
extern int cl_aADS[4];       // ?cl_aADS@@3PAHA (cl.o)
extern const char* BG_GetWeaponSlotNameForIndex(int iSlot);  // game.o 0x6072B0
extern vmCvar_t bg_nofatigue;  // ?bg_nofatigue@@3UvmCvar_t@@A (game.o)
extern vmCvar_t g_gravity;     // ?g_gravity@@3UvmCvar_t@@A
extern weaponFileInfo_t** bg_weaponInfo;  // ?bg_weaponInfo@@3PAPAUweaponFileInfo_t@@A (game.o)
extern const char** pEventNamesList;      // ?pEventNamesList@@3PAPBDA (game.o)
extern const char* szWeapTypeNames[9];    // ?szWeapTypeNames@@3PAPBDA (game.o)
extern Entity* GetPlayer(int idx);        // ?GetPlayer@@YAPAVEntity@@H@Z (g.o)
extern Entity* EntityHandleDb_GetObject(unsigned int val);  // game.o
extern int LocalClient_ClientToPort(int client);  // ?ClientToPort@LocalClient@@YAHH@Z
extern bool CL_IsADS(int client);                // ?CL_IsADS@@YA_NH@Z
extern float CL_GamepadAxisValue(unsigned int virtualAxis);  // cl.o
extern float intersect(const math::Position3& po, const math::Dir3& pn,
                       const math::Position3& ro, const math::Dir3& rd);
    // ?intersect@@YAMABVPosition3@math@@ABVDir3@2@01@Z (cdl_base)
extern void g_AddDebugString(const float* xyz, const float* color, float scale,
                             const char* pszText);  // ?g_AddDebugString (g_main)
extern void vectosignedangles(const float* vec, float* angles);  // core.o
extern serverStatic_t svs;  // ?svs@@3UserverStatic_t@@A (sv.o)
extern cvar_t* g_gameskill; // g.o
extern float gStickyBoxScaleEasy;   // ?gStickyBoxScaleEasy@@3MA (game.o)
extern float gStickyBoxScaleNormal; // ?gStickyBoxScaleNormal@@3MA (game.o)
extern float gStickyBoxScaleHard;   // ?gStickyBoxScaleHard@@3MA (game.o)
extern float gExtraDistanceSticky;  // ?gExtraDistanceSticky@@3MA (game.o)
extern float tangent;               // ?tangent (game.o @ 0xDF8DC4)
extern float accel_slow_factor;     // ?accel_slow_factor (game.o @ 0xDF8DC8)
extern float clostDist;             // ?clostDist (game.o @ 0xDF8DCC)
extern float xy;                    // ?xy (game.o @ 0xDF8DD0)
extern float boundingMin;           // ?boundingMin (game.o @ 0xDF8DD4)
extern float depthScale;            // ?depthScale (game.o @ 0xDF8DD8)
extern cvar_t* bg_stickyAimRender;  // ?bg_stickyAimRender@@3PAUcvar_t@@A (game.o)
extern cgGlobal_t cgGlobal;         // ?cgGlobal@@3UcgGlobal_t@@A (cg.o)
extern float player_breath_hold_time;   // @ 0xDF6B3C
extern float player_breath_gasp_time;   // @ 0xDF6B40
extern float player_breath_hold_lerp;   // @ 0xDF6B44
extern float player_breath_gasp_lerp;   // @ 0xDF6B48
extern float player_breath_gasp_scale;  // @ 0xDF6B4C
extern float* dword_F63B8C[4 * 1580];   // ?dword_F63B8C (game.o @ 0xF63B8C)
extern float DiffTrack(float tgt, float cur, float rate, float deltaTime);
    // ?DiffTrack@@YAMMMMM@Z (core.o q_math.cpp)
extern bool FindClosestVisibleBone(Entity* closestEnt,
                                   const math::Position3& playerPosition,
                                   const math::Position3& hitPosition,
                                   math::Position3& enemyOrigin);
    // game.o 0x615320

// game.o static weapon-type names (recovered from .rdata)
static const char* const s_szWeapTypeNames[9] = {
    "bullet", "grenade", "projectile", "spotter", "item",
    "gas", "interact", "mine", "flag",
};

extern void Cvar_VariableStringBuffer(const char* var_name, char* buffer,
                                      int bufsize);  // ?Cvar_VariableStringBuffer (sv_decl)
extern void Com_Printf(const char* fmt, ...);        // ?Com_Printf (core.o)
extern void mem_heap_free(void* ptr);                // ?mem_heap_free (mem_heap)
extern void BG_AddPredictableEventToPlayerstate(int newEvent, int eventParm,
                                                PlayerState* ps);  // bg_misc.cpp
extern void ProjectPointOnPlane(float* dst, const float* p,
                                const float* normal);  // core.o q_math.cpp
int  PM_WeaponAmmoAvailable(int wp);  // game.o 0x607E50
int  PM_WeaponClipEmpty(int wp);      // game.o 0x607E80
int  PM_Weapon_FinishRechamber();     // game.o 0x607FD0

// ============================================================================
// AngleClamp - ea: 0x604A90
// ============================================================================
float AngleClamp(float angle)
{
    float v1 = angle;
    if (angle > 180.0f)
    {
        do
            v1 = v1 - 360.0f;
        while (v1 > 180.0f);
        angle = v1;
    }
    for (; v1 < -180.0f; angle = v1)
        v1 = v1 + 360.0f;
    return angle;
}

// ============================================================================
// PM_AddEvent - ea: 0x604AF0
// ============================================================================
void PM_AddEvent(int newEvent)
{
    if (pm == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
        AeAssert::gCurrentLine = 538;
        AeAssert::gCurrentExpr = "pm";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    BG_AddPredictableEventToPlayerstate(newEvent, 0, pm->ps);
}


// ============================================================================
// PM_AddTouchEnt - ea: 0x604B60
// ============================================================================
void PM_AddTouchEnt(DbLinkedHandle<EntityHandleDb, Entity> entity)
{
    if (entity.mHandle.mVal == 0)
        return;
    pmove_t* v1 = pm;
    if (pm == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
        AeAssert::gCurrentLine = 550;
        AeAssert::gCurrentExpr = "pm";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        v1 = pm;
    }
    int numtouch = v1->numtouch;
    if (numtouch == 32)
        return;
    DbLinkedHandle<EntityHandleDb, Entity>* touchents =
        (DbLinkedHandle<EntityHandleDb, Entity>*)v1->touchents;
    int v3 = 0;
    if (numtouch <= 0)
    {
        touchents[numtouch] = entity;
        ++pm->numtouch;
        return;
    }
    while (touchents->mHandle.mVal != entity.mHandle.mVal)
    {
        ++v3;
        ++touchents;
        if (v3 >= numtouch)
        {
            touchents[numtouch] = entity;
            ++pm->numtouch;
            return;
        }
    }
}

// ============================================================================
// PM_SwitchIfEmpty - ea: 0x6085D0 (bg_weapons.cpp)
// ============================================================================
// ea: 0x006085D0
void PM_SwitchIfEmpty()
{
    if (BG_GetInfoForWeapon(pm->ps->weapon)->bClipOnly != 0
        && ((weaponFileInfo_t*)pml.pWeap)->weapClass != WEAPCLASS_SPOTTER
        && pm->ps->ammoclip[BG_GetInfoForWeapon(pm->ps->weapon)->iClipIndex]
               == 0
        && pm->ps->ammo[BG_GetInfoForWeapon(pm->ps->weapon)->iAmmoIndex]
               == 0)
    {
        PM_AddEvent(174);
    }
}

// ============================================================================
// PM_Weapon_FinishRechamber - ea: 0x607FD0 (bg_weapons.cpp)
// ============================================================================
// ea: 0x00607FD0
int PM_Weapon_FinishRechamber()
{
    int result = 0;
    PlayerState* ps = pm->ps;
    if (pm->cmd.weapon != 0)
    {
        if (ps->fWeaponPosFrac > 0.89999998f)
            result = 23;
        if ((ps->weapAnim & 0xFFFFFDFF) == result)
        {
            result = (int)pm->ps;
        }
        else
        {
            PM_StartWeaponAnim(result);
            result = (int)pm;
        }
        pm->ps->weaponstate = 0;
    }
    else
    {
        ps->weaponstate = 0;
    }
    return result;
}

// ============================================================================
// PM_Weapon_WeaponTimeAdjust - ea: 0x6088A0 (bg_weapons.cpp)
// ============================================================================
// ea: 0x006088A0
int PM_Weapon_WeaponTimeAdjust()
{
    if (pm->ps->weaponTime != 0)
    {
        pm->ps->weaponTime -= pml.msec;
        if (pm->ps->weaponTime <= 0)
        {
            if (((weaponFileInfo_t*)pml.pWeap)->bSemiAuto != 0
                && (pm->cmd.buttons & 1) != 0
                && pm->ps->weapon == pm->cmd.weapon
                && PM_WeaponAmmoAvailable(pm->ps->weapon) != 0)
            {
                pm->ps->weaponTime = 1;
                int weaponstate = pm->ps->weaponstate;
                if (weaponstate == 4)
                {
                    PM_Weapon_FinishRechamber();
                }
                else if (weaponstate == 3 || weaponstate == 10
                         || weaponstate == 11)
                {
                    PM_ContinueWeaponAnim(0);
                    pm->ps->weaponstate = 0;
                }
            }
            else
            {
                pm->ps->weaponTime = 0;
            }
        }
    }
    if (pm->ps->weaponDelay == 0)
        return 0;
    pm->ps->weaponDelay -= pml.msec;
    if (pm->ps->weaponDelay > 0)
        return 0;
    pm->ps->weaponDelay = 0;
    return 1;
}

// ============================================================================
// PM_UpdateHoldBreath - ea: 0x631260 (bg_pmove.cpp)
// ============================================================================
// ea: 0x00631260
void PM_UpdateHoldBreath()
{
    float deltaT = ServerTime::sInst.mTickDelta;
    int v0 = (int)(ServerTime::sInst.mTickDelta * 1000.0f);
    Client* client = EntityManager::sInst->GetPlayer(currCl)->client;
    int v2 = (int)(player_breath_hold_time * 1000.0f);
    int targetScale = (int)(player_breath_gasp_time * 1000.0f);
    if (v2 <= 0)
    {
        client->ps.mFlags &= ~2u;
        client->ps.mHoldBreathScale = 1.0f;
        client->ps.mHoldBreathTimer = 0;
        return;
    }
    Entity* mObject = (Entity*)EntityHandleDb_GetObject(
        pm->ps->mClient.mHandle.mVal);
    float* v5 = mObject != nullptr ? dword_F63B8C[1580 * mObject->GetPlayerIndex()]
                                   : nullptr;
    if (v5 != nullptr)
    {
        int v6 = *(int*)(v5 + 1620 / 4);
        if (client->ps.fWeaponPosFrac == 1.0f
            && (v6 == 3 || v6 == 2)
            && (client->ps.mFlags & 1) != 0)
        {
            if (client->ps.mHoldBreathTimer != 0)
                goto LABEL_15;
            client->ps.mFlags |= 2u;
        }
        else
        {
            client->ps.mFlags &= 0xFFFFFFFD;
        }
LABEL_15:
        int mHoldBreathTimer = client->ps.mHoldBreathTimer;
        if ((client->ps.mFlags & 2) != 0)
            client->ps.mHoldBreathTimer = v0 + mHoldBreathTimer;
        else
            client->ps.mHoldBreathTimer = mHoldBreathTimer - v0;
        if (client->ps.mHoldBreathTimer < 0)
            client->ps.mHoldBreathTimer = 0;
        if ((client->ps.mFlags & 2) != 0
            && client->ps.mHoldBreathTimer > v2)
        {
            client->ps.mHoldBreathTimer = v2 + targetScale;
            client->ps.mFlags &= ~2u;
        }
        float v10;
        float targetScalea;
        if ((client->ps.mFlags & 2) != 0)
        {
            targetScalea = 0.0f;
            v10 = player_breath_hold_lerp;
        }
        else
        {
            v10 = player_breath_gasp_lerp;
            targetScalea =
                ((float)client->ps.mHoldBreathTimer
                 / (float)(v2 + targetScale))
                    * (player_breath_gasp_scale - 1.0f)
                + 1.0f;
        }
        float targetScaleb =
            (targetScalea - 1.0f) * client->ps.fWeaponPosFrac + 1.0f;
        client->ps.mHoldBreathScale =
            DiffTrack(targetScaleb, client->ps.mHoldBreathScale, v10,
                      deltaT);
    }
}

// ============================================================================
// Pmove / PmoveSingle - ea: 0x6464C0 / 0x645CD0 (bg_pmove.cpp)
// ============================================================================
extern int c_pmove;   // ?c_pmove@@3HA (game.o)
void PM_CheckDuck();                        // game.o 0x644B80
extern void PmoveSingle(pmove_t* pmove,
                        bool isThisThePredictStep);  // game.o 0x645CD0
extern bool GamePause_IsGamePaused(int client);   // ?IsGamePaused@GamePause@@SA_NH@Z
extern void PM_Weapon();                          // game.o 0x6408B0
extern void PM_Footsteps();                       // game.o 0x63CB60
void PM_LadderMove(const collision_context_t& context);  // game.o 0x6458E0
extern void PM_WalkMove(const collision_context_t& context);    // game.o 0x643E40
extern void PM_AirMove(const collision_context_t& context);     // game.o 0x643C50
extern void PM_GroundTrace();                     // game.o 0x63C340
extern void PM_NoclipMove();                      // game.o 0x6055A0
extern void PM_UFOMove();                         // game.o 0x605850
extern void PM_DeadMove();                        // game.o 0x605420
extern void PM_CheckLadderMove();                 // game.o 0x63DDF0
extern void PM_FoliageSounds();                   // game.o 0x63D100
extern void PM_WaterEvents();                     // game.o 0x606280
extern void PM_DropTimers();                      // game.o 0x606320
void PM_UpdateViewAngles(
    PlayerState* ps, usercmd_s* cmd, usercmd_s* oldcmd, int msec,
    void (__cdecl* capsuleTrace)(trace_t*, const math::Position3&,
                                 const math::Position3&, const math::Position3&,
                                 const math::Position3&,
                                 const collision_context_t&));
    // game.o 0x63D2B0
extern int  PM_InteruptWeaponWithProneMove();     // game.o 0x6171B0
extern int  PM_InteruptWeaponWithSprintMove();    // game.o 0x617250
extern pmove_t* PM_UpdatePlayerWalkingFlag();     // game.o 0x6065B0
extern PlayerState* PM_UpdatePlayerSprintingFlag(); // game.o 0x62F070
extern PlayerState* PM_UpdateFatigue();           // game.o 0x606440
extern void PM_UpdateAimDownSightFlag();          // game.o 0x62F2F0
extern void PM_UpdateAimDownSightLerp();          // game.o 0x62F670
extern PlayerState* PM_AdjustAimSpreadScale();    // game.o 0x608670

// ea: 0x006464C0
void Pmove(pmove_t* pmove, bool isThisThePredictStep)
{
    PlayerState* ps = pmove->ps;
    if ((pmove->ps->pm_flags & 0x4000) != 0)
    {
        pmove->cmd.forwardmove = 0;
        pmove->cmd.rightmove = 0;
        pmove->cmd.upmove = 0;
        pmove->cmd.buttons = 0;
    }
    int serverTime = pmove->cmd.serverTime;
    int commandTime = ps->commandTime;
    if (serverTime >= commandTime)
    {
        if (serverTime > commandTime + 1000)
            ps->commandTime = serverTime - 1000;
        pm = pmove;
        pmove->numtouch = 0;
        if ((0x100000 & pm->ps->pm_flags) != 0)
        {
            PM_CheckDuck();
        }
        else
        {
            PlayerState* v6 = pmove->ps;
            while (pmove->ps->commandTime != serverTime)
            {
                int v7 = v6->commandTime;
                int pmove_msec = serverTime - v7;
                if (pmove->pmove_fixed != 0)
                {
                    if (pmove_msec > pmove->pmove_msec)
                        pmove_msec = pmove->pmove_msec;
                }
                else if (pmove_msec > 666)
                {
                    pmove_msec = 666;
                }
                pmove->cmd.serverTime = pmove_msec + v7;
                PmoveSingle(pmove, isThisThePredictStep);
                v6 = pmove->ps;
                if ((pmove->ps->pm_flags & 8) != 0)
                    pmove->cmd.upmove = 20;
            }
            pm = nullptr;
            memset(&pml, 0, sizeof(pml));
        }
    }
}

// ============================================================================
// PM_UpdateStickyAim - ea: 0x62E070 (bg_pmove.cpp)
// ============================================================================
// ea: 0x0062E070
void PM_UpdateStickyAim(PlayerState* ps, usercmd_s* cmd, usercmd_s* oldcmd)
{
    float distAwayFromTargetXY = 3.4028235e38f;
    float distToPlane = 0.0f;
    bool bSlowFactor = false;
    float fLastDist = 0.0f;

    float boxScale;
    if (g_gameskill->integer == 0)
        boxScale = gStickyBoxScaleEasy;
    else if (g_gameskill->integer == 2)
        boxScale = gStickyBoxScaleHard;
    else
        boxScale = gStickyBoxScaleNormal;

    // playerPosition (eye pos) + view forward
    math::Position3 playerPos;
    playerPos.v = _mm_setr_ps(ps->origin.v.m128_f32[0],
                              ps->origin.v.m128_f32[1],
                              ps->viewHeightCurrent
                                  + ps->origin.v.m128_f32[2],
                              0.0f);
    float forward[3];
    AnglesToForward(ps->viewangles, forward);
    math::Dir3 viewDir;
    viewDir.v = _mm_setr_ps(forward[0], forward[1], forward[2], 0.0f);

    Entity* selfEnt = (Entity*)EntityHandleDb_GetObject(
        ps->mClient.mHandle.mVal);
    if (selfEnt == nullptr || selfEnt->client == nullptr)
        return;
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(ps->weapon);

    Entity* selfEnt2 = (Entity*)EntityHandleDb_GetObject(
        ps->mClient.mHandle.mVal);
    int playerIndex = selfEnt2->GetPlayerIndex();
    bool bStickyAim =
        gSaveGameData[LocalClient_ClientToPort(playerIndex)]
            .mStubData.mStickyAim;
    if (InfoForWeapon != nullptr
        && (InfoForWeapon->weapClass == WEAPCLASS_SPOTTER
            || InfoForWeapon->weapClass == 16))
    {
        bStickyAim = false;
    }
    Entity* selfEnt3 = (Entity*)EntityHandleDb_GetObject(
        ps->mClient.mHandle.mVal);
    if (InfoForWeapon->weapClass == 10  // WEAPCLASS_SNIPER (verified vs disasm)
        && CL_IsADS(selfEnt3->GetPlayerIndex()))
    {
        bStickyAim = false;
    }

    Entity* selfEnt4 = (Entity*)EntityHandleDb_GetObject(
        ps->mClient.mHandle.mVal);
    Entity* pPlayer = selfEnt4;

    for (int clientIdx = 0;
         clientIdx < 0x13700;
         clientIdx += 0x1370)
    {
        client_s* cl = (client_s*)((char*)svs.clients + clientIdx);
        if (cl->state == 0)
            continue;
        unsigned int handle = cl->mEntityHandle.mHandle.mVal;
        Entity* ent = (Entity*)EntityHandleDb_GetObject(handle);
        if (ent == nullptr || ent == pPlayer)
            continue;
        if (cgGlobal.teamGame
            && ent->sentient != nullptr && pPlayer->sentient != nullptr
            && ent->sentient->eTeam
                   == *(int*)((char*)pPlayer->sentient + 4))
        {
            continue;
        }
        Client* client = ent->client;
        if (client == nullptr || client->pers.playerState != 3
            || (0x100000 & client->ps.eFlags) != 0
            || (0x400000 & ent->flags) != 0)
        {
            continue;
        }

        math::Position3 enemyPos;
        enemyPos.v = _mm_setr_ps(ent->r.currentOrigin.v.m128_f32[0],
                                 ent->r.currentOrigin.v.m128_f32[1],
                                 ent->r.currentOrigin.v.m128_f32[2] + 40.0f,
                                 0.0f);
        __m128 delta = _mm_sub_ps(playerPos.v, enemyPos.v);
        __m128 d2 = _mm_mul_ps(delta, delta);
        float dist = sqrtf(d2.m128_f32[0] + d2.m128_f32[1]
                           + d2.m128_f32[2]);
        math::Dir3 toTarget;
        toTarget.v = _mm_div_ps(delta, _mm_set1_ps(dist));
        if (dist >= -0.1f && dist <= 0.1f)
            return;

        float t = intersect(enemyPos, toTarget, playerPos, viewDir);
        if (t > 0.0f)
        {
            __m128 proj = _mm_mul_ps(viewDir.v,
                                     _mm_xor_ps(toTarget.v,
                                                _mm_set1_ps(-0.0f)));
            float dot = proj.m128_f32[0] + proj.m128_f32[1]
                      + proj.m128_f32[2];
            if (dot > 0.92f)
            {
                math::Position3 hitPos;
                hitPos.v = _mm_add_ps(
                    playerPos.v, _mm_mul_ps(viewDir.v, _mm_set1_ps(t)));
                math::Position3 enemyOrigin;
                if (FindClosestVisibleBone(ent, playerPos, hitPos,
                                           enemyOrigin)
                    && SmokeGrenadeMgr::sInst
                        && ((SmokeGrenadeMgr*)SmokeGrenadeMgr::sInst)
                               ->EntityCanSeeEntity(pPlayer, ent, 0.4f))
                {
                    __m128 v25 = _mm_sub_ps(hitPos.v,
                                            ent->r.currentOrigin.v);
                    float dz = v25.m128_f32[2];
                    __m128 flat = v25;
                    flat.m128_f32[2] = 0.0f;
                    __m128 f2 = _mm_mul_ps(flat, flat);
                    float distXY = sqrtf(f2.m128_f32[0] + f2.m128_f32[1]
                                         + f2.m128_f32[2]);
                    float depth = dist / depthScale;
                    if (depth < 0.0f)
                        depth = 0.0f;
                    else if (depth > 12.0f)
                        depth = 12.0f;

                    float boxHalf = depth * boxScale;
                    float boxLow = ((boxHalf * 2.0f) + 1.0f) * -2.0f;
                    float boxHigh;
                    if (ent->actor != nullptr
                        || (ent->flags & 0x2000000) != 0
                        || ent->client != nullptr)
                    {
                        boxHigh = (depth + 1.0f) * xy;
                    }
                    else
                    {
                        float maxs = ent->r.maxs.v.m128_f32[0]
                                <= ent->r.maxs.v.m128_f32[1]
                            ? ent->r.maxs.v.m128_f32[1]
                            : ent->r.maxs.v.m128_f32[0];
                        if (maxs <= boundingMin)
                            boxHigh = boundingMin;
                        else
                        {
                            boxHigh = maxs + boundingMin;
                            boxLow = -boxHigh;
                        }
                    }
                    if ((ent->client->ps.pm_flags & 1) != 0)
                        boxHigh = 30.0f;
                    else if ((ent->client->ps.pm_flags & 2) != 0)
                        boxHigh = 60.0f;
                    boxHigh = ((boxHalf * 0.15f) + 1.0f) * boxHigh;
                    float v40 = boxHigh * boxScale;
                    if (boxHigh > dz && dz > boxLow && v40 > distXY)
                    {
                        distAwayFromTargetXY = t;
                        bSlowFactor = true;
                        distToPlane = (float)(intptr_t)ent;
                    }
                }
            }
        }
    }

    if (ps->prevTargetPointValid != 0)
    {
        unsigned int handle = ps->currentTargetHandle.mHandle.mVal;
        Entity* target = (Entity*)EntityHandleDb_GetObject(handle);
        if (target == nullptr)
            goto LABEL_113;
        if (!bSlowFactor || !bStickyAim)
            goto LABEL_113;
        Entity* entA = (Entity*)EntityHandleDb_GetObject(handle);
        Entity* entB = (Entity*)EntityHandleDb_GetObject(handle);
        Entity* entC = (Entity*)EntityHandleDb_GetObject(handle);
        math::Position3 targetPos;
        targetPos.v = _mm_setr_ps(entC->r.currentOrigin.v.m128_f32[0],
                                  entA->r.currentOrigin.v.m128_f32[1],
                                  entB->r.currentOrigin.v.m128_f32[2],
                                  0.0f);
        math::Position3 relPt;
        relPt.v = _mm_setr_ps(ps->prevTargetRelPt[0],
                              ps->prevTargetRelPt[1],
                              ps->prevTargetRelPt[2], 0.0f);
        math::Position3 pt;
        pt.v = _mm_add_ps(targetPos.v, relPt.v);
        __m128 d = _mm_sub_ps(pt.v, playerPos.v);
        float fDistance = 0.6f;
        if (distAwayFromTargetXY > 200.0f)
        {
            if (distAwayFromTargetXY <= 750.0f)
                fDistance = ((distAwayFromTargetXY - 200.0f)
                             * gExtraDistanceSticky)
                            * 0.0018181818f
                          + 0.6f;
            else
                fDistance = gExtraDistanceSticky + 0.6f;
        }
        if (fLastDist > clostDist)
        {
            fDistance = fDistance
                      - (((fLastDist - clostDist) * fDistance)
                         / (1.0f - clostDist));
        }
        if (cmd->rightmove != 0)
            fDistance += 0.1f;
        if (fabsf(CL_GamepadAxisValue(3)) > 0.95f)
            fDistance *= accel_slow_factor;
        math::Position3 aimPos;
        aimPos.v = _mm_add_ps(
            playerPos.v, _mm_mul_ps(d, _mm_set1_ps(fDistance)));
        __m128 a2 = _mm_sub_ps(aimPos.v, playerPos.v);
        __m128 s2 = _mm_mul_ps(a2, a2);
        float len = sqrtf(s2.m128_f32[0] + s2.m128_f32[1]
                          + s2.m128_f32[2]);
        math::Dir3 aimDir;
        aimDir.v = _mm_div_ps(a2, _mm_set1_ps(len));
        if (cmd->angles[1] != oldcmd->angles[1]
            || cmd->angles[0] != oldcmd->angles[0]
            || cmd->forwardmove != 0
            || cmd->rightmove != 0
            || (ps->fWeaponPosFrac > 0.0f && ps->fWeaponPosFrac < 1.0f))
        {
            float angles[3];
            vectosignedangles(aimDir.v.m128_f32, angles);
            ps->delta_angles[0] +=
                (int)((angles[0] - ps->viewangles[0]) * 182.04445f);
            ps->delta_angles[1] +=
                (int)((angles[1] - ps->viewangles[1]) * 182.04445f);
            ps->delta_angles[2] +=
                (int)((angles[2] - ps->viewangles[2]) * 182.04445f);
            ps->viewangles[0] = angles[0];
            ps->viewangles[1] = angles[1];
            ps->viewangles[2] = angles[2];
        }
        if (bSlowFactor)
        {
            if (distAwayFromTargetXY > 130.0f)
            {
                ps->prevTargetPointValid = 1;
                ps->currentTargetHandle.mHandle.mVal =
                    ((Entity*)(intptr_t)distToPlane)->mHandle.mHandle.mVal;
                math::Position3 tpos;
                tpos.v = _mm_setr_ps(
                    ((Entity*)(intptr_t)distToPlane)
                        ->r.currentOrigin.v.m128_f32[0],
                    ((Entity*)(intptr_t)distToPlane)
                        ->r.currentOrigin.v.m128_f32[1],
                    ((Entity*)(intptr_t)distToPlane)
                        ->r.currentOrigin.v.m128_f32[2],
                    0.0f);
                __m128 v71 = _mm_sub_ps(aimPos.v, tpos.v);
                ps->prevTargetRelPt[0] = v71.m128_f32[0];
                ps->prevTargetRelPt[1] = v71.m128_f32[1];
                ps->prevTargetRelPt[2] = v71.m128_f32[2];
                ps->mClosestStickyAimDistance = distAwayFromTargetXY;
                goto LABEL_118;
            }
        }
        ps->prevTargetPointValid = 0;
LABEL_118:
        ;
    }
LABEL_113:
    return;
}

// ea: 0x00645CD0
void PmoveSingle(pmove_t* pmove, bool isThisThePredictStep)
{
    pm = pmove;
    ++c_pmove;
    pmove->watertype = 0;
    pm->waterlevel = 0;
    if ((pm->ps->pm_flags & 0x4000) != 0
        || GamePause_IsGamePaused(currCl)
        || pm->ps->pm_type == 4
        || (pm->ps->pm_flags & 0x40000000) != 0)
    {
        pmove->cmd.buttons &= 0x6108u;
        pmove->cmd.forwardmove = 0;
        pmove->cmd.rightmove = 0;
        pmove->cmd.upmove = 0;
    }
    pm->ps->pm_flags &= ~0x8000u;
    pmove_t* v4 = pm;
    if (pm->ps->pm_type >= 6)
    {
        pm->tracemask &= ~0x2000000u;
        v4 = pm;
    }
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(v4->ps->weapon);
    pml.pWeap = InfoForWeapon;
    if (InfoForWeapon->bHoldToFire != 0
        && (pmove->cmd.buttons & 0x82) != 0)
    {
        int v6 = pmove->oldcmd.angles[2];
        int v7 = pmove->oldcmd.angles[1];
        pmove->cmd.buttons &= 0x8Au;
        int v8 = pmove->oldcmd.angles[0];
        pmove->cmd.forwardmove = 0;
        pmove->cmd.rightmove = 0;
        pmove->cmd.upmove = 0;
        pmove->cmd.angles[0] = v8;
        pmove->cmd.angles[1] = v7;
        pmove->cmd.angles[2] = v6;
        InfoForWeapon = (weaponFileInfo_t*)pml.pWeap;
    }
    PlayerState* ps = pm->ps;
    int pm_flags = pm->ps->pm_flags;
    if ((pm_flags & 1) == 0 || InfoForWeapon->type == WEAPTYPE_GRENADE)
        goto L26;
    {
        char forwardmove = pm->cmd.forwardmove;
        char v12 = pm->oldcmd.forwardmove;
        if (forwardmove == v12 || fabs((float)v12) >= fabs((float)forwardmove))
        {
            char rightmove = pm->cmd.rightmove;
            char v14 = pm->oldcmd.rightmove;
            if (rightmove == v14 || fabs((float)v14) >= fabs((float)rightmove))
            {
                if ((pm_flags & 0x20) != 0)
                    goto L19;
                unsigned int weaponstate = ps->weaponstate;
                if (weaponstate > 2 && weaponstate != 5)
                    goto L19;
L26:
                ps->pm_flags &= ~0x400u;
                goto L19;
            }
        }
        if (PM_InteruptWeaponWithProneMove() != 0)
        {
            pm->ps->pm_flags &= ~0x400u;
            pm->ps->pm_flags &= ~0x20u;
        }
    }
L19:
    if ((pm->ps->pm_flags & 0x10000) != 0)
        PM_InteruptWeaponWithSprintMove();
    PlayerState* v15 = pm->ps;
    int viewHeightTarget = pm->ps->viewHeightTarget;
    int stance;
    if (viewHeightTarget == pm->ps->crouchViewHeight)
        stance = 2;
    else
        stance = viewHeightTarget == v15->proneViewHeight;
    if ((v15->pm_flags & 0x20) != 0 && stance == 1)
    {
        pmove->cmd.forwardmove = 0;
        pmove->cmd.rightmove = 0;
    }
    if ((pm->ps->pm_flags & 0x20) != 0
        && BG_GetInfoForWeapon(pm->ps->weapon)->weapClass == WEAPCLASS_LMG)
    {
        pmove->cmd.forwardmove = 0;
        pmove->cmd.rightmove = 0;
    }
    pm->ps->eFlags &= ~0x200u;
    pmove_t* v18 = pm;
    PlayerState* v19 = pm->ps;
    if (pm->ps->pm_type != 5 && (v19->pm_flags & 0x800) == 0)
    {
        int v20 = v19->weaponstate;
        if (v20 == 0 || v20 == 3)
        {
            weaponFileInfo_t* v21 = BG_GetInfoForWeapon(v19->weapon);
            v18 = pm;
            PlayerState* v22 = pm->ps;
            if ((pm->ps->ammoclip[v21->iClipIndex] != 0
                 || ((v22->eFlags & 0x6000) != 0)
                 || (0x100000 & v22->eFlags) != 0 && v22->vehPos == 1)
                && (v22->pm_flags & 0x10000) == 0
                && (pm->cmd.buttons & 1) != 0)
            {
                v22->eFlags |= 0x200u;
                v18 = pm;
            }
        }
    }
    if (v18->ps->pm_type < 6 && (v18->cmd.buttons & 1) == 0)
    {
        v18->ps->pm_flags &= ~0x800u;
        v18 = pm;
    }
    memset(&pml, 0, sizeof(pml));
    int v24 = pmove->cmd.serverTime - v18->ps->commandTime;
    pml.msec = v24;
    if (v24 >= 1)
    {
        if (v24 > 200)
            pml.msec = 200;
    }
    else
    {
        pml.msec = 1;
    }
    v18->ps->commandTime = pmove->cmd.serverTime;
    memcpy(pml.previous_origin, pm->ps, sizeof(pml.previous_origin));
    memcpy(pml.previous_velocity, &pm->ps->velocity,
           sizeof(pml.previous_velocity));
    pml.frametime = pml.msec * 0.001f;
    pml.pWeap = BG_GetInfoForWeapon(pm->ps->weapon);
    PM_AdjustAimSpreadScale();
    PM_UpdateViewAngles(pm->ps, &pm->cmd, &pm->oldcmd, pml.msec,
                        pm->capsuletrace);
    AngleVectors(pm->ps->viewangles, pml.forward, pml.right, pml.up);
    pmove_t* v26 = pm;
    if (pm->cmd.upmove < 10)
    {
        pm->ps->pm_flags &= ~8u;
        v26 = pm;
    }
    char v27 = v26->cmd.forwardmove;
    if (v27 < 0)
    {
        PlayerState* v28 = v26->ps;
        v28->pm_flags |= 0x40;
        v26 = pm;
        goto L60;
    }
    if (v27 > 0 || v26->cmd.rightmove != 0)
    {
        PlayerState* v28 = v26->ps;
        v28->pm_flags &= ~0x40;
        v26 = pm;
        goto L60;
    }
L60:
    if (v26->ps->pm_type >= 6)
    {
        v26->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;
        pm->cmd.upmove = 0;
        v26 = pm;
    }
    if (stance == 1 && (v26->ps->pm_flags & 0x400) != 0)
    {
        v26->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;
    }
    PlayerState* v30 = pm->ps;
    switch (pm->ps->pm_type)
    {
    case 1:
    case 7:
        v30->pm_flags &= ~0x10u;
        pm->ps->mGroundEntity.mHandle.mVal = 0;
        pml.groundPlane = 0;
        pml.walking = 0;
        PM_UpdateAimDownSightFlag();
        PM_UpdatePlayerWalkingFlag();
        PM_UpdatePlayerSprintingFlag();
        PM_CheckDuck();
        PM_DropTimers();
        PM_UpdateFatigue();
        if (!isThisThePredictStep)
            PM_Weapon();
        PM_Footsteps();
        break;
    case 2:
        v30->pm_flags &= ~0x10u;
        PM_UpdateAimDownSightFlag();
        PM_UpdatePlayerWalkingFlag();
        PM_UpdatePlayerSprintingFlag();
        PM_NoclipMove();
        if (!isThisThePredictStep)
            PM_Weapon();
        PM_DropTimers();
        PM_UpdateFatigue();
        break;
    case 3:
        v30->pm_flags &= ~0x10u;
        PM_UpdateAimDownSightFlag();
        PM_UpdatePlayerWalkingFlag();
        PM_UpdatePlayerSprintingFlag();
        PM_UFOMove();
        if (!isThisThePredictStep)
            PM_Weapon();
        PM_DropTimers();
        PM_UpdateFatigue();
        break;
    case 4:
        v30->pm_flags &= ~0x10u;
        PM_UpdateAimDownSightFlag();
        PM_UpdatePlayerWalkingFlag();
        PM_UpdatePlayerSprintingFlag();
        PM_CheckDuck();
        PM_DropTimers();
        PM_UpdateFatigue();
        break;
    case 5:
        v30->pm_flags &= ~0x10u;
        break;
    default:
        if ((0x106000 & v30->eFlags) != 0)
        {
            v30->pm_flags &= ~0x10u;
            pm->ps->mGroundEntity.mHandle.mVal = 0;
            pml.groundPlane = 0;
            pml.walking = 0;
            PM_UpdateAimDownSightFlag();
            PM_UpdatePlayerWalkingFlag();
            PM_UpdatePlayerSprintingFlag();
            PM_CheckDuck();
            PM_DropTimers();
            PM_UpdateFatigue();
            PM_UpdateAimDownSightLerp();
            PM_Footsteps();
        }
        else
        {
            pml.previous_waterlevel = pmove->waterlevel;
            PM_CheckDuck();
            PM_GroundTrace();
            PM_UpdateAimDownSightFlag();
            PM_UpdatePlayerWalkingFlag();
            PM_UpdatePlayerSprintingFlag();
            PM_UpdatePronePitch();
            if (pm->ps->pm_type == 6)
                PM_DeadMove();
            PM_CheckLadderMove();
            PM_DropTimers();
            PM_UpdateFatigue();
            int tracemask = pm->tracemask;
            collision_context_t context;
            context.__vftable = (collision_context_t_vtbl*)0x00CD8F78;
            context.pass_entity1 = pm->ps->mClient;
            memset(&context.pass_entity2, 0, 12);
            context.contentmask = tracemask;
            if ((pm->ps->pm_flags & 0x10) != 0)
            {
                PM_LadderMove(context);
            }
            else if (pml.walking != 0)
            {
                PM_WalkMove(context);
            }
            else
            {
                PM_AirMove(context);
            }
            PM_GroundTrace();
            PM_Footsteps();
            if (!isThisThePredictStep)
            {
                PM_Weapon();
                PM_FoliageSounds();
            }
            PM_WaterEvents();
            float v33 = pm->ps->origin.v.m128_f32[1]
                - pml.previous_origin[1];
            float v34 = pm->ps->origin.v.m128_f32[2]
                - pml.previous_origin[2];
            float v35 = pm->ps->origin.v.m128_f32[0]
                - pml.previous_origin[0];
            float vel2 = pm->ps->velocity.v.m128_f32[1]
                    * pm->ps->velocity.v.m128_f32[1]
                + pm->ps->velocity.v.m128_f32[2]
                    * pm->ps->velocity.v.m128_f32[2]
                + pm->ps->velocity.v.m128_f32[0]
                    * pm->ps->velocity.v.m128_f32[0];
            float moved2 = (v34 * v34 + v33 * v33 + v35 * v35)
                / (pml.frametime * pml.frametime);
            if (vel2 * 0.25f > moved2)
            {
                pm->ps->velocity.v.m128_f32[0] = (1.0f / pml.frametime) * v35;
                pm->ps->velocity.v.m128_f32[1] = (1.0f / pml.frametime) * v33;
                pm->ps->velocity.v.m128_f32[2] = (1.0f / pml.frametime) * v34;
            }
        }
        break;
    }
}

// ============================================================================
// PM_FootstepEvent + surface-type helpers - ea: 0x63C8E0 (bg_pmove.cpp)
// ============================================================================
static int PM_GroundSurfaceType()
{
    if (pm == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
        AeAssert::gCurrentLine = 1069;
        AeAssert::gCurrentExpr = "pm";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if ((pml.groundTrace.surfaceFlags & 0x2000) != 0)
        return 0;
    unsigned int v1 = (pml.groundTrace.surfaceFlags >> 20) & 0x1F;
    if (v1 >= 0x17)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
        AeAssert::gCurrentLine = 1075;
        AeAssert::gCurrentExpr = "iSurfType >= 0 && iSurfType < 23";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return v1;
}

static int PM_FootstepForSurface(int iPMFlags)
{
    int result = PM_GroundSurfaceType();
    int v2 = result;
    if (result != 0)
    {
        if ((iPMFlags & 1) != 0)
        {
            result += 47;
        }
        else if ((0x10000 & iPMFlags) != 0)
        {
            result += 70;
        }
        else
        {
            if ((iPMFlags & 0x80u) != 0)
                return v2 + 24;
            ++result;
            if (pm->ps->leanf != 0.0f)
                return v2 + 24;
        }
    }
    return result;
}

void PM_trace(trace_t* results, const math::Position3& start,
              const math::Position3& mins, const math::Position3& maxs,
              const math::Position3& end,
              const collision_context_t& context);  // ea: 0x63BCA0 (below)

// ea: 0x0063C8E0
void PM_FootstepEvent(char iOldBobCycle, char iNewBobCycle, int bFootStep)
{
    if (((iNewBobCycle + 64) ^ (iOldBobCycle + 64)) & 0x80u)
    {
        unsigned char waterlevel = pm->waterlevel;
        if (waterlevel != 0)
        {
            if (waterlevel == 1 || waterlevel == 2)
            {
                int pm_flags = pm->ps->pm_flags;
                if ((pm_flags & 1) != 0)
                {
                    PM_AddEvent(67);
                }
                else if ((pm_flags & 0x80u) == 0 && pm->ps->leanf == 0.0f)
                {
                    if ((0x10000 & pm_flags) != 0)
                        PM_AddEvent(90);
                    else
                        PM_AddEvent(21);
                }
                else
                {
                    PM_AddEvent(44);
                }
            }
        }
        else
        {
            PlayerState* ps = pm->ps;
            if (pm->ps->mGroundEntity.mHandle.mVal != 0)
            {
                int v9 = ps->pm_flags;
                if ((v9 & 3) == 0 && bFootStep != 0)
                    PM_AddEvent(PM_FootstepForSurface(v9));
            }
            else if (bFootStep != 0 && (ps->pm_flags & 0x10) != 0)
            {
                math::Position3 mins = pm->mins;
                math::Position3 maxs = pm->maxs;
                mins.v.m128_f32[0] -= 6.0f;
                mins.v.m128_f32[1] -= 6.0f;
                mins.v.m128_f32[2] -= 6.0f;
                if (mins.v.m128_f32[2] < 8.0f)
                    mins.v.m128_f32[2] = 8.0f;
                maxs.v.m128_f32[0] += 6.0f;
                maxs.v.m128_f32[1] += 6.0f;
                maxs.v.m128_f32[2] += 6.0f;
                maxs.v.m128_f32[3] = 8.0f;
                collision_context_t context;
                context.__vftable = (collision_context_t_vtbl*)0x00CD8F78;
                context.pass_entity1 = pm->ps->mClient;
                memset(&context.pass_entity2, 0, 12);
                context.contentmask = pm->tracemask & 0xFDFE3FFF;
                math::Position3 start;
                start.v = pm->ps->origin.v;
                start.v = _mm_sub_ps(
                    start.v,
                    _mm_mul_ps(
                        _mm_setr_ps(pm->ps->vLadderVec[0],
                                    pm->ps->vLadderVec[1],
                                    pm->ps->vLadderVec[2], 0.0f),
                        _mm_set1_ps(31.0f)));
                trace_t tr;
                PM_trace(&tr, start, mins, maxs, start, context);
                int v8 = (int)(tr.normal.v.m128_f32[2] * 0x100000) >> 20
                    & 0x1F;
                if (tr.normal.v.m128_f32[1] == 1.0f || v8 == 0)
                    v8 = 13;
                PM_AddEvent(v8 + 1);
            }
        }
    }
}

// ============================================================================
// PM_trace - ea: 0x63BCA0 (bg_pmove.cpp)
// ============================================================================
extern void TraceSphereFull(const proximity_data_t* proximity_data,
                            trace_t* results, const math::Position3* start,
                            const math::Position3* mins,
                            const math::Position3* maxs,
                            const math::Position3* end,
                            const collision_context_t* context);  // sv_world.cpp
extern void filter_proximity_data(const math::Position3& lo,
                                  const math::Position3& hi, int contents,
                                  const proximity_data_t& in,
                                  proximity_data_t& out);  // game.o
extern void query_proximity_data(const math::Position3& lo,
                                 const math::Position3& hi,
                                 proximity_data_t& out);  // game.o
extern Entity* EntityHandleDb_GetObject(unsigned int val);  // game.o
static math::Dir3 rdir_3;   // ?rdir_3 (game.o @ 0xF58F10)
static int s_pmtrace_init;  // $S22_2 @ 0xF58F24

// ea: 0x0063BCA0
void PM_trace(trace_t* results, const math::Position3& start,
              const math::Position3& mins, const math::Position3& maxs,
              const math::Position3& end,
              const collision_context_t& context)
{
    Entity* ent = (Entity*)EntityHandleDb_GetObject(
        context.pass_entity1.mHandle.mVal);
    proximity_data_t filtered;
    math::Position3 lo;
    lo.v = _mm_min_ps(start.v, end.v);
    math::Position3 hi;
    hi.v = _mm_max_ps(start.v, end.v);
    if (ent != nullptr && ent->proximity_data != nullptr)
    {
        math::Position3 size;
        size.v = _mm_sub_ps(maxs.v,
            _mm_mul_ps(_mm_add_ps(mins.v, maxs.v), _mm_set1_ps(0.5f)));
        float v12 = size.v.m128_f32[0] > size.v.m128_f32[2]
            ? size.v.m128_f32[2]
            : size.v.m128_f32[0];
        size.v.m128_f32[0] = v12;
        size.v.m128_f32[1] = v12;
        math::Position3 boxMin;
        boxMin.v = _mm_sub_ps(lo.v, size.v);
        math::Position3 boxMax;
        boxMax.v = _mm_add_ps(hi.v, _mm_set1_ps(v12));
        if ((_mm_movemask_ps(_mm_cmplt_ps(
                 _mm_max_ps(
                     _mm_sub_ps(ent->proximity_data->lo.v, boxMin.v),
                     _mm_sub_ps(hi.v, ent->proximity_data->hi.v)),
                 _mm_setzero_ps()))
             & 7) != 7)
        {
            if ((s_pmtrace_init & 1) == 0)
            {
                s_pmtrace_init |= 1;
                rdir_3.v = _mm_set1_ps(20.0f);
            }
            math::Position3 qlo = boxMin;
            qlo.v = _mm_sub_ps(boxMin.v, rdir_3.v);
            math::Position3 qhi = boxMax;
            qhi.v = _mm_add_ps(boxMax.v, rdir_3.v);
            query_proximity_data(qlo, qhi, *ent->proximity_data);
        }
        filter_proximity_data(lo, hi, context.contentmask,
                              *ent->proximity_data, filtered);
        TraceSphereFull(&filtered, results, &start, &mins, &maxs, &end,
                        &context);
    }
    else
    {
        pm->trace(results, start, mins, maxs, end, context);
    }
    if (results->startsolid != 0 && (results->contents & 0x2000000) != 0)
    {
        PM_AddTouchEnt(results->mEntity);
        pm->tracemask &= ~0x2000000u;
        collision_context_t c2;
        c2.__vftable = (collision_context_t_vtbl*)0x00CD8F78;
        c2.pass_entity1.mHandle.mVal = 0;
        c2.pass_entity2.mHandle.mVal = 0;
        c2.contentmask = context.contentmask & 0xFDFFFFFF;
        c2.pass_owner1.mHandle.mVal = 0;
        c2.pass_owner2.mHandle.mVal = 0;
        if (ent != nullptr && ent->proximity_data != nullptr)
        {
            filter_proximity_data(lo, hi, context.contentmask & 0xFDFFFFFF,
                                  *ent->proximity_data, filtered);
            TraceSphereFull(&filtered, results, &start, &mins, &maxs, &end,
                            &c2);
        }
        else
        {
            pm->trace(results, start, mins, maxs, end, c2);
        }
    }
}

// ============================================================================
// Character collision resolve - ea: 0x63BF90..0x643850 (bg_pmove.cpp)
// ============================================================================
extern int CM_AreaEntities(const math::Position3& mins,
                           const math::Position3& maxs,
                           DbLinkedHandle<EntityHandleDb, Entity>* entityList,
                           int maxcount, int contentmask);  // game.o
extern float overpush;   // ?overpush (game.o)
extern float radius_3;   // ?radius_3 (game.o)
extern bool push_in_world(pmove_t& pm, float radius,
                          const collision_context_t& context);  // game.o
extern bool tunnel_test(pmove_t& pm, float radius,
                        const math::Position3& p0,
                        const math::Position3& p1);  // game.o
extern Entity* EntityHandleDb_GetObject(unsigned int val);  // game.o
extern "C" int __fpclass(float);  // CRT

static TouchEntityData s_entities_3;  // ?entities_3 (game.o @ 0xF58F30)
static int s_entities_3_init;         // $S23_6 @ 0xF591D0

// ea: 0x0063BF90
bool resolve_character_collisions(pmove_t& pm, float radius)
{
    PlayerState* ps = pm.ps;
    bool hit = false;
    Entity* self = (Entity*)EntityHandleDb_GetObject(
        ps->mClient.mHandle.mVal);
    math::Position3 lo = ps->origin;
    math::Position3 p1;
    p1.v = ps->origin.v;
    p1.v.m128_f32[2] =
        ((pm.mins.v.m128_f32[2] + pm.maxs.v.m128_f32[2]) * 0.5f)
        + ps->origin.v.m128_f32[2];
    __m128 v4 = _mm_set1_ps(2.0f);
    __m128 v5 = _mm_mul_ps(_mm_set1_ps(radius), v4);
    math::Position3 p1a = p1;
    p1a.v = _mm_sub_ps(p1.v, v5);
    p1a.v.m128_f32[2] -= 100.0f;
    math::Position3 p2 = p1;
    p2.v = _mm_add_ps(p1.v, _mm_mul_ps(_mm_set1_ps(radius), v4));
    p2.v.m128_f32[2] += 100.0f;
    if ((s_entities_3_init & 1) == 0)
    {
        s_entities_3_init |= 1;
        memset(s_entities_3.touch, 0, sizeof(s_entities_3.touch));
    }
    s_entities_3.mins = p1a;
    s_entities_3.maxs = p2;
    s_entities_3.num = CM_AreaEntities(
        p1a, p2, s_entities_3.touch, 128, pm.tracemask);
    math::Position3 v9 = lo;
    for (int v8 = 0; v8 < s_entities_3.num; ++v8)
    {
        Entity* mObject = (Entity*)EntityHandleDb_GetObject(
            s_entities_3.touch[v8].mHandle.mVal);
        if (mObject != nullptr && mObject != self)
        {
            math::Position3 entOrigin = mObject->r.currentOrigin;
            if (entOrigin.v.m128_f32[2]
                    <= ((pm.maxs.v.m128_f32[2] + lo.v.m128_f32[2]) + 2.0f)
                && lo.v.m128_f32[2] <= (entOrigin.v.m128_f32[2] + 72.0f))
            {
                __m128 v17 = _mm_sub_ps(entOrigin.v, v9.v);
                v17.m128_f32[2] = 0.0f;
                __m128 sq = _mm_mul_ps(v17, v17);
                float dist2 =
                    sq.m128_f32[0] + sq.m128_f32[1] + sq.m128_f32[2];
                if ((radius * radius) * 4.0f > dist2)
                {
                    __m128 v14 = _mm_xor_ps(v17, _mm_set1_ps(-0.0f));
                    float dist = sqrtf(dist2);
                    __m128 v15;
                    if (dist <= 0.0099999998f)
                        v15 = _mm_set1_ps(1.0f);
                    else
                        v15 = _mm_div_ps(v14, _mm_set1_ps(dist));
                    float pushDist = (radius * 2.0f) + overpush;
                    hit = true;
                    v9.v = _mm_add_ps(
                        entOrigin.v,
                        _mm_mul_ps(v15, _mm_set1_ps(pushDist)));
                    lo = v9;
                }
            }
        }
    }
    if ((__fpclass(lo.v.m128_f32[0]) & 0x297) == 0
        && (__fpclass(lo.v.m128_f32[1]) & 0x297) == 0
        && (__fpclass(lo.v.m128_f32[2]) & 0x297) == 0)
        pm.ps->origin = lo;
    return hit;
}

// ea: 0x00643850
void resolve_collisions(const collision_context_t& context,
                        const math::Position3& old_pos)
{
    int tracemask = pm->tracemask;
    pm->tracemask = 0x2000000;
    int v24 = tracemask;
    int v4 = 0;
    if (resolve_character_collisions(*pm, radius_3))
    {
        bool v5;
        while (1)
        {
            v5 = resolve_character_collisions(*pm, radius_3);
            if (++v4 > 4)
                break;
            if (!v5)
                goto done_char;
        }
        if (v5)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
            AeAssert::gCurrentLine = 1381;
            AeAssert::gCurrentExpr = "!hitb";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("stuck between ai-s"))
                __debugbreak();
        }
    }
done_char:
    float v6 = radius_3;
    pm->tracemask = tracemask & 0xFDFFFFFF;
    int v7 = 0;
    if (push_in_world(*pm, v6, context))
    {
        bool v8;
        do
        {
            v8 = push_in_world(*pm, radius_3, context);
            ++v7;
        } while (v7 <= 4 && v8);
    }
    float v9 = old_pos.v.m128_f32[1];
    float v10 = old_pos.v.m128_f32[2];
    float v11 = old_pos.v.m128_f32[3];
    math::Position3 p0;
    p0.v.m128_f32[0] = old_pos.v.m128_f32[0];
    p0.v.m128_f32[1] = v9;
    p0.v.m128_f32[2] = v10;
    p0.v.m128_f32[3] = v11;
    math::Position3 p1;
    p1.v = pm->ps->origin.v;
    __m128 v16 = _mm_sub_ps(p1.v, p0.v);
    __m128 v17 = _mm_mul_ps(v16, v16);
    float dist2 = v17.m128_f32[0] + v17.m128_f32[1] + v17.m128_f32[2];
    if (dist2 > (radius_3 * radius_3))
    {
        float v18 = (pm->maxs.v.m128_f32[2] - pm->mins.v.m128_f32[2])
            * 0.333f;
        p0.v.m128_f32[2] += v18;
        p1.v.m128_f32[2] += v18;
        if (tunnel_test(*pm, radius_3, p0, p1))
        {
            pm->ps->origin.v = old_pos.v;
            if ((__fpclass(pm->ps->origin.v.m128_f32[0]) & 0x297) != 0
                || (__fpclass(pm->ps->origin.v.m128_f32[1]) & 0x297) != 0
                || (__fpclass(pm->ps->origin.v.m128_f32[2]) & 0x297) != 0)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
                AeAssert::gCurrentLine = 1405;
                AeAssert::gCurrentExpr =
                    "!IS_NAN((pm->ps->origin)[0]) && !IS_NAN((pm->ps->origin)[1]) && !IS_NAN((pm->ps->origin)[2])";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Invalid vector"))
                    __debugbreak();
            }
        }
        p0.v.m128_f32[2] += v18;
        p1.v.m128_f32[2] += v18;
        bool v20 = tunnel_test(*pm, radius_3, p0, p1);
        if (!v20)
        {
            pm->tracemask = v24;
            return;
        }
        pm->ps->origin.v = old_pos.v;
        if ((__fpclass(pm->ps->origin.v.m128_f32[0]) & 0x297) == 0
            && (__fpclass(pm->ps->origin.v.m128_f32[1]) & 0x297) == 0
            && (__fpclass(pm->ps->origin.v.m128_f32[2]) & 0x297) == 0)
        {
            pm->tracemask = v24;
            return;
        }
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
        AeAssert::gCurrentLine = 1412;
        AeAssert::gCurrentExpr =
            "!IS_NAN((pm->ps->origin)[0]) && !IS_NAN((pm->ps->origin)[1]) && !IS_NAN((pm->ps->origin)[2])";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    pm->tracemask = v24;
}

// ============================================================================
// push_in_world - ea: 0x63AC30 (bg_pmove.cpp)
// ============================================================================
extern float threshold_0;  // ?threshold_0 (game.o)
static TouchEntityData s_entities_2;  // ?entities_2 (game.o @ 0xF58C60)
static int s_entities_2_init;         // $S21_3 @ 0xF58F00
extern bool collide_sphere_brush(math::Position3& sphere_center,
                                 float sphere_radius,
                                 const cdl_object_t& obj,
                                 const cdlPlane* sides, unsigned int nsides,
                                 math::Position3& new_sphere_center);
    // game.o 0x61E660
extern bool collide_sphere_box(const math::Position3& sphere_center,
                               float sphere_radius, const cdl_object_t& box,
                               math::Position3& new_sphere_center);
    // game.o 0x61E890
extern bool new_push_out_sphere_triangle(const math::Position3& sphere_center,
                                         float sphere_radius,
                                         const math::Position3& v0,
                                         const math::Position3& v1,
                                         const math::Position3& v2,
                                         const math::Dir3& normal,
                                         math::Position3& new_sphere_center);
    // game.o 0x60D860
extern math::Vector4 calc_normal(const math::Position3& v0,
                                 const math::Position3& v1,
                                 const math::Position3& v2);  // game.o 0x60C400
extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* msg);  // core/tl_system.cpp
extern void AnglesToAxis(const math::Position3* angles,
                         const math::Position3* origin,
                         math::Mat43* mat);
    // ?AnglesToAxis@@YAXABVPosition3@math@@0AAVMat43@2@@Z (core.o)

// ea: 0x0063AC30
bool push_in_world(pmove_t& pm, float radius,
                   const collision_context_t& context)
{
    PlayerState* ps = pm.ps;
    float midZ = (pm.mins.v.m128_f32[2] + pm.maxs.v.m128_f32[2]) * 0.5f;
    math::Position3 center;
    center.v.m128_f32[0] = ps->origin.v.m128_f32[0];
    center.v.m128_f32[1] = ps->origin.v.m128_f32[1];
    center.v.m128_f32[2] = midZ + ps->origin.v.m128_f32[2];
    center.v.m128_f32[3] = ps->origin.v.m128_f32[3];
    if ((0x100000 & ps->eFlags) != 0)
        return false;
    proximity_data_t filtered;
    Entity* self = (Entity*)EntityHandleDb_GetObject(
        pm.ps->mClient.mHandle.mVal);
    math::Position3 boxMin;
    boxMin.v = _mm_sub_ps(center.v, _mm_set1_ps(radius * 2.0f));
    boxMin.v.m128_f32[2] -= 100.0f;
    math::Position3 boxMax;
    boxMax.v = _mm_add_ps(center.v, _mm_set1_ps(radius * 2.0f));
    boxMax.v.m128_f32[2] += 100.0f;
    if (self != nullptr && self->proximity_data != nullptr
        && (_mm_movemask_ps(_mm_cmplt_ps(
                _mm_max_ps(
                    _mm_sub_ps(self->proximity_data->lo.v, boxMin.v),
                    _mm_sub_ps(boxMax.v, self->proximity_data->hi.v)),
                _mm_setzero_ps()))
            & 7) != 7)
        query_proximity_data(boxMin, boxMax, *self->proximity_data);
    filter_proximity_data(boxMin, boxMax, pm.tracemask,
                          *self->proximity_data, filtered);
    if ((s_entities_2_init & 1) == 0)
    {
        s_entities_2_init |= 1;
        memset(s_entities_2.touch, 0, sizeof(s_entities_2.touch));
    }
    s_entities_2.mins = boxMin;
    s_entities_2.maxs = boxMax;
    s_entities_2.num = CM_AreaEntities(
        boxMin, boxMax, s_entities_2.touch, 128, pm.tracemask);
    float halfMaxZ = pm.maxs.v.m128_f32[2] * 0.5f;
    float v15 = pm.maxs.v.m128_f32[2] - radius;
    bool hit = false;
    int step = 0;
    do
    {
        math::Position3 probe;
        probe.v.m128_f32[0] = pm.ps->origin.v.m128_f32[0];
        probe.v.m128_f32[1] = pm.ps->origin.v.m128_f32[1];
        float stepOff = step == 0 ? radius
            : (step == 1 ? radius : v15);
        probe.v.m128_f32[2] = pm.ps->origin.v.m128_f32[2] + stepOff;
        probe.v.m128_f32[3] = pm.ps->origin.v.m128_f32[3];
        for (int i = 0; i < s_entities_2.num; ++i)
        {
            Entity* ent = (Entity*)EntityHandleDb_GetObject(
                s_entities_2.touch[i].mHandle.mVal);
            if (ent == nullptr || ent == self)
                continue;
            if (context.__vftable->filter((collision_context_t*)&context,
                                          ent))
                continue;
            if (ent->r.bmodel != nullptr)
            {
                math::Mat43 mat;
                if ((0x800000 & ent->r.contents) == 0 || step != 0)
                {
                    mat = ent->CalcRotTranMat43();
                }
                else
                {
                    math::Position3 angles = ent->r.currentAngles;
                    if (threshold_0 > fabs(angles.v.m128_f32[2]))
                        angles.v.m128_f32[2] = 0.0f;
                    if (threshold_0 > fabs(angles.v.m128_f32[0]))
                        angles.v.m128_f32[0] = 0.0f;
                    AnglesToAxis(&angles, &ent->r.currentOrigin, &mat);
                }
                math::Position3 localProbe;
                localProbe.v = _mm_add_ps(
                    _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(_mm_set1_ps(probe.v.m128_f32[0]),
                                       mat.x.v),
                            _mm_mul_ps(_mm_set1_ps(probe.v.m128_f32[1]),
                                       mat.y.v)),
                        _mm_mul_ps(_mm_set1_ps(probe.v.m128_f32[2]),
                                   mat.z.v)),
                    mat.w.v);
                DCGSet* bmodel = ent->r.bmodel;
                int nbrushes = bmodel->nbrushes;
                for (int b = 0; b < nbrushes; ++b)
                {
                    unsigned int oi = b + bmodel->nboxes;
                    if (oi >= (unsigned int)bmodel->objects_m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                            "index >= 0 && index < size()", "invalid index"))
                        __debugbreak();
                    cdl_object_t obj;
                    obj = ((cdl_object_t*)bmodel->objects_m_elements)[oi];
                    if (b >= (unsigned int)bmodel->brushes_m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                            "index >= 0 && index < size()", "invalid index"))
                        __debugbreak();
                    unsigned short* brush =
                        &((unsigned short*)bmodel->brushes_m_elements)[2 * b];
                    if ((obj.cflags & pm.tracemask) != 0)
                    {
                        unsigned int firstSide = brush[0];
                        if (firstSide
                                >= (unsigned int)bmodel->brush_sides_m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                89, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                        hit |= collide_sphere_brush(
                            localProbe, radius, obj,
                            (cdlPlane*)bmodel->brush_sides_m_elements
                                + firstSide,
                            brush[1], localProbe);
                    }
                }
                int nboxes = bmodel->nboxes;
                for (int b = 0; b < nboxes; ++b)
                {
                    if (b >= (unsigned int)bmodel->objects_m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                            "index >= 0 && index < size()", "invalid index"))
                        __debugbreak();
                    const cdl_object_t* obj =
                        &((cdl_object_t*)bmodel->objects_m_elements)[b];
                    if ((obj->cflags & pm.tracemask) != 0)
                        hit |= collide_sphere_box(localProbe, radius, *obj,
                                                  localProbe);
                }
                if (hit)
                {
                    probe.v = _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(_mm_set1_ps(localProbe.v.m128_f32[0]),
                                       mat.x.v),
                            _mm_mul_ps(_mm_set1_ps(localProbe.v.m128_f32[1]),
                                       mat.y.v)),
                        _mm_add_ps(
                            _mm_mul_ps(_mm_set1_ps(localProbe.v.m128_f32[2]),
                                       mat.z.v),
                            mat.w.v));
                }
            }
            else
            {
                __m128 d = _mm_sub_ps(
                    probe.v,
                    _mm_min_ps(_mm_max_ps(probe.v, ent->r.absmin.v),
                               ent->r.absmax.v));
                __m128 ds = _mm_mul_ps(d, d);
                float dist2 = ds.m128_f32[0] + ds.m128_f32[1]
                    + ds.m128_f32[2];
                if (dist2 >= radius * radius || dist2 <= 0.001f)
                    continue;
                float dist = sqrtf(dist2);
                float push = radius - dist + 0.0099999998f;
                probe.v = _mm_add_ps(
                    probe.v,
                    _mm_mul_ps(_mm_div_ps(d, _mm_set1_ps(dist)),
                               _mm_set1_ps(push)));
                hit = true;
            }
        }
        // proximity brushes
        for (int i = 0; i < filtered.brushes_count; ++i)
        {
            const proxy_obj_t& slot = filtered.brushes_slot[i];
            CGBank* bank =
                ((CGBankManager*)CGBankManager::sInst)->mBankArray[slot.bi];
            unsigned int oi = slot.oi;
            if (oi >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            cdl_object_t* obj =
                &((cdl_object_t*)bank->objects.m_elements)[oi];
            unsigned int brushIdx = oi - bank->nboxes;
            if (brushIdx >= (unsigned int)bank->brushes.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            cdl_brush_t* brush =
                &((cdl_brush_t*)bank->brushes.m_elements)[brushIdx];
            if (brush->first_side >= (unsigned int)bank->brush_sides.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            hit |= collide_sphere_brush(
                probe, radius, *obj,
                (cdlPlane*)bank->brush_sides.m_elements
                    + brush->first_side,
                brush->num_sides, probe);
        }
        // proximity boxes
        for (int i = 0; i < filtered.boxes_count; ++i)
        {
            const proxy_obj_t& slot = filtered.boxes_slot[i];
            CGBank* bank =
                ((CGBankManager*)CGBankManager::sInst)->mBankArray[slot.bi];
            unsigned int oi = slot.oi;
            if (oi >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            hit |= collide_sphere_box(
                probe, radius,
                ((cdl_object_t*)bank->objects.m_elements)[oi], probe);
        }
        // proximity polies
        for (int i = 0; i < filtered.polies_count; ++i)
        {
            const bounded_proxy_obj_t& slot = filtered.polies_slot[i];
            CGBank* bank =
                ((CGBankManager*)CGBankManager::sInst)->mBankArray[slot.bi];
            unsigned int oi = slot.oi;
            if (oi >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            cdl_object_t* obj =
                &((cdl_object_t*)bank->objects.m_elements)[oi];
            unsigned int patchIdx = oi - bank->nbrushes - bank->nboxes;
            if (patchIdx >= (unsigned int)bank->patches.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            cdl_patch_t* patch =
                &((cdl_patch_t*)bank->patches.m_elements)[patchIdx];
            unsigned int inds = patch->first_index + 3 * slot.ti;
            if (inds >= (unsigned int)bank->patch_inds.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            unsigned int* pvi =
                &((unsigned int*)bank->patch_inds.m_elements)[inds];
            math::Position3 base;
            base.v = _mm_add_ps(
                _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f),
                _mm_mul_ps(
                    _mm_setr_ps((float)(pvi[0] & 0x7FF),
                                (float)((pvi[0] >> 11) & 0x7FF),
                                (float)(pvi[0] >> 22), 0.0f),
                    _mm_set1_ps(0.25f)));
            math::Position3 v0 = base;
            math::Position3 v1 = base;
            math::Position3 v2 = base;
            v0.v = _mm_add_ps(
                base.v,
                _mm_mul_ps(
                    _mm_setr_ps((float)(pvi[1] & 0x7FF),
                                (float)((pvi[1] >> 11) & 0x7FF),
                                (float)(pvi[1] >> 22), 0.0f),
                    _mm_set1_ps(0.25f)));
            v1.v = _mm_add_ps(
                base.v,
                _mm_mul_ps(
                    _mm_setr_ps((float)(pvi[2] & 0x7FF),
                                (float)((pvi[2] >> 11) & 0x7FF),
                                (float)(pvi[2] >> 22), 0.0f),
                    _mm_set1_ps(0.25f)));
            v2.v = _mm_add_ps(
                base.v,
                _mm_mul_ps(
                    _mm_setr_ps((float)(pvi[3] & 0x7FF),
                                (float)((pvi[3] >> 11) & 0x7FF),
                                (float)(pvi[3] >> 22), 0.0f),
                    _mm_set1_ps(0.25f)));
            math::Vector4 n = calc_normal(v0, v1, v2);
            math::Dir3 nd;
            nd.v = n.v;
            hit |= new_push_out_sphere_triangle(
                probe, radius, v0, v1, v2, nd, probe);
        }
        if ((__fpclass(probe.v.m128_f32[0]) & 0x297) == 0
            && (__fpclass(probe.v.m128_f32[1]) & 0x297) == 0
            && (__fpclass(probe.v.m128_f32[2]) & 0x297) == 0)
        {
            pm.ps->origin.v = probe.v;
            pm.ps->origin.v.m128_f32[2] -= stepOff;
        }
        ++step;
    } while (step < 3);
    return hit;
}

// ============================================================================
// PM_SlideMove - ea: 0x63E850 (bg_pmove.cpp)
// ============================================================================
extern float VectorNormalize2(const math::Dir3& v, math::Dir3& out);
    // ?VectorNormalize2@@YAMABVDir3@math@@AAV12@@Z
extern void Com_Printf(const char* fmt, ...);  // core.o

// ea: 0x0063E850
int PM_SlideMove(int gravity)
{
    int v2 = 0;
    PlayerState* ps = pm->ps;
    float orig_vx = ps->velocity.v.m128_f32[0];
    float orig_vy = ps->velocity.v.m128_f32[1];
    if (gravity != 0)
    {
        float v4 = ps->gravity * pml.frametime;
        orig_vx = ps->velocity.v.m128_f32[0];
        orig_vy = ps->velocity.v.m128_f32[1];
        float v5 = ps->velocity.v.m128_f32[2] - v4;
        ps->velocity.v.m128_f32[2] =
            (ps->velocity.v.m128_f32[2] + v5) * 0.5f;
        float orig_vz = v5;
        if (pml.groundPlane != 0)
        {
            PlayerState* v6 = pm->ps;
            float v7 = (v6->velocity.v.m128_f32[1]
                            * pml.groundTrace.normal.v.m128_f32[1]
                        + v6->velocity.v.m128_f32[2]
                            * pml.groundTrace.normal.v.m128_f32[2])
                + pml.groundTrace.normal.v.m128_f32[0]
                    * v6->velocity.v.m128_f32[0];
            float v8 = v7 >= 0.0f ? v7 * 0.99900097f : v7 * 1.001f;
            v6->velocity.v.m128_f32[0] -=
                v8 * pml.groundTrace.normal.v.m128_f32[0];
            v6->velocity.v.m128_f32[1] -=
                v8 * pml.groundTrace.normal.v.m128_f32[1];
            v6->velocity.v.m128_f32[2] -=
                v8 * pml.groundTrace.normal.v.m128_f32[2];
        }
        float frametime = pml.frametime;
        float planes[8][3];
        if (pml.groundPlane != 0)
        {
            planes[0][0] = pml.groundTrace.normal.v.m128_f32[0];
            planes[0][1] = pml.groundTrace.normal.v.m128_f32[1];
            planes[0][2] = pml.groundTrace.normal.v.m128_f32[2];
            v2 = 1;
        }
        math::Dir3 vel;
        vel.v = pm->ps->velocity.v;
        VectorNormalize2(vel, *(math::Dir3*)&planes[v2]);
        int v11 = v2 + 1;
        collision_context_t context;
        context.__vftable = (collision_context_t_vtbl*)0x00CD8F78;
        context.pass_entity1 = pm->ps->mClient;
        context.pass_entity2.mHandle.mVal = 0;
        context.contentmask = pm->tracemask;
        context.pass_owner1.mHandle.mVal = 0;
        context.pass_owner2.mHandle.mVal = 0;
        int bumpcount = 0;
        float time_left = pml.frametime;
        float into = 0.0f;
        math::Dir3 clipVelocity[8];
        math::Dir3 primal_velocity;
        while (1)
        {
            math::Position3 end;
            end.v.m128_f32[0] = (ps->velocity.v.m128_f32[0] * frametime)
                + ps->origin.v.m128_f32[0];
            end.v.m128_f32[1] = (ps->velocity.v.m128_f32[1] * frametime)
                + ps->origin.v.m128_f32[1];
            end.v.m128_f32[2] = (ps->velocity.v.m128_f32[2] * frametime)
                + ps->origin.v.m128_f32[2];
            end.v.m128_f32[3] = 0.0f;
            trace_t trace;
            PM_trace(&trace, ps->origin, pm->mins, pm->maxs, end, context);
            if (trace.allsolid)
            {
                pm->ps->velocity.v.m128_f32[2] = 0.0f;
                return true;
            }
            if (trace.fraction > 0.0f)
            {
                pm->ps->origin.v.m128_f32[0] =
                    ps->origin.v.m128_f32[0]
                    + (ps->velocity.v.m128_f32[0] * frametime
                       * trace.fraction);
                pm->ps->origin.v.m128_f32[1] =
                    ps->origin.v.m128_f32[1]
                    + (ps->velocity.v.m128_f32[1] * frametime
                       * trace.fraction);
                pm->ps->origin.v.m128_f32[2] =
                    ps->origin.v.m128_f32[2]
                    + (ps->velocity.v.m128_f32[2] * frametime
                       * trace.fraction);
            }
            if (trace.fraction == 1.0f)
                break;
            PM_AddTouchEnt(trace.mEntity);
            time_left -= trace.fraction * frametime;
            if (v11 >= 8)
                break;
            int v12 = 0;
            int found = -1;
            for (int i = 0; i < v11; ++i)
            {
                if ((planes[i][0] * trace.normal.v.m128_f32[0]
                     + planes[i][1] * trace.normal.v.m128_f32[1]
                     + planes[i][2] * trace.normal.v.m128_f32[2])
                    > 0.99900001f)
                {
                    found = i;
                    break;
                }
            }
            if (pm->debugLevel >= 2 && found >= 0)
                Com_Printf("%i:recollided with plane normal (%.2f, %.2f, %.2f)\n",
                           c_pmove, trace.normal.v.m128_f32[0],
                           trace.normal.v.m128_f32[1],
                           trace.normal.v.m128_f32[2]);
            if (found >= 0)
            {
                pm->ps->velocity.v.m128_f32[0] +=
                    trace.normal.v.m128_f32[0];
                pm->ps->velocity.v.m128_f32[1] +=
                    trace.normal.v.m128_f32[1];
                pm->ps->velocity.v.m128_f32[2] +=
                    trace.normal.v.m128_f32[2];
            }
            clipVelocity[v11 - 1].v = trace.normal.v;
            ++v11;
            int v18 = 0;
            int impactIdx = -1;
            float impact = 0.0f;
            for (int i = 0; i < v11; ++i)
            {
                float d = planes[i][0] * ps->velocity.v.m128_f32[0]
                    + planes[i][1] * ps->velocity.v.m128_f32[1]
                    + planes[i][2] * ps->velocity.v.m128_f32[2];
                if (d >= 0.1f)
                    break;
                impactIdx = i;
                impact = d;
            }
            if (impactIdx >= 0)
            {
                if (-impact > pml.impactSpeed)
                    pml.impactSpeed = -impact;
                float d = planes[impactIdx][0] * ps->velocity.v.m128_f32[0]
                    + planes[impactIdx][1] * ps->velocity.v.m128_f32[1]
                    + planes[impactIdx][2] * ps->velocity.v.m128_f32[2];
                float f = d >= 0.0f ? d * 0.99900097f : d * 1.001f;
                math::Dir3 newVel;
                newVel.v.m128_f32[0] =
                    ps->velocity.v.m128_f32[0] - planes[impactIdx][0] * f;
                newVel.v.m128_f32[1] =
                    ps->velocity.v.m128_f32[1] - planes[impactIdx][1] * f;
                newVel.v.m128_f32[2] =
                    ps->velocity.v.m128_f32[2] - planes[impactIdx][2] * f;
                float dp = planes[impactIdx][0] * orig_vx
                    + planes[impactIdx][1] * orig_vy
                    + planes[impactIdx][2] * (ps->velocity.v.m128_f32[2]
                        + pml.frametime * ps->gravity);
                float f2 = dp >= 0.0f ? dp * 0.99900097f : dp * 1.001f;
                math::Dir3 primal;
                primal.v.m128_f32[0] = orig_vx - planes[impactIdx][0] * f2;
                primal.v.m128_f32[1] = orig_vy - planes[impactIdx][1] * f2;
                primal.v.m128_f32[2] =
                    (ps->velocity.v.m128_f32[2]
                     + pml.frametime * ps->gravity)
                    - planes[impactIdx][2] * f2;
                // clip against all other planes
                for (int i = 0; i < v11; ++i)
                {
                    if (i == impactIdx)
                        continue;
                    float dn = planes[i][0] * newVel.v.m128_f32[0]
                        + planes[i][1] * newVel.v.m128_f32[1]
                        + planes[i][2] * newVel.v.m128_f32[2];
                    if (dn >= 0.1f)
                        continue;
                    float fn = dn >= 0.0f ? dn * 0.99900097f : dn * 1.001f;
                    newVel.v.m128_f32[0] -= planes[i][0] * fn;
                    newVel.v.m128_f32[1] -= planes[i][1] * fn;
                    newVel.v.m128_f32[2] -= planes[i][2] * fn;
                    float dpn = planes[i][0] * primal.v.m128_f32[0]
                        + planes[i][1] * primal.v.m128_f32[1]
                        + planes[i][2] * primal.v.m128_f32[2];
                    float fpn = dpn >= 0.0f ? dpn * 0.99900097f
                                            : dpn * 1.001f;
                    primal.v.m128_f32[0] -= planes[i][0] * fpn;
                    primal.v.m128_f32[1] -= planes[i][1] * fpn;
                    primal.v.m128_f32[2] -= planes[i][2] * fpn;
                }
                ps->velocity.v.m128_f32[0] = newVel.v.m128_f32[0];
                ps->velocity.v.m128_f32[1] = newVel.v.m128_f32[1];
                ps->velocity.v.m128_f32[2] = newVel.v.m128_f32[2];
            }
            if (++bumpcount >= 4)
                break;
        }
        if (pm->debugLevel >= 2)
            Com_Printf("%i:MAX_CLIP_PLANES\n", c_pmove);
        pm->ps->velocity.v.m128_f32[2] = 0.0f;
        pm->ps->velocity.v.m128_f32[1] = 0.0f;
        pm->ps->velocity.v.m128_f32[0] = 0.0f;
        return true;
    }
    return false;
}

// ============================================================================
// PM_StepSlideMove - ea: 0x63F230 (bg_pmove.cpp)
// ============================================================================
extern void BG_AddPredictableEventToPlayerstate(int newEvent, int eventParm,
                                                PlayerState* ps);  // game.o
extern int PM_VerifyPronePosition(const math::Position3& vFallbackOrg,
                                  const math::Position3& vFallbackVel);
    // game.o 0x63D2B0? (defined below)
extern void PM_FootstepEvent(char iOldBobCycle, char iNewBobCycle,
                             int bFootStep);  // game.o 0x63C8E0
extern int PM_ShouldMakeFootsteps();          // game.o 0x606250

// ea: 0x0063F230
void PM_StepSlideMove(int gravity)
{
    PlayerState* ps = pm->ps;
    int pm_flags = pm->ps->pm_flags;
    math::Position3 start_o;
    start_o.v = _mm_setzero_ps();
    float fStepAmount = 0.0f;
    int bHadGround = 0;
    float fStepSize = 0.0f;
    if ((pm_flags & 0x10) != 0)
    {
        ps->pm_flags = pm_flags & 0xFFFFDFFF;
        PlayerState* v5 = pm->ps;
        start_o.v.m128_f32[2] = 0.0f;
        v5->fJumpOriginZ = 0.0f;
    }
    else
    {
        if (pml.groundPlane != 0)
        {
            start_o.v.m128_f32[2] = 1.0f;
            goto L9;
        }
        start_o.v.m128_f32[2] = 0.0f;
        if ((pm_flags & 0x2000) == 0 || ps->pm_time == 0)
            goto L9;
        ps->pm_flags &= ~0x2000u;
        pm->ps->fJumpOriginZ = 0.0f;
    }
L9:
    math::Position3 down_o;
    down_o.v.m128_f32[0] = pm->ps->origin.v.m128_f32[0];
    down_o.v.m128_f32[1] = pm->ps->origin.v.m128_f32[1];
    down_o.v.m128_f32[2] = pm->ps->origin.v.m128_f32[2];
    down_o.v.m128_f32[3] = pm->ps->origin.v.m128_f32[3];
    math::Position3 down_v;
    down_v.v = down_o.v;
    int v7 = PM_SlideMove(gravity);
    pmove_t* v8 = pm;
    PlayerState* v9 = pm->ps;
    int v10 = pm->ps->pm_flags;
    if ((v10 & 1) != 0)
        start_o.v.m128_f32[3] = 10.0f;
    else
        start_o.v.m128_f32[3] = 18.0f;
    float v11;
    if (v9->mGroundEntity.mHandle.mVal != 0)
    {
        v11 = down_o.v.m128_f32[2];
    }
    else
    {
        if ((v10 & 0x2000) != 0 && v9->pm_time != 0)
        {
            v9->pm_flags &= ~0x2000u;
            pm->ps->fJumpOriginZ = 0.0f;
            v8 = pm;
        }
        v11 = down_o.v.m128_f32[2];
        float v12;
        if (v7 != 0
            && (v9 = v8->ps, (v8->ps->pm_flags & 0x2000) != 0)
            && ((v12 = v9->fJumpOriginZ + 39.0f)
                > down_o.v.m128_f32[2]))
        {
            start_o.v.m128_f32[3] = 18.0f;
            if ((down_o.v.m128_f32[2] + 18.0f) > v12)
            {
                start_o.v.m128_f32[3] = v12 - down_o.v.m128_f32[2];
                if ((v12 - down_o.v.m128_f32[2]) < 1.0f)
                    return;
            }
            fStepAmount = 1.0f;
        }
        else
        {
            v9 = v8->ps;
            if ((v8->ps->pm_flags & 0x10) == 0
                || v9->velocity.v.m128_f32[2] <= 0.0f)
                return;
        }
    }
    float v13 = v9->origin.v.m128_f32[1];
    float v14 = v9->origin.v.m128_f32[0];
    unsigned int tracemask = v8->tracemask;
    float v16 = v9->velocity.v.m128_f32[2];
    down_v.v.m128_f32[0] = v14;
    down_v.v.m128_f32[1] = v13;
    down_v.v.m128_f32[2] = v9->origin.v.m128_f32[2];
    down_v.v.m128_f32[3] = v9->origin.v.m128_f32[3];
    math::Position3 down;
    down.v.m128_f32[0] = v9->velocity.v.m128_f32[0];
    down.v.m128_f32[1] = v9->velocity.v.m128_f32[1];
    down.v.m128_f32[2] = v16;
    down.v.m128_f32[3] = v9->origin.v.m128_f32[3];
    float start_vx = v14 - down_o.v.m128_f32[0];
    float start_vy = v13 - down_o.v.m128_f32[1];
    collision_context_t context;
    context.__vftable = (collision_context_t_vtbl*)0x00CD8F78;
    context.pass_entity1 = pm->ps->mClient;
    context.pass_entity2.mHandle.mVal = 0;
    context.contentmask = tracemask;
    context.pass_owner1.mHandle.mVal = 0;
    context.pass_owner2.mHandle.mVal = 0;
    trace_t trace;
    math::Position3 up;
    math::Position3 v52;
    math::Position3 start_v;
    if (v7 != 0)
    {
        up.v.m128_f32[0] = down_o.v.m128_f32[0];
        up.v.m128_f32[1] = down_o.v.m128_f32[1];
        up.v.m128_f32[2] = (v11 + start_o.v.m128_f32[3]) + 1.0f;
        PM_trace(&trace, down_v, pm->mins, pm->maxs, up, context);
        float v18 = ((start_o.v.m128_f32[3] + 1.0f)
                     * trace.normal.v.m128_f32[1]) - 1.0f;
        start_o.v.m128_f32[1] = v18;
        if (v18 >= 1.0f)
        {
            pm->ps->origin.v = up.v;
            pm->ps->origin.v.m128_f32[2] = down_o.v.m128_f32[2] + v18;
            pm->ps->velocity.v = down.v;
            PM_SlideMove(gravity);
            v8 = pm;
        }
        else
        {
            v8 = pm;
            if (pm->debugLevel != 0)
                Com_Printf("%i:not enough step room\n", c_pmove);
            start_o.v.m128_f32[1] = 0.0f;
        }
    }
    if (start_o.v.m128_f32[2] != 0.0f || start_o.v.m128_f32[1] != 0.0f)
    {
        v52.v = v8->ps->origin.v;
        float flatDelta[2];
        flatDelta[0] = v8->ps->origin.v.m128_f32[1];
        float v21 = v8->ps->origin.v.m128_f32[2]
            - start_o.v.m128_f32[1];
        flatDelta[1] = v21;
        if (start_o.v.m128_f32[2] != 0.0f)
            flatDelta[1] = v21 - 9.0f;
        math::Position3 end;
        end.v.m128_f32[0] = v52.v.m128_f32[0];
        end.v.m128_f32[1] = flatDelta[0];
        end.v.m128_f32[2] = flatDelta[1];
        PM_trace(&trace, v8->ps->origin, pm->mins, pm->maxs, end, context);
        if (trace.mEntity.mHandle.mVal != 0)
        {
            Entity* mObject = (Entity*)EntityHandleDb_GetObject(
                trace.mEntity.mHandle.mVal);
            if (mObject != nullptr && mObject->client != nullptr)
            {
                pm->ps->origin.v = down_v.v;
                pm->ps->velocity.v = down.v;
                return;
            }
        }
        if (trace.normal.v.m128_f32[1] >= 1.0f)
        {
            if (start_o.v.m128_f32[1] != 0.0f)
                pm->ps->origin.v.m128_f32[2] -= start_o.v.m128_f32[1];
        }
        else
        {
            pm->ps->origin.v.m128_f32[0] = trace.endpos.v.m128_f32[0];
            pm->ps->origin.v.m128_f32[1] = trace.endpos.v.m128_f32[1];
            pm->ps->origin.v.m128_f32[2] = trace.endpos.v.m128_f32[2];
            PlayerState* v24 = pm->ps;
            float v25 = trace.endpos.v.m128_f32[1]
                    * pm->ps->velocity.v.m128_f32[0]
                + trace.endpos.v.m128_f32[2]
                    * pm->ps->velocity.v.m128_f32[1]
                + trace.endpos.v.m128_f32[3]
                    * pm->ps->velocity.v.m128_f32[2];
            float v26 = v25 >= 0.0f ? v25 * 0.99900097f : v25 * 1.001f;
            v24->velocity.v.m128_f32[0] -=
                trace.endpos.v.m128_f32[1] * v26;
            v24->velocity.v.m128_f32[1] -=
                trace.endpos.v.m128_f32[2] * v26;
            v24->velocity.v.m128_f32[2] -=
                trace.endpos.v.m128_f32[3] * v26;
        }
        v8 = pm;
    }
    float v28 = fStepAmount;
    math::Position3* p_origin = &v8->ps->origin;
    if ((v8->ps->velocity.v.m128_f32[0] * start_vx
         + v8->ps->velocity.v.m128_f32[1] * start_vy
         + 0.0049999999f)
        >= (v8->ps->velocity.v.m128_f32[1]
                * (v8->ps->origin.v.m128_f32[1] - down_o.v.m128_f32[1])
            + v8->ps->velocity.v.m128_f32[0]
                * (v8->ps->origin.v.m128_f32[0] - down_o.v.m128_f32[0])))
        goto L58;
    if (fStepAmount == 0.0f)
        goto L75;
    if (p_origin->v.m128_f32[2] >= (pm->ps->fJumpOriginZ + 39.0f))
    {
L58:
        p_origin->v = down_v.v;
        pm->ps->velocity.v = down.v;
        v8 = pm;
        if (pm->debugLevel > 1)
        {
            if (v28 == 0.0f)
                Com_Printf("%i:didn't use step results\n", c_pmove);
            else
                Com_Printf(
                    "%i:didn't use jump step results because it went too high\n",
                    c_pmove);
            v8 = pm;
        }
        if (start_o.v.m128_f32[2] != 0.0f)
        {
            math::Position3 end;
            end.v = v8->ps->origin.v;
            end.v.m128_f32[2] -= 9.0f;
            PM_trace(&trace, v8->ps->origin, pm->mins, pm->maxs, end,
                     context);
            if (trace.normal.v.m128_f32[1] >= 1.0f)
                goto L66;
            pm->ps->origin.v.m128_f32[0] = trace.endpos.v.m128_f32[0];
            pm->ps->origin.v.m128_f32[1] = trace.endpos.v.m128_f32[1];
            pm->ps->origin.v.m128_f32[2] = trace.endpos.v.m128_f32[2];
            PlayerState* v29 = pm->ps;
            float v30 = trace.endpos.v.m128_f32[1]
                    * pm->ps->velocity.v.m128_f32[0]
                + trace.endpos.v.m128_f32[2]
                    * pm->ps->velocity.v.m128_f32[1]
                + trace.endpos.v.m128_f32[3]
                    * pm->ps->velocity.v.m128_f32[2];
            float v31 = v30 >= 0.0f ? v30 * 0.99900097f : v30 * 1.001f;
            v29->velocity.v.m128_f32[0] -=
                trace.endpos.v.m128_f32[1] * v31;
            v29->velocity.v.m128_f32[1] -=
                trace.endpos.v.m128_f32[2] * v31;
            v29->velocity.v.m128_f32[2] -=
                trace.endpos.v.m128_f32[3] * v31;
            v8 = pm;
            if (pm->debugLevel > 1)
                Com_Printf(
                    "%i:did down step after not using step results\n",
                    c_pmove);
        }
    }
L66:
    if (v28 != 0.0f)
    {
        if ((v8->ps->origin.v.m128_f32[2] - down_v.v.m128_f32[2]) > 0.0f)
        {
            fStepAmount =
                (pm->ps->fJumpOriginZ + 39.0f)
                - v8->ps->origin.v.m128_f32[2];
            if (fStepAmount < 0.1f)
            {
                v8->ps->velocity.v.m128_f32[2] = 0.0f;
                goto L75;
            }
            float v33 = sqrtf((fStepAmount + fStepAmount)
                              * pm->ps->gravity);
            fStepAmount = v33;
            if (v8->ps->velocity.v.m128_f32[2] > v33)
            {
                if (v8->debugLevel != 0)
                    Com_Printf("%i:adjusted jump vel: %.1f -> %.1f\n",
                               c_pmove, v8->ps->velocity.v.m128_f32[2],
                               fStepAmount);
                v8->ps->velocity.v.m128_f32[2] = fStepAmount;
                goto L75;
            }
        }
    }
L75:
    if (start_o.v.m128_f32[2] != 0.0f
        && v8->ps->pm_type < 6
        && PM_VerifyPronePosition(down_v, down) != 0)
    {
        float v34 = pm->ps->origin.v.m128_f32[2] - down_v.v.m128_f32[2];
        fStepAmount = v34;
        if (fabs(v34) > 0.5f)
        {
            int v35 = (int)(fStepAmount + 0.5f);
            if (v35 != 0)
            {
                if (pm->debugLevel != 0)
                {
                    if (v28 == 0.0f)
                        Com_Printf("%i:stepped %2i\n", c_pmove, v35);
                    else
                        Com_Printf("%i:jump step %2i\n", c_pmove, v35);
                }
                if (v35 < -16)
                    v35 = -16;
                if (v35 > 24)
                    v35 = 24;
                int v36 = v35 + 128;
                BG_AddPredictableEventToPlayerstate(168, v36, pm->ps);
                fStepAmount = fabsf(
                    pm->ps->origin.v.m128_f32[2] - down_o.v.m128_f32[2]);
                float v38 = ((1.0f - (fStepAmount / start_o.v.m128_f32[3]))
                             * 0.80000001f) + 0.19999999f;
                pm->ps->velocity.v.m128_f32[0] *= v38;
                pm->ps->velocity.v.m128_f32[1] *= v38;
                pm->ps->velocity.v.m128_f32[2] *= v38;
                int v39 = abs(v36 - 128);
                if (v39 > 3)
                {
                    if (pm->ps->mGroundEntity.mHandle.mVal != 0
                        && PM_ShouldMakeFootsteps() != 0)
                    {
                        int v41 = v39 / 2;
                        if (v41 > 4)
                            v41 = 4;
                        char oldBob = (char)pm->ps->bobCycle;
                        pm->ps->bobCycle =
                            (int)(((float)v41 * 1.25f) + 7.0f)
                            + pm->ps->bobCycle;
                        PM_FootstepEvent(oldBob, (char)pm->ps->bobCycle,
                                         1);
                    }
                }
            }
        }
    }
}

// ============================================================================
// tunnel_test - ea: 0x643690 (bg_pmove.cpp)
// ============================================================================
extern void query_proximity_data(const math::Position3& lo,
                                 const math::Position3& hi,
                                 proximity_data_t& out);  // game.o
extern void filter_proximity_data(const math::Position3& lo,
                                  const math::Position3& hi, int contents,
                                  const proximity_data_t& in,
                                  proximity_data_t& out);  // game.o
extern void TracePoint(const proximity_data_t& data, trace_t* results,
                       const math::Position3& start,
                       const math::Position3& end,
                       int brushmask);  // game.o

// ea: 0x00643690
bool tunnel_test(pmove_t& pm, float radius, const math::Position3& p0,
                 const math::Position3& p1)
{
    Entity* mObject = (Entity*)EntityHandleDb_GetObject(
        pm.ps->mClient.mHandle.mVal);
    math::Position3 lo;
    math::Position3 hi;
    lo.v = _mm_sub_ps(
        _mm_min_ps(p0.v, p1.v), _mm_set1_ps(radius));
    hi.v = _mm_add_ps(
        _mm_max_ps(p0.v, p1.v), _mm_set1_ps(radius));
    lo.v.m128_f32[2] -= 100.0f;
    hi.v.m128_f32[2] += 100.0f;
    if ((_mm_movemask_ps(_mm_cmplt_ps(
             _mm_max_ps(_mm_sub_ps(mObject->proximity_data->lo.v, lo.v),
                        _mm_sub_ps(hi.v, mObject->proximity_data->hi.v)),
             _mm_setzero_ps()))
         & 7) != 7)
        query_proximity_data(lo, hi, *mObject->proximity_data);
    proximity_data_t filtered;
    filter_proximity_data(lo, hi, pm.tracemask & 0xFDFFFFFF,
                          *mObject->proximity_data, filtered);
    trace_t results;
    memset(&results, 0, sizeof(results));
    TracePoint(filtered, &results, p0, p1, pm.tracemask & 0xFDFFFFFF);
    return results.endpos.v.m128_f32[0] < 1.0f;
}

// ============================================================================
// BG_Bullet_Endpos - ea: 0x606680 (bg_weapons.cpp)
// ============================================================================
extern void gunrandom(float* x, float* y);  // core.o

// ea: 0x00606680
void BG_Bullet_Endpos(float spread, float* end, weaponParms* wp)
{
    if ((__fpclass(spread) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 338;
        AeAssert::gCurrentExpr = "!IS_NAN(spread)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    float fAimOffset =
        (float)(tan(spread * 3.1415927f * 0.0055555557f) * 10000.0);
    if ((__fpclass(fAimOffset) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 342;
        AeAssert::gCurrentExpr = "!IS_NAN(fAimOffset)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    float r, u;
    gunrandom(&r, &u);
    u *= fAimOffset;
    r *= fAimOffset;
    if ((__fpclass(r) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 348;
        AeAssert::gCurrentExpr = "!IS_NAN(r)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    if ((__fpclass(u) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 349;
        AeAssert::gCurrentExpr = "!IS_NAN(u)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    if ((__fpclass(wp->muzzleTrace[0]) & 0x297) != 0
        || (__fpclass(wp->muzzleTrace[1]) & 0x297) != 0
        || (__fpclass(wp->muzzleTrace[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 350;
        AeAssert::gCurrentExpr =
            "!IS_NAN((wp->muzzleTrace)[0]) && !IS_NAN((wp->muzzleTrace)[1]) && !IS_NAN((wp->muzzleTrace)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((__fpclass(wp->forward[0]) & 0x297) != 0
        || (__fpclass(wp->forward[1]) & 0x297) != 0
        || (__fpclass(wp->forward[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 351;
        AeAssert::gCurrentExpr =
            "!IS_NAN((wp->forward)[0]) && !IS_NAN((wp->forward)[1]) && !IS_NAN((wp->forward)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((__fpclass(wp->right[0]) & 0x297) != 0
        || (__fpclass(wp->right[1]) & 0x297) != 0
        || (__fpclass(wp->right[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 352;
        AeAssert::gCurrentExpr =
            "!IS_NAN((wp->right)[0]) && !IS_NAN((wp->right)[1]) && !IS_NAN((wp->right)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((__fpclass(wp->up[0]) & 0x297) != 0
        || (__fpclass(wp->up[1]) & 0x297) != 0
        || (__fpclass(wp->up[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 353;
        AeAssert::gCurrentExpr =
            "!IS_NAN((wp->up)[0]) && !IS_NAN((wp->up)[1]) && !IS_NAN((wp->up)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    end[0] = (wp->forward[0] * 10000.0f) + wp->muzzleTrace[0];
    end[1] = (wp->forward[1] * 10000.0f) + wp->muzzleTrace[1];
    end[2] = (wp->forward[2] * 10000.0f) + wp->muzzleTrace[2];
    if ((__fpclass(end[0]) & 0x297) != 0
        || (__fpclass(end[1]) & 0x297) != 0
        || (__fpclass(end[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 357;
        AeAssert::gCurrentExpr =
            "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    end[0] += wp->right[0] * r;
    end[1] += wp->right[1] * r;
    end[2] += wp->right[2] * r;
    end[0] += wp->up[0] * u;
    end[1] += wp->up[1] * u;
    end[2] += wp->up[2] * u;
    if ((__fpclass(end[0]) & 0x297) != 0
        || (__fpclass(end[1]) & 0x297) != 0
        || (__fpclass(end[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 362;
        AeAssert::gCurrentExpr =
            "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
}

// ============================================================================
// PM_ClipVelocity - ea: 0x604C00
// ============================================================================
void PM_ClipVelocity(const math::Dir3* in, const math::Dir3* normal,
                     math::Dir3* out, float overbounce)
{
    float v4 = (in->v.m128_f32[1] * normal->v.m128_f32[1])
        + (normal->v.m128_f32[2] * in->v.m128_f32[2])
        + (in->v.m128_f32[0] * normal->v.m128_f32[0]);
    float v5 = v4 >= 0.0f ? v4 / overbounce : v4 * overbounce;
    out->v.m128_f32[0] = in->v.m128_f32[0] - (normal->v.m128_f32[0] * v5);
    out->v.m128_f32[1] = in->v.m128_f32[1] - (normal->v.m128_f32[1] * v5);
    out->v.m128_f32[2] = in->v.m128_f32[2] - (normal->v.m128_f32[2] * v5);
}

// ============================================================================
// PM_GetEffectiveStance - ea: 0x604C90
// ============================================================================
int PM_GetEffectiveStance(PlayerState* ps)
{
    int viewHeightTarget = ps->viewHeightTarget;
    if (viewHeightTarget == ps->crouchViewHeight)
        return 2;
    return viewHeightTarget == ps->proneViewHeight;
}

// ============================================================================
// PM_GetViewHeightLerpTime - ea: 0x606140
// ============================================================================
int PM_GetViewHeightLerpTime(const PlayerState* ps, int iTarget, int bDown)
{
    if (iTarget == ps->proneViewHeight)
        return 400;
    if (iTarget == ps->crouchViewHeight)
        return bDown != 0 ? 200 : 400;
    return 200;
}

// ============================================================================
// PM_ShouldMakeFootsteps - ea: 0x606250
// ============================================================================
int PM_ShouldMakeFootsteps()
{
    return 1;
}

// ============================================================================
// PM_PlayFatigueSound - ea: 0x6063E0
// ============================================================================
PlayerState* PM_PlayFatigueSound()
{
    PlayerState* result = pm->ps;
    if ((0x20000 & pm->ps->pm_flags) != 0)
    {
        result = (PlayerState*)(result->iFatigueSoundTime + 1700);
        if ((int)result < pm->cmd.serverTime)
        {
            PM_AddEvent(163);
            result = pm->ps;
            pm->ps->iFatigueSoundTime = pm->cmd.serverTime;
        }
    }
    else
    {
        int iFatigueSoundTime = result->iFatigueSoundTime;
        if (iFatigueSoundTime > 0 && iFatigueSoundTime + 1700 < pm->cmd.serverTime)
            result->iFatigueSoundTime = 0;
    }
    return result;
}

// ============================================================================
// PM_UpdateFatigue - ea: 0x606440
// ============================================================================
PlayerState* PM_UpdateFatigue()
{
    PlayerState* result = pm->ps;
    if ((0x10000 & pm->ps->pm_flags) == 0)
    {
        if ((pm->cmd.buttons & 4) == 0)
        {
            if (pm->cmd.serverTime < result->lastSprintTime + 1000)
                return result;
            result->fatigueScale =
                (pml.msec * 0.001f * 0.16666667f) + result->fatigueScale;
            if (pm->ps->fatigueScale >= 1.0f)
            {
                pm->ps->fatigueScale = 1.0f;
                int pm_flags = pm->ps->pm_flags;
                if ((0x20000 & pm_flags) != 0)
                    pm->ps->pm_flags = pm_flags & 0xFFFDFFFF;
            }
        }
        return PM_PlayFatigueSound();
    }
    if (result->pm_type != 2 && bg_nofatigue.integer == 0)
    {
        result->lastSprintTime = pm->cmd.serverTime;
        pm->ps->fatigueScale =
            pm->ps->fatigueScale - (pml.msec * 0.001f * 0.33333334f);
        if (pm->ps->fatigueScale < 0.5f)
        {
            PM_AddEvent(163);
            pm->ps->pm_flags |= 0x20000;
        }
        result = pm->ps;
        if (pm->ps->fatigueScale < 0.0f)
            result->fatigueScale = 0.0f;
    }
    return result;
}

// ============================================================================
// PM_SetProneMovementOverride - ea: 0x606590
// ============================================================================
PlayerState* PM_SetProneMovementOverride()
{
    PlayerState* result = pm->ps;
    if ((pm->ps->pm_flags & 1) != 0)
        result->pm_flags |= 0x400u;
    return result;
}

// ============================================================================
// PM_UpdatePlayerWalkingFlag - ea: 0x6065B0
// ============================================================================
pmove_t* PM_UpdatePlayerWalkingFlag()
{
    pm->ps->pm_flags &= ~0x80u;
    pmove_t* result = pm;
    PlayerState* ps = pm->ps;
    if (pm->ps->pm_type < 6 && (pm->cmd.buttons & 8) != 0)
    {
        result = (pmove_t*)ps->pm_flags;
        if (((unsigned int)result & 1) == 0 && ((unsigned int)result & 0x20) != 0
            && (0x100000 & ps->eFlags) == 0
            && (0x10000 & (unsigned int)result) == 0)
        {
            result = (pmove_t*)ps->weaponstate;
            if ((unsigned int)result != 5 && (unsigned int)result != 7
                && (unsigned int)result != 9 && (unsigned int)result != 8
                && (unsigned int)result != 6)
                ps->pm_flags |= 0x80u;
        }
    }
    return result;
}

// ============================================================================
// PM_ClearAimDownSightFlag - ea: 0x607A00
// ============================================================================
PlayerState* PM_ClearAimDownSightFlag()
{
    PlayerState* result = pm->ps;
    pm->ps->pm_flags &= ~0x20u;
    return result;
}

// ============================================================================
// BG_EvaluateTrajectoryDelta - ea: 0x604100
// ============================================================================
void BG_EvaluateTrajectoryDelta(const trajectory_t* tr, int atTime,
                                float* result)
{
    float value = g_gravity.value;
    if (tr->trGravityOverride != 0.0f)
        value = tr->trGravityOverride;
    switch (tr->trType)
    {
    case TR_STATIONARY:
    case TR_INTERPOLATE:
    zero:
        result[0] = 0.0f;
        result[1] = 0.0f;
        result[2] = 0.0f;
        return;
    case TR_LINEAR:
        result[0] = tr->trDelta[0];
        result[1] = tr->trDelta[1];
        result[2] = tr->trDelta[2];
        return;
    case TR_LINEAR_STOP:
        if (atTime > tr->trTime + tr->trDuration)
            goto zero;
        result[0] = tr->trDelta[0];
        result[1] = tr->trDelta[1];
        result[2] = tr->trDelta[2];
        return;
    case TR_SINE:
    {
        float v5 = cosf((atTime - tr->trTime) / tr->trDuration * 6.2831855f)
            * 0.5f;
        result[0] = v5 * tr->trDelta[0];
        result[1] = v5 * tr->trDelta[1];
        result[2] = v5 * tr->trDelta[2];
        return;
    }
    case TR_GRAVITY:
    {
        float v6 = (float)(atTime - tr->trTime);
        result[0] = tr->trDelta[0];
        result[1] = tr->trDelta[1];
        result[2] = tr->trDelta[2] - ((v6 * 0.001f) * value);
        return;
    }
    case TR_GRAVITY_LOW:
    {
        float v7 = (float)(atTime - tr->trTime);
        result[0] = tr->trDelta[0];
        result[1] = tr->trDelta[1];
        result[2] = tr->trDelta[2] - ((value * 0.30000001f) * (v7 * 0.001f));
        return;
    }
    case TR_GRAVITY_FLOAT:
    {
        float v8 = (float)(atTime - tr->trTime);
        result[0] = tr->trDelta[0];
        result[1] = tr->trDelta[1];
        result[2] = tr->trDelta[2] - ((value * 0.2f) * (v8 * 0.001f));
        return;
    }
    case TR_ACCELERATE:
    {
        int trTime = tr->trTime;
        if (atTime > trTime + tr->trDuration)
            goto zero;
        float v10 = ((atTime - trTime) * 0.001f) * ((atTime - trTime) * 0.001f);
        result[0] = tr->trDelta[0] * v10;
        result[1] = tr->trDelta[1] * v10;
        result[2] = tr->trDelta[2] * v10;
        return;
    }
    case TR_DECCELERATE:
    {
        int v11 = tr->trTime;
        if (atTime > v11 + tr->trDuration)
            goto zero;
        float v10 = (atTime - v11) * 0.001f;
        result[0] = tr->trDelta[0] * v10;
        result[1] = tr->trDelta[1] * v10;
        result[2] = tr->trDelta[2] * v10;
        return;
    }
    default:
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_misc.cpp";
        AeAssert::gCurrentLine = 767;
        AeAssert::gCurrentExpr = "bad trType";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        return;
    }
}

// ============================================================================
// BG_GetMarkDir - ea: 0x604380
// ============================================================================
void BG_GetMarkDir(const float* dir, const float* normal, float* out)
{
    float lnormal[3];
    float minDot = 0.30000001f;
    if (sqrtf(normal[0] * normal[0] + normal[1] * normal[1]
              + normal[2] * normal[2]) >= 1.0f)
    {
        lnormal[0] = normal[0];
        lnormal[1] = normal[1];
        lnormal[2] = normal[2];
    }
    else
    {
        lnormal[0] = 0.0f;
        lnormal[1] = 0.0f;
        lnormal[2] = 1.0f;
    }
    float ndir[3] = { -dir[0], -dir[1], -dir[2] };
    VectorNormalize(ndir);
    if (normal[2] > 0.80000001f)
        minDot = 0.69999999f;
    float v7 = ndir[2];
    float v8 = ndir[1];
    float v9 = ndir[0];
    if (minDot > ((ndir[2] * lnormal[2]) + (ndir[1] * lnormal[1])
                  + (ndir[0] * lnormal[0])))
    {
        float i = lnormal[0] * 0.5f;
        float v10 = i;
        for (;;)
        {
            ndir[0] = v10 + v9;
            ndir[1] = (lnormal[1] * 0.5f) + v8;
            ndir[2] = (lnormal[2] * 0.5f) + v7;
            VectorNormalize(ndir);
            v7 = ndir[2];
            v8 = ndir[1];
            v9 = ndir[0];
            if (minDot <= ((ndir[2] * lnormal[2]) + (ndir[1] * lnormal[1])
                           + (ndir[0] * lnormal[0])))
                break;
            v10 = i;
        }
    }
    out[0] = v9;
    out[1] = v8;
    out[2] = v7;
}

// ============================================================================
// BG_AddPredictableEventToPlayerstate - ea: 0x604540
// ============================================================================
void BG_AddPredictableEventToPlayerstate(int newEvent, int eventParm,
                                         PlayerState* ps)
{
    if (newEvent == 0)
        return;
    if (newEvent >= 256)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_misc.cpp";
        AeAssert::gCurrentLine = 823;
        AeAssert::gCurrentExpr = "newEvent < 256";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (eventParm >= 256)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_misc.cpp";
        AeAssert::gCurrentLine = 824;
        AeAssert::gCurrentExpr = "eventParm < 256";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    char buf[256];
    Cvar_VariableStringBuffer("showevents", buf, 256);
    if (atof(buf) != 0.0)
    {
        Com_Printf("Cgame event svt %5d -> %5d: num = %20s parm %d\n",
                   ps->commandTime, ps->event.eventSequence,
                   pEventNamesList[newEvent], eventParm);
    }
    ps->event.events[ps->event.eventSequence & 3] = newEvent;
    ps->event.eventParms[ps->event.eventSequence++ & 3] = eventParm;
}

// ============================================================================
// BG_PlayerStateToEntityState - ea: 0x604680
// ============================================================================
void BG_PlayerStateToEntityState(PlayerState* ps, EntityState* s)
{
    s->eType = 1;
    s->pos.trType = TR_INTERPOLATE;
    memcpy(s->pos.trBase, ps, sizeof(s->pos.trBase));
    s->apos.trType = TR_INTERPOLATE;
    s->apos.trBase[0] = ps->viewangles[0];
    s->apos.trBase[1] = ps->viewangles[1];
    s->apos.trBase[2] = ps->viewangles[2];
    int movementDir = ps->movementDir;
    float v3 = (float)movementDir;
    if (movementDir > 128)
        v3 = v3 - 256.0f;
    s->angles2.v.m128_f32[1] = v3;
    s->eFlags = ps->eFlags;
    int eventSequence = ps->event.eventSequence;
    if (ps->event.entityEventSequence - eventSequence >= 0)
    {
        s->eventParm = 0;
    }
    else
    {
        if (eventSequence - ps->event.entityEventSequence > 4)
            ps->event.entityEventSequence = eventSequence - 4;
        int v5 = ps->event.entityEventSequence & 3;
        if (ps->event.eventParms[v5] >= 0x100u)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_misc.cpp";
            AeAssert::gCurrentLine = 969;
            AeAssert::gCurrentExpr =
                "(ps->event.eventParms[seq] >= 0) && (ps->event.eventParms[seq] < 256)";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        s->eventParm = ps->event.eventParms[v5];
        ++ps->event.entityEventSequence;
    }
    for (int i = ps->event.oldEventSequence; i != ps->event.eventSequence; ++i)
    {
        int v7 = i & 3;
        s->events[s->eventSequence & 3] = ps->event.events[v7];
        s->eventParms[s->eventSequence++ & 3] = ps->event.eventParms[v7];
    }
    unsigned char weapon = ps->weapon;
    ps->event.oldEventSequence = ps->event.eventSequence;
    s->weapon = weapon;
    s->mGroundEntity.mHandle.mVal = ps->mGroundEntity.mHandle.mVal;
}

// ============================================================================
// BG_PlayerStateToEntityStateExtrapolate - ea: 0x604860
// ============================================================================
void BG_PlayerStateToEntityStateExtrapolate(PlayerState* ps, EntityState* s,
                                            int time)
{
    s->pos.trType = TR_LINEAR_STOP;
    memcpy(s->pos.trBase, ps, sizeof(s->pos.trBase));
    memcpy(s->pos.trDelta, &ps->velocity, sizeof(s->pos.trDelta));
    s->pos.trTime = time;
    s->pos.trDuration = 50;
    s->apos.trType = TR_INTERPOLATE;
    s->apos.trBase[0] = ps->viewangles[0];
    s->apos.trBase[1] = ps->viewangles[1];
    s->apos.trBase[2] = ps->viewangles[2];
    s->angles2.v.m128_f32[1] = (float)ps->movementDir;
    s->eFlags = ps->eFlags;
    int eventSequence = ps->event.eventSequence;
    if (ps->event.entityEventSequence - eventSequence >= 0)
    {
        s->eventParm = 0;
    }
    else
    {
        if (eventSequence - ps->event.entityEventSequence > 4)
            ps->event.entityEventSequence = eventSequence - 4;
        int v4 = ps->event.entityEventSequence & 3;
        if (ps->event.eventParms[v4] >= 0x100u)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_misc.cpp";
            AeAssert::gCurrentLine = 1052;
            AeAssert::gCurrentExpr =
                "(ps->event.eventParms[seq] >= 0) && (ps->event.eventParms[seq] < 256)";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        s->eventParm = ps->event.eventParms[v4];
        ++ps->event.entityEventSequence;
    }
    int v5 = ps->event.eventSequence;
    if (ps->event.oldEventSequence - v5 > 0)
        ps->event.oldEventSequence = v5;
    int oldEventSequence = ps->event.oldEventSequence;
    if (oldEventSequence != v5)
    {
        do
        {
            int v7 = oldEventSequence & 3;
            s->events[s->eventSequence & 3] = ps->event.events[v7];
            s->eventParms[s->eventSequence++ & 3] = ps->event.eventParms[v7];
            ++oldEventSequence;
        } while (oldEventSequence != ps->event.eventSequence);
    }
    unsigned char weapon = ps->weapon;
    ps->event.oldEventSequence = ps->event.eventSequence;
    s->weapon = weapon;
    s->mGroundEntity.mHandle.mVal = ps->mGroundEntity.mHandle.mVal;
    s->eType = 1;
}

// ============================================================================
// BG_AllowPlayerWeaponAtVehiclePos - ea: 0x604A60
// ============================================================================
bool BG_AllowPlayerWeaponAtVehiclePos(int vehType, int vehPos)
{
    return vehType == 1 && (vehPos == 2 || vehPos == 10);
}

// ============================================================================
// BG_GetWeaponTypeName - ea: 0x606620
// ============================================================================
const char* BG_GetWeaponTypeName(int type)
{
    if (type >= 9)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 293;
        AeAssert::gCurrentExpr = "((unsigned) type) < WEAPTYPE_NUM";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return s_szWeapTypeNames[type];
}

// ============================================================================
// BG_FreeWeaponInfo - ea: 0x606F90
// ============================================================================
void BG_FreeWeaponInfo()
{
    weaponFileInfo_t** v0 = bg_weaponInfo;
    for (int i = 0; i < 92; ++i)
    {
        if (v0[i] != nullptr)
        {
            mem_heap_free(v0[i]);
            v0 = bg_weaponInfo;
            bg_weaponInfo[i] = nullptr;
        }
    }
}

// ============================================================================
// Weapon/ammo info helpers (bg_weapons.cpp)
// ============================================================================
extern int bg_iNumWeapons;          // ?bg_iNumWeapons@@3HA (game.o)
extern int bg_iNumAmmoTypes;        // ?bg_iNumAmmoTypes@@3HA (game.o)
extern int bg_iNumWeapClips;        // ?bg_iNumWeapClips@@3HA (game.o)
extern int bg_iNumSharedAmmoCaps;   // ?bg_iNumSharedAmmoCaps@@3HA (game.o)
extern int* bg_iWeapAmmoMaxs;       // ?bg_iWeapAmmoMaxs@@3PAHA (game.o)
extern int* bg_iWeapClipSizes;      // ?bg_iWeapClipSizes@@3PAHA (game.o)
extern int* bg_iSharedAmmoCaps;     // ?bg_iSharedAmmoCaps@@3PAHA (game.o)
extern const char** bg_szWeapAmmoNames;  // ?bg_szWeapAmmoNames@@3PAPBDA (game.o)
extern const char** bg_szWeapClipNames;  // ?bg_szWeapClipNames@@3PAPBDA (game.o)
extern bool gInfinteAmmo;           // ?gInfinteAmmo@@3_NA (game.o)
extern int cg_aWeaponSelect[4];     // ?cg_aWeaponSelect@@3PAHA (cg.o)
extern int cg_aWeaponSelectTime[4]; // ?cg_aWeaponSelectTime@@3PAHA (cg.o)
extern int cl_aADS[4];              // ?cl_aADS@@3PAHA (cl.o)
extern int cgGlobal_time;           // cgGlobal.time (cg.o)
extern void EffectEventSys_StopEffect(void* sInst, unsigned int handle,
                                      bool kill);  // ?StopEffect@EffectEventSys@@QAEXVHandle@@_N@Z
extern void* EffectEventSys_sInst;  // ?sInst@EffectEventSys@@2PAV1@A

// game.o static weapon-slot names (recovered from .rdata, szWeapSlotNames)
static const char* const s_szWeapSlotNames[10] = {
    "none", "primary", "primaryb", "pistol", "grenade",
    "smokegrenade", "interact", "binocular", "flag", "special",
};

// ============================================================================
// BG_GetInfoForWeapon - ea: 0x606FD0
// ============================================================================
weaponFileInfo_t* BG_GetInfoForWeapon(int iWeapon)
{
    if (iWeapon < 0 || iWeapon > bg_iNumWeapons)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 1468;
        AeAssert::gCurrentExpr =
            "(iWeapon >= 0) && (iWeapon <= bg_iNumWeapons)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return bg_weaponInfo[iWeapon];
}

// ea: 0x0062F1B0
weaponFileInfo_t* BG_GetPlayerWeaponInfo()
{
    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    if ((player->client->ps.eFlags & 0x6000) != 0)
    {
        unsigned int mVal = player->client->ps.mViewLockedEntity.mHandle.mVal;
        unsigned int v2 = mVal & 0xFFF;
        if (v2 < 0x540
            && mVal >> 12
                == (unsigned int)EntityHandleDb::sInst.mElements[v2].mKey)
        {
            Entity* mObject = EntityHandleDb::sInst.mElements[v2].mObject;
            if (mObject != nullptr)
                return BG_GetInfoForWeapon(mObject->s.weapon);
        }
    }
    else if ((0x100000 & player->client->ps.eFlags) != 0)
    {
        Entity* v6 = EntityManager::sInst->GetPlayer(currCl);
        if (v6 != nullptr)
        {
            Entity* mObject = HandleDbToEnt(v6->r.mOwner);
            if (mObject != nullptr)
            {
                int vehPos = v6->client->ps.vehPos;
                if (vehPos == 0)
                    return BG_GetInfoForWeapon(mObject->s.weapon);
                if (vehPos == 1)
                {
                    scr_vehicle_t* scr_vehicle = mObject->scr_vehicle;
                    if (scr_vehicle != nullptr)
                    {
                        int gunnerWeapon = scr_vehicle->gunnerWeapon;
                        if (gunnerWeapon > 0)
                            return BG_GetInfoForWeapon(gunnerWeapon);
                    }
                }
            }
        }
    }
    else
    {
        Entity* v10 = EntityManager::sInst->GetPlayer(currCl);
        return BG_GetInfoForWeapon(v10->client->ps.weapon);
    }
    return nullptr;
}

// ea: 0x00621F60
int BG_TakePlayerWeapon(PlayerState* pPS, int iWeaponIndex)
{
    int v3 = 1 << (iWeaponIndex & 0x1F);
    int v11 = iWeaponIndex >> 5;
    if ((v3 & pPS->weapons[v11]) == 0)
        return 0;
    weaponFileInfo_t* pWeap = BG_GetInfoForWeapon(iWeaponIndex);
    weapSlot_t v5 = BG_IsPlayerWeaponInSlot(pPS, iWeaponIndex, 1);
    weapSlot_t slot = v5;
    if (v5 != WEAPSLOT_NONE)
    {
        if (pWeap->bSlotStackable != 0)
        {
            int v6 = 1;
            if (bg_iNumWeapons >= 1)
            {
                while (1)
                {
                    weaponFileInfo_t* InfoForWeapon =
                        BG_GetInfoForWeapon(v6);
                    if (InfoForWeapon->bSlotStackable != 0
                        && InfoForWeapon->slot == pWeap->slot
                        && ((1 << (v6 & 0x1F)) & pPS->weapons[v6 >> 5]) != 0
                        && BG_IsPlayerWeaponInSlot(pPS, v6, 1)
                            == WEAPSLOT_NONE)
                        break;
                    if (++v6 > bg_iNumWeapons)
                        goto LABEL_13;
                }
                pPS->weaponslots[slot] = (char)v6;
                if (v6 <= bg_iNumWeapons)
                    goto LABEL_15;
            LABEL_13:
                v5 = slot;
            }
        }
        pPS->weaponslots[v5] = 0;
    }
LABEL_15:
    pPS->weapons[v11] = pPS->weapons[v11] & ~v3;
    for (int i = pWeap->iAltWeaponIndex; i != 0;
         i = BG_GetInfoForWeapon(i)->iAltWeaponIndex)
    {
        int v9 = 1 << (i & 0x1F);
        int v10 = pPS->weapons[i >> 5];
        if ((v10 & v9) == 0)
            break;
        pPS->weapons[i >> 5] = ~v9 & v10;
    }
    return 1;
}

// ea: 0x00621820
bool BG_PlayerTouchesItem(PlayerState* ps, EntityState* item, int atTime)
{
    math::Position3 v8;
    BG_EvaluateTrajectory(&item->pos, atTime, v8);
    float v4 = ps->origin.v.m128_f32[0] - v8.v.m128_f32[0];
    bool result = false;
    if (v4 <= 36.0f && v4 >= -36.0f)
    {
        float v5 = ps->origin.v.m128_f32[1] - v8.v.m128_f32[1];
        if (v5 <= 36.0f && v5 >= -36.0f)
        {
            float v6 = ps->origin.v.m128_f32[2] - v8.v.m128_f32[2];
            if (v6 <= 18.0f && v6 >= -88.0f)
                return true;
        }
    }
    return result;
}

// ea: 0x006218C0
int BG_CanItemBeGrabbed(const EntityState* ent, const PlayerState* ps,
                        int bTouched)
{
    unsigned short brushmodel = ent->brushmodel;
    if (brushmodel == 0 || brushmodel >= 0x89)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_misc.cpp";
        AeAssert::gCurrentLine = 466;
        AeAssert::gCurrentExpr = "false";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "BG_CanItemBeGrabbed: index out of range: tell stavro"))
            __debugbreak();
        return 0;
    }
    gitem_s* v4 = &bg_itemlist[brushmodel];
    switch (v4->giType)
    {
    case IT_BAD:
        Com_Error(ERR_DROP, "IT_BAD");
        return 0;
    case IT_WEAPON:
        if (BG_GetInfoForWeapon(v4->giTag)->slot == WEAPSLOT_GRENADE
            && bTouched != 0)
            return BG_GetMaxPickupableAmmo(ps, v4->giTag) > 0;
        if (BG_GetInfoForWeapon(v4->giTag)->slot == WEAPSLOT_SMOKE_GRENADE
            && bTouched != 0)
            return BG_GetMaxPickupableAmmo(ps, v4->giTag) > 0;
        if (BG_GetInfoForWeapon(v4->giTag)->slot == WEAPSLOT_PISTOL)
        {
            const char* AmmoTypeName = BG_GetAmmoTypeName(v4->giTag);
            const char* v8 = BG_GetAmmoTypeName(ps->weaponslots[3]);
            if (_strnicmp(v8, AmmoTypeName, (size_t)strlen(AmmoTypeName))
                == 0)
                return BG_GetMaxPickupableAmmo(ps, ps->weaponslots[3]) > 0;
        }
        {
            int giTag = v4->giTag;
            if (Com_BitCheck(ps->weapons, giTag) != 0)
            {
                if (BG_GetMaxPickupableAmmo(ps, giTag) <= 0)
                    return 0;
            }
            else if (bTouched != 0)
            {
                return 0;
            }
        }
        return 1;
    case IT_AMMO:
        {
            int v10 = v4->giTag;
            if (Com_BitCheck(ps->weapons, v10) != 0)
            {
                if (BG_GetMaxPickupableAmmo(ps, v10) <= 0)
                    return 0;
            }
            else if (BG_WeaponIsClipOnly(v10) == 0
                     || BG_GetMaxPickupableAmmo(ps, v4->giTag) <= 0)
            {
                break;
            }
        }
        return 1;
    case IT_HEALTH:
    case IT_WEAPON_HEALTH:
        return ps->stats[0] < ps->stats[2];
    case IT_WEAPON_AMMO:
        if (gpBrocAPI->mBrocExports.mCallbackCanPickupAmmoPack == nullptr)
            return 0;
        return gpBrocAPI->mBrocExports.mCallbackCanPickupAmmoPack(
            ps->mClient.mHandle.mVal);
    case IT_KIT:
        return bTouched == 0;
    default:
        return 0;
    }
    return 0;
}

// ============================================================================
// BG_GetWeaponForInfo - ea: 0x607050
// ============================================================================
int BG_GetWeaponForInfo(weaponFileInfo_t* pWeapInfo)
{
    return pWeapInfo->index;
}

// ============================================================================
// BG_GetNumWeapons - ea: 0x607060
// ============================================================================
int BG_GetNumWeapons()
{
    return bg_iNumWeapons;
}

// ============================================================================
// BG_GetNumAmmoTypes - ea: 0x607070
// ============================================================================
int BG_GetNumAmmoTypes()
{
    return bg_iNumAmmoTypes;
}

// ============================================================================
// BG_GetAmmoTypeMax - ea: 0x607080
// ============================================================================
int BG_GetAmmoTypeMax(int iAmmoIndex)
{
    if (iAmmoIndex < 0 || iAmmoIndex >= bg_iNumAmmoTypes)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 1514;
        AeAssert::gCurrentExpr =
            "(iAmmoIndex >= 0) && (iAmmoIndex < bg_iNumAmmoTypes)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return bg_iWeapAmmoMaxs[iAmmoIndex];
}

// ============================================================================
// BG_GetNumAmmoClips - ea: 0x6070E0
// ============================================================================
int BG_GetNumAmmoClips()
{
    return bg_iNumWeapClips;
}

// ============================================================================
// BG_GetAmmoClipSize - ea: 0x6070F0
// ============================================================================
int BG_GetAmmoClipSize(int iClipIndex)
{
    if (iClipIndex < 0 || iClipIndex >= bg_iNumWeapClips)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 1536;
        AeAssert::gCurrentExpr =
            "(iClipIndex >= 0) && (iClipIndex < bg_iNumWeapClips)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return bg_iWeapClipSizes[iClipIndex];
}

// ============================================================================
// BG_GetSharedAmmoCapSize - ea: 0x607150
// ============================================================================
int BG_GetSharedAmmoCapSize(int iCapIndex)
{
    if (iCapIndex < 0 || iCapIndex >= bg_iNumSharedAmmoCaps)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 1548;
        AeAssert::gCurrentExpr =
            "(iCapIndex >= 0) && (iCapIndex < bg_iNumSharedAmmoCaps)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return bg_iSharedAmmoCaps[iCapIndex];
}

// ============================================================================
// BG_GetAmmoTypeName - ea: 0x6071B0
// ============================================================================
const char* BG_GetAmmoTypeName(int iAmmoIndex)
{
    if (iAmmoIndex < 0 || iAmmoIndex >= bg_iNumAmmoTypes)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 1560;
        AeAssert::gCurrentExpr =
            "(iAmmoIndex >= 0) && (iAmmoIndex < bg_iNumAmmoTypes)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return bg_szWeapAmmoNames[iAmmoIndex];
}

// ============================================================================
// BG_GetAmmoClipName - ea: 0x607210
// ============================================================================
const char* BG_GetAmmoClipName(int iClipIndex)
{
    if (iClipIndex < 0 || iClipIndex >= bg_iNumWeapClips)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 1572;
        AeAssert::gCurrentExpr =
            "(iClipIndex >= 0) && (iClipIndex < bg_iNumWeapClips)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return bg_szWeapClipNames[iClipIndex];
}

// ============================================================================
// BG_GetWeaponSlotForName - ea: 0x607270
// ============================================================================
int BG_GetWeaponSlotForName(const char* pszSlotName)
{
    int v1 = 0;
    while (_stricmp(pszSlotName, s_szWeapSlotNames[v1]) != 0)
    {
        if (++v1 >= 10)
            return 0;
    }
    return v1;
}

// ============================================================================
// BG_GetWeaponSlotNameForIndex - ea: 0x6072B0
// ============================================================================
const char* BG_GetWeaponSlotNameForIndex(unsigned int iSlot)
{
    if (iSlot >= 0xA)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 1647;
        AeAssert::gCurrentExpr = "(iSlot >= 0) && (iSlot < WEAPSLOT_NUM)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return s_szWeapSlotNames[iSlot];
}

// ============================================================================
// BG_GetWeaponIndexForName(uint) - ea: 0x607310
// ============================================================================
unsigned char BG_GetWeaponIndexForName(unsigned int name)
{
    int v1 = bg_iNumWeapons;
    if (bg_iNumWeapons >= 256)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 1662;
        AeAssert::gCurrentExpr = "bg_iNumWeapons < 256";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        v1 = bg_iNumWeapons;
    }
    int v2 = 0;
    if (v1 < 0)
        return 0;
    while (name != bg_weaponInfo[v2]->internalNameHash)
    {
        if (++v2 > v1)
            return 0;
    }
    return (unsigned char)v2;
}

// ============================================================================
// BG_GetWeaponIndexForName(const char*) - ea: 0x6073A0
// ============================================================================
unsigned char BG_GetWeaponIndexForName(const char* pszName)
{
    if (pszName == nullptr || *pszName == 0)
        return 0;
    unsigned int v1 = HashString::CalcHash(pszName);
    unsigned char WeaponIndexForName = BG_GetWeaponIndexForName(v1);
    if (WeaponIndexForName == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 1679;
        AeAssert::gCurrentExpr = "rv != 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Couldn't find weapon \"%s\"\n", pszName))
            __debugbreak();
    }
    return WeaponIndexForName;
}

// ============================================================================
// BG_GetWeaponIndexForWorldModelName - ea: 0x607420
// ============================================================================
unsigned char BG_GetWeaponIndexForWorldModelName(const char* pszModelName)
{
    int v1 = 0;
    if (pszModelName == nullptr || *pszModelName == 0)
        return 0;
    if (bg_iNumWeapons >= 256)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 1688;
        AeAssert::gCurrentExpr = "bg_iNumWeapons < 256";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (bg_iNumWeapons < 0)
        return 0;
    while (_stricmp(pszModelName, bg_weaponInfo[v1]->szWorldModel) != 0)
    {
        if (++v1 > bg_iNumWeapons)
            return 0;
    }
    return (unsigned char)v1;
}

// ============================================================================
// BG_IsAimDownSightWeapon - ea: 0x6074D0
// ============================================================================
int BG_IsAimDownSightWeapon(int iWeapon)
{
    return BG_GetInfoForWeapon(iWeapon)->bADSPositionInfo;
}

// ============================================================================
// BG_GetEmptySlotForWeapon - ea: 0x6074F0
// ============================================================================
weapSlot_t BG_GetEmptySlotForWeapon(const PlayerState* pPS, int iWeaponIndex)
{
    weapSlot_t result = (weapSlot_t)BG_GetInfoForWeapon(iWeaponIndex)->slot;
    switch (result)
    {
    case WEAPSLOT_PRIMARY:
    case WEAPSLOT_PRIMARYB:
        if (pPS->weaponslots[1] != 0)
        {
            if (pPS->weaponslots[2] != 0)
                goto slot_none;
            result = WEAPSLOT_PRIMARYB;
        }
        else
        {
            result = WEAPSLOT_PRIMARY;
        }
        break;
    case WEAPSLOT_PISTOL:
    case WEAPSLOT_GRENADE:
    case WEAPSLOT_SMOKE_GRENADE:
    case WEAPSLOT_INTERACT:
    case WEAPSLOT_BINOCS:
    case WEAPSLOT_SATCHEL:
    case WEAPSLOT_SPECIAL:
        if (pPS->weaponslots[result] != 0)
            goto slot_none;
        break;
    default:
    slot_none:
        result = WEAPSLOT_NONE;
        break;
    }
    return result;
}

// ============================================================================
// BG_GetStackSlotForWeapon - ea: 0x607570
// ============================================================================
weapSlot_t BG_GetStackSlotForWeapon(const PlayerState* pPS, int iWeaponIndex,
                                    weapSlot_t preferedSlot)
{
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(iWeaponIndex);
    if (InfoForWeapon->bSlotStackable == 0)
        return WEAPSLOT_NONE;
    weapSlot_t result = (weapSlot_t)InfoForWeapon->slot;
    switch (result)
    {
    case WEAPSLOT_PRIMARY:
    case WEAPSLOT_PRIMARYB:
        if ((preferedSlot == WEAPSLOT_PRIMARY
             || preferedSlot == WEAPSLOT_PRIMARYB)
            && (pPS->weaponslots[preferedSlot] == 0
                || BG_GetInfoForWeapon(
                       pPS->weaponslots[preferedSlot])->bSlotStackable != 0))
        {
            result = preferedSlot;
        }
        else
        {
            unsigned char v6 = pPS->weaponslots[1];
            if (v6 == 0
                || BG_GetInfoForWeapon(v6)->bSlotStackable != 0)
            {
                result = WEAPSLOT_PRIMARY;
            }
            else
            {
                unsigned char v7 = pPS->weaponslots[2];
                if (v7 != 0
                    && BG_GetInfoForWeapon(v7)->bSlotStackable == 0)
                    goto slot_none;
                result = WEAPSLOT_PRIMARYB;
            }
        }
        break;
    case WEAPSLOT_PISTOL:
    case WEAPSLOT_GRENADE:
    case WEAPSLOT_SMOKE_GRENADE:
    case WEAPSLOT_SATCHEL:
    case WEAPSLOT_SPECIAL:
    {
        unsigned char v8 = pPS->weaponslots[result];
        if (v8 != 0)
        {
            if (BG_GetInfoForWeapon(v8)->bSlotStackable == 0)
                goto slot_none;
            result = (weapSlot_t)InfoForWeapon->slot;
        }
        break;
    }
    default:
    slot_none:
        result = WEAPSLOT_NONE;
        break;
    }
    return result;
}

// ============================================================================
// BG_IsPlayerWeaponAnAlt - ea: 0x607690
// ============================================================================
int BG_IsPlayerWeaponAnAlt(int iWeaponIndex, int iAltIndex)
{
    int iAltWeaponIndex =
        BG_GetInfoForWeapon(iWeaponIndex)->iAltWeaponIndex;
    if (iAltWeaponIndex == 0)
        return 0;
    while (iAltWeaponIndex != iAltIndex)
    {
        if (iAltWeaponIndex != iWeaponIndex)
        {
            iAltWeaponIndex = BG_GetInfoForWeapon(
                iAltWeaponIndex)->iAltWeaponIndex;
            if (iAltWeaponIndex != 0)
                continue;
        }
        return 0;
    }
    return 1;
}

// ============================================================================
// BG_SelectWeaponIndex - ea: 0x6076E0
// ============================================================================
int BG_SelectWeaponIndex(int iWeaponIndex, int client)
{
    if (client != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 2293;
        AeAssert::gCurrentExpr = "client >= 0 && client < 1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid client index"))
            __debugbreak();
    }
    if (client != 0)
        return 0;
    cg_aWeaponSelectTime[0] = cgGlobal_time;
    if (cg_aWeaponSelect[0] == iWeaponIndex)
        return 0;
    bool v3 = iWeaponIndex != 0
        && iWeaponIndex == BG_GetInfoForWeapon(
            cg_aWeaponSelect[0])->iAltWeaponIndex;
    cg_aWeaponSelect[0] = iWeaponIndex;
    if (!v3)
        cl_aADS[0] = 1;
    return 1;
}

// ============================================================================
// BG_GetConeAngleForWeapon - ea: 0x6077B0
// ============================================================================
float BG_GetConeAngleForWeapon(const PlayerState* pPS, int iWeaponIndex,
                               int iTime, int bAds)
{
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(iWeaponIndex);
    float coneAngle;
    if (bAds == 0
        || (coneAngle = InfoForWeapon->fAdsBulletConeAngle,
            InfoForWeapon->fAdsZoomFov < 20.0f))
        coneAngle = InfoForWeapon->fBulletConeAngle;
    return coneAngle * 0.017455f;
}

// ============================================================================
// BG_GetMinSpreadForWeapon - ea: 0x607800
// ============================================================================
float BG_GetMinSpreadForWeapon(const PlayerState* pPS, int iWeaponIndex,
                               int iTime, int bAds)
{
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(iWeaponIndex);
    int viewHeightLerpTarget = pPS->viewHeightLerpTarget;
    int viewHeightLerpTime = pPS->viewHeightLerpTime;
    if (viewHeightLerpTarget == pPS->viewHeightCurrent || viewHeightLerpTime == 0)
    {
        int pm_flags = pPS->pm_flags;
        if (bAds != 0)
        {
            if ((pm_flags & 1) != 0)
                return InfoForWeapon->fAdsSpreadProne;
            if ((pm_flags & 2) != 0)
                return InfoForWeapon->fAdsSpreadDucked;
            return InfoForWeapon->fAdsSpread;
        }
        if ((pm_flags & 1) != 0)
            return InfoForWeapon->fHipSpreadProneMin;
        if ((pm_flags & 2) != 0)
            return InfoForWeapon->fHipSpreadDuckedMin;
        return InfoForWeapon->fHipSpreadStandMin;
    }
    int v9;
    if (viewHeightLerpTarget == pPS->proneViewHeight)
        v9 = 400;
    else if (viewHeightLerpTarget == pPS->crouchViewHeight)
        v9 = pPS->viewHeightLerpDown != 0 ? 200 : 400;
    else
        v9 = 200;
    float fLerpFrac = (float)(iTime - viewHeightLerpTime) / (float)v9;
    if (fLerpFrac < 0.0f)
        fLerpFrac = 0.0f;
    if (fLerpFrac > 1.0f)
        fLerpFrac = 1.0f;
    if (bAds != 0)
    {
        if (viewHeightLerpTarget == pPS->proneViewHeight)
            return (InfoForWeapon->fAdsSpreadProne
                    - InfoForWeapon->fAdsSpreadDucked) * fLerpFrac
                + InfoForWeapon->fAdsSpreadDucked;
        if (viewHeightLerpTarget == pPS->standViewHeight)
            return (InfoForWeapon->fAdsSpread
                    - InfoForWeapon->fAdsSpreadDucked) * fLerpFrac
                + InfoForWeapon->fAdsSpreadDucked;
        if (pPS->viewHeightLerpDown != 0)
            return (InfoForWeapon->fAdsSpreadDucked
                    - InfoForWeapon->fAdsSpread) * fLerpFrac
                + InfoForWeapon->fAdsSpread;
        return (InfoForWeapon->fAdsSpreadDucked
                - InfoForWeapon->fAdsSpreadProne) * fLerpFrac
            + InfoForWeapon->fAdsSpreadProne;
    }
    if (viewHeightLerpTarget == pPS->proneViewHeight)
        return (InfoForWeapon->fHipSpreadProneMin
                - InfoForWeapon->fHipSpreadDuckedMin) * fLerpFrac
            + InfoForWeapon->fHipSpreadDuckedMin;
    if (viewHeightLerpTarget == pPS->standViewHeight)
        return (InfoForWeapon->fHipSpreadStandMin
                - InfoForWeapon->fHipSpreadDuckedMin) * fLerpFrac
            + InfoForWeapon->fHipSpreadDuckedMin;
    if (pPS->viewHeightLerpDown != 0)
        return (InfoForWeapon->fHipSpreadDuckedMin
                - InfoForWeapon->fHipSpreadStandMin) * fLerpFrac
            + InfoForWeapon->fHipSpreadStandMin;
    return (InfoForWeapon->fHipSpreadDuckedMin
            - InfoForWeapon->fHipSpreadProneMin) * fLerpFrac
        + InfoForWeapon->fHipSpreadProneMin;
}

// ============================================================================
// BG_ClipForWeapon - ea: 0x607A10
// ============================================================================
int BG_ClipForWeapon(int iWeapon)
{
    return BG_GetInfoForWeapon(iWeapon)->iClipIndex;
}

// ============================================================================
// BG_AmmoForWeapon - ea: 0x607A30
// ============================================================================
int BG_AmmoForWeapon(int iWeapon)
{
    return BG_GetInfoForWeapon(iWeapon)->iAmmoIndex;
}

// ============================================================================
// BG_WeaponIsClipOnly - ea: 0x607A50
// ============================================================================
int BG_WeaponIsClipOnly(int iWeapon)
{
    return BG_GetInfoForWeapon(iWeapon)->bClipOnly;
}

// ============================================================================
// BG_WeaponAmmo - ea: 0x607A70
// ============================================================================
int BG_WeaponAmmo(const PlayerState* pPS, int iWeapon)
{
    int iAmmoIndex = BG_GetInfoForWeapon(iWeapon)->iAmmoIndex;
    return pPS->ammo[iAmmoIndex]
        + pPS->ammoclip[BG_GetInfoForWeapon(iWeapon)->iClipIndex];
}

// ============================================================================
// BG_GetRandomAmmoCounts - ea: 0x607AB0
// ============================================================================
void BG_GetRandomAmmoCounts(int* ammo, int* clip, int weaponIndex)
{
    if (weaponIndex < 0 || weaponIndex >= bg_iNumWeapons)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 2969;
        AeAssert::gCurrentExpr =
            "weaponIndex >= 0 && weaponIndex < bg_iNumWeapons";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid weapon index"))
            __debugbreak();
    }
    BG_GetInfoForWeapon(weaponIndex);
    int iClipIndex = BG_GetInfoForWeapon(weaponIndex)->iClipIndex;
    int iDropAmmoMax = BG_GetInfoForWeapon(weaponIndex)->iDropAmmoMax;
    int iDropAmmoMin = BG_GetInfoForWeapon(weaponIndex)->iDropAmmoMin;
    if (iDropAmmoMax < iDropAmmoMin)
    {
        iDropAmmoMax = iDropAmmoMin;
        iDropAmmoMin = BG_GetInfoForWeapon(weaponIndex)->iDropAmmoMax;
    }
    if (iDropAmmoMax != 0)
    {
        if (iDropAmmoMax < 0)
        {
            *ammo = 0;
            *clip = 0;
            return;
        }
    }
    else if (iDropAmmoMin == 0)
    {
        int v6;
        float v13 = (float)rand() * 0.000030517578f + 1.0f;
        *ammo = (int)(((v13 * (BG_GetAmmoClipSize(iClipIndex) - 1)) * 0.5f)
                      + 0.5f) + 1;
        v6 = (int)(((float)rand() * 0.000015258789f + 0.25f)
                   * (float)*ammo + 0.5f);
        *clip = v6;
        *ammo -= v6;
        return;
    }
    bool v7 = iDropAmmoMax == iDropAmmoMin;
    if (iDropAmmoMax < iDropAmmoMin)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 2998;
        AeAssert::gCurrentExpr = "iMax >= iMin";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        v7 = iDropAmmoMax == iDropAmmoMin;
    }
    int v8 = v7 ? iDropAmmoMin
                : iDropAmmoMin + rand() % (iDropAmmoMax - iDropAmmoMin);
    *ammo = v8;
    if (v8 > 0)
    {
        int AmmoClipSize = BG_GetAmmoClipSize(iClipIndex);
        bool v10 = AmmoClipSize == 0;
        if (AmmoClipSize < 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
            AeAssert::gCurrentLine = 3009;
            AeAssert::gCurrentExpr = "size >= 0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
            v10 = AmmoClipSize == 0;
        }
        int v11 = v10 ? 0 : rand() % AmmoClipSize;
        *clip = v11;
        int v12 = *ammo;
        if (v11 < *ammo)
        {
            *ammo = v12 - v11;
        }
        else
        {
            *clip = v12;
            *ammo = 0;
        }
    }
    else
    {
        *ammo = 0;
        *clip = 0;
    }
}

// ============================================================================
// PM_WeaponUseAmmo - ea: 0x607E10
// ============================================================================
void PM_WeaponUseAmmo(int wp, int amount)
{
    int iClipIndex = BG_GetInfoForWeapon(wp)->iClipIndex;
    int v3 = gInfinteAmmo ? 0 : amount;
    pm->ps->ammoclip[iClipIndex] -= v3;
}

// ============================================================================
// PM_WeaponAmmoAvailable - ea: 0x607E50
// ============================================================================
int PM_WeaponAmmoAvailable(int wp)
{
    return pm->ps->ammoclip[BG_GetInfoForWeapon(wp)->iClipIndex];
}

// ============================================================================
// PM_WeaponClipEmpty - ea: 0x607E80
// ============================================================================
int PM_WeaponClipEmpty(int wp)
{
    return pm->ps->ammoclip[BG_GetInfoForWeapon(wp)->iClipIndex] == 0;
}

// ============================================================================
// PM_KillQueuedReloadSound - ea: 0x6080E0
// ============================================================================
void PM_KillQueuedReloadSound(PlayerState* ps)
{
    unsigned int mVal = ps->queuedReloadSound.mVal;
    if (mVal != 0)
    {
        EffectEventSys_StopEffect(EffectEventSys_sInst, mVal, false);
        ps->queuedReloadSound.mVal = 0;
        ps->queuedReloadSoundPlayStarted = false;
    }
}

// ============================================================================
// PM_AdjustAimSpreadScale - ea: 0x608670
// ============================================================================
PlayerState* PM_AdjustAimSpreadScale()
{
    pmove_t* v1 = pm;
    weaponFileInfo_t* pWeap = (weaponFileInfo_t*)pml.pWeap;
    float fHipSpreadDecayRate = pWeap->fHipSpreadDecayRate;
    float v8;
    float v12;
    float viewchange;
    if (fHipSpreadDecayRate == 0.0f)
    {
        v8 = 1.0f;
        v12 = 0.0f;
        goto spread_apply;
    }
    PlayerState* ps = pm->ps;
    if (pm->ps->mGroundEntity.mHandle.mVal == 0 && ps->pm_type != 1)
    {
        fHipSpreadDecayRate = fHipSpreadDecayRate * 0.5f;
        goto decay_ready;
    }
    int eFlags = ps->eFlags;
    if ((eFlags & 0x40) != 0)
    {
        fHipSpreadDecayRate = pWeap->fHipSpreadProneDecay
            * fHipSpreadDecayRate;
    }
    else if ((eFlags & 0x20) != 0)
    {
        fHipSpreadDecayRate = pWeap->fHipSpreadDuckedDecay
            * fHipSpreadDecayRate;
    }
decay_ready:
    v8 = pml.frametime * fHipSpreadDecayRate;
    if (ps->fWeaponPosFrac == 1.0f)
    {
        v12 = 0.0f;
        goto spread_apply;
    }
    viewchange = 0.0f;
    if (pWeap->fHipSpreadTurnAdd != 0.0f)
    {
        for (int i = 16; i < 24; i += 4)
        {
            float a1 = AngleSubtract(*(float*)((char*)v1 + i) * 0.0054931641f,
                                     *(float*)((char*)&v1->cmd.gunZOfs + i) * 0.0054931641f);
            pWeap = (weaponFileInfo_t*)pml.pWeap;
            v1 = pm;
            a1 = fabsf(a1) * pWeap->fHipSpreadTurnAdd * 0.0099999998f
                / pml.frametime + viewchange;
            viewchange = a1;
        }
    }
    if (pWeap->fHipSpreadMoveAdd != 0.0f)
    {
        char forwardmove = v1->cmd.forwardmove;
        if (abs(v1->cmd.rightmove) > abs(forwardmove))
        {
            viewchange = fabsf((float)v1->cmd.rightmove * 0.0078125f)
                * pWeap->fHipSpreadMoveAdd + viewchange;
        }
        else if (forwardmove != 0)
        {
            viewchange = fabsf((float)forwardmove * 0.0078125f)
                * pWeap->fHipSpreadMoveAdd + viewchange;
        }
    }
    if (v1->ps->mGroundEntity.mHandle.mVal != 0 || v1->ps->pm_type == 1)
        v12 = pml.frametime * viewchange;
    else
        v12 = pml.frametime * ((viewchange + 1.28f) + 1.28f);
spread_apply:
    v1->ps->aimSpreadScale = ((v12 - v8) * 255.0f) + v1->ps->aimSpreadScale;
    PlayerState* result = pm->ps;
    if (pm->ps->aimSpreadScale >= 0.0f)
    {
        if (result->aimSpreadScale > 255.0f)
            result->aimSpreadScale = 255.0f;
    }
    else
    {
        result->aimSpreadScale = 0.0f;
    }
    return result;
}

// ============================================================================
// PM_Weapon_CheckFriendlyFireUse - ea: 0x608AC0
// ============================================================================
int PM_Weapon_CheckFriendlyFireUse()
{
    return 1;
}

// ============================================================================
// BG_FindItemForWeapon - ea: 0x612E70
// ============================================================================
// ea: 0x00612E70
const gitem_s* BG_FindItemForWeapon(int weapon)
{
    if (weapon < 0 || weapon > bg_iNumWeapons)
        Com_Error(ERR_DROP,
                  "BG_FindItemForWeapon: weapon out of range %i", weapon);
    return &bg_itemlist[weapon];
}

// ============================================================================
// BG_FindItem - ea: 0x612EA0
// ============================================================================
// ea: 0x00612EA0
const gitem_s* BG_FindItem(const char* pickupName)
{
    int v1 = 1;
    int iIndex = 1;
    char** p_classname = &bg_itemlist[1].classname;
    while (1)
    {
        if (v1 <= bg_iNumWeapons)
        {
            char* szInternalName =
                BG_GetInfoForWeapon(v1)->szInternalName;
            if (pickupName != nullptr && szInternalName != nullptr
                && ae_stricmpn(pickupName, szInternalName, 0x7FFFFFFF) == 0)
                return &bg_itemlist[v1];
        }
        else
        {
            const char* v5 = p_classname[6];
            if (v5 != nullptr && pickupName != nullptr
                && ae_stricmpn(v5, pickupName, 0x7FFFFFFF) == 0)
                break;
            if (*p_classname != nullptr && pickupName != nullptr
                && ae_stricmpn(*p_classname, pickupName, 0x7FFFFFFF) == 0)
                break;
            v1 = iIndex;
        }
        ++v1;
        p_classname += 13;
        iIndex = v1;
        if (p_classname >= &bg_itemlist[137].classname)
            return nullptr;
    }
    return reinterpret_cast<const gitem_s*>(p_classname - 1);
}

// ============================================================================
// BG_EvaluateTrajectory - ea: 0x613020
// ============================================================================
// ea: 0x00613020
void BG_EvaluateTrajectory(const trajectory_t* tr, int atTime,
                           math::Position3& result)
{
    float value = g_gravity.value;
    if (tr->trGravityOverride != 0.0f)
        value = (float)tr->trGravityOverride;
    switch (tr->trType)
    {
    case TR_STATIONARY:
    case TR_INTERPOLATE:
    case TR_GRAVITY_PAUSED:
        result.v.m128_f32[0] = tr->trBase[0];
        result.v.m128_f32[1] = tr->trBase[1];
        result.v.m128_f32[2] = tr->trBase[2];
        break;
    case TR_LINEAR:
    {
        float v5 = (atTime - tr->trTime) * 0.001f;
        result.v.m128_f32[0] = (tr->trDelta[0] * v5) + tr->trBase[0];
        result.v.m128_f32[1] = (tr->trDelta[1] * v5) + tr->trBase[1];
        result.v.m128_f32[2] = (tr->trDelta[2] * v5) + tr->trBase[2];
        break;
    }
    case TR_LINEAR_STOP:
    {
        int trTime = tr->trTime;
        int v8 = atTime;
        if (atTime > trTime + tr->trDuration)
            v8 = trTime + tr->trDuration;
        float v9 = (v8 - trTime) * 0.001f;
        if (v9 < 0.0f)
            v9 = 0.0f;
        result.v.m128_f32[0] = (tr->trDelta[0] * v9) + tr->trBase[0];
        result.v.m128_f32[1] = (tr->trDelta[1] * v9) + tr->trBase[1];
        result.v.m128_f32[2] = (tr->trDelta[2] * v9) + tr->trBase[2];
        break;
    }
    case TR_SINE:
    {
        int v32 = atTime - tr->trTime;
        float v6 =
            sinf((float)v32 / (float)tr->trDuration * 6.2831855f);
        result.v.m128_f32[0] = v6 * tr->trDelta[0] + tr->trBase[0];
        result.v.m128_f32[1] = v6 * tr->trDelta[1] + tr->trBase[1];
        result.v.m128_f32[2] = v6 * tr->trDelta[2] + tr->trBase[2];
        break;
    }
    case TR_GRAVITY:
    {
        float v10 = (atTime - tr->trTime) * 0.001f;
        result.v.m128_f32[0] = (tr->trDelta[0] * v10) + tr->trBase[0];
        result.v.m128_f32[1] = (tr->trDelta[1] * v10) + tr->trBase[1];
        result.v.m128_f32[2] = (tr->trDelta[2] * v10) + tr->trBase[2];
        result.v.m128_f32[2] =
            result.v.m128_f32[2] - (((v10 * v10) * value) * 0.5f);
        break;
    }
    case TR_GRAVITY_LOW:
    case TR_GRAVITY_FLOAT:
    {
        float v11;
        float v12;
        if (tr->trType == TR_GRAVITY_LOW)
        {
            v11 = (atTime - tr->trTime) * 0.001f;
            v12 = (value * 0.30000001f) * v11;
        }
        else
        {
            v12 = value * 0.2f;
            v11 = (atTime - tr->trTime) * 0.001f;
        }
        result.v.m128_f32[0] = (tr->trDelta[0] * v11) + tr->trBase[0];
        result.v.m128_f32[1] = (tr->trDelta[1] * v11) + tr->trBase[1];
        float v13 = (tr->trDelta[2] * v11) + tr->trBase[2];
        result.v.m128_f32[2] = v13;
        result.v.m128_f32[2] = v13 - ((v12 * v11) * 0.5f);
        break;
    }
    case TR_ACCELERATE:
    case TR_DECCELERATE:
    {
        int trTime = tr->trTime;
        int trDuration = tr->trDuration;
        int v18 = atTime;
        if (atTime > trDuration + trTime)
            v18 = trDuration + trTime;
        float v32 = (v18 - trTime) * 0.001f;
        float speed = (float)sqrt(
            (double)(tr->trDelta[0] * tr->trDelta[0]
                     + tr->trDelta[1] * tr->trDelta[1]
                     + tr->trDelta[2] * tr->trDelta[2]))
            / ((float)trDuration * 0.001f);
        const math::Dir3 dir = native_to_cdl_dir3(tr->trDelta);
        VectorNormalize2(&dir, (math::Dir3*)&result);
        float v20 = ((speed * v32) * v32) * 0.5f;
        if (tr->trType == TR_DECCELERATE)
            v20 = -v20;
        result.v.m128_f32[0] = (v20 * result.v.m128_f32[0])
            + ((v32 * tr->trDelta[0]) + tr->trBase[0]);
        result.v.m128_f32[1] = (v20 * result.v.m128_f32[1])
            + ((v32 * tr->trDelta[1]) + tr->trBase[1]);
        result.v.m128_f32[2] = (v20 * result.v.m128_f32[2])
            + ((v32 * tr->trDelta[2]) + tr->trBase[2]);
        break;
    }
    default:
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_misc.cpp";
        AeAssert::gCurrentLine = 677;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "BG_EvaluateTrajectory: unknown trType"))
            __debugbreak();
        result.v.m128_f32[2] = 0.0f;
        result.v.m128_f32[0] = 0.0f;
        break;
    }
}

// ============================================================================
// PM_CanSimulateFiringWeapon - ea: 0x614700
// ============================================================================
// ea: 0x00614700
bool PM_CanSimulateFiringWeapon(int iWeapon)
{
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(iWeapon);
    if (InfoForWeapon == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_misc.cpp";
        AeAssert::gCurrentLine = 1764;
        AeAssert::gCurrentExpr = "wInfo";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return InfoForWeapon->type == WEAPTYPE_BULLET
        && InfoForWeapon->iFireDelay == 0
        && InfoForWeapon->iFireTime <= 250
        && InfoForWeapon->bSemiAuto == 0
        && InfoForWeapon->bBoltAction == 0;
}

// ============================================================================
// BG_PlayerTouchesMine - ea: 0x612F60
// ============================================================================
// ea: 0x00612F60
bool BG_PlayerTouchesMine(PlayerState* ps, EntityState* item, int atTime)
{
    float v6[3];
    memcpy(v6, item->pos.trBase, 12);
    bool result = false;
    if ((ps->eFlags & 0x100000) == 0)
    {
        weaponFileInfo_t* InfoForWeapon =
            BG_GetInfoForWeapon(item->weapon);
        if (InfoForWeapon->iTriggerRadius != 0)
        {
            float dx = v6[0] - ps->origin.v.m128_f32[0];
            float dy = v6[1] - ps->origin.v.m128_f32[1];
            float dz = v6[2] - ps->origin.v.m128_f32[2];
            float dist2 = dx * dx + dy * dy + dz * dz;
            if ((float)(InfoForWeapon->iTriggerRadius
                        * InfoForWeapon->iTriggerRadius)
                > dist2)
                return true;
        }
    }
    return result;
}

// ============================================================================
// Collision context filters - ea: 0x615AF0..0x615B20
// ============================================================================
struct player_collision_context_t : collision_context_t {
    virtual bool filter(Entity* ent) const;  // ?filter@player_collision_context_t@@UBE_NPAVEntity@@@Z
};

// ea: 0x00615AF0
bool player_collision_context_t::filter(Entity* ent) const
{
    return ent == nullptr || (ent->mFlags & 1u) == 0;
}

struct ai_collision_context_t : collision_context_t {
    virtual bool filter(Entity* ent) const;  // ?filter@ai_collision_context_t@@UBE_NPAVEntity@@@Z
};

// ea: 0x00615B20
bool ai_collision_context_t::filter(Entity* ent) const
{
    return ent == nullptr || (ent->mFlags & 1u) == 0;
}

// ============================================================================
// BG_GetSpreadForWeapon - ea: 0x615C90
// ============================================================================
// ea: 0x00615C90
void BG_GetSpreadForWeapon(const PlayerState* ps, int weaponIndex,
                           float* minSpread, float* maxSpread)
{
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(weaponIndex);
    float viewHeightCurrent = ps->viewHeightCurrent;
    if (viewHeightCurrent <= (float)bg_viewheight_crouched.integer)
    {
        *minSpread = (((viewHeightCurrent - (float)bg_viewheight_prone.integer)
                      / ((float)bg_viewheight_crouched.integer
                         - (float)bg_viewheight_prone.integer))
                     * (InfoForWeapon->fHipSpreadDuckedMin
                        - InfoForWeapon->fHipSpreadProneMin))
                    + InfoForWeapon->fHipSpreadProneMin;
    }
    else
    {
        *minSpread = (((viewHeightCurrent - (float)bg_viewheight_crouched.integer)
                      / ((float)bg_viewheight_standing.integer
                         - (float)bg_viewheight_crouched.integer))
                     * (InfoForWeapon->fHipSpreadStandMin
                        - InfoForWeapon->fHipSpreadDuckedMin))
                    + InfoForWeapon->fHipSpreadDuckedMin;
    }
    *maxSpread = InfoForWeapon->fHipSpreadMax;
}

// ============================================================================
// bg_weapons.cpp item/ammo setup - ea: 0x616040..0x6168A0
// ============================================================================
// game.o data globals (bg_weapons.cpp)
extern char** bg_szSharedAmmoCapNames;  // 0xF3E7D0

// ea: 0x00616040
int BG_FillInWeaponItems()
{
    int v0 = 1;
    gitem_s* v1 = &bg_itemlist[1];
    int result = bg_iNumWeapons;
    if (bg_iNumWeapons >= 1)
    {
        do
        {
            weaponFileInfo_t* v2 = bg_weaponInfo[v0];
            char* szRadiantName = v2->szRadiantName;
            v1->classname = szRadiantName;
            v1->classname_hash = HashString::CalcHash(szRadiantName);
            char* szPickupModel = v2->szPickupModel;
            if (szPickupModel != nullptr && *szPickupModel != 0)
                v1->world_model[0] = szPickupModel;
            else
                v1->world_model[0] = v2->szWorldModel;
            v1->world_model[1] = nullptr;
            v1->icon = v2->szHudIcon;
            v1->ammoicon = v2->szAmmoIcon;
            v1->pickup_name = v2->szDisplayName;
            int iStartAmmo = v2->iStartAmmo;
            v1->giTag = v0;
            v1->quantity = iStartAmmo;
            v1->giType = IT_WEAPON;
            v1->giAmmoIndex = v2->iAmmoIndex;
            result = bg_iNumWeapons;
            v1->giClipIndex = v2->iClipIndex;
            ++v0;
            ++v1;
        } while (v0 <= result);
    }
    if (v0 < 137)
    {
        int* p_giTag = &v1->giTag;
        int v10 = 137 - v0;
        do
        {
            if (*(p_giTag - 1) == IT_AMMO)
            {
                int j = 1;
                if (bg_iNumWeapons >= 1)
                {
                    weaponFileInfo_t* v7 = nullptr;
                    while (1)
                    {
                        v7 = bg_weaponInfo[j];
                        if (ae_stricmpn((const char*)*(p_giTag - 3), v7->szInternalName,
                                        (int)strlen(v7->szInternalName)) == 0)
                            break;
                        if (++j > bg_iNumWeapons)
                        {
                            v7 = nullptr;
                            goto LABEL_15;
                        }
                    }
                    *p_giTag = j;
                    p_giTag[1] = v7->iAmmoIndex;
                    p_giTag[2] = v7->iClipIndex;
                }
            LABEL_15:
                if (*p_giTag == -1)
                {
                    Com_Printf(
                        "^3WARNING^7: Could not find weapon for ammo item %s\n",
                        (const char*)*(p_giTag - 3));
                    weaponFileInfo_t* v8 = bg_weaponInfo[1];
                    if (v8 == nullptr)
                        v8 = *bg_weaponInfo;
                    *p_giTag = 1;
                    p_giTag[1] = v8->iAmmoIndex;
                    p_giTag[2] = v8->iClipIndex;
                }
            }
            p_giTag += 13;
            result = --v10;
        } while (v10 != 0);
    }
    return result;
}

// ea: 0x006161E0
int BG_SetupAmmoIndexes()
{
    int i = 1;
    if (bg_iNumWeapons >= 1)
    {
        int v15 = 1;
        do
        {
            weaponFileInfo_t* v1 = bg_weaponInfo[v15];
            char* szAmmoName = v1->szAmmoName;
            if (*szAmmoName != 0)
            {
                char v3;
                do
                {
                    *szAmmoName = (char)tolower(*szAmmoName);
                    v3 = *++szAmmoName;
                } while (v3 != 0);
            }
            int v4 = 0;
            if (bg_iNumAmmoTypes > 0)
            {
                while (1)
                {
                    const char* v5 = bg_szWeapAmmoNames[v4];
                    const char* v6 = v1->szAmmoName;
                    if (v5 != nullptr && v6 != nullptr
                        && ae_stricmpn(v5, v6, 0x7FFFFFFF) == 0)
                        break;
                    if (++v4 >= bg_iNumAmmoTypes)
                        goto LABEL_23;
                }
                int iMaxAmmo = v1->iMaxAmmo;
                v1->iAmmoIndex = v4;
                int iIndex = v4;
                if (bg_iWeapAmmoMaxs[v4] != iMaxAmmo && v4 != 0)
                {
                    int v8 = 1;
                    if (v15 > 1)
                    {
                        do
                        {
                            const char* v9 = bg_szWeapAmmoNames[iIndex];
                            weaponFileInfo_t* v10 = bg_weaponInfo[v8];
                            const char* v11 = v10->szAmmoName;
                            if (v9 != nullptr && v11 != nullptr
                                && ae_stricmpn(v9, v11, 0x7FFFFFFF) == 0
                                && v10->iMaxAmmo == bg_iWeapAmmoMaxs[iIndex])
                            {
                                AeAssert::gCurrentAuthor = AeAssert::JRS;
                                AeAssert::gCurrentFile =
                                    "c:\\cod\\code\\game\\bg_weapons.cpp";
                                AeAssert::gCurrentLine = 870;
                                AeAssert::gCurrentExpr = nullptr;
                                if (!AeAssert::IsIgnored()
                                    && AeAssert::Warning(
                                        "Max ammo mismatch for \"%s\" ammo: "
                                        "'%s\" set it to %i, but \"%s\" "
                                        "already set it to %i.\n",
                                        v1->szAmmoName, v1->szInternalName,
                                        v1->iMaxAmmo, v10->szInternalName,
                                        v10->iMaxAmmo))
                                    __debugbreak();
                            }
                            ++v8;
                        } while (v8 < i);
                        v4 = iIndex;
                    }
                }
            }
        LABEL_23:
            int v12 = bg_iNumAmmoTypes;
            if (v4 == bg_iNumAmmoTypes)
            {
                bg_szWeapAmmoNames[v4] = v1->szAmmoName;
                bg_iWeapAmmoMaxs[v4] = v1->iMaxAmmo;
                v1->iAmmoIndex = v4;
                bg_iNumAmmoTypes = v12 + 1;
            }
            int result = i + 1;
            bool v13 = ++i <= bg_iNumWeapons;
            ++v15;
            if (!v13)
                return result;
        } while (1);
    }
    return 1;
}

// ea: 0x006163E0
int BG_SetupSharedAmmoIndexes()
{
    int result = bg_iNumWeapons;
    for (int i = 1; i <= bg_iNumWeapons; ++i)
    {
        weaponFileInfo_t* v2 = bg_weaponInfo[i];
        const char* szSharedAmmoCapName = v2->szSharedAmmoCapName;
        v2->iSharedAmmoCapIndex = -1;
        if (*szSharedAmmoCapName != 0)
        {
            Com_DPrintf("%s: %s\n", v2->szInternalName, szSharedAmmoCapName);
            char* v4 = v2->szSharedAmmoCapName;
            if (*v4 != 0)
            {
                char v5;
                do
                {
                    *v4 = (char)tolower(*v4);
                    v5 = *++v4;
                } while (v5 != 0);
            }
            int v6 = bg_iNumSharedAmmoCaps;
            bg_szSharedAmmoCapNames[bg_iNumSharedAmmoCaps] =
                v2->szSharedAmmoCapName;
            bg_iSharedAmmoCaps[v6] = v2->iSharedAmmoCap;
            v2->iSharedAmmoCapIndex = v6;
            bg_iNumSharedAmmoCaps = v6 + 1;
        }
        result = bg_iNumWeapons;
    }
    return result;
}

// ea: 0x006164A0
int BG_SetupClipIndexes()
{
    int i = 1;
    if (bg_iNumWeapons >= 1)
    {
        int v15 = 1;
        do
        {
            weaponFileInfo_t* v1 = bg_weaponInfo[v15];
            char* szClipName = v1->szClipName;
            if (*szClipName != 0)
            {
                char v3;
                do
                {
                    *szClipName = (char)tolower(*szClipName);
                    v3 = *++szClipName;
                } while (v3 != 0);
            }
            int v4 = 0;
            if (bg_iNumWeapClips > 0)
            {
                while (1)
                {
                    const char* v5 = bg_szWeapClipNames[v4];
                    const char* v6 = v1->szClipName;
                    if (v5 != nullptr && v6 != nullptr
                        && ae_stricmpn(v5, v6, 0x7FFFFFFF) == 0)
                        break;
                    if (++v4 >= bg_iNumWeapClips)
                        goto LABEL_23;
                }
                int iClipSize = v1->iClipSize;
                v1->iClipIndex = v4;
                int iIndex = v4;
                if (bg_iWeapClipSizes[v4] != iClipSize && v4 != 0)
                {
                    int v8 = 1;
                    if (v15 > 1)
                    {
                        do
                        {
                            const char* v9 = bg_szWeapClipNames[iIndex];
                            weaponFileInfo_t* v10 = bg_weaponInfo[v8];
                            const char* v11 = v10->szClipName;
                            if (v9 != nullptr && v11 != nullptr
                                && ae_stricmpn(v9, v11, 0x7FFFFFFF) == 0
                                && v10->iClipSize == bg_iWeapClipSizes[iIndex])
                            {
                                AeAssert::gCurrentAuthor = AeAssert::JRS;
                                AeAssert::gCurrentFile =
                                    "c:\\cod\\code\\game\\bg_weapons.cpp";
                                AeAssert::gCurrentLine = 1002;
                                AeAssert::gCurrentExpr = nullptr;
                                if (!AeAssert::IsIgnored()
                                    && AeAssert::Warning(
                                        "Clip Size mismatch for \"%s\" clip: "
                                        "'%s\" set it to %i, but \"%s\" "
                                        "already set it to %i.\n",
                                        v1->szAmmoName, v1->szInternalName,
                                        v1->iClipSize, v10->szInternalName,
                                        v10->iClipSize))
                                    __debugbreak();
                            }
                            ++v8;
                        } while (v8 < i);
                        v4 = iIndex;
                    }
                }
            }
        LABEL_23:
            int v12 = bg_iNumWeapClips;
            if (v4 == bg_iNumWeapClips)
            {
                bg_szWeapClipNames[v4] = v1->szClipName;
                bg_iWeapClipSizes[v4] = v1->iClipSize;
                v1->iClipIndex = v4;
                bg_iNumWeapClips = v12 + 1;
            }
            int result = i + 1;
            bool v13 = ++i <= bg_iNumWeapons;
            ++v15;
            if (!v13)
                return result;
        } while (1);
    }
    return 1;
}

// ea: 0x006166A0
int compare_weaponfile_names(const void* pe1, const void* pe2)
{
    const char* v2 = *(const char**)pe2;
    if (*(const char**)pe1 != nullptr && v2 != nullptr)
        return ae_stricmpn(*(const char**)pe1, v2, 0x7FFFFFFF);
    return -1;
}

// ea: 0x006166D0
bool BG_IsLMGMounted(const PlayerState* ps)
{
    if (ps == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 1429;
        AeAssert::gCurrentExpr = "ps";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "BG_IsCookingOffGrenade: Invalid PlayerState"))
            __debugbreak();
    }
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(ps->weapon);
    return InfoForWeapon != nullptr
        && InfoForWeapon->weapClass == WEAPCLASS_LMG
        && (ps->pm_flags & 0x20) != 0;
}

// ea: 0x00616750
bool BG_IsCookingOffGrenade(const PlayerState* ps)
{
    if (ps == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 1447;
        AeAssert::gCurrentExpr = "ps";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "BG_IsCookingOffGrenade: Invalid PlayerState"))
            __debugbreak();
    }
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(ps->weapon);
    bool result = false;
    if (InfoForWeapon != nullptr
        && InfoForWeapon->type == WEAPTYPE_GRENADE
        && InfoForWeapon->bCookOffHold != 0)
    {
        int grenadeTimeLeft = ps->grenadeTimeLeft;
        if (grenadeTimeLeft != 0
            && grenadeTimeLeft < InfoForWeapon->iFuseTime)
            return true;
    }
    return result;
}

// ea: 0x006167E0
int BG_GetAmmoTypeForName(const char* pszName)
{
    int v1 = 0;
    if (bg_iNumAmmoTypes <= 0)
    {
    LABEL_6:
        Com_DPrintf("Couldn't find ammo type \"%s\"\n", pszName);
        return 0;
    }
    while (1)
    {
        const char* v2 = bg_szWeapAmmoNames[v1];
        if (v2 != nullptr && pszName != nullptr
            && ae_stricmpn(v2, pszName, 0x7FFFFFFF) == 0)
            return v1;
        if (++v1 >= bg_iNumAmmoTypes)
            goto LABEL_6;
    }
}

// ea: 0x00616840
int BG_GetAmmoClipForName(const char* pszName)
{
    int v1 = 0;
    if (bg_iNumWeapClips <= 0)
    {
    LABEL_6:
        Com_DPrintf("Couldn't find ammo clip \"%s\"\n", pszName);
        return 0;
    }
    while (1)
    {
        const char* v2 = bg_szWeapClipNames[v1];
        if (v2 != nullptr && pszName != nullptr
            && ae_stricmpn(v2, pszName, 0x7FFFFFFF) == 0)
            return v1;
        if (++v1 >= bg_iNumWeapClips)
            goto LABEL_6;
    }
}

// ============================================================================
// BG_GivePlayerWeapon / slot + ammo queries - ea: 0x6168A0..0x617150
// ============================================================================
// ea: 0x006168A0
int BG_GivePlayerWeapon(PlayerState* pPS, int iWeaponIndex)
{
    int v3 = 1 << (iWeaponIndex & 0x1F);
    if ((v3 & pPS->weapons[iWeaponIndex >> 5]) != 0)
        return 0;
    weaponFileInfo_t* pWeap = BG_GetInfoForWeapon(iWeaponIndex);
    int weapClass = pWeap->weapClass;
    if (weapClass == WEAPCLASS_TURRET
        || weapClass == WEAPCLASS_NON_PLAYER)
        return 0;
    RegisterItem(iWeaponIndex, 1);
    int v6 = iWeaponIndex >> 5;
    pPS->weapons[v6] = (pPS->weapons[v6] | v3);
    int v9 = ~v3;
    pPS->weaponrechamber[v6] = (pPS->weaponrechamber[v6] & ~v3);
    int slot = pWeap->slot;
    switch (slot)
    {
    case WEAPSLOT_PRIMARY:
    case WEAPSLOT_PRIMARYB:
        if (pPS->weaponslots[1] != 0)
        {
            if (pPS->weaponslots[2] == 0)
                pPS->weaponslots[2] = (char)iWeaponIndex;
        }
        else
        {
            pPS->weaponslots[1] = (char)iWeaponIndex;
        }
        break;
    case WEAPSLOT_PISTOL:
    case WEAPSLOT_GRENADE:
    case WEAPSLOT_SMOKE_GRENADE:
    case WEAPSLOT_INTERACT:
    case WEAPSLOT_BINOCS:
    case WEAPSLOT_SATCHEL:
    case WEAPSLOT_SPECIAL:
        if (pPS->weaponslots[slot] == 0)
            pPS->weaponslots[slot] = (char)iWeaponIndex;
        break;
    default:
        break;
    }
    for (int i = pWeap->iAltWeaponIndex; i != 0;
         i = BG_GetInfoForWeapon(i)->iAltWeaponIndex)
    {
        int iWeaponIndexa = 1 << (i & 0x1F);
        if ((iWeaponIndexa & pPS->weapons[i >> 5]) != 0)
            break;
        RegisterItem(i, 1);
        pPS->weapons[i >> 5] =
            (pPS->weapons[i >> 5] | iWeaponIndexa);
        pPS->weaponrechamber[v6] =
            (pPS->weaponrechamber[v6] & v9);
    }
    return 1;
}

// ea: 0x00616A10
int BG_SetPlayerWeaponForSlot(PlayerState* pPS, int slot, int iWeaponIndex)
{
    if (((1 << (iWeaponIndex & 0x1F)) & pPS->weapons[iWeaponIndex >> 5]) == 0)
        return 0;
    int v3 = BG_GetInfoForWeapon(iWeaponIndex)->slot;
    int v4;
    switch (v3)
    {
    case WEAPSLOT_PRIMARY:
    case WEAPSLOT_PRIMARYB:
        v4 = slot;
        if (slot != 1 && slot != 2)
            return 0;
        break;
    case WEAPSLOT_PISTOL:
    case WEAPSLOT_GRENADE:
    case WEAPSLOT_SMOKE_GRENADE:
    case WEAPSLOT_SATCHEL:
    case WEAPSLOT_SPECIAL:
        v4 = slot;
        if (slot != v3)
            return 0;
        break;
    default:
        return 0;
    }
    pPS->weaponslots[v4] = (char)iWeaponIndex;
    return 1;
}

// ea: 0x00616AA0
weapSlot_t BG_IsPlayerWeaponInSlot(const PlayerState* pPS,
                                   int iWeaponIndex, int bAnyMode)
{
    if (((1 << (iWeaponIndex & 0x1F)) & pPS->weapons[iWeaponIndex >> 5]) == 0)
        return WEAPSLOT_NONE;
    int v4 = iWeaponIndex;
    while (1)
    {
        weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(v4);
        int slot = InfoForWeapon->slot;
        weapSlot_t result;
        switch (slot)
        {
        case WEAPSLOT_PRIMARY:
        case WEAPSLOT_PRIMARYB:
            if (pPS->weaponslots[1] == v4)
            {
                result = WEAPSLOT_PRIMARY;
            }
            else
            {
                if (pPS->weaponslots[2] != v4)
                    goto LABEL_9;
                result = WEAPSLOT_PRIMARYB;
            }
            break;
        case WEAPSLOT_PISTOL:
        case WEAPSLOT_GRENADE:
        case WEAPSLOT_SMOKE_GRENADE:
        case WEAPSLOT_INTERACT:
        case WEAPSLOT_SATCHEL:
        case WEAPSLOT_SPECIAL:
            if (pPS->weaponslots[slot] != v4)
            {
            LABEL_9:
                if (bAnyMode != 0)
                {
                    int iAltWeaponIndex = InfoForWeapon->iAltWeaponIndex;
                    if (iAltWeaponIndex != 0)
                        v4 = iAltWeaponIndex;
                }
                if (v4 != iWeaponIndex)
                    continue;
                goto LABEL_13;
            }
            result = (weapSlot_t)InfoForWeapon->slot;
            break;
        default:
        LABEL_13:
            result = WEAPSLOT_NONE;
            break;
        }
        return result;
    }
}

// ea: 0x00616B70
int BG_GetMaxPickupableAmmo(const PlayerState* pPS, int iWeaponIndex)
{
    if (iWeaponIndex == 0)
        return 0;
    int iAmmoIndex = BG_GetInfoForWeapon(iWeaponIndex)->iAmmoIndex;
    int iClipIndex = BG_GetInfoForWeapon(iWeaponIndex)->iClipIndex;
    int bAmmoCounted[92];
    int bClipCounted[92];
    memset(bAmmoCounted, 0, sizeof(bAmmoCounted));
    memset(bClipCounted, 0, sizeof(bClipCounted));
    weaponFileInfo_t* pWeap = BG_GetInfoForWeapon(iWeaponIndex);
    int iSharedAmmoCapIndex = pWeap->iSharedAmmoCapIndex;
    if (iSharedAmmoCapIndex < 0)
    {
        if (BG_GetInfoForWeapon(iWeaponIndex)->bClipOnly != 0)
            return BG_GetAmmoClipSize(iClipIndex) - pPS->ammoclip[iClipIndex];
        return BG_GetAmmoTypeMax(iAmmoIndex) - pPS->ammo[iAmmoIndex];
    }
    int SharedAmmoCapSize =
        BG_GetSharedAmmoCapSize(iSharedAmmoCapIndex);
    int v7 = 1;
    if (bg_iNumWeapons >= 1)
    {
        while (1)
        {
            if (((1 << (v7 & 0x1F)) & pPS->weapons[v7 >> 5]) != 0
                && BG_GetInfoForWeapon(v7)->iSharedAmmoCapIndex
                    == pWeap->iSharedAmmoCapIndex)
            {
                if (BG_GetInfoForWeapon(v7)->bClipOnly != 0)
                {
                    if (bClipCounted[BG_GetInfoForWeapon(v7)->iClipIndex] == 0)
                    {
                        bClipCounted[BG_GetInfoForWeapon(v7)->iClipIndex] = 1;
                        int v8 = pPS->ammoclip[BG_GetInfoForWeapon(v7)->iClipIndex];
                        SharedAmmoCapSize -= v8;
                    }
                }
                else if (bAmmoCounted[BG_GetInfoForWeapon(v7)->iAmmoIndex] == 0)
                {
                    bAmmoCounted[BG_GetInfoForWeapon(v7)->iAmmoIndex] = 1;
                    int v8 = pPS->ammo[BG_GetInfoForWeapon(v7)->iAmmoIndex];
                    SharedAmmoCapSize -= v8;
                }
            }
            if (++v7 > bg_iNumWeapons)
                return SharedAmmoCapSize;
        }
    }
    return SharedAmmoCapSize;
}

// ea: 0x00616D50
int BG_GetTotalAmmoReserve(const PlayerState* pPS, int iWeaponIndex)
{
    int v2 = 0;
    int iAmmoIndex = BG_GetInfoForWeapon(iWeaponIndex)->iAmmoIndex;
    int iClipIndex = BG_GetInfoForWeapon(iWeaponIndex)->iClipIndex;
    int bAmmoCounted[92];
    int bClipCounted[92];
    memset(bAmmoCounted, 0, sizeof(bAmmoCounted));
    memset(bClipCounted, 0, sizeof(bClipCounted));
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(iWeaponIndex);
    int iSharedAmmoCapIndex = InfoForWeapon->iSharedAmmoCapIndex;
    weaponFileInfo_t* pWeap = InfoForWeapon;
    if (iSharedAmmoCapIndex < 0)
    {
        if (BG_GetInfoForWeapon(iWeaponIndex)->bClipOnly != 0)
            return pPS->ammoclip[iClipIndex];
        return pPS->ammo[iAmmoIndex];
    }
    for (int i = 1; i <= bg_iNumWeapons; ++i)
    {
        if (((1 << (i & 0x1F)) & pPS->weapons[i >> 5]) == 0)
            continue;
        weaponFileInfo_t* v6 = BG_GetInfoForWeapon(i);
        if (v6->iSharedAmmoCapIndex == pWeap->iSharedAmmoCapIndex)
        {
            if (BG_GetInfoForWeapon(i)->bClipOnly != 0)
            {
                if (bClipCounted[BG_GetInfoForWeapon(i)->iClipIndex] == 0)
                {
                    bClipCounted[BG_GetInfoForWeapon(i)->iClipIndex] = 1;
                    v2 += pPS->ammoclip[BG_GetInfoForWeapon(i)->iClipIndex];
                }
            }
            else if (bAmmoCounted[BG_GetInfoForWeapon(i)->iAmmoIndex] == 0)
            {
                bAmmoCounted[BG_GetInfoForWeapon(i)->iAmmoIndex] = 1;
                v2 += pPS->ammo[BG_GetInfoForWeapon(i)->iAmmoIndex];
            }
        }
    }
    return v2;
}

// ea: 0x00616F10
int BG_GetTotalAmmo(const PlayerState* pPS, int iWeaponIndex)
{
    int v2 = 0;
    int iAmmoIndex = BG_GetInfoForWeapon(iWeaponIndex)->iAmmoIndex;
    int iClipIndex = BG_GetInfoForWeapon(iWeaponIndex)->iClipIndex;
    int bAmmoCounted[92];
    int bClipCounted[92];
    memset(bAmmoCounted, 0, sizeof(bAmmoCounted));
    memset(bClipCounted, 0, sizeof(bClipCounted));
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(iWeaponIndex);
    int iSharedAmmoCapIndex = InfoForWeapon->iSharedAmmoCapIndex;
    weaponFileInfo_t* pWeap = InfoForWeapon;
    if (iSharedAmmoCapIndex < 0)
    {
        if (BG_GetInfoForWeapon(iWeaponIndex)->bClipOnly != 0)
            return pPS->ammoclip[iClipIndex];
        return pPS->ammo[iAmmoIndex] + pPS->ammoclip[iClipIndex];
    }
    int v5 = 1;
    if (bg_iNumWeapons >= 1)
    {
        while (1)
        {
            if (((1 << (v5 & 0x1F)) & pPS->weapons[v5 >> 5]) != 0)
            {
                weaponFileInfo_t* v6 = BG_GetInfoForWeapon(v5);
                if (v6->iSharedAmmoCapIndex != pWeap->iSharedAmmoCapIndex)
                    goto LABEL_13;
                if (BG_GetInfoForWeapon(v5)->bClipOnly == 0)
                    break;
                if (bClipCounted[BG_GetInfoForWeapon(v5)->iClipIndex] == 0)
                {
                    bClipCounted[BG_GetInfoForWeapon(v5)->iClipIndex] = 1;
                    v2 += pPS->ammoclip[BG_GetInfoForWeapon(v5)->iClipIndex];
                }
            }
        LABEL_13:
            if (++v5 > bg_iNumWeapons)
                return v2;
        }
        if (bClipCounted[BG_GetInfoForWeapon(v5)->iClipIndex] == 0)
        {
            bClipCounted[BG_GetInfoForWeapon(v5)->iClipIndex] = 1;
            v2 += pPS->ammoclip[BG_GetInfoForWeapon(v5)->iClipIndex];
        }
        if (bAmmoCounted[BG_GetInfoForWeapon(v5)->iAmmoIndex] != 0)
            goto LABEL_13;
        bAmmoCounted[BG_GetInfoForWeapon(v5)->iAmmoIndex] = 1;
        v2 += pPS->ammo[BG_GetInfoForWeapon(v5)->iAmmoIndex];
        goto LABEL_13;
    }
    return v2;
}

// ============================================================================
// ADS / weapon interrupt helpers - ea: 0x617120..0x617250
// ============================================================================

// ea: 0x00607EC0
void PM_StartWeaponAnim(int anim)
{
    pmove_t* v2 = pm;
    if (pm->ps->pm_type < 6 && pm->cmd.weapon != 0)
    {
        if (pml.pWeap == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
            AeAssert::gCurrentLine = 3140;
            AeAssert::gCurrentExpr = "pml.pWeap";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
            v2 = pm;
        }
        if (((weaponFileInfo_t*)pml.pWeap)->type == WEAPTYPE_GAS)
        {
            v2->ps->weapAnim = anim;
        }
        else
        {
            if (anim == 0 && v2->ps->fWeaponPosFrac > 0.89999998f)
                anim = 23;
            v2->ps->weapAnim = anim | ~v2->ps->weapAnim & 0x200;
        }
    }
}

// ea: 0x00607F80
void PM_ContinueWeaponAnim(int anim)
{
    if (pm->cmd.weapon != 0)
    {
        if (anim == 0 && pm->ps->fWeaponPosFrac > 0.89999998f)
            anim = 23;
        if ((pm->ps->weapAnim & 0xFFFFFDFF) != anim)
            PM_StartWeaponAnim(anim);
    }
}

// ea: 0x00617120
bool PM_CanStartADSAnim()
{
    int weaponstate = pm->ps->weaponstate;
    return weaponstate != 5
        && weaponstate != 6
        && weaponstate != 7
        && weaponstate != 8
        && weaponstate != 9
        && weaponstate != 14
        && weaponstate != 10
        && weaponstate != 11
        && weaponstate != 2
        && weaponstate != 4
        && weaponstate != 12
        && weaponstate != 13
        && (((weaponFileInfo_t*)pml.pWeap)->bRechamberWhileAds == 0
            || ((weaponFileInfo_t*)pml.pWeap)->bBoltAction == 0
            || Com_BitCheck(pm->ps->weaponrechamber, pm->ps->weapon) == 0);
}

// ea: 0x006171B0
int PM_InteruptWeaponWithProneMove()
{
    if ((pm->ps->pm_flags & 0x20) == 0
        || BG_GetInfoForWeapon(pm->ps->weapon)->weapClass != WEAPCLASS_LMG)
    {
        unsigned int weaponstate = pm->ps->weaponstate;
        if (weaponstate <= 2
            || weaponstate == 5
            || weaponstate == 7
            || weaponstate == 9
            || weaponstate == 8
            || weaponstate == 6
            || weaponstate == 4)
            return 1;
        if (weaponstate != 3 && weaponstate != 11)
        {
            pm->ps->weaponTime = 0;
            pm->ps->weaponDelay = 0;
            pm->ps->weaponstate = 0;
            PM_ContinueWeaponAnim(0);
            return 1;
        }
    }
    return 0;
}

// ea: 0x00617250
int PM_InteruptWeaponWithSprintMove()
{
    unsigned int weaponstate = pm->ps->weaponstate;
    if (weaponstate <= 2
        || weaponstate == 5
        || weaponstate == 7
        || weaponstate == 9
        || weaponstate == 8
        || weaponstate == 6
        || weaponstate == 4)
        return 1;
    if (weaponstate != 3 && weaponstate != 10 && weaponstate != 11)
    {
        pm->ps->weaponTime = 0;
        pm->ps->weaponDelay = 0;
        pm->ps->weaponstate = 0;
        if (pm->cmd.weapon != 0 && (pm->ps->weapAnim & 0xFFFFFDFF) != 0xA)
            PM_StartWeaponAnim(10);
        return 1;
    }
    return 0;
}

// ============================================================================
// Prone movement checks - ea: 0x6134F0..0x615B50 (bg_misc.cpp)
// ============================================================================
extern float AngleNormalize180Accurate(float angle);  // core.o 0x4BFD90
extern float vectopitch(const float* vec);            // core.o q_math.cpp
extern float AngleNormalize360Accurate(float angle);  // core.o 0x4BFD90
extern float AngleDelta(float angle1, float angle2);  // core.o q_math.cpp
extern vmCvar_t bg_prone_yawcap;   // ?bg_prone_yawcap@@3UvmCvar_t@@A (game.o)
extern vmCvar_t bg_lmg_yawcap;     // ?bg_lmg_yawcap@@3UvmCvar_t@@A (game.o)
extern vmCvar_t bg_ladder_yawcap;  // ?bg_ladder_yawcap@@3UvmCvar_t@@A (game.o)
extern void PM_UpdateStickyAim(PlayerState* ps, usercmd_s* cmd,
                               usercmd_s* oldcmd);  // game.o 0x62E070

// ea: 0x00615200
int BG_CheckProneTurned(
    PlayerState* ps, int a2, float a3,
    void (__cdecl* a4)(trace_t*, const math::Position3*,
                       const math::Position3*, const math::Position3*,
                       const math::Position3*, const collision_context_t&))
{
    float v4 = AngleDelta(a3, ps->viewangles[1]);
    float v11 = fabs(v4) * 0.0041666669f;
    float v10 = 1.0f - v11;
    float v12 = AngleNormalize360Accurate(a3 - (v10 * v4));
    unsigned int mVal = ps->mGroundEntity.mHandle.mVal;
    math::Dir3 v8;
    v8.v.m128_f32[0] = 0.0f;
    v8.v.m128_f32[1] = 0.0f;
    v8.v.m128_f32[2] = 0.69999999f;
    v8.v.m128_f32[3] = 0.0f;
    float v6 = ps->maxs[0];
    return BG_CheckProneValid(
        ps->mClient, &ps->origin, v6, 30.0f, v12, &ps->fTorsoHeight,
        &ps->fTorsoPitch, &ps->fWaistPitch, 1, mVal != 0, &v8, a4,
        nullptr, nullptr, PCT_CLIENT,
        ((1.0f - v11) * 60.0f) + (v11 * 45.0f));
}

// ea: 0x006134F0
int BG_CheckProneValid(
    DbLinkedHandle<EntityHandleDb, Entity> passEntity,
    const math::Position3* vPos, float fSize, float fHeight, float fYaw,
    float* pfTorsoHeight, float* pfTorsoPitch, float* pfWaistPitch,
    int bAlreadyProne, int bOnGround, const math::Dir3* vGroundNormal,
    void (__cdecl* traceFunc)(trace_t*, const math::Position3*,
                              const math::Position3*, const math::Position3*,
                              const math::Position3*, const collision_context_t&),
    void (__cdecl* boxTraceFunc)(trace_t*, const math::Position3*,
                                 const math::Position3*, const math::Position3*,
                                 const math::Position3*, const collision_context_t&),
    int (__cdecl* pointcontents)(const math::Position3*,
                                 const collision_context_t&),
    proneCheckType_t proneCheckType, float prone_feet_dist)
{
    int v17 = 0;
    trace_t trace;
    math::Position3 vMaxs;
    math::Position3 vMins;
    math::Position3 vEnd;
    math::Position3 point;
    float v54[3];
    float vRight[3];
    float vUp[3];
    collision_context_t context;
    float fTraceHeight;
    float fLegsPitch;
    float fFirstTraceDist;
    float vForward[3];
    float vTorsoPos[3];
    float v69;
    float fTorsoPitch;
    float fPitchDiff;
    float vDelta[3];
    float vFeetPos[3];
    float vWaistPos[3];
    float angle;
    float anglea;
    int v31;
    float v59;
    float v60;
    float v61;
    int integer;
    void (__cdecl* v36)(trace_t*, const math::Position3*, const math::Position3*,
                        const math::Position3*, const math::Position3*,
                        const collision_context_t&);

    trace.surfaceFlags = 0;
    trace.contents = 0;
    if (traceFunc == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_misc.cpp";
        AeAssert::gCurrentLine = 1226;
        AeAssert::gCurrentExpr = "traceFunc != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        v17 = 0;
    }
    float v18 = fSize;
    float v19 = vPos->v.m128_f32[0];
    float v20 = 0.0f - fSize;
    vMaxs.v.m128_f32[2] = (0.0f - fSize) + vPos->v.m128_f32[1];
    vMaxs.v.m128_f32[3] = vPos->v.m128_f32[2];
    vTorsoPos[0] = vPos->v.m128_f32[0] + fSize;
    vTorsoPos[1] = fSize + vPos->v.m128_f32[1];
    float v21 = fHeight + vPos->v.m128_f32[2];
    vMins.v.m128_f32[3] = 0.0f - fSize;
    vMaxs.v.m128_f32[1] = v19 + (0.0f - fSize);
    vTorsoPos[2] = v21;
    if (g_debugProneCheck.integer != 0)
    {
        G_DebugBox(&vMaxs.v.m128_f32[1], vTorsoPos, colorMdCyan,
                   g_debugProneCheckDepthCheck.integer, 1, 0);
        v20 = vMins.v.m128_f32[3];
        v18 = fSize;
        v17 = 0;
    }
    if (bAlreadyProne == 0)
    {
        if (boxTraceFunc == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_misc.cpp";
            AeAssert::gCurrentLine = 1245;
            AeAssert::gCurrentExpr = "boxTraceFunc != 0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
            v20 = vMins.v.m128_f32[3];
            v18 = fSize;
        }
        vMaxs.v.m128_f32[1] = v20;
        vMaxs.v.m128_f32[2] = v20;
        vMaxs.v.m128_f32[3] = 0.0f;
        vTorsoPos[2] = fHeight;
        vEnd.v.m128_f32[1] = vPos->v.m128_f32[0];
        vEnd.v.m128_f32[2] = vPos->v.m128_f32[1];
        float v22 = vPos->v.m128_f32[2];
        collision_context_t boxContext;
        memset(&boxContext, 0, sizeof(boxContext));
        boxContext.__vftable =
            (collision_context_t_vtbl*)0x00CD8F6C;
        boxContext.pass_entity1 = passEntity;
        boxContext.contentmask =
            0x81002F + 2 + (proneCheckType != PCT_CLIENT ? 0xFFE0 : 0);
        point.v.m128_f32[0] = 0.0f;
        point.v.m128_f32[1] = 0.0f;
        vEnd.v.m128_f32[3] = v22;
        vWaistPos[0] = vPos->v.m128_f32[0];
        vWaistPos[1] = vPos->v.m128_f32[1];
        float v23 = vPos->v.m128_f32[2] + 10.0f;
        vTorsoPos[0] = v18;
        vTorsoPos[1] = v18;
        vWaistPos[2] = v23;
        boxTraceFunc(&trace, (const math::Position3*)&vEnd.v.m128_f32[1],
                     (const math::Position3*)&vMaxs.v.m128_f32[1],
                     (const math::Position3*)vTorsoPos,
                     (const math::Position3*)vWaistPos, boxContext);
        if ((trace.mEntity.mHandle.mVal & 0xFF) != 0)
            return 0;
        v17 = 0;
    }
    if (proneCheckType == PCT_CLIENT && pointcontents != nullptr)
    {
        player_collision_context_t water_context;
        memset(&water_context, 0, sizeof(water_context));
        water_context.__vftable =
            (collision_context_t_vtbl*)0x00CD8F78;
        water_context.pass_owner2 = passEntity;
        water_context.contentmask = 32;
        point.v.m128_f32[0] = vPos->v.m128_f32[0];
        point.v.m128_f32[1] = vPos->v.m128_f32[1];
        point.v.m128_f32[2] = vPos->v.m128_f32[2] + 6.0f;
        v17 = pointcontents(&point, water_context);
        if (v17 != 0)
            return 0;
    }
    if (bOnGround != v17
        && (((vGroundNormal->v.m128_f32[0] * vGroundNormal->v.m128_f32[0])
             + (vGroundNormal->v.m128_f32[1]
                * vGroundNormal->v.m128_f32[1]))
            + (vGroundNormal->v.m128_f32[2]
               * vGroundNormal->v.m128_f32[2])) > 0.1f
        && vGroundNormal->v.m128_f32[2] < 0.69999999f)
        return 0;
    vMaxs.v.m128_f32[1] = -6.0f;
    vMaxs.v.m128_f32[2] = -6.0f;
    vMaxs.v.m128_f32[3] = -6.0f;
    vTorsoPos[0] = 6.0f;
    vTorsoPos[1] = 6.0f;
    vTorsoPos[2] = 6.0f;
    vWaistPos[0] = 0.0f;
    vWaistPos[1] = fYaw - 180.0f;
    vWaistPos[2] = 0.0f;
    AngleVectors((const math::Position3*)vWaistPos, &fTraceHeight, v54,
                 vRight);
    float v26 = vPos->v.m128_f32[0];
    float v27 = vPos->v.m128_f32[1];
    float v28 = vPos->v.m128_f32[2] + (fHeight - 6.0f);
    vUp[2] = 0.0f;
    memset(&context, 0, sizeof(context));
    context.__vftable = (collision_context_t_vtbl*)0x00CD8F6C;
    context.pass_entity1 = passEntity;
    v59 = fHeight - 6.0f;
    vEnd.v.m128_f32[1] = v26;
    v60 = prone_feet_dist - 6.0f;
    vEnd.v.m128_f32[2] = v27;
    vEnd.v.m128_f32[3] = v28;
    vWaistPos[0] = ((prone_feet_dist - 6.0f) * fTraceHeight) + v26;
    vWaistPos[1] = ((prone_feet_dist - 6.0f) * fLegsPitch) + v27;
    vWaistPos[2] = ((prone_feet_dist - 6.0f) * fFirstTraceDist) + v28;
    context.pass_entity2.mHandle.mVal =
        0x81002F + 2 + (proneCheckType != PCT_CLIENT ? 0xFFE0 : 0);
    traceFunc(&trace, (const math::Position3*)&vEnd.v.m128_f32[1], (const math::Position3*)&vMaxs.v.m128_f32[1], (const math::Position3*)vTorsoPos,
              (const math::Position3*)vWaistPos, context);
    integer = g_debugProneCheck.integer;
    if (g_debugProneCheck.integer != 0)
    {
        G_DebugCircleEx(&vEnd.v.m128_f32[1], 6.0f, v54, colorMdCyan,
                        g_debugProneCheckDepthCheck.integer, 1);
        integer = g_debugProneCheck.integer;
        if (g_debugProneCheck.integer != 0)
        {
            G_DebugCircleEx(&vEnd.v.m128_f32[1], 6.0f, vRight, colorMdCyan,
                            g_debugProneCheckDepthCheck.integer, 1);
            integer = g_debugProneCheck.integer;
        }
    }
    if (trace.normal.v.m128_f32[1] >= 1.0f)
    {
        if (integer != 0)
            G_DebugLine(&vEnd.v.m128_f32[1], &trace.endpos.v.m128_f32[0],
                        colorGreen, g_debugProneCheckDepthCheck.integer, 1);
        v31 = (int)vMins.v.m128_f32[2];
        goto LABEL_45;
    }
    float v30 = (v60 * trace.normal.v.m128_f32[1]) + 6.0f;
    if (bOnGround == 0 || (v31 = 1, v61 = v30, (fSize + 2.0f) > v30))
    {
        if (integer != 0)
            G_DebugLine(&vEnd.v.m128_f32[1], &trace.endpos.v.m128_f32[0],
                        colorRed, g_debugProneCheckDepthCheck.integer, 1);
        return 0;
    }
    vMins.v.m128_f32[2] = (v59 * 0.69999999f) + 24.0f;
    if (vMins.v.m128_f32[2] > v30)
    {
        if (integer != 0)
            G_DebugLine(&vEnd.v.m128_f32[1], &trace.endpos.v.m128_f32[0],
                        colorRed, g_debugProneCheckDepthCheck.integer, 1);
        v69 = vWaistPos[0] - vEnd.v.m128_f32[1];
        vWaistPos[2] = vWaistPos[2] + 22.0f;
        v31 = 0;
        fTorsoPitch = vWaistPos[1] - vEnd.v.m128_f32[2];
        fPitchDiff = vWaistPos[2] - vEnd.v.m128_f32[3];
        vMins.v.m128_f32[3] =
            VectorNormalize2(&v69, &fTraceHeight);
        traceFunc(&trace, (const math::Position3*)&vEnd.v.m128_f32[1], (const math::Position3*)&vMaxs.v.m128_f32[1], (const math::Position3*)vTorsoPos, (const math::Position3*)vWaistPos, context);
        integer = g_debugProneCheck.integer;
        if (trace.normal.v.m128_f32[1] < 1.0f)
        {
            v31 = 1;
            v61 = (vMins.v.m128_f32[3] * trace.normal.v.m128_f32[1]) + 6.0f;
            if (vMins.v.m128_f32[2] > v61)
            {
                if (g_debugProneCheck.integer != 0)
                {
                    G_DebugLine(&vEnd.v.m128_f32[1],
                                &trace.endpos.v.m128_f32[0], colorRed,
                                g_debugProneCheckDepthCheck.integer, 1);
                    return 0;
                }
                return 0;
            }
            goto LABEL_38;
        }
        if (g_debugProneCheck.integer != 0)
            G_DebugLine(&vEnd.v.m128_f32[1], &trace.endpos.v.m128_f32[0],
                        colorGreen, g_debugProneCheckDepthCheck.integer, 1);
    LABEL_45:
        v61 = prone_feet_dist;
        goto LABEL_46;
    }
LABEL_38:
    if (integer != 0)
        G_DebugLine(&vEnd.v.m128_f32[1], &trace.endpos.v.m128_f32[0],
                    colorYellow, g_debugProneCheckDepthCheck.integer, 1);
LABEL_46:
    float v39, v40, v41;
    vDelta[0] = trace.endpos.v.m128_f32[0];
    vDelta[1] = trace.endpos.v.m128_f32[1];
    float v33 = (fTraceHeight * 24.0f) + vPos->v.m128_f32[0];
    vDelta[2] = trace.endpos.v.m128_f32[2];
    vEnd.v.m128_f32[1] = v33;
    vWaistPos[0] = v33;
    float v34 = (fLegsPitch * 24.0f) + vPos->v.m128_f32[1];
    vEnd.v.m128_f32[3] = ((fFirstTraceDist * 24.0f) + vPos->v.m128_f32[2]) + v59;
    vEnd.v.m128_f32[2] = v34;
    vWaistPos[1] = v34;
    vMins.v.m128_f32[2] = ((fSize * 2.5f) + v59) - 6.0f;
    vWaistPos[2] = vEnd.v.m128_f32[3] - vMins.v.m128_f32[2];
    traceFunc(&trace, (const math::Position3*)&vEnd.v.m128_f32[1], (const math::Position3*)&vMaxs.v.m128_f32[1], (const math::Position3*)vTorsoPos,
              (const math::Position3*)vWaistPos, context);
    if (trace.normal.v.m128_f32[1] == 1.0f)
    {
        if (g_debugProneCheck.integer != 0)
            G_DebugLine(&vEnd.v.m128_f32[1], &trace.endpos.v.m128_f32[0],
                        colorRed, g_debugProneCheckDepthCheck.integer, 1);
        goto fail;
    }
    if (trace.normal.v.m128_f32[2] < 0.69999999f)
        return 0;
    int v35 = g_debugProneCheck.integer;
    if (g_debugProneCheck.integer != 0)
    {
        G_DebugLine(&vEnd.v.m128_f32[1], &trace.endpos.v.m128_f32[0],
                    colorGreen, g_debugProneCheckDepthCheck.integer, 1);
        v35 = g_debugProneCheck.integer;
    }
    vFeetPos[0] = trace.endpos.v.m128_f32[0];
    vFeetPos[1] = trace.endpos.v.m128_f32[1];
    vFeetPos[2] = trace.endpos.v.m128_f32[2];
    if (v31 != 0)
    {
        if ((((vMins.v.m128_f32[2] * trace.normal.v.m128_f32[1]) + 6.0f)
             * -0.75f)
            > (v61 - ((vMins.v.m128_f32[2] * trace.normal.v.m128_f32[1])
                      + 6.0f)))
        {
            if (v35 != 0)
                G_DebugLine(vDelta, vFeetPos, colorRed,
                            g_debugProneCheckDepthCheck.integer, 1);
            goto fail;
        }
        if (v35 != 0)
            G_DebugLine(vDelta, vFeetPos, colorMdCyan,
                        g_debugProneCheckDepthCheck.integer, 1);
        fTorsoPitch = (fLegsPitch * 6.0f) + (vDelta[1] - vFeetPos[1]);
        v69 = (fTraceHeight * 6.0f) + (vDelta[0] - vFeetPos[0]);
        fPitchDiff = ((fFirstTraceDist * 6.0f) + (vDelta[2] - vFeetPos[2]))
            + 6.0f;
        VectorNormalize(&v69);
        v36 = traceFunc;
        float v37 = (v60 * fTraceHeight) + vPos->v.m128_f32[0];
        float v38 = (v60 * fLegsPitch) + vPos->v.m128_f32[1];
        vWaistPos[2] = ((v60 - 24.0f) * fPitchDiff) + vEnd.v.m128_f32[3];
        vWaistPos[0] = (v37 + (((v60 - 24.0f) * v69) + vEnd.v.m128_f32[1]))
            * 0.5f;
        vWaistPos[1] =
            (v38 + (((v60 - 24.0f) * fTorsoPitch) + vEnd.v.m128_f32[2]))
            * 0.5f;
        traceFunc(&trace, (const math::Position3*)&vEnd.v.m128_f32[1], (const math::Position3*)&vMaxs.v.m128_f32[1], (const math::Position3*)vTorsoPos, (const math::Position3*)vWaistPos, context);
        if (trace.normal.v.m128_f32[1] < 1.0f)
        {
            if (g_debugProneCheck.integer != 0)
                G_DebugLine(&vEnd.v.m128_f32[1], &trace.endpos.v.m128_f32[0],
                            colorRed, g_debugProneCheckDepthCheck.integer, 1);
            vEnd.v.m128_f32[1] = trace.endpos.v.m128_f32[0];
            vEnd.v.m128_f32[2] = trace.endpos.v.m128_f32[1];
            vEnd.v.m128_f32[3] = trace.endpos.v.m128_f32[2] + 18.0f;
            vWaistPos[2] = vWaistPos[2] + 18.0f;
            traceFunc(&trace, (const math::Position3*)&vEnd.v.m128_f32[1], (const math::Position3*)&vMaxs.v.m128_f32[1], (const math::Position3*)vTorsoPos, (const math::Position3*)vWaistPos, context);
            if (trace.normal.v.m128_f32[1] < 1.0f)
                goto LABEL_71;
        }
        if (g_debugProneCheck.integer != 0)
            G_DebugLine(&vEnd.v.m128_f32[1], &trace.endpos.v.m128_f32[0],
                        colorGreen, g_debugProneCheckDepthCheck.integer, 1);
        v39 = trace.endpos.v.m128_f32[0];
        v40 = trace.endpos.v.m128_f32[1];
        v41 = trace.endpos.v.m128_f32[2];
        vDelta[0] = trace.endpos.v.m128_f32[0];
        vDelta[1] = trace.endpos.v.m128_f32[1];
        vDelta[2] = trace.endpos.v.m128_f32[2];
    }
    else
    {
        v41 = vDelta[2];
        v40 = vDelta[1];
        v39 = vDelta[0];
        v36 = traceFunc;
    }
    vEnd.v.m128_f32[2] = v40;
    vWaistPos[1] = v40;
    vEnd.v.m128_f32[3] = v41;
    vEnd.v.m128_f32[1] = v39;
    vWaistPos[0] = v39;
    vWaistPos[2] = v41 - (((v41 - vFeetPos[2]) * 2.0f) + fSize);
    v36(&trace, (const math::Position3*)&vEnd.v.m128_f32[1], (const math::Position3*)&vMaxs.v.m128_f32[1], (const math::Position3*)vTorsoPos,
        (const math::Position3*)vWaistPos, context);
    if (trace.normal.v.m128_f32[1] == 1.0f)
        goto LABEL_71;
    if (trace.normal.v.m128_f32[2] < 0.69999999f)
        return 0;
    if (g_debugProneCheck.integer != 0)
        G_DebugLine(&vEnd.v.m128_f32[1], &trace.endpos.v.m128_f32[0],
                    colorGreen, g_debugProneCheckDepthCheck.integer, 1);
    vDelta[0] = trace.endpos.v.m128_f32[0];
    vDelta[1] = trace.endpos.v.m128_f32[1];
    vDelta[2] = trace.endpos.v.m128_f32[2];
    vEnd.v.m128_f32[1] = vPos->v.m128_f32[0];
    vEnd.v.m128_f32[2] = vPos->v.m128_f32[1];
    vEnd.v.m128_f32[3] = vPos->v.m128_f32[2] + v59;
    vWaistPos[0] = vPos->v.m128_f32[0];
    vWaistPos[1] = vPos->v.m128_f32[1];
    vWaistPos[2] = vPos->v.m128_f32[2] - (fSize * 1.5f);
    v36(&trace, (const math::Position3*)&vEnd.v.m128_f32[1], (const math::Position3*)&vMaxs.v.m128_f32[1], (const math::Position3*)vTorsoPos,
        (const math::Position3*)vWaistPos, context);
    if (trace.normal.v.m128_f32[1] == 1.0f)
        goto LABEL_71;
    if (trace.normal.v.m128_f32[2] < 0.69999999f)
        return 0;
    if (g_debugProneCheck.integer != 0)
        G_DebugLine(&vEnd.v.m128_f32[1], &trace.endpos.v.m128_f32[0],
                    colorGreen, g_debugProneCheckDepthCheck.integer, 1);
    vForward[0] = trace.endpos.v.m128_f32[0];
    fTorsoPitch = vFeetPos[1] - trace.endpos.v.m128_f32[1];
    vForward[1] = trace.endpos.v.m128_f32[1];
    vForward[2] = trace.endpos.v.m128_f32[2];
    v69 = vFeetPos[0] - trace.endpos.v.m128_f32[0];
    fPitchDiff = vFeetPos[2] - trace.endpos.v.m128_f32[2];
    vMins.v.m128_f32[2] = vectopitch(&v69);
    v69 = vDelta[0] - vFeetPos[0];
    fTorsoPitch = vDelta[1] - vFeetPos[1];
    fPitchDiff = vDelta[2] - vFeetPos[2];
    double v44 = vectopitch(&v69);
    v60 = (float)v44;
    AngleSubtract(v60, vMins.v.m128_f32[2]);
    vMins.v.m128_f32[3] = (float)v44;
    if (v44 < -50.0 || vMins.v.m128_f32[3] > 70.0)
    {
        if (g_debugProneCheck.integer == 0)
            goto fail;
        G_DebugLine(vForward, vFeetPos, colorMagenta,
                    g_debugProneCheckDepthCheck.integer, 1);
        if (g_debugProneCheck.integer == 0)
            goto fail;
        G_DebugLine(vFeetPos, vDelta, colorMagenta,
                    g_debugProneCheckDepthCheck.integer, 1);
        goto fail;
    }
    memset(&vMaxs.v.m128_f32[1], 0, 12);
    memset(vTorsoPos, 0, sizeof(vTorsoPos));
    vEnd.v.m128_f32[1] = vForward[0];
    vEnd.v.m128_f32[2] = vForward[1];
    vEnd.v.m128_f32[3] = vForward[2] + 5.0f;
    vWaistPos[0] = vFeetPos[0];
    vWaistPos[1] = vFeetPos[1];
    vWaistPos[2] = vFeetPos[2] + 5.0f;
    v36(&trace, (const math::Position3*)&vEnd.v.m128_f32[1], (const math::Position3*)&vMaxs.v.m128_f32[1], (const math::Position3*)vTorsoPos,
        (const math::Position3*)vWaistPos, context);
    int v45 = g_debugProneCheck.integer;
    if (trace.normal.v.m128_f32[1] >= 1.0f)
    {
        if (g_debugProneCheck.integer != 0)
            G_DebugLine(&vEnd.v.m128_f32[1], vWaistPos, colorGreen,
                        g_debugProneCheckDepthCheck.integer, 1);
        vEnd.v.m128_f32[1] = vWaistPos[0];
        vEnd.v.m128_f32[2] = vWaistPos[1];
        vEnd.v.m128_f32[3] = vWaistPos[2];
        vWaistPos[0] = vDelta[0];
        vWaistPos[1] = vDelta[1];
        vWaistPos[2] = vDelta[2] + 5.0f;
        v36(&trace, (const math::Position3*)&vEnd.v.m128_f32[1], (const math::Position3*)&vMaxs.v.m128_f32[1], (const math::Position3*)vTorsoPos,
            (const math::Position3*)vWaistPos, context);
        v45 = g_debugProneCheck.integer;
        if (trace.normal.v.m128_f32[1] >= 1.0f)
        {
            if (g_debugProneCheck.integer != 0)
            {
                G_DebugLine(&vEnd.v.m128_f32[1], vWaistPos, colorGreen,
                            g_debugProneCheckDepthCheck.integer, 1);
                if (g_debugProneCheck.integer != 0)
                {
                    G_DebugCircleEx(vForward, 6.0f, v54, colorMdCyan,
                                    g_debugProneCheckDepthCheck.integer, 1);
                    if (g_debugProneCheck.integer != 0)
                    {
                        G_DebugCircleEx(vForward, 6.0f, vRight, colorMdCyan,
                                        g_debugProneCheckDepthCheck.integer,
                                        1);
                        if (g_debugProneCheck.integer != 0)
                        {
                            G_DebugCircleEx(vFeetPos, 6.0f, v54, colorMdCyan,
                                            g_debugProneCheckDepthCheck.integer,
                                            1);
                            if (g_debugProneCheck.integer != 0)
                            {
                                G_DebugCircleEx(vFeetPos, 6.0f, vRight,
                                                colorMdCyan,
                                                g_debugProneCheckDepthCheck.integer,
                                                1);
                                if (g_debugProneCheck.integer != 0)
                                {
                                    G_DebugCircleEx(vDelta, 6.0f, v54,
                                                    colorMdCyan,
                                                    g_debugProneCheckDepthCheck.integer,
                                                    1);
                                    if (g_debugProneCheck.integer != 0)
                                    {
                                        G_DebugCircleEx(vDelta, 6.0f, vRight,
                                                        colorMdCyan,
                                                        g_debugProneCheckDepthCheck.integer,
                                                        1);
                                        if (g_debugProneCheck.integer != 0)
                                        {
                                            G_DebugLine(vForward, vFeetPos,
                                                        colorCyan,
                                                        g_debugProneCheckDepthCheck.integer,
                                                        1);
                                            if (g_debugProneCheck.integer
                                                != 0)
                                                G_DebugLine(
                                                    vFeetPos, vDelta, colorCyan,
                                                    g_debugProneCheckDepthCheck.integer,
                                                    1);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            float v46 = vForward[2];
            if (pfTorsoHeight != nullptr)
                *pfTorsoHeight = (vForward[2] - vPos->v.m128_f32[2]) - 6.0f;
            if (pfTorsoPitch != nullptr)
            {
                v69 = vForward[0] - vFeetPos[0];
                fTorsoPitch = vForward[1] - vFeetPos[1];
                fPitchDiff = v46 - vFeetPos[2];
                angle = vectopitch(&v69);
                *pfTorsoPitch = AngleNormalize180(angle);
            }
            if (pfWaistPitch != nullptr)
            {
                v69 = vFeetPos[0] - vDelta[0];
                fTorsoPitch = vFeetPos[1] - vDelta[1];
                fPitchDiff = vFeetPos[2] - vDelta[2];
                anglea = vectopitch(&v69);
                *pfWaistPitch = AngleNormalize180(anglea);
            }
            return 1;
        }
    }
    if (v45 != 0)
        G_DebugLine(&vEnd.v.m128_f32[1], vWaistPos, colorRed,
                    g_debugProneCheckDepthCheck.integer, 1);
fail:
    if (bOnGround != 0)
        return 0;
    if (pfTorsoHeight != nullptr)
        *pfTorsoHeight = 0.0f;
    if (pfTorsoPitch != nullptr)
        *pfTorsoPitch = 0.0f;
    if (pfWaistPitch != nullptr)
        *pfWaistPitch = 0.0f;
    return 1;
LABEL_71:
    if (g_debugProneCheck.integer != 0)
    {
        G_DebugLine(&vEnd.v.m128_f32[1], &trace.endpos.v.m128_f32[0], colorRed,
                    g_debugProneCheckDepthCheck.integer, 1);
    }
    goto fail;
}

// ============================================================================
// BG_CheckProne - ea: 0x6146F0 (tail-calls BG_CheckProneValid)
// ============================================================================
// ea: 0x006146F0
int BG_CheckProne(
    DbLinkedHandle<EntityHandleDb, Entity> passEntity,
    const math::Position3* vPos, float fSize, float fHeight, float fYaw,
    float* pfTorsoHeight, float* pfTorsoPitch, float* pfWaistPitch,
    int bAlreadyProne, int bOnGround, const math::Dir3* vGroundNormal,
    void (__cdecl* traceFunc)(trace_t*, const math::Position3*,
                              const math::Position3*, const math::Position3*,
                              const math::Position3*, const collision_context_t&),
    void (__cdecl* boxTraceFunc)(trace_t*, const math::Position3*,
                                 const math::Position3*, const math::Position3*,
                                 const math::Position3*, const collision_context_t&),
    int (__cdecl* pointcontents)(const math::Position3*,
                                 const collision_context_t&),
    proneCheckType_t proneCheckType, float prone_feet_dist)
{
    return BG_CheckProneValid(
        passEntity, vPos, fSize, fHeight, fYaw, pfTorsoHeight, pfTorsoPitch,
        pfWaistPitch, bAlreadyProne, bOnGround, vGroundNormal, traceFunc,
        boxTraceFunc, pointcontents, proneCheckType, prone_feet_dist);
}

// ea: 0x00615B50
int PM_VerifyPronePosition(const math::Position3& vFallbackOrg,
                           const math::Position3& vFallbackVel)
{
    PlayerState* ps = pm->ps;
    if ((pm->ps->pm_flags & 1) == 0)
        return 1;
    float proneDirection = ps->proneDirection;
    math::Dir3 v9;
    v9.v.m128_f32[0] = 0.0f;
    v9.v.m128_f32[1] = 0.0f;
    v9.v.m128_f32[2] = 0.69999999f;
    v9.v.m128_f32[3] = 0.0f;
    typedef void (__cdecl* ProneTrace)(trace_t*, const math::Position3*,
                                       const math::Position3*, const math::Position3*,
                                       const math::Position3*,
                                       const collision_context_t&);
    typedef int (__cdecl* ProneContents)(const math::Position3*,
                                         const collision_context_t&);
    int result = BG_CheckProneValid(
        ps->mClient, &ps->origin, ps->maxs[0], 30.0f, proneDirection,
        &ps->fTorsoHeight, &ps->fTorsoPitch, &ps->fWaistPitch, 1, 1,
        &v9, (ProneTrace)pm->capsuletrace, (ProneTrace)pm->boxtrace,
        (ProneContents)pm->pointcontents, PCT_CLIENT, 60.0f);
    if (result == 0)
    {
        pm->ps->origin.v.m128_f32[0] = vFallbackOrg.v.m128_f32[0];
        pm->ps->origin.v.m128_f32[1] = vFallbackOrg.v.m128_f32[1];
        pm->ps->origin.v.m128_f32[2] = vFallbackOrg.v.m128_f32[2];
        pm->ps->velocity.v.m128_f32[0] = vFallbackVel.v.m128_f32[0];
        pm->ps->velocity.v.m128_f32[1] = vFallbackVel.v.m128_f32[1];
        pm->ps->velocity.v.m128_f32[2] = vFallbackVel.v.m128_f32[2];
    }
    return result;
}

// ea: 0x006156B0
void PM_UpdatePronePitch()
{
    pmove_t* v2 = pm;
    if (pm == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
        AeAssert::gCurrentLine = 5253;
        AeAssert::gCurrentExpr = "pm";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        v2 = pm;
    }
    PlayerState* ps = v2->ps;
    if ((v2->ps->pm_flags & 1) != 0)
    {
        if (ps->mGroundEntity.mHandle.mVal != 0)
        {
            if (pml.groundPlane == 0)
                goto LABEL_22;
            if (pml.groundTrace.normal.v.m128_f32[2] < 0.69999999f)
            {
                BG_AddPredictableEventToPlayerstate(166, 0, v2->ps);
                v2 = pm;
            }
        }
        else
        {
            typedef void (__cdecl* ProneTrace)(trace_t*, const math::Position3*,
                                               const math::Position3*,
                                               const math::Position3*,
                                               const math::Position3*,
                                               const collision_context_t&);
            typedef int (__cdecl* ProneContents)(const math::Position3*,
                                                 const collision_context_t&);
            int v10 = ps->mGroundEntity.mHandle.mVal != 0;
            float proneDirection = ps->proneDirection;
            math::Dir3 groundNormal;
            if (pml.groundPlane != 0)
                groundNormal = pml.groundTrace.normal;
            else
            {
                groundNormal.v.m128_f32[0] = 0.0f;
                groundNormal.v.m128_f32[1] = 0.0f;
                groundNormal.v.m128_f32[2] = 1.0f;
                groundNormal.v.m128_f32[3] = 0.0f;
            }
            int v6 = BG_CheckProneValid(
                ps->mClient, &ps->origin, ps->maxs[0], 30.0f, proneDirection,
                &ps->fTorsoHeight, &ps->fTorsoPitch, &ps->fWaistPitch, 1,
                v10, &groundNormal, (ProneTrace)v2->capsuletrace,
                (ProneTrace)v2->boxtrace, (ProneContents)v2->pointcontents,
                PCT_CLIENT, 60.0f);
            v2 = pm;
            if (v6 == 0 || pm->waterlevel != 0)
            {
                PlayerState* v7 = pm->ps;
                char v12[256];
                Cvar_VariableStringBuffer("showevents", v12, 256);
                if (atof(v12) != 0.0)
                    Com_Printf(
                        "Cgame event svt %5d -> %5d: num = %20s parm %d\n",
                        v7->commandTime, v7->event.eventSequence,
                        pEventNamesList[166], 0);
                v7->event.events[v7->event.eventSequence & 3] = 166;
                v7->event.eventParms[v7->event.eventSequence++ & 3] = 0;
                pm->ps->pm_flags |= 0x8000u;
                v2 = pm;
            }
        }
        if (pml.groundPlane != 0)
        {
            float pronePitch =
                PitchForYawOnNormal(v2->ps->proneDirection,
                                    &pml.groundTrace.normal.v.m128_f32[0]);
            float v15 = AngleDelta(pronePitch, v2->ps->proneDirectionPitch);
            if (v15 != 0.0f)
            {
                if (fabsf(v15) <= (pml.frametime * 70.0f))
                {
                    pm->ps->proneDirectionPitch =
                        pm->ps->proneDirectionPitch + v15;
                }
                else
                {
                    pm->ps->proneDirectionPitch =
                        ((1 - 2 * (v15 < 0.0f)) * (pml.frametime * 70.0f))
                        + pm->ps->proneDirectionPitch;
                }
                pm->ps->proneDirectionPitch =
                    AngleNormalize180Accurate(pm->ps->proneDirectionPitch);
            }
            v15 = AngleDelta(0.0f, pm->ps->proneTorsoPitch);
            if (v15 != 0.0f)
            {
                if (fabsf(v15) <= (pml.frametime * 70.0f))
                {
                    pm->ps->proneTorsoPitch =
                        pm->ps->proneTorsoPitch + v15;
                }
                else
                {
                    pm->ps->proneTorsoPitch =
                        ((1 - 2 * (v15 < 0.0f)) * (pml.frametime * 70.0f))
                        + pm->ps->proneTorsoPitch;
                }
                pm->ps->proneTorsoPitch =
                    AngleNormalize180Accurate(pm->ps->proneTorsoPitch);
            }
            return;
        }
    LABEL_22:
        float proneDirection = 0.0f;
        float v15 = AngleDelta(proneDirection, v2->ps->proneDirectionPitch);
        if (v15 != 0.0f)
        {
            if (fabsf(v15) <= (pml.frametime * 70.0f))
            {
                pm->ps->proneDirectionPitch =
                    pm->ps->proneDirectionPitch + v15;
            }
            else
            {
                pm->ps->proneDirectionPitch =
                    ((1 - 2 * (v15 < 0.0f)) * (pml.frametime * 70.0f))
                    + pm->ps->proneDirectionPitch;
            }
            pm->ps->proneDirectionPitch =
                AngleNormalize180Accurate(pm->ps->proneDirectionPitch);
        }
        v15 = AngleDelta(0.0f, pm->ps->proneTorsoPitch);
        if (v15 != 0.0f)
        {
            if (fabsf(v15) <= (pml.frametime * 70.0f))
            {
                pm->ps->proneTorsoPitch = pm->ps->proneTorsoPitch + v15;
            }
            else
            {
                pm->ps->proneTorsoPitch =
                    ((1 - 2 * (v15 < 0.0f)) * (pml.frametime * 70.0f))
                    + pm->ps->proneTorsoPitch;
            }
            pm->ps->proneTorsoPitch =
                AngleNormalize180Accurate(pm->ps->proneTorsoPitch);
        }
    }
}

// ============================================================================
// PM_UpdatePlayerSprintingFlag - ea: 0x62F070 (bg_pmove.cpp)
// ============================================================================
// ea: 0x0062F070
PlayerState* PM_UpdatePlayerSprintingFlag()
{
    int pm_flags = pm->ps->pm_flags;
    pm->ps->pm_flags = pm_flags & 0xFFFEFFFF;
    PlayerState* ps = pm->ps;
    unsigned int v3 = 0x10000 & pm_flags;
    if (pm->ps->pm_type < 4)
    {
        int buttons = pm->cmd.buttons;
        if ((buttons & 4) != 0 && ps->fatigueScale > 0.0f
            && (v3 != 0 || ps->fatigueScale > 0.25f)
            && (ps->pm_flags & 3) == 0
            && (pm->cmd.forwardmove != 0 || pm->cmd.rightmove != 0))
        {
            int type = ((weaponFileInfo_t*)pml.pWeap)->type;
            if (type != 8 && (dword_106000 & ps->eFlags) == 0)
            {
                int weaponstate = ps->weaponstate;
                if (weaponstate != 13
                    && (type != WEAPTYPE_GRENADE
                        || (((weaponFileInfo_t*)pml.pWeap)->bCookOffHold == 0
                            || ps->grenadeTimeLeft
                                   >= ((weaponFileInfo_t*)pml.pWeap)->iFuseTime
                            || ps->grenadeTimeLeft == 0
                            || Com_BitCheck(ps->weapons, ps->weapon) == 0)
                            && (buttons & 1) == 0
                            && weaponstate != 3))
                {
                    Entity* v6 = HandleDbToEnt(ps->mClient);
                    cl_aADS[v6->GetPlayerIndex()] = 1;
                    pm->ps->pm_flags |= 0x10000;
                }
            }
        }
    }
    return pm->ps;
}

// ============================================================================
// PM_MeleeAssistAccelerate - ea: 0x62DCF0 (bg_pmove.cpp)
// ============================================================================
// ea: 0x0062DCF0
unsigned int PM_MeleeAssistAccelerate()
{
    unsigned int result = pm->ps->mMeleeAssistTarget.mHandle.mVal;
    if (result != 0)
    {
        unsigned int v1 = pm->ps->mMeleeAssistTarget.mHandle.mVal & 0xFFF;
        if (v1 < 0x540)
        {
            result >>= 12;
            if (result
                == (unsigned int)EntityHandleDb::sInst.mElements[v1].mKey)
            {
                Entity* target = EntityHandleDb::sInst.mElements[v1].mObject;
                if (target != nullptr)
                {
                    float aimDir = target->r.currentOrigin.v.m128_f32[0]
                        - pm->ps->origin.v.m128_f32[0];
                    float v3 = target->r.currentOrigin.v.m128_f32[1]
                        - pm->ps->origin.v.m128_f32[1];
                    float v4 = target->r.currentOrigin.v.m128_f32[2]
                        - pm->ps->origin.v.m128_f32[2];
                    VectorNormalize(&aimDir);
                    pm->ps->velocity.v.m128_f32[2] = 0.0f;
                    pm->ps->velocity.v.m128_f32[1] = 0.0f;
                    pm->ps->velocity.v.m128_f32[0] = 0.0f;
                    pm->ps->velocity.v.m128_f32[0] =
                        pm->ps->mMeleeAssistSpeed * aimDir
                        + pm->ps->velocity.v.m128_f32[0];
                    pm->ps->velocity.v.m128_f32[1] =
                        pm->ps->mMeleeAssistSpeed * v3
                        + pm->ps->velocity.v.m128_f32[1];
                    pm->ps->velocity.v.m128_f32[2] =
                        pm->ps->mMeleeAssistSpeed * v4
                        + pm->ps->velocity.v.m128_f32[2];
                }
            }
        }
    }
    return result;
}

// ============================================================================
// BG_GetMaxAmmoPakAmmo - ea: 0x62DB90 (bg_pmove.cpp)
// ============================================================================
// ea: 0x0062DB90
int BG_GetMaxAmmoPakAmmo(const PlayerState* pPS, int iSlot)
{
    int v2 = pPS->weaponslots[iSlot];
    if (pPS->weaponslots[iSlot] == 0)
        return 0;
    Entity* mObject = HandleDbToEnt(pPS->mClient);
    if (mObject->sentient == nullptr)
        return 0;
    int iAmmoIndex = BG_GetInfoForWeapon(v2)->iAmmoIndex;
    int iClipIndex = BG_GetInfoForWeapon(v2)->iClipIndex;
    BG_GetInfoForWeapon(v2);
    int clipCount = 0;
    if (gpBrocAPI->mBrocExports.mCallbackGetSlotClipCount != nullptr)
    {
        int v12 = HandleDbToEnt(pPS->mClient)->sentient->eTeam == TEAM_ALLIES;
        unsigned int rank = HandleDbToEnt(pPS->mClient)->client->pers.rank;
        unsigned int playerClass =
            HandleDbToEnt(pPS->mClient)->client->pers.playerClass;
        const char* WeaponSlotNameForIndex =
            BG_GetWeaponSlotNameForIndex(iSlot);
        clipCount = gpBrocAPI->mBrocExports.mCallbackGetSlotClipCount(
            WeaponSlotNameForIndex, playerClass, rank, v12);
    }
    int v8 = clipCount * BG_GetAmmoClipSize(iClipIndex);
    if (BG_GetInfoForWeapon(v2)->bClipOnly != 0)
        return v8 - pPS->ammoclip[iClipIndex];
    return v8 - pPS->ammoclip[iClipIndex] - pPS->ammo[iAmmoIndex];
}

// ============================================================================
// PM_UpdateAimDownSightFlag - ea: 0x62F2F0 (bg_pmove.cpp)
// ============================================================================
// ea: 0x0062F2F0
void PM_UpdateAimDownSightFlag()
{
    PlayerState* ps = pm->ps;
    int pm_type = pm->ps->pm_type;
    if (pm_type >= 6)
        goto LABEL_40;
    if ((pm->cmd.buttons & 8) != 0 && (dword_106000 & ps->eFlags) != 0)
        goto LABEL_33;
    if ((0x10000 & ps->pm_flags) != 0 || (pm->cmd.buttons & 8) == 0
        || ((weaponFileInfo_t*)pml.pWeap)->bADSPositionInfo == 0
        || (ps->weaponstate == 2 || ps->weaponstate == 1
            || ps->weaponstate == 10 || ps->weaponstate == 11)
        || (pml.groundPlane == 0 && pm_type != 1))
    {
    LABEL_40:
        ps->pm_flags &= ~0x20u;
        return;
    }
    if (BG_GetInfoForWeapon(ps->weapon)->weapClass == WEAPCLASS_LMG)
    {
        if ((pm->ps->pm_flags & 0x20) != 0)
            return;
        float angles[3];
        angles[1] = pm->ps->viewangles[1];
        angles[2] = 0.0f;
        angles[0] = 0.0f;
        math::Dir3 v20;
        AnglesToForward(angles, v20.v.m128_f32);
        if (pm->ps->serverCursorHint == 12)
        {
            if ((pm->ps->pm_flags & 0x20) == 0)
            {
                pm->ps->proneDirection = pm->ps->viewangles[1];
            }
            pm->ps->pm_flags |= 0x20u;
            if (pm->ps->serverCursorHintVal != 0)
                pm->ps->pm_flags |= 2;
            else
                pm->ps->pm_flags &= 0xFFFFFFFD;
            pm->ps->pm_flags &= ~1u;
            return;
        }
        if ((pm->ps->pm_flags & 1) == 0)
        {
            if (pm->ps->mGroundEntity.mHandle.mVal == 0)
                goto LABEL_25;
            v20.v = _mm_setr_ps(0.0f, 0.0f, 0.69999999f, 0.0f);
            float fSize = pm->maxs.v.m128_f32[0];
            float fYaw = pm->ps->viewangles[1];
            typedef void (__cdecl* ProneTrace)(
                trace_t*, const math::Position3*, const math::Position3*,
                const math::Position3*, const math::Position3*,
                const collision_context_t&);
            typedef int (__cdecl* ProneContents)(
                const math::Position3*, const collision_context_t&);
            int v11 = BG_CheckProneValid(
                pm->ps->mClient, &pm->ps->origin, fSize, 30.0f, fYaw,
                &pm->ps->fTorsoHeight, &pm->ps->fTorsoPitch,
                &pm->ps->fWaistPitch, false,
                pm->ps->mGroundEntity.mHandle.mVal != 0, &v20,
                (ProneTrace)pm->capsuletrace, (ProneTrace)pm->boxtrace,
                (ProneContents)pm->pointcontents,
                PCT_CLIENT, 60.0f);
            if (v11 == 0)
            {
            LABEL_25:
                if ((pm->ps->pm_flags & 1) == 0)
                {
                    HandleDbToEnt(pm->ps->mClient)->client->mProneBlockedTime =
                        level.time;
                    Entity* v13 = HandleDbToEnt(pm->ps->mClient);
                    int PlayerIndex = v13->GetPlayerIndex();
                    cl_aADS[PlayerIndex] = 1;
                    pm->ps->pm_flags |= 0x8000u;
                    if ((pm->cmd.buttons & 0x100) == 0)
                    {
                        if ((pm->ps->pm_flags & 2) != 0)
                            BG_AddPredictableEventToPlayerstate(166, 0,
                                                                pm->ps);
                        else
                            BG_AddPredictableEventToPlayerstate(165, 0,
                                                                pm->ps);
                    }
                }
                return;
            }
        }
        if ((pm->ps->pm_flags & 0x21) == 0)
        {
            pm->ps->proneDirection = pm->ps->viewangles[1];
        }
        pm->ps->pm_flags |= 1u;
        ps = pm->ps;
    LABEL_33:
        ps->pm_flags |= 0x20u;
        return;
    }
    PlayerState* v16 = pm->ps;
    if ((pm->ps->pm_flags & 1) != 0)
    {
        if ((pm->oldcmd.buttons & 8) == 0
            || (pm->cmd.forwardmove == 0 && pm->cmd.rightmove == 0))
        {
            v16->pm_flags |= 0x20u;
            pm->ps->pm_flags |= 0x400u;
        }
    }
    else
    {
        v16->pm_flags |= 0x20u;
    }
}

// ============================================================================
// PM_UpdateAimDownSightLerp - ea: 0x62F670 (bg_pmove.cpp)
// ============================================================================
extern bool BG_AllowPlayerWeaponAtVehiclePos(int vehType, int vehPos);

// ea: 0x0062F670
void PM_UpdateAimDownSightLerp()
{
    weaponFileInfo_t* pWeap = (weaponFileInfo_t*)pml.pWeap;
    if (pWeap->bADSPositionInfo == 0 && (dword_106000 & pm->ps->eFlags) == 0)
    {
        pm->ps->fWeaponPosFrac = 0.0f;
        return;
    }
    PlayerState* ps = pm->ps;
    int weaponstate = pm->ps->weaponstate;
    if (weaponstate != 0 && weaponstate != 3
        || pWeap->bBoltAction == 0
        || ((1 << (ps->weapon & 0x1F))
            & ps->weaponrechamber[ps->weapon >> 5]) == 0)
    {
        int eFlags = ps->eFlags;
        int v4 = 0;
        if ((eFlags & 0x6000) != 0)
        {
            if ((ps->pm_flags & 0x20) != 0)
                ps->fWeaponPosFrac = 1.0f;
            else
                ps->fWeaponPosFrac = 0.0f;
            return;
        }
        if (pWeap->bSegmentedReload != 0)
        {
            if (pWeap->weapClass != WEAPCLASS_LMG
                && (weaponstate == 5 || weaponstate == 6
                    || weaponstate == 14 || weaponstate == 7
                    || weaponstate == 8
                    || (weaponstate == 9 && ps->weaponTime > 0)))
                goto LABEL_29;
        }
        else if ((weaponstate == 5 || weaponstate == 14)
                 && ps->weaponTime > 0
                 && pWeap->weapClass != WEAPCLASS_LMG)
        {
            goto LABEL_29;
        }
        if (pWeap->bRechamberWhileAds != 0 || weaponstate != 4)
        {
            if ((ps->pm_flags & 0x20) != 0)
                v4 = 1;
        LABEL_32:
            if (pWeap->bADSFire != 0 && ps->weaponDelay != 0
                && weaponstate == 3)
                v4 = 1;
            if (pWeap->weapClass == WEAPCLASS_SPOTTER)
            {
                if (weaponstate == 13)
                    v4 = 0;
                else if (weaponstate == 12)
                    v4 = 1;
            }
            if ((0x10000 & ps->pm_flags) != 0)
            {
                v4 = 0;
            }
            else if (v4 != 0)
            {
                if (ps->fWeaponPosFrac == 1.0f)
                    return;
                goto LABEL_44;
            }
            if (ps->fWeaponPosFrac == 0.0f)
                return;
        LABEL_44:
            if ((0x100000 & eFlags) != 0
                && !BG_AllowPlayerWeaponAtVehiclePos(ps->vehType,
                                                     ps->vehPos))
            {
                if (v4 != 0)
                    ps->fWeaponPosFrac = 1.0f;
                else
                    ps->fWeaponPosFrac = 0.0f;
            }
            else
            {
                float fWeaponPosFrac = ps->fWeaponPosFrac;
                if (v4 != 0)
                {
                    if (fWeaponPosFrac == 0.0f && PM_CanStartADSAnim())
                    {
                        PM_StartWeaponAnim(21);
                        pWeap = (weaponFileInfo_t*)pml.pWeap;
                    }
                    pm->ps->fWeaponPosFrac =
                        pml.msec * pWeap->fOOPosAnimLength[0]
                        + pm->ps->fWeaponPosFrac;
                }
                else
                {
                    if (fWeaponPosFrac == 1.0f && PM_CanStartADSAnim())
                    {
                        PM_StartWeaponAnim(22);
                        pWeap = (weaponFileInfo_t*)pml.pWeap;
                    }
                    pm->ps->fWeaponPosFrac =
                        pm->ps->fWeaponPosFrac
                        - (pml.msec * pWeap->fOOPosAnimLength[1]);
                }
            }
            PlayerState* v6 = pm->ps;
            if (pm->ps->fWeaponPosFrac < 1.0f)
            {
                if (v6->fWeaponPosFrac > 0.5f || v6->fWeaponPosFrac <= 0.0f)
                {
                    if (v6->fWeaponPosFrac <= 0.0f)
                    {
                        v6->fWeaponPosFrac = 0.0f;
                        int v9 = pm->ps->weaponstate;
                        if (v9 != 7 && v9 != 5 && v9 != 4 && v9 != 10
                            && v9 != 11)
                            PM_StartWeaponAnim(0);
                    }
                }
                else if (pWeap->slot == WEAPSLOT_BINOCS
                         && v6->weaponstate == 13)
                {
                    v6->weaponstate = 2;
                    pm->ps->fWeaponPosFrac = 0.0f;
                    PM_StartWeaponAnim(9);
                    Entity* v7 = HandleDbToEnt(pm->ps->mClient);
                    int PlayerIndex = v7->GetPlayerIndex();
                    BG_SelectWeaponIndex(pm->ps->lastWeapon, PlayerIndex);
                }
            }
            else
            {
                v6->fWeaponPosFrac = 1.0f;
                if (pm->ps->weaponstate == 4)
                {
                    if (pm->cmd.weapon != 0
                        && (pm->ps->weapAnim & 0xFFFFFDFF) != 7)
                        PM_StartWeaponAnim(7);
                }
                else
                {
                    PM_StartWeaponAnim(23);
                }
            }
            return;
        }
    LABEL_29:
        v4 = 0;
        goto LABEL_32;
    }
}

// ============================================================================
// PM_UpdateLean - ea: 0x621B60 (bg_pmove.cpp)
// ============================================================================
extern void AddLeanToPosition(float* vPosition, float fViewYaw,
                              float fLeanFrac, float fViewRoll,
                              float fLeanDist);  // game.o 0x61FBA0
extern float UnGetLeanFraction(float fFrac);    // game.o 0x6116C0

// ea: 0x00621B60
void PM_UpdateLean(PlayerState* ps, usercmd_s* cmd,
                   void (__cdecl* capsuleTrace)(
                       trace_t*, const math::Position3*,
                       const math::Position3*, const math::Position3*,
                       const math::Position3*, const collision_context_t*))
{
    int buttons = cmd->buttons;
    int v5 = 0;
    if ((buttons & 0x1800) != 0)
    {
        int pm_flags = ps->pm_flags;
        if ((pm_flags & 0x4000) == 0)
        {
            int pm_type = ps->pm_type;
            if (pm_type < 6
                && (ps->mGroundEntity.mHandle.mVal != 0 || pm_type == 1))
            {
                ps->pm_flags = pm_flags & 0xCFFFFFFF;
                int v8 = cmd->buttons;
                if ((v8 & 0x800) != 0)
                    v5 = -1;
                if ((v8 & 0x1000) != 0)
                    ++v5;
            }
        }
    }
    if ((dword_106000 & ps->eFlags) != 0
        || ((ps->pm_flags & 0x20) != 0
            && BG_GetInfoForWeapon(ps->weapon)->weapClass == WEAPCLASS_LMG))
    {
        v5 = 0;
    }
    int viewHeightTarget = ps->viewHeightTarget;
    float v10;
    if (viewHeightTarget == ps->crouchViewHeight
        || viewHeightTarget != ps->proneViewHeight)
        v10 = 0.5f;
    else
        v10 = 0.25f;
    float leanf = ps->leanf;
    if (v5 != 0)
    {
        int v12 = ps->pm_flags;
        if (v5 <= 0)
        {
            ps->pm_flags = v12 | 0x10000000;
            if (leanf > -v10)
                leanf -= (pml.msec * 0.0028571428f) * v10;
            if (-v10 > leanf)
                leanf = -v10;
        }
        else
        {
            ps->pm_flags = v12 | 0x20000000;
            if (v10 > leanf)
                leanf += (pml.msec * 0.0028571428f) * v10;
            if (leanf > v10)
                leanf = v10;
        }
    }
    else if (leanf <= 0.0f)
    {
        if (leanf < 0.0f)
        {
            leanf += (pml.msec * 0.0035714286f) * v10;
            if (leanf > 0.0f)
                leanf = 0.0f;
        }
    }
    else
    {
        leanf -= (pml.msec * 0.0035714286f) * v10;
        if (leanf < 0.0f)
            leanf = 0.0f;
    }
    ps->leanf = leanf;
    if (leanf != 0.0f && ps->pm_type != 1)
    {
        float fViewYaw = ps->viewangles[1];
        float v14 = ps->origin.v.m128_f32[0];
        float v15 = ps->origin.v.m128_f32[1];
        float LeanFraction = leanf;
        float v22 = ps->viewHeightCurrent + ps->origin.v.m128_f32[2];
        math::Position3 start;
        start.v = _mm_setr_ps(v14, v15, v22, 0.0f);
        AddLeanToPosition(start.v.m128_f32, fViewYaw,
                          (1 - 2 * (leanf < 0.0f)), 16.0f, 20.0f);
        math::Position3 v19;
        math::Position3 v25;
        v25.v = _mm_setr_ps(8.0f, 8.0f, 12.0f, 0.0f);
        v19.v = v25.v;
        v25.v = _mm_setr_ps(-8.0f, -8.0f, -12.0f, 0.0f);
        player_collision_context_t context;
        context.__vftable = nullptr;
        context.pass_entity1.mHandle.mVal = 0;
        context.pass_entity2.mHandle.mVal = ps->mClient.mHandle.mVal;
        context.pass_owner1.mHandle.mVal = 0;
        context.pass_owner2.mHandle.mVal = 0;
        context.contentmask = 0;
        trace_t tr;
        capsuleTrace(&tr, &start, &v25, &v19, &start,
                     (const collision_context_t*)&context);
        LeanFraction = UnGetLeanFraction(tr.fraction);
        if (fabsf(ps->leanf) > LeanFraction)
        {
            unsigned int v = (unsigned int)ps->leanf;
            ps->leanf =
                (1 - 2 * ((v & 0x80000000) != 0)) * LeanFraction;
        }
    }
}

// ============================================================================
// viewLerpWaypoint_s + PM_ViewHeightTableLerp - ea: 0x615050 (bg_pmove.cpp)
// ============================================================================
struct viewLerpWaypoint_s {
    int   iFrac;        // +0x00
    float fViewHeight;  // +0x04
    int   iOffset;      // +0x08
};
static_assert(sizeof(viewLerpWaypoint_s) == 0xC, "viewLerpWaypoint_s size mismatch");

// .rdata lerp tables (byte-verified against IDA 0xDF5830..0xDF5A28)
static const viewLerpWaypoint_s viewLerp_StandCrouch[] = {
    { 0, 60.0f, 0 }, { 1, 59.5f, 0 }, { 4, 58.5f, 0 }, { 30, 56.0f, 0 },
    { 80, 44.0f, 0 }, { 90, 41.5f, 0 }, { 95, 40.5f, 0 }, { 100, 40.0f, 0 },
    { -1, 0.0f, 0 },
};
static const viewLerpWaypoint_s viewLerp_CrouchStand[] = {
    { 0, 40.0f, 0 }, { 5, 40.5f, 0 }, { 10, 41.5f, 0 }, { 20, 44.0f, 0 },
    { 70, 56.0f, 0 }, { 96, 58.5f, 0 }, { 99, 59.5f, 0 }, { 100, 60.0f, 0 },
    { -1, 0.0f, 0 },
};
static const viewLerpWaypoint_s viewLerp_CrouchProne[] = {
    { 0, 40.0f, 0 }, { 11, 38.0f, 0 }, { 22, 33.0f, 0 }, { 34, 25.0f, 0 },
    { 45, 16.0f, 0 }, { 50, 15.0f, 0 }, { 55, 16.0f, 0 }, { 70, 18.0f, 0 },
    { 90, 17.0f, 0 }, { 100, 11.0f, 0 }, { -1, 0.0f, 0 },
};
static const viewLerpWaypoint_s viewLerp_ProneCrouch[] = {
    { 0, 11.0f, 0 }, { 5, 10.0f, 0 }, { 30, 21.0f, 0 }, { 50, 25.0f, 0 },
    { 67, 31.0f, 0 }, { 83, 34.0f, 0 }, { 100, 40.0f, 0 }, { -1, 0.0f, 0 },
};

// ea: 0x00615050
static float PM_ViewHeightTableLerp(int iFrac, const viewLerpWaypoint_s* pTable,
                                    float* pfPosOfs)
{
    if (iFrac == 0)
    {
        *pfPosOfs = (float)pTable->iOffset;
        return pTable->fViewHeight;
    }
    if (iFrac >= 100)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
        AeAssert::gCurrentLine = 2545;
        AeAssert::gCurrentExpr = "iFrac < 100";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    const viewLerpWaypoint_s* pCurr = pTable + 1;
    int v6 = pCurr->iFrac;
    for (int i = 1; ; ++i)
    {
        if (v6 == iFrac)
        {
            *pfPosOfs = (float)pCurr->iOffset;
            return pCurr->fViewHeight;
        }
        if (v6 > iFrac)
            break;
        v6 = pCurr[1].iFrac;
        ++pCurr;
        if (v6 == -1)
        {
            if (va("No encapsulating table entries found for fraction %i",
                   iFrac) == nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
                AeAssert::gCurrentLine = 2580;
                AeAssert::gCurrentExpr =
                    "va(\"No encapsulating table entries found for fraction %i\", iFrac)";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            *pfPosOfs = (float)pTable->iOffset;
            return pTable->fViewHeight;
        }
    }
    const viewLerpWaypoint_s* pPrev = pCurr - 1;
    if (pCurr->iFrac - pPrev->iFrac <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
        AeAssert::gCurrentLine = 2567;
        AeAssert::gCurrentExpr = "(pCurr->iFrac - pPrev->iFrac) > 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float v10 = (float)(iFrac - pPrev->iFrac)
              / (float)(pCurr->iFrac - pPrev->iFrac);
    *pfPosOfs = (float)pPrev->iOffset
              + (float)(pCurr->iOffset - pPrev->iOffset) * v10;
    return pPrev->fViewHeight
         + (pCurr->fViewHeight - pPrev->fViewHeight) * v10;
}

// ============================================================================
// PM_ViewHeightAdjust - ea: 0x6444E0 (bg_pmove.cpp)
// ============================================================================
// ea: 0x006444E0
static void PM_ViewHeightAdjust()
{
    if (pm == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
        AeAssert::gCurrentLine = 2644;
        AeAssert::gCurrentExpr = "pm";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    PlayerState* ps = pm->ps;
    int viewHeightTarget = ps->viewHeightTarget;
    if (viewHeightTarget == 0 || ps->viewHeightCurrent == 0.0f)
    {
        if (ps->pm_type == 4)
            ps->viewHeightCurrent = 0.0f;
        else
            ps->viewHeightCurrent = (float)viewHeightTarget;
        return;
    }
    if (ps->viewHeightCurrent == (float)viewHeightTarget
        && ps->viewHeightLerpTime == 0)
    {
        return;
    }
    if (viewHeightTarget != ps->proneViewHeight
        && viewHeightTarget != ps->crouchViewHeight
        && viewHeightTarget != ps->standViewHeight)
    {
        ps->viewHeightLerpTime = 0;
        float v6 = pml.frametime * 180.0f;
        if ((float)pm->ps->viewHeightTarget <= pm->ps->viewHeightCurrent)
        {
            pm->ps->viewHeightCurrent -= v6;
            if ((float)pm->ps->viewHeightTarget
                >= pm->ps->viewHeightCurrent)
            {
                pm->ps->viewHeightCurrent =
                    (float)pm->ps->viewHeightTarget;
            }
        }
        else
        {
            pm->ps->viewHeightCurrent += v6;
            if (pm->ps->viewHeightCurrent
                >= (float)pm->ps->viewHeightTarget)
            {
                pm->ps->viewHeightCurrent =
                    (float)pm->ps->viewHeightTarget;
            }
        }
        return;
    }
    if (ps->viewHeightLerpTime != 0)
    {
        int viewHeightLerpTarget = ps->viewHeightLerpTarget;
        int ViewHeightLerpTime = PM_GetViewHeightLerpTime(
            ps, viewHeightLerpTarget, ps->viewHeightLerpDown);
        int v12 = 100 * (pm->cmd.serverTime - ps->viewHeightLerpTime)
                / ViewHeightLerpTime;
        int iFrac = v12;
        if (v12 < 0)
        {
            iFrac = 0;
        }
        else if (v12 > 100)
        {
            iFrac = 100;
            ps->viewHeightCurrent = (float)viewHeightLerpTarget;
            pm->ps->viewHeightLerpTime = 0;
            pm->ps->viewHeightLerpPosAdj = 0.0f;
            goto L39;
        }
        else if (v12 == 100)
        {
            ps->viewHeightCurrent = (float)viewHeightLerpTarget;
            pm->ps->viewHeightLerpTime = 0;
            pm->ps->viewHeightLerpPosAdj = 0.0f;
            goto L39;
        }
        float fNewPosOfs;
        const viewLerpWaypoint_s* pTable;
        if (viewHeightLerpTarget == ps->proneViewHeight)
            pTable = viewLerp_CrouchProne;
        else if (viewHeightLerpTarget == ps->crouchViewHeight)
            pTable = ps->viewHeightLerpDown == 0 ? viewLerp_ProneCrouch
                                                 : viewLerp_StandCrouch;
        else
            pTable = viewLerp_CrouchStand;
        float fViewHeight =
            PM_ViewHeightTableLerp(iFrac, pTable, &fNewPosOfs);
        pm->ps->viewHeightCurrent = fViewHeight;
        if (fabsf(pm->ps->viewHeightLerpPosAdj - fNewPosOfs) > 0.05f
            && (0x100000 & pm->ps->pm_flags) == 0)
        {
            float vOrigVel[3];
            vOrigVel[0] = pm->ps->velocity.v.m128_f32[0];
            vOrigVel[1] = pm->ps->velocity.v.m128_f32[1];
            vOrigVel[2] = pm->ps->velocity.v.m128_f32[2];
            float v15 = fNewPosOfs - pm->ps->viewHeightLerpPosAdj;
            if (pm->ps->mGroundEntity.mHandle.mVal == 0)
                v15 = v15 * 0.5f;
            float fOffset = v15 / pml.frametime;
            float vFlatForward[3];
            vFlatForward[0] = pml.forward[0];
            vFlatForward[1] = pml.forward[1];
            vFlatForward[2] = 0.0f;
            VectorNormalize(vFlatForward);
            pm->ps->velocity.v.m128_f32[0] = vFlatForward[0] * fOffset;
            pm->ps->velocity.v.m128_f32[1] = vFlatForward[1] * fOffset;
            pm->ps->velocity.v.m128_f32[2] = vFlatForward[2] * fOffset;
            PM_StepSlideMove(1);
            pm->ps->velocity.v.m128_f32[0] = vOrigVel[0];
            pm->ps->velocity.v.m128_f32[1] = vOrigVel[1];
            pm->ps->velocity.v.m128_f32[2] = vOrigVel[2];
            pm->ps->viewHeightLerpPosAdj = fNewPosOfs;
        }
L39:
        ps = pm->ps;
        if (ps->viewHeightLerpTime != 0)
        {
            if (ps->viewHeightTarget == ps->viewHeightLerpTarget)
                return;
            if ((ps->viewHeightTarget < ps->viewHeightLerpTarget
                 && ps->viewHeightLerpDown == 0)
                || (ps->viewHeightTarget > ps->viewHeightLerpTarget
                    && ps->viewHeightLerpDown != 0))
            {
                ps->viewHeightLerpDown ^= 1;
                int v20 = ps->viewHeightLerpTarget;
                if (ps->viewHeightLerpDown != 0)
                {
                    if (v20 == ps->standViewHeight)
                        ps->viewHeightLerpTarget = ps->crouchViewHeight;
                    else if (v20 == ps->crouchViewHeight)
                        ps->viewHeightLerpTarget = ps->proneViewHeight;
                }
                else if (v20 == ps->proneViewHeight)
                {
                    ps->viewHeightLerpTarget = ps->crouchViewHeight;
                }
                else if (v20 == ps->crouchViewHeight)
                {
                    ps->viewHeightLerpTarget = ps->standViewHeight;
                }
                if (iFrac == 0)
                {
                    pm->ps->viewHeightCurrent =
                        (float)pm->ps->viewHeightLerpTarget;
                    pm->ps->viewHeightLerpTime = 0;
                    pm->ps->viewHeightLerpPosAdj = 0.0f;
                    return;
                }
                int LerpTime = PM_GetViewHeightLerpTime(
                    ps, ps->viewHeightLerpTarget, ps->viewHeightLerpDown);
                pm->ps->viewHeightLerpTime =
                    pm->cmd.serverTime
                    - (int)(LerpTime * (100 - iFrac) * 0.01f);
                int v27 = ps->viewHeightLerpTarget;
                if (v27 == ps->proneViewHeight)
                {
                    PM_ViewHeightTableLerp(100 - iFrac,
                                           viewLerp_CrouchProne,
                                           &fNewPosOfs);
                }
                else if (v27 == ps->crouchViewHeight)
                {
                    if (ps->viewHeightLerpDown == 0)
                        PM_ViewHeightTableLerp(100 - iFrac,
                                               viewLerp_ProneCrouch,
                                               &fNewPosOfs);
                    else
                        PM_ViewHeightTableLerp(100 - iFrac,
                                               viewLerp_StandCrouch,
                                               &fNewPosOfs);
                }
                else
                {
                    PM_ViewHeightTableLerp(100 - iFrac,
                                           viewLerp_CrouchStand,
                                           &fNewPosOfs);
                }
                pm->ps->viewHeightLerpPosAdj = fNewPosOfs;
            }
            return;
        }
        if (ps->viewHeightCurrent == (float)ps->viewHeightTarget)
            return;
        ps->viewHeightLerpTime = pm->cmd.serverTime;
        int v30 = ps->viewHeightTarget;
        if (v30 == ps->proneViewHeight)
        {
            ps->viewHeightLerpDown = 1;
            if (ps->viewHeightCurrent <= (float)ps->crouchViewHeight)
            {
                ps->viewHeightLerpTarget = ps->proneViewHeight;
                return;
            }
        }
        else if (v30 == ps->crouchViewHeight)
        {
            ps->viewHeightLerpDown = ps->viewHeightCurrent > (float)v30;
        }
        else
        {
            if (v30 != ps->standViewHeight)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
                AeAssert::gCurrentLine = 2863;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored())
                {
                    const char* v32 = va(
                        "View height lerp to %i reached bad place\n",
                        pm->ps->viewHeightTarget);
                    if (AeAssert::Warning(v32))
                        __debugbreak();
                }
                return;
            }
            ps->viewHeightLerpDown = 0;
            if ((float)ps->crouchViewHeight <= ps->viewHeightCurrent)
            {
                ps->viewHeightLerpTarget = ps->standViewHeight;
                return;
            }
        }
        ps->viewHeightLerpTarget = ps->crouchViewHeight;
    }
}

// ============================================================================
// PM_CheckDuck - ea: 0x644B80 (bg_pmove.cpp)
// ============================================================================
// ea: 0x00644B80
void PM_CheckDuck()
{
    if (pm == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
        AeAssert::gCurrentLine = 2888;
        AeAssert::gCurrentExpr = "pm";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }

    player_collision_context_t context;
    context.__vftable = (collision_context_t_vtbl*)0x00CD8F78;
    context.pass_entity1.mHandle.mVal = pm->ps->mClient.mHandle.mVal;
    context.pass_entity2.mHandle.mVal = 0;
    context.pass_owner1.mHandle.mVal = 0;
    context.pass_owner2.mHandle.mVal = 0;
    context.contentmask = pm->tracemask & 0xFDFFFFFF;

    trace_t trace;
    trace.mEntity.mHandle.mVal = 0;
    trace.partName.mHash = 0;

    PlayerState* ps = pm->ps;
    int pm_flags = ps->pm_flags;
    if ((0x100000 & pm_flags) != 0)
    {
        Entity* player = EntityManager::sInst->GetPlayer(currCl);
        int spectatorClient = player->client->ps.spectatorClient;
        if (spectatorClient < 0)
            return;
        PlayerState* v9 = pm->ps;
        int spectatorFlags =
            EntityManager::sInst->GetPlayer(spectatorClient)
                ->client->ps.pm_flags;
        if (pm->ps->viewHeightLerpTime != 0)
        {
            PM_ViewHeightAdjust();
            return;
        }
        if ((spectatorFlags & 1) != 0)
        {
            if (v9->viewHeightTarget == v9->standViewHeight)
                v9->viewHeightTarget = v9->crouchViewHeight;
            else
                v9->viewHeightTarget = v9->proneViewHeight;
            PM_ViewHeightAdjust();
            return;
        }
        if (v9->viewHeightTarget == v9->proneViewHeight)
            v9->viewHeightTarget = v9->crouchViewHeight;
        else if ((spectatorFlags & 2) != 0)
            v9->viewHeightTarget = v9->crouchViewHeight;
        else
            v9->viewHeightTarget = v9->standViewHeight;
        PM_ViewHeightAdjust();
        return;
    }

    if (ps->pm_type == 4)
    {
        pm->mins.v.m128_f32[0] = -8.0f;
        pm->mins.v.m128_f32[1] = -8.0f;
        pm->mins.v.m128_f32[2] = -8.0f;
        pm->maxs.v.m128_f32[0] = 8.0f;
        pm->maxs.v.m128_f32[1] = 8.0f;
        pm->maxs.v.m128_f32[2] = 16.0f;
        pm->ps->pm_flags &= 0xFFFFFFFC;
        if ((pm->cmd.buttons & 0x2000) != 0)
        {
            pm->cmd.buttons &= 0xFFFFDFFF;
            BG_AddPredictableEventToPlayerstate(165, 0, pm->ps);
        }
        pm->trace = pm->capsuletrace;
        pm->ps->eFlags |= 0x10;
        pm->ps->viewHeightTarget = 0;
        pm->ps->viewHeightCurrent = 0.0f;
        return;
    }

    pm->mins.v.m128_f32[0] = ps->mins[0];
    int bWasProne = pm_flags & 1;
    pm->mins.v.m128_f32[1] = pm->ps->mins[1];
    pm->maxs.v.m128_f32[0] = pm->ps->maxs[0];
    pm->maxs.v.m128_f32[1] = pm->ps->maxs[1];
    pm->mins.v.m128_f32[2] = pm->ps->mins[2];
    if (pm->ps->pm_type >= 6)
    {
        pm->maxs.v.m128_f32[2] = ps->maxs[2];
        pm->ps->viewHeightTarget = pm->ps->deadViewHeight;
        pm->trace = (pm->ps->pm_flags & 1) != 0 ? pm->boxtrace
                                                : pm->capsuletrace;
        pm->ps->eFlags |= 0x10;
        PM_ViewHeightAdjust();
        return;
    }
    int eFlags = ps->eFlags;
    if ((0x100000 & eFlags) != 0 && (0x400000 & eFlags) == 0)
    {
        pm->maxs.v.m128_f32[2] = ps->maxs[2];
        pm->ps->viewHeightTarget = pm->ps->standViewHeight;
        pm->cmd.buttons &= ~0x2000u;
        pm->ps->pm_flags &= 0xFFFFFFFC;
        BG_AddPredictableEventToPlayerstate(165, 0, pm->ps);
        pm->trace = pm->capsuletrace;
        pm->ps->eFlags |= 0x10;
        PM_ViewHeightAdjust();
        return;
    }
    if ((dword_106000 & eFlags) != 0)
    {
        if ((eFlags & 0x2000) != 0)
        {
            if ((eFlags & 0x4000) == 0)
            {
                ps->pm_flags |= 1;
                pm->ps->pm_flags &= ~2;
                goto L79;
            }
        }
        else if ((eFlags & 0x4000) == 0)
        {
            goto L37;
        }
        if ((eFlags & 0x2000) == 0)
        {
            ps->pm_flags |= 2;
            pm->ps->pm_flags &= ~1;
            goto L79;
        }
L37:
        ps->pm_flags &= 0xFFFFFFFC;
        goto L79;
    }
    int v19 = ps->pm_flags;
    if ((v19 & 0x4000) != 0)
        goto L80;
    if ((v19 & 0x20) != 0)
    {
        if (BG_GetInfoForWeapon(ps->weapon)->weapClass == WEAPCLASS_LMG)
            goto L79;
    }
    if (((weaponFileInfo_t*)pml.pWeap)->bHoldToFire != 0
        && (pm->cmd.buttons & 0x82) != 0)
    {
        if ((pm->ps->pm_flags & 1) == 0)
        {
            pm->ps->pm_flags |= 2;
            goto L79;
        }
        goto L80;
    }
    if ((pm->ps->pm_flags & 0x10) != 0
        && (pm->cmd.buttons & 0x6000) != 0)
    {
        pm->cmd.buttons &= 0xFFFF9FFF;
        BG_AddPredictableEventToPlayerstate(165, 0, pm->ps);
    }
    if ((pm->cmd.buttons & 0x2000) != 0)
    {
        math::Dir3 groundNormal;
        groundNormal.v.m128_f32[0] = 0.0f;
        groundNormal.v.m128_f32[1] = 0.0f;
        groundNormal.v.m128_f32[2] = 0.7f;
        groundNormal.v.m128_f32[3] = 0.0f;
        if ((pm->ps->pm_flags & 1) == 0)
        {
            typedef void (__cdecl* ProneTrace)(
                trace_t*, const math::Position3*, const math::Position3*,
                const math::Position3*, const math::Position3*,
                const collision_context_t&);
            typedef int (__cdecl* ProneContents)(
                const math::Position3*, const collision_context_t&);
            if (ps->mGroundEntity.mHandle.mVal == 0
                || pm->waterlevel != 0
                || BG_CheckProneValid(
                       ps->mClient, &ps->origin,
                       pm->maxs.v.m128_f32[0], 30.0f, ps->viewangles[1],
                       &ps->fTorsoHeight, &ps->fTorsoPitch,
                       &ps->fWaistPitch, 0,
                       ps->mGroundEntity.mHandle.mVal != 0, &groundNormal,
                       (ProneTrace)pm->capsuletrace,
                       (ProneTrace)pm->boxtrace,
                       (ProneContents)pm->pointcontents, PCT_CLIENT,
                       60.0f) == 0)
            {
                if (pm->ps->mGroundEntity.mHandle.mVal == 0)
                    goto L80;
                pm->ps->pm_flags |= 0x8000;
                if (GetPlayer(currCl) != nullptr
                    && GetPlayer(currCl)->client != nullptr)
                {
                    GetPlayer(currCl)->client->mProneBlockedTime = level.time;
                }
                if ((pm->cmd.buttons & 0x100) != 0)
                    goto L80;
                if ((pm->ps->pm_flags & 2) != 0)
                {
                    BG_AddPredictableEventToPlayerstate(166, 0, pm->ps);
                    goto L79;
                }
                BG_AddPredictableEventToPlayerstate(165, 0, pm->ps);
                goto L79;
            }
        }
        pm->ps->pm_flags |= 1;
        pm->ps->pm_flags &= ~2;
        goto L79;
    }
    if ((pm->cmd.buttons & 0x4000) != 0)
    {
        if ((pm->ps->pm_flags & 1) == 0)
        {
            pm->ps->pm_flags |= 2;
            goto L79;
        }
    }
    else
    {
        if ((ps->pm_flags & 1) != 0)
        {
            pm->maxs.v.m128_f32[2] = ps->maxs[2];
            pm->capsuletrace(&trace, pm->ps->origin, pm->mins, pm->maxs,
                             pm->ps->origin,
                             context);
            if (trace.allsolid == 0)
            {
                pm->ps->pm_flags &= 0xFFFFFFFC;
                goto L79;
            }
            if ((pm->cmd.buttons & 0x100) != 0)
                goto L80;
            BG_AddPredictableEventToPlayerstate(167, 0, pm->ps);
            goto L79;
        }
        if ((ps->pm_flags & 2) != 0)
        {
            pm->maxs.v.m128_f32[2] = ps->maxs[2];
            pm->capsuletrace(&trace, pm->ps->origin, pm->mins, pm->maxs,
                             pm->ps->origin,
                             context);
            if (trace.allsolid == 0)
            {
                pm->ps->pm_flags &= ~2;
                goto L79;
            }
            if ((pm->cmd.buttons & 0x100) != 0)
                goto L80;
            BG_AddPredictableEventToPlayerstate(166, 0, pm->ps);
            goto L79;
        }
        goto L80;
    }
    pm->maxs.v.m128_f32[2] = 50.0f;
    pm->capsuletrace(&trace, pm->ps->origin, pm->mins, pm->maxs,
                     pm->ps->origin, context);
    if (trace.allsolid == 0)
    {
        pm->ps->pm_flags &= ~1;
        pm->ps->pm_flags |= 2;
        goto L79;
    }
    if ((pm->cmd.buttons & 0x100) == 0)
    {
        BG_AddPredictableEventToPlayerstate(167, 0, pm->ps);
        goto L79;
    }
L79:
L80:
    ps = pm->ps;
    if (ps->viewHeightLerpTime == 0)
    {
        if ((ps->pm_flags & 1) != 0)
        {
            if (ps->viewHeightTarget == ps->standViewHeight)
            {
                ps->viewHeightTarget = ps->crouchViewHeight;
            }
            else
            {
                if (g_debugProneCheck.integer == 2)
                {
                    math::Dir3 groundNormal2;
                    groundNormal2.v.m128_f32[0] = 0.0f;
                    groundNormal2.v.m128_f32[1] = 0.0f;
                    groundNormal2.v.m128_f32[2] = 0.7f;
                    groundNormal2.v.m128_f32[3] = 0.0f;
                    typedef void (__cdecl* ProneTrace)(
                        trace_t*, const math::Position3*,
                        const math::Position3*, const math::Position3*,
                        const math::Position3*, const collision_context_t&);
                    typedef int (__cdecl* ProneContents)(
                        const math::Position3*,
                        const collision_context_t&);
                    BG_CheckProneValid(
                        ps->mClient, &ps->origin,
                        pm->maxs.v.m128_f32[0], 30.0f, ps->viewangles[1],
                        nullptr, nullptr, nullptr, 0,
                        ps->mGroundEntity.mHandle.mVal != 0, &groundNormal2,
                        (ProneTrace)pm->capsuletrace,
                        (ProneTrace)pm->boxtrace,
                        (ProneContents)pm->pointcontents, PCT_CLIENT,
                        60.0f);
                }
                if (ps->viewHeightTarget != ps->proneViewHeight)
                {
                    ps->viewHeightTarget = ps->proneViewHeight;
                    pm->ps->pm_flags |= 0x2000;
                    pm->ps->pm_time = 1800;
                }
            }
        }
        else if (ps->viewHeightTarget == ps->proneViewHeight)
        {
            ps->viewHeightTarget = ps->crouchViewHeight;
        }
        else if ((ps->pm_flags & 2) != 0)
        {
            ps->viewHeightTarget = ps->crouchViewHeight;
        }
        else
        {
            ps->viewHeightTarget = ps->standViewHeight;
        }
    }
    PM_ViewHeightAdjust();
    int EffectiveStance = PM_GetEffectiveStance(pm->ps);
    if (EffectiveStance == 1)
    {
        if ((ps->pm_flags & 3) != 0)
            pm->maxs.v.m128_f32[2] = 30.0f;
        else
            pm->maxs.v.m128_f32[2] = ps->maxs[2];
        pm->ps->eFlags |= 0x40;
        pm->ps->eFlags &= ~0x20;
    }
    else if (EffectiveStance == 2)
    {
        if ((ps->pm_flags & 3) != 0)
            pm->maxs.v.m128_f32[2] = 50.0f;
        else
            pm->maxs.v.m128_f32[2] = ps->maxs[2];
        pm->ps->eFlags |= 0x20;
        pm->ps->eFlags &= ~0x40;
    }
    else
    {
        pm->maxs.v.m128_f32[2] = ps->maxs[2];
        pm->ps->eFlags &= ~0x60;
    }
    if ((pm->ps->pm_flags & 1) != 0)
    {
        pm->trace = pm->boxtrace;
        pm->ps->eFlags |= 0x10;
        if (bWasProne == 0)
        {
            if (pm->cmd.forwardmove != 0 || pm->cmd.rightmove != 0)
            {
                pm->ps->pm_flags &= ~0x400;
                pm->ps->pm_flags &= ~0x20;
            }
            float startFlat[3];
            startFlat[0] = ps->origin.v.m128_f32[0];
            startFlat[1] = ps->origin.v.m128_f32[1];
            startFlat[2] = ps->origin.v.m128_f32[2] + 10.0f;
            math::Position3 startPos;
            native_to_cdl_pos3(&startPos, startFlat);
            pm->boxtrace(&trace, ps->origin, pm->mins, pm->maxs, startPos,
                         context);
            math::Position3 endPos;
            native_to_cdl_pos3(&endPos, trace.endpos.v.m128_f32);
            pm->boxtrace(&trace, endPos, pm->mins, pm->maxs, ps->origin,
                         context);
            ps->origin.v.m128_f32[0] = trace.endpos.v.m128_f32[0];
            ps->origin.v.m128_f32[1] = trace.endpos.v.m128_f32[1];
            ps->origin.v.m128_f32[2] = trace.endpos.v.m128_f32[2];
            ps->proneDirection = ps->viewangles[1];
            float downFlat[3];
            downFlat[0] = ps->origin.v.m128_f32[0];
            downFlat[1] = ps->origin.v.m128_f32[1];
            downFlat[2] = ps->origin.v.m128_f32[2] - 0.25f;
            math::Position3 downPos;
            native_to_cdl_pos3(&downPos, downFlat);
            pm->boxtrace(&trace, ps->origin, pm->mins, pm->maxs, downPos,
                         context);
            if (trace.startsolid != 0 || trace.fraction >= 1.0f)
            {
                pm->ps->proneDirectionPitch = 0.0f;
            }
            else
            {
                if (trace.normal.v.m128_f32[0] == 0.0f
                    && trace.normal.v.m128_f32[1] == 0.0f
                    && trace.normal.v.m128_f32[2] == 0.0f)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\bg_pmove.cpp";
                    AeAssert::gCurrentLine = 3448;
                    AeAssert::gCurrentExpr =
                        "trace.normal[0] || trace.normal[1] || trace.normal[2]";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                pm->ps->proneDirectionPitch = PitchForYawOnNormal(
                    pm->ps->proneDirection, trace.normal.v.m128_f32);
            }
            float v48 = AngleDelta(pm->ps->proneDirectionPitch,
                                   pm->ps->viewangles[0]);
            if (v48 < -45.0f)
                pm->ps->proneTorsoPitch = pm->ps->viewangles[0] - 45.0f;
            else if (v48 > 45.0f)
                pm->ps->proneTorsoPitch = pm->ps->viewangles[0] + 45.0f;
            else
                pm->ps->proneTorsoPitch = pm->ps->proneDirectionPitch;
        }
        return;
    }
    pm->trace = pm->capsuletrace;
    pm->ps->eFlags |= 0x10;
}

// ============================================================================
// FloatSign - ea: 0x65B9E0 (game.o inline, raw sign-bit test)
// ============================================================================
// ea: 0x0065B9E0
static int FloatSign(float x)
{
    return *(int*)&x < 0 ? -1 : 1;
}

// ============================================================================
// PM_Jump - ea: 0x614AF0 (bg_pmove.cpp)
// ============================================================================
// ea: 0x00614AF0
static void PM_Jump(float height)
{
    pml.groundPlane = 0;
    pml.walking = 0;
    pm->ps->mGroundEntity.mHandle.mVal = 0;
    pm->ps->velocity.v.m128_f32[2] =
        sqrtf((float)pm->ps->gravity * (height + height));
    pm->ps->pm_flags |= 0x2008;
    pm->ps->pm_time = 0;
    pm->ps->fJumpOriginZ = pm->ps->origin.v.m128_f32[2];
    MultiplayerMgr::sInst->AnimEvent(9);
}

// ============================================================================
// PM_JumpForSurface - ea: 0x6053F0 (bg_pmove.cpp)
// ============================================================================
// ea: 0x006053F0
static unsigned int PM_JumpForSurface()
{
    if ((pm->ps->pm_flags & 0x10) != 0)
        return 106;
    unsigned int result = PM_GroundSurfaceType();
    if (result != 0)
        result += 93;
    return result;
}

// ============================================================================
// PM_CheckJump - ea: 0x614B60 (bg_pmove.cpp)
// ============================================================================
// ea: 0x00614B60
static int PM_CheckJump()
{
    if (pm == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
        AeAssert::gCurrentLine = 1139;
        AeAssert::gCurrentExpr = "pm";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pm->cmd.serverTime - pm->ps->jumpTime < 1000)
        return 0;
    int pm_flags = pm->ps->pm_flags;
    if ((pm_flags & 0x800) != 0)
        return 0;
    int viewHeightTarget = pm->ps->viewHeightTarget;
    if (viewHeightTarget == pm->ps->crouchViewHeight
        || viewHeightTarget == pm->ps->proneViewHeight)
    {
        return 0;
    }
    if ((pm_flags & 0x20) != 0)
    {
        if (BG_GetInfoForWeapon(pm->ps->weapon)->weapClass == WEAPCLASS_LMG)
            return 0;
    }
    if (pm->cmd.upmove < 10)
        return 0;
    if ((pm->ps->pm_flags & 8) != 0)
    {
        pm->cmd.upmove = 0;
        return 0;
    }
    PM_Jump(39.0f);
    if ((pm->ps->pm_flags & 0x10) != 0)
    {
        pm->ps->velocity.v.m128_f32[2] =
            pm->ps->velocity.v.m128_f32[2] * 0.75f;
        float vFlatForward[3];
        vFlatForward[0] = pml.forward[0];
        vFlatForward[1] = pml.forward[1];
        vFlatForward[2] = 0.0f;
        VectorNormalize(vFlatForward);
        float vPushOffDir[3];
        if ((pm->ps->vLadderVec[0] * pml.forward[0])
                + (pm->ps->vLadderVec[2] * pml.forward[2])
                + (pm->ps->vLadderVec[1] * pml.forward[1])
            >= 0.0f)
        {
            vPushOffDir[0] = vFlatForward[0];
            vPushOffDir[1] = vFlatForward[1];
            vPushOffDir[2] = vFlatForward[2];
        }
        else
        {
            float v7 = (((pm->ps->vLadderVec[0] * vFlatForward[0])
                       + (pm->ps->vLadderVec[2] * vFlatForward[2]))
                       + (pm->ps->vLadderVec[1] * vFlatForward[1]))
                     * -2.0f;
            vPushOffDir[0] = (pm->ps->vLadderVec[0] * v7) + vFlatForward[0];
            vPushOffDir[1] = (pm->ps->vLadderVec[1] * v7) + vFlatForward[1];
            vPushOffDir[2] = (pm->ps->vLadderVec[2] * v7) + vFlatForward[2];
            VectorNormalize(vPushOffDir);
        }
        pm->ps->velocity.v.m128_f32[0] = vPushOffDir[0] * 128.0f;
        pm->ps->velocity.v.m128_f32[1] = vPushOffDir[1] * 128.0f;
        pm->ps->pm_flags &= ~0x10;
    }
    int v9 = PM_JumpForSurface();
    PM_AddEvent(v9);
    pm->ps->aimSpreadScale = pm->ps->aimSpreadScale + 64.0f;
    if (pm->ps->aimSpreadScale > 255.0f)
        pm->ps->aimSpreadScale = 255.0f;
    if ((0x10000 & pm->ps->pm_flags) != 0)
        pm->ps->fatigueScale = pm->ps->fatigueScale - 0.33333334f;
    if (pm->ps->fatigueScale < 0.0f)
        pm->ps->fatigueScale = 0.0f;
    return 1;
}

// ============================================================================
// PM_CmdScale - ea: 0x604FA0 (bg_pmove.cpp)
// ============================================================================
// ea: 0x00604FA0
static float PM_CmdScale(usercmd_s* cmd)
{
    int v2 = abs(cmd->forwardmove);
    int v3 = abs(cmd->rightmove);
    int scale = v2;
    if (v3 > v2)
    {
        v2 = v3;
        scale = v3;
    }
    int v4 = abs(cmd->upmove);
    if (v4 > v2)
    {
        v2 = v4;
        scale = v4;
    }
    if (v2 == 0)
        return 0.0f;
    if (pm == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
        AeAssert::gCurrentLine = 852;
        AeAssert::gCurrentExpr = "pm";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    PlayerState* ps = pm->ps;
    double forwardmove = cmd->forwardmove;
    int pm_flags = ps->pm_flags;
    float scalea = (float)ps->speed * (float)scale
                 / (sqrtf((float)(cmd->rightmove * cmd->rightmove)
                        + (float)(cmd->upmove * cmd->upmove)
                        + (float)(forwardmove * forwardmove))
                    * 127.0f);
    float result;
    if ((0x10000 & pm_flags) != 0)
        result = scalea;
    else if ((pm_flags & 0x80) == 0 && ps->leanf == 0.0f)
        result = ps->runSpeedScale * scalea;
    else
        result = ps->walkSpeedScale * scalea;
    if (ps->pm_type == 2)
        result = result * 3.0f;
    if (ps->pm_type == 3)
        return result * 6.0f;
    return result;
}

// ============================================================================
// PM_Accelerate - ea: 0x604E70 (bg_pmove.cpp)
// ============================================================================
// ea: 0x00604E70
static void PM_Accelerate(float* wishdir, float wishspeed, float accel)
{
    if (pm == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
        AeAssert::gCurrentLine = 787;
        AeAssert::gCurrentExpr = "pm";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    PlayerState* ps = pm->ps;
    float v4 = wishspeed
             - ((ps->velocity.v.m128_f32[1] * wishdir[1])
              + (ps->velocity.v.m128_f32[2] * wishdir[2])
              + (wishdir[0] * ps->velocity.v.m128_f32[0]));
    if (v4 > 0.0f)
    {
        float v5 = 100.0f;
        if (wishspeed >= 100.0f)
            v5 = wishspeed;
        float v6 = (pml.frametime * v5) * accel;
        if (v6 > v4)
            v6 = v4;
        if (ps->mGroundEntity.mHandle.mVal != 0)
            v6 = (1.0f / ps->friction) * v6;
        if (v6 > v4)
            v6 = v4;
        ps->velocity.v.m128_f32[0] = (wishdir[0] * v6) + ps->velocity.v.m128_f32[0];
        ps->velocity.v.m128_f32[1] = (wishdir[1] * v6) + ps->velocity.v.m128_f32[1];
        ps->velocity.v.m128_f32[2] = (v6 * wishdir[2]) + ps->velocity.v.m128_f32[2];
    }
}

// ============================================================================
// PM_LadderMove - ea: 0x6458E0 (bg_pmove.cpp)
// ============================================================================
// ea: 0x006458E0
void PM_LadderMove(const collision_context_t& context)
{
    if (pm == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_pmove.cpp";
        AeAssert::gCurrentLine = 5659;
        AeAssert::gCurrentExpr = "pm";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (PM_CheckJump() != 0)
    {
        PM_AirMove(context);
        pm->ps->jumpTime = pm->cmd.serverTime;
        return;
    }
    pml.forward[2] = 0.0f;
    VectorNormalize(pml.forward);
    pml.right[2] = 0.0f;
    float vTempRight[3];
    VectorNormalize2(pml.right, vTempRight);
    ProjectPointOnPlane(pml.right, vTempRight, pm->ps->vLadderVec);
    float fwdScale = PM_CmdScale(&pm->cmd);
    float wishvel[3];
    wishvel[0] = 0.0f;
    wishvel[1] = 0.0f;
    wishvel[2] = 0.0f;
    if (pm->cmd.forwardmove != 0)
        wishvel[2] = (float)pm->cmd.forwardmove * fwdScale * 0.5f;
    pm->cmd.rightmove = 0;
    float wishdir[3];
    float scale = VectorNormalize2(wishvel, wishdir);
    PM_Accelerate(wishdir, scale, 9.0f);
    if (pm->cmd.forwardmove == 0)
    {
        float v3 = (float)pm->ps->gravity * pml.frametime;
        if (pm->ps->velocity.v.m128_f32[2] <= 0.0f)
        {
            pm->ps->velocity.v.m128_f32[2] =
                v3 + pm->ps->velocity.v.m128_f32[2];
            if (pm->ps->velocity.v.m128_f32[2] > 0.0f)
                pm->ps->velocity.v.m128_f32[2] = 0.0f;
        }
        else
        {
            pm->ps->velocity.v.m128_f32[2] =
                pm->ps->velocity.v.m128_f32[2] - v3;
            if (pm->ps->velocity.v.m128_f32[2] < 0.0f)
                pm->ps->velocity.v.m128_f32[2] = 0.0f;
        }
    }
    if (pm->cmd.rightmove == 0)
    {
        float vSideDir[2];
        vSideDir[0] = pml.right[0];
        vSideDir[1] = pml.right[1];
        VectorNormalize2D(vSideDir);
        float fSideSpeed = (pm->ps->velocity.v.m128_f32[1] * vSideDir[1])
                         + (vSideDir[0] * pm->ps->velocity.v.m128_f32[0]);
        if (fSideSpeed != 0.0f)
        {
            pm->ps->velocity.v.m128_f32[0] =
                ((0.0f - fSideSpeed) * vSideDir[0])
                + pm->ps->velocity.v.m128_f32[0];
            pm->ps->velocity.v.m128_f32[1] =
                ((0.0f - fSideSpeed) * vSideDir[1])
                + pm->ps->velocity.v.m128_f32[1];
            float fSpeedDrop = pml.frametime * fSideSpeed * 16.0f;
            scale = fabsf(fSpeedDrop);
            if (fabsf(fSideSpeed) > scale)
            {
                float v10;
                if (scale >= 1.0f)
                    v10 = fSpeedDrop;
                else
                    v10 = (float)FloatSign(fSpeedDrop);
                pm->ps->velocity.v.m128_f32[0] =
                    ((fSideSpeed - v10) * vSideDir[0])
                    + pm->ps->velocity.v.m128_f32[0];
                pm->ps->velocity.v.m128_f32[1] =
                    ((fSideSpeed - v10) * vSideDir[1])
                    + pm->ps->velocity.v.m128_f32[1];
            }
        }
    }
    if (pml.walking == 0)
    {
        float v12 = 0.0f
                  - ((pm->ps->vLadderVec[1]
                      * pm->ps->velocity.v.m128_f32[1])
                   + (pm->ps->vLadderVec[0]
                      * pm->ps->velocity.v.m128_f32[0]));
        pm->ps->velocity.v.m128_f32[0] =
            (pm->ps->vLadderVec[0] * v12) + pm->ps->velocity.v.m128_f32[0];
        pm->ps->velocity.v.m128_f32[1] =
            (pm->ps->vLadderVec[1] * v12) + pm->ps->velocity.v.m128_f32[1];
        float v13 = wishvel[2] > 0.0f ? -500.0f : -250.0f;
        pm->ps->velocity.v.m128_f32[0] =
            (pm->ps->vLadderVec[0] * v13) + pm->ps->velocity.v.m128_f32[0];
        pm->ps->velocity.v.m128_f32[1] =
            (pm->ps->vLadderVec[1] * v13) + pm->ps->velocity.v.m128_f32[1];
    }
    PM_StepSlideMove(0);
    int v16 = (int)AngleDelta(vectoyaw(pm->ps->vLadderVec) + 180.0f,
                              pm->ps->viewangles[1]);
    if (abs(v16) > 75)
        v16 = v16 <= 0 ? 75 : -75;
    pm->ps->movementDir = v16;
}

// ============================================================================
// FindClosestVisibleBone - ea: 0x615320 (bg_misc.cpp)
// ============================================================================
static unsigned int spine_hash;   // ?spine_hash (game.o BSS 0xF58BB0)
static unsigned int helmet_hash;  // ?helmet_hash (game.o BSS 0xF58BAC)
static int s_fcvb_init;           // $S25_1 (game.o BSS 0xF58BB4)

// ea: 0x00615320
bool FindClosestVisibleBone(Entity* closestEnt,
                            const math::Position3& playerPosition,
                            const math::Position3& hitPosition,
                            math::Position3& enemyOrigin)
{
    if ((s_fcvb_init & 1) == 0)
    {
        s_fcvb_init |= 1;
        spine_hash = HashString::CalcHash("Bip01 Spine2");
    }
    if ((s_fcvb_init & 2) == 0)
    {
        s_fcvb_init |= 2;
        helmet_hash = HashString::CalcHash("Bip01 Helmet");
    }
    collision_context_t context;
    context.__vftable = (collision_context_t_vtbl*)0x00CD8F6C;
    context.pass_entity1.mHandle.mVal = closestEnt->mHandle.mHandle.mVal;
    context.pass_entity2.mHandle.mVal =
        EntityManager::sInst->GetPlayer(currCl)->mHandle.mHandle.mVal;
    context.pass_owner1.mHandle.mVal = 0;
    context.pass_owner2.mHandle.mVal = 0;
    context.contentmask = 0x2803021;
    int hit = 0;
    math::Position3 zeroMins;
    math::Position3 zeroMaxs;
    zeroMins.v = _mm_setzero_ps();
    zeroMaxs.v = _mm_setzero_ps();

    DObjSkelMat spineMat;
    bool bSpine = G_DObjGetWorldTagMatrix(closestEnt, spine_hash, &spineMat) != 0;
    DObjSkelMat helmetMat;
    bool bHelmet =
        G_DObjGetWorldTagMatrix(closestEnt, helmet_hash, &helmetMat) != 0;
    if (bSpine || bHelmet)
    {
        float spineDist2 = 3.4028235e38f;
        math::Position3 spinePos;
        math::Position3 helmetPos;
        if (bSpine)
        {
            spinePos.v.m128_f32[0] = spineMat.origin[0];
            spinePos.v.m128_f32[1] = spineMat.origin[1];
            spinePos.v.m128_f32[2] = spineMat.origin[2];
            spinePos.v.m128_f32[3] = spineMat.origin[3];
            g_SightTrace(&hit, &playerPosition, &zeroMins, &zeroMaxs,
                         &spinePos, &context);
            if (hit != 0)
            {
                bSpine = false;
            }
            else
            {
                __m128 v8 = _mm_sub_ps(spinePos.v, hitPosition.v);
                __m128 v9 = _mm_mul_ps(v8, v8);
                spineDist2 = v9.m128_f32[0]
                           + (v9.m128_f32[1] + v9.m128_f32[2] + v9.m128_f32[3]);
            }
        }
        if (bHelmet)
        {
            helmetPos.v.m128_f32[0] = helmetMat.origin[0];
            helmetPos.v.m128_f32[1] = helmetMat.origin[1];
            helmetPos.v.m128_f32[2] = helmetMat.origin[2];
            helmetPos.v.m128_f32[3] = helmetMat.origin[3];
            g_SightTrace(&hit, &playerPosition, &zeroMins, &zeroMaxs,
                         &helmetPos, &context);
            if (hit == 0)
            {
                __m128 v10 = _mm_sub_ps(helmetPos.v, hitPosition.v);
                __m128 v11 = _mm_mul_ps(v10, v10);
                float helmetDist2 = v11.m128_f32[0]
                                  + (v11.m128_f32[1] + v11.m128_f32[2]
                                     + v11.m128_f32[3]);
                if (bSpine && spineDist2 <= helmetDist2)
                {
                    enemyOrigin = spinePos;
                    return true;
                }
                enemyOrigin = helmetPos;
                return true;
            }
            if (!bSpine)
                goto LABEL_24;
            enemyOrigin = spinePos;
        }
        if (bSpine)
            return true;
LABEL_24:
        enemyOrigin.v.m128_f32[2] =
            (fabsf(closestEnt->r.absmin.v.m128_f32[2])
             + fabsf(closestEnt->r.absmax.v.m128_f32[2]))
            * 0.5f;
        return false;
    }
    math::Position3 end;
    g_SightTrace(&hit, &playerPosition, &zeroMins, &zeroMaxs, &end, &context);
    if (hit == 0)
    {
        enemyOrigin = closestEnt->r.currentOrigin;
        return true;
    }
    return false;
}

// ============================================================================
// bg_weapons.cpp config-string parsing (game.o)
// ============================================================================
extern char emptyString;                    // game.o BSS 0xF4EBFD
extern int  gInteractArmsWeaponIndex;       // ?gInteractArmsWeaponIndex@@3HA (game.o)
extern void* mem_heap_malloc(unsigned int size);  // ?mem_heap_malloc (mem_heap)
struct nglTexture;
extern nglTexture* cdGetTexture(TPakId pakId, const tlFixedString& name);
    // ?cdGetTexture@@YAPAUnglTexture@@W4TPakId@@ABVtlFixedString@@@Z (streamer.o)
extern void* Com_GetWeaponInfoMemory(int iSize, int* piParsed);
    // ?Com_GetWeaponInfoMemory@@YAPAXHPAH@Z (g_weapon.cpp wrapper)

// game.o static name tables (recovered from .rdata 0xDF5A5C..0xDF5B28)
static const char* const s_szWeapClassNames[18] = {
    "rifle", "mg", "smg", "lmg", "pistol", "grenade", "rocketlauncher",
    "turret", "spotter", "non-player", "sniper", "revive", "health",
    "ammo", "interact", "mine", "flag", "spread",
};
static const char* const s_szWeapAmmoTypeNames[6] = {
    "smg", "pistol", "rifle", "lmg", "hmg", "umg",
};
static const char* const s_szWeapOverlayReticleNames[5] = {
    "none", "crosshair", "FG42", "Springfield", "Gewehr43",
};
static const char* const s_szWeapStanceNames[3] = {
    "stand", "duck", "prone",
};
static const char* const s_szProjectileExplosionNames[9] = {
    "grenade", "smoke", "rocket", "molotov", "artillery", "mortar",
    "tank", "b17", "none",
};

// material_names - decal-suffix table (.rdata 0xDF8DF0, null terminated)
static const char* const material_names[24] = {
    "NONE", "BARK", "BRICK", "CARPET", "CLOTH", "CONCRETE", "DIRT",
    "FLESH", "FOLIAGE", "GLASS", "GRASS", "GRAVEL", "ICE", "METAL",
    "MUD", "PAPER", "PLASTER", "ROCK", "SAND", "SNOW", "WATER", "WOOD",
    "ASPHALT", nullptr,
};

// weaponInfoFields - bg_weapons.cpp cspField_t table (334 entries, .rdata 0xDF5B30)
static const cspField_t weaponInfoFields[334] = {
    { "displayName", 12, 0 },
    { "AIOverlayDescription", 16, 0 },
    { "modeName", 168, 0 },
    { "gunModel", 20, 0 },
    { "handModel", 24, 0 },
    { "attach1Model", 28, 0 },
    { "attach2Model", 32, 0 },
    { "attach3Model", 36, 0 },
    { "attach4Model", 40, 0 },
    { "attach5Model", 44, 0 },
    { "attach1Tag", 48, 0 },
    { "attach2Tag", 52, 0 },
    { "attach3Tag", 56, 0 },
    { "attach4Tag", 60, 0 },
    { "attach5Tag", 64, 0 },
    { "idleAnim", 72, 0 },
    { "emptyIdleAnim", 76, 0 },
    { "adsIdleAnim", 80, 0 },
    { "fireAnim", 84, 0 },
    { "holdFireAnim", 88, 0 },
    { "lastShotAnim", 92, 0 },
    { "rechamberAnim", 96, 0 },
    { "meleeAnim", 100, 0 },
    { "reloadAnim", 104, 0 },
    { "reloadEmptyAnim", 108, 0 },
    { "reloadStartAnim", 112, 0 },
    { "reloadEndAnim", 116, 0 },
    { "raiseAnim", 120, 0 },
    { "dropAnim", 124, 0 },
    { "altRaiseAnim", 128, 0 },
    { "altDropAnim", 132, 0 },
    { "adsFireAnim", 136, 0 },
    { "adsLastShotAnim", 140, 0 },
    { "adsRechamberAnim", 144, 0 },
    { "lmgDeployedAnim", 148, 0 },
    { "lmgDeployAnim", 152, 0 },
    { "lmgBreakdownAnim", 156, 0 },
    { "adsUpAnim", 160, 0 },
    { "adsDownAnim", 164, 0 },
    { "script", 2236, 0 },
    { "weaponType", 172, 8 },
    { "weaponClass", 176, 9 },
    { "weaponSlot", 180, 12 },
    { "slotStackable", 184, 5 },
    { "ammoType", 192, 10 },
    { "pickupWithoutSelect", 196, 5 },
    { "reticleCenter", 1240, 0 },
    { "reticleSide", 1244, 0 },
    { "reticleCenterSize", 1248, 4 },
    { "reticleSideSize", 1252, 4 },
    { "reticleMinOfs", 1256, 4 },
    { "sprintMoveF", 1260, 6 },
    { "sprintMoveR", 1264, 6 },
    { "sprintMoveU", 1268, 6 },
    { "duckedOfsF", 1308, 6 },
    { "duckedOfsR", 1312, 6 },
    { "duckedOfsU", 1316, 6 },
    { "proneOfsF", 1344, 6 },
    { "proneOfsR", 1348, 6 },
    { "proneOfsU", 1352, 6 },
    { "standMoveF", 1284, 6 },
    { "standMoveR", 1288, 6 },
    { "standMoveU", 1292, 6 },
    { "duckedMoveF", 1320, 6 },
    { "duckedMoveR", 1324, 6 },
    { "duckedMoveU", 1328, 6 },
    { "proneMoveF", 1356, 6 },
    { "proneMoveR", 1360, 6 },
    { "proneMoveU", 1364, 6 },
    { "sprintRotP", 1272, 6 },
    { "sprintRotY", 1276, 6 },
    { "sprintRotR", 1280, 6 },
    { "standRotP", 1296, 6 },
    { "standRotY", 1300, 6 },
    { "standRotR", 1304, 6 },
    { "duckedRotP", 1332, 6 },
    { "duckedRotY", 1336, 6 },
    { "duckedRotR", 1340, 6 },
    { "proneRotP", 1368, 6 },
    { "proneRotY", 1372, 6 },
    { "proneRotR", 1376, 6 },
    { "posMoveRate", 1380, 6 },
    { "posProneMoveRate", 1384, 6 },
    { "sprintMoveMinSpeed", 1388, 6 },
    { "standMoveMinSpeed", 1392, 6 },
    { "duckedMoveMinSpeed", 1396, 6 },
    { "proneMoveMinSpeed", 1400, 6 },
    { "posRotRate", 1404, 6 },
    { "posProneRotRate", 1408, 6 },
    { "standRotMinSpeed", 1416, 6 },
    { "duckedRotMinSpeed", 1420, 6 },
    { "proneRotMinSpeed", 1424, 6 },
    { "radiantName", 1428, 0 },
    { "worldModel", 1432, 0 },
    { "pickupModel", 1436, 0 },
    { "hudIcon", 1440, 0 },
    { "modeIcon", 1444, 0 },
    { "ammoIcon", 1448, 0 },
    { "startAmmo", 1452, 4 },
    { "ammoName", 1456, 0 },
    { "clipName", 1464, 0 },
    { "maxAmmo", 1472, 4 },
    { "clipSize", 1476, 4 },
    { "sharedAmmoCapName", 1480, 0 },
    { "sharedAmmoCap", 1488, 4 },
    { "damage", 1492, 4 },
    { "meleeDamage", 1512, 4 },
    { "sensitivityScale", 1592, 6 },
    { "damageInnerRadius", 1504, 4 },
    { "damageOuterRadius", 1508, 4 },
    { "minDamagePercent", 1500, 4 },
    { "fireDelay", 1520, 7 },
    { "meleeDelay", 1524, 7 },
    { "fireTime", 1528, 7 },
    { "rechamberTime", 1532, 7 },
    { "rechamberBoltTime", 1536, 7 },
    { "holdFireTime", 1540, 7 },
    { "meleeTime", 1544, 7 },
    { "reloadTime", 1548, 7 },
    { "reloadEmptyTime", 1552, 7 },
    { "reloadAddTime", 1556, 7 },
    { "reloadStartTime", 1560, 7 },
    { "reloadStartAddTime", 1564, 7 },
    { "reloadEndTime", 1568, 7 },
    { "dropTime", 1572, 7 },
    { "raiseTime", 1576, 7 },
    { "altDropTime", 1580, 7 },
    { "altRaiseTime", 1584, 7 },
    { "fuseTime", 1588, 7 },
    { "moveSpeedScale", 1596, 6 },
    { "idleCrouchFactor", 1700, 6 },
    { "idleProneFactor", 1704, 6 },
    { "gunMaxPitch", 1708, 6 },
    { "gunMaxYaw", 1712, 6 },
    { "swayMaxAngle", 1716, 6 },
    { "swayLerpSpeed", 1720, 6 },
    { "swayPitchScale", 1724, 6 },
    { "swayYawScale", 1728, 6 },
    { "swayHorizScale", 1732, 6 },
    { "swayVertScale", 1736, 6 },
    { "swayShellShockScale", 1740, 6 },
    { "adsSwayMaxAngle", 1744, 6 },
    { "adsSwayLerpSpeed", 1748, 6 },
    { "adsSwayPitchScale", 1752, 6 },
    { "adsSwayYawScale", 1756, 6 },
    { "adsSwayHorizScale", 1760, 6 },
    { "adsSwayVertScale", 1764, 6 },
    { "rifleBullet", 1772, 5 },
    { "twoHanded", 1768, 5 },
    { "semiAuto", 1776, 5 },
    { "boltAction", 1780, 5 },
    { "aimDownSight", 1784, 5 },
    { "rechamberWhileAds", 1788, 5 },
    { "clipOnly", 1820, 5 },
    { "cookOffHold", 1792, 5 },
    { "noBounce", 1796, 5 },
    { "noTumble", 1800, 5 },
    { "canMantle", 1804, 5 },
    { "smoke", 1808, 5 },
    { "offHand", 1812, 5 },
    { "cloth", 1816, 5 },
    { "wideListIcon", 1824, 5 },
    { "adsFire", 1828, 5 },
    { "adsOnly", 1832, 5 },
    { "animateCamReload", 1836, 5 },
    { "animateCamMelee", 1840, 5 },
    { "animateCamFire", 1844, 5 },
    { "doNotDrop", 1848, 5 },
    { "canSpot", 1852, 5 },
    { "holdToFire", 1856, 5 },
    { "killIcon", 1860, 0 },
    { "wideKillIcon", 1864, 5 },
    { "noPartialReload", 1868, 5 },
    { "segmentedReload", 1872, 5 },
    { "reloadAmmoAdd", 1876, 4 },
    { "reloadStartAdd", 1880, 4 },
    { "altWeapon", 1888, 0 },
    { "shotCount", 1896, 4 },
    { "dropAmmoMin", 1900, 4 },
    { "dropAmmoMax", 1904, 4 },
    { "takedamage", 1496, 4 },
    { "explosionRadius", 1912, 4 },
    { "explosionInnerDamage", 1916, 4 },
    { "explosionOuterDamage", 1920, 4 },
    { "projectileSpeed", 1924, 4 },
    { "projectileSpeedUp", 1928, 4 },
    { "projectileModel", 1932, 0 },
    { "projExplosionType", 1936, 14 },
    { "projExplosionEffect", 1940, 0 },
    { "projExplosionSound", 1944, 0 },
    { "projImpactExplode", 1948, 5 },
    { "triggerRadius", 1908, 4 },
    { "lobWeapon", 1952, 5 },
    { "projectileDLight", 1976, 4 },
    { "projectileRed", 1980, 6 },
    { "projectileGreen", 1984, 6 },
    { "projectileBlue", 1988, 6 },
    { "projectileRadius", 1960, 4 },
    { "projectileCount", 1956, 4 },
    { "projectileDelay", 1964, 4 },
    { "projectileSpacingMin", 1968, 4 },
    { "projectileSpacingMax", 1972, 4 },
    { "adsTransInTime", 1684, 7 },
    { "adsTransOutTime", 1688, 7 },
    { "adsIdleAmount", 1692, 6 },
    { "adsZoomFov", 1600, 6 },
    { "adsSensitivityScale", 1604, 6 },
    { "adsZoomInFrac", 1608, 6 },
    { "adsZoomOutFrac", 1612, 6 },
    { "adsOverlayShader", 1616, 0 },
    { "adsOverlayReticle", 1620, 11 },
    { "adsOverlayWidth", 1624, 6 },
    { "adsOverlayHeight", 1628, 6 },
    { "adsBobFactor", 1632, 6 },
    { "adsViewBobMult", 1636, 6 },
    { "adsAimPitch", 1992, 6 },
    { "adsCrosshairInFrac", 1996, 6 },
    { "adsCrosshairOutFrac", 2000, 6 },
    { "adsReloadTransTime", 2148, 7 },
    { "adsTransBlendTime", 2152, 7 },
    { "adsGunKickPitchMin", 2004, 6 },
    { "adsGunKickPitchMax", 2008, 6 },
    { "adsGunKickYawMin", 2012, 6 },
    { "adsGunKickYawMax", 2016, 6 },
    { "adsGunKickAccel", 2020, 6 },
    { "adsGunKickSpeedMax", 2024, 6 },
    { "adsGunKickSpeedDecay", 2028, 6 },
    { "adsGunKickStaticDecay", 2032, 6 },
    { "adsViewKickPitchMin", 2036, 6 },
    { "adsViewKickPitchMax", 2040, 6 },
    { "adsViewKickYawMin", 2044, 6 },
    { "adsViewKickYawMax", 2048, 6 },
    { "adsViewKickCenterSpeed", 2052, 6 },
    { "adsSpread", 2064, 6 },
    { "adsSpreadDucked", 2068, 6 },
    { "adsSpreadProne", 2072, 6 },
    { "hipSpreadStandMin", 1640, 6 },
    { "hipSpreadDuckedMin", 1644, 6 },
    { "hipSpreadProneMin", 1648, 6 },
    { "hipSpreadMax", 1652, 6 },
    { "hipSpreadDecayRate", 1656, 6 },
    { "hipSpreadFireAdd", 1660, 6 },
    { "hipSpreadTurnAdd", 1664, 6 },
    { "hipSpreadMoveAdd", 1668, 6 },
    { "hipSpreadDuckedDecay", 1672, 6 },
    { "hipSpreadProneDecay", 1676, 6 },
    { "hipReticleSidePos", 1680, 6 },
    { "hipIdleAmount", 1696, 6 },
    { "hipGunKickPitchMin", 2076, 6 },
    { "hipGunKickPitchMax", 2080, 6 },
    { "hipGunKickYawMin", 2084, 6 },
    { "hipGunKickYawMax", 2088, 6 },
    { "hipGunKickAccel", 2092, 6 },
    { "hipGunKickSpeedMax", 2096, 6 },
    { "hipGunKickSpeedDecay", 2100, 6 },
    { "hipGunKickStaticDecay", 2104, 6 },
    { "hipViewKickPitchMin", 2108, 6 },
    { "hipViewKickPitchMax", 2112, 6 },
    { "hipViewKickYawMin", 2116, 6 },
    { "hipViewKickYawMax", 2120, 6 },
    { "hipViewKickCenterSpeed", 2124, 6 },
    { "leftArc", 2156, 6 },
    { "rightArc", 2160, 6 },
    { "topArc", 2164, 6 },
    { "bottomArc", 2168, 6 },
    { "accuracy", 2172, 6 },
    { "vertTurnSpeed", 2176, 6 },
    { "horTurnSpeed", 2180, 6 },
    { "convergenceTime", 2184, 6 },
    { "maxRange", 2188, 6 },
    { "animHorRotateInc", 2192, 6 },
    { "playerPositionDist", 2196, 6 },
    { "stance", 188, 13 },
    { "useHintString", 2200, 0 },
    { "turretFov", 2208, 6 },
    { "turretAdsFov", 1600, 6 },
    { "minSpread", 1640, 6 },
    { "maxSpread", 1652, 6 },
    { "horizViewJitter", 2220, 6 },
    { "vertViewJitter", 2224, 6 },
    { "fireHeat", 2212, 6 },
    { "cooldownRate", 2216, 6 },
    { "aiEffectiveRange", 2136, 6 },
    { "aiMissRange", 2140, 6 },
    { "aiDamageMod", 2144, 6 },
    { "bulletConeAngle", 2228, 6 },
    { "adsBulletConeAngle", 2232, 6 },
    { "animIKOffsetTime", 2248, 6 },
    { "animIKOffsetForce", 2252, 6 },
    { "animIKOffsetDist", 2256, 6 },
    { "animIKPitchTime", 2260, 6 },
    { "animIKPitchForce", 2264, 6 },
    { "animIKPitchAngle", 2268, 6 },
    { "animIKTorsoRecoilPitchTime", 2272, 6 },
    { "animIKTorsoRecoilPitchForce", 2276, 6 },
    { "animIKTorsoRecoilPitchAngle", 2280, 6 },
    { "swirlControl", 1884, 5 },
    { "playerForwardOffset", 1344, 6 },
    { "playerRightOffset", 1348, 6 },
    { "playerUpOffset", 1352, 6 },
    { "damageMP", 1492, 4 },
    { "meleeDamageMP", 1512, 4 },
    { "damageInnerRadiusMP", 1504, 4 },
    { "damageOuterRadiusMP", 1508, 4 },
    { "minDamagePercentMP", 1500, 4 },
    { "fireDelayMP", 1520, 7 },
    { "meleeDelayMP", 1524, 7 },
    { "fireTimeMP", 1528, 7 },
    { "meleeTimeMP", 1544, 7 },
    { "explosionRadiusMP", 1912, 4 },
    { "explosionInnerDamageMP", 1916, 4 },
    { "explosionOuterDamageMP", 1920, 4 },
    { "weaponSlotMP", 180, 12 },
    { "maxAmmoMP", 1472, 4 },
    { "sensitivityScaleMP", 1592, 6 },
    { "adsZoomFovMP", 1600, 6 },
    { "adsTransInTimeMP", 1684, 7 },
    { "adsTransOutTimeMP", 1688, 7 },
    { "adsSensitivityScaleMP", 1604, 6 },
    { "adsSpreadMP", 2064, 6 },
    { "adsSpreadDuckedMP", 2068, 6 },
    { "adsSpreadProneMP", 2072, 6 },
    { "hipSpreadStandMinMP", 1640, 6 },
    { "hipSpreadDuckedMinMP", 1644, 6 },
    { "hipSpreadProneMinMP", 1648, 6 },
    { "hipSpreadMaxMP", 1652, 6 },
    { "hipSpreadDecayRateMP", 1656, 6 },
    { "hipSpreadFireAddMP", 1660, 6 },
    { "hipSpreadTurnAddMP", 1664, 6 },
    { "hipSpreadMoveAddMP", 1668, 6 },
    { "hipSpreadDuckedDecayMP", 1672, 6 },
    { "hipSpreadProneDecayMP", 1676, 6 },
    { "lmgDeployAnimMP", 152, 0 },
    { "lmgBreakdownAnimMP", 156, 0 },
};

// GdbFile result holder (mirrors g_cm_load.cpp TU-local struct)
struct GdbFile {
    void* mLayout;    // +0x00 (InplaceTree<uint,uint>*)
    void* mRecords;   // +0x04 (InplaceVector<GdbFileSet::Value>*)
};
class GdbFileManager {
public:
    static GdbFileManager* sInst;  // ?sInst@GdbFileManager@@2PAV1@A @ 0xF4F434
    GdbFile* GetGdbFile(GdbFile* result, TPakId pakId, const char* name,
                        const char* type);  // game.o 0x638750
};

// ============================================================================
// InitWeaponInfo - ea: 0x621ED0 (bg_weapons.cpp)
// ============================================================================
// ea: 0x00621ED0
static weaponFileInfo_t* InitWeaponInfo(int index, const cspField_t* pFieldList,
                                        int iNumFields)
{
    void* v3 = mem_heap_malloc(0x948u);
    weaponFileInfo_t* v4 = nullptr;
    if (v3 != nullptr)
    {
        // match-array segment tails zeroed before the full memset
        *(int*)((char*)v3 + 0x1C8) = 0;
        *(int*)((char*)v3 + 0x2CC) = 0;
        *(int*)((char*)v3 + 0x3D0) = 0;
        *(int*)((char*)v3 + 0x4D4) = 0;
        v4 = (weaponFileInfo_t*)v3;
    }
    memset(v4, 0, sizeof(weaponFileInfo_t));
    bg_weaponInfo[index] = v4;
    v4->index = index;
    v4->szInternalName = &emptyString;
    v4->internalNameHash = HashString::CalcHash(defaultFileName);
    if (iNumFields > 0)
    {
        const int* p_iOffset = &pFieldList->iOffset;
        for (int i = iNumFields; i != 0; --i)
        {
            if (p_iOffset[1] == 0)
                *(char**)((char*)v4 + p_iOffset[0]) = &emptyString;
            p_iOffset += 3;
        }
    }
    return v4;
}

// ============================================================================
// SetConfigString2 - ea: 0x606C10 (bg_weapons.cpp)
// ============================================================================
// ea: 0x00606C10
static void SetConfigString2(unsigned char* pMember, const char* pszKeyValue)
{
    if (*pszKeyValue != 0)
        *(const char**)pMember = pszKeyValue;
    else
        *(const char**)pMember = &emptyString;
}

// ============================================================================
// BG_ParseWeaponInfoSpecificFieldType - ea: 0x615D50 (bg_weapons.cpp)
// ============================================================================
// ea: 0x00615D50
static int BG_ParseWeaponInfoSpecificFieldType(unsigned char* pStruct,
                                               const char* pValue,
                                               int iFieldType)
{
    const char* szInternalName = *(const char**)(pStruct + 8);
    switch (iFieldType)
    {
    case 8:
    {
        int v3 = 0;
        while (_stricmp(pValue, s_szWeapTypeNames[v3]) != 0)
        {
            if (++v3 >= 9)
                break;
        }
        *(int*)(pStruct + 0xAC) = v3;
        if (v3 == 9)
            Com_Error(ERR_DROP, "Unknown weapon type \"%s\" in \"%s\"\n",
                      pValue, szInternalName);
        return 1;
    }
    case 9:
    {
        int v5 = 0;
        while (_stricmp(pValue, s_szWeapClassNames[v5]) != 0)
        {
            if (++v5 >= 18)
                break;
        }
        *(int*)(pStruct + 0xB0) = v5;
        if (v5 == 18)
            Com_Error(ERR_DROP, "Unknown weapon class \"%s\" in \"%s\"\n",
                      pValue, szInternalName);
        return 1;
    }
    case 10:
    {
        int v6 = 0;
        while (_stricmp(pValue, s_szWeapAmmoTypeNames[v6]) != 0)
        {
            if (++v6 >= 6)
                break;
        }
        *(int*)(pStruct + 0xC0) = v6;
        if (v6 == 6)
            Com_Error(ERR_DROP, "Unknown ammo type \"%s\" in \"%s\"\n",
                      pValue, szInternalName);
        return 1;
    }
    case 11:
    {
        int v7 = 0;
        while (_stricmp(pValue, s_szWeapOverlayReticleNames[v7]) != 0)
        {
            if (++v7 >= 5)
                break;
        }
        *(int*)(pStruct + 0x654) = v7;
        if (v7 == 5)
            Com_Error(ERR_DROP,
                      "Unknown weapon overlay reticle \"%s\" in \"%s\"\n",
                      pValue, szInternalName);
        return 1;
    }
    case 12:
    {
        int v8 = 0;
        while (_stricmp(pValue, s_szWeapSlotNames[v8]) != 0)
        {
            if (++v8 >= 10)
                break;
        }
        *(int*)(pStruct + 0xB4) = v8;
        if (v8 == 10)
            Com_Error(ERR_DROP, "Unknown weapon slot \"%s\" in \"%s\"\n",
                      pValue, szInternalName);
        return 1;
    }
    case 13:
    {
        int v9 = 0;
        while (_stricmp(pValue, s_szWeapStanceNames[v9]) != 0)
        {
            if (++v9 >= 3)
                break;
        }
        *(int*)(pStruct + 0xBC) = v9;
        if (v9 == 3)
            Com_Error(ERR_DROP, "Unknown weapon stance \"%s\" in \"%s\"\n",
                      pValue, szInternalName);
        return 1;
    }
    case 14:
    {
        int v10 = 0;
        while (_stricmp(pValue, s_szProjectileExplosionNames[v10]) != 0)
        {
            if (++v10 >= 9)
                break;
        }
        *(int*)(pStruct + 0x790) = v10;
        if (v10 == 9)
            Com_Error(ERR_DROP,
                      "Unknown projectile explosion \"%s\" in \"%s\"\n",
                      pValue, szInternalName);
        return 1;
    }
    default:
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 487;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored())
        {
            const char* v11 = va("Bad field type %i in %s\n", iFieldType,
                                 szInternalName);
            if (AeAssert::Warning(v11))
                __debugbreak();
        }
        Com_Error(ERR_DROP, "Bad field type %i in %s\n", iFieldType,
                  szInternalName);
        return 0;
    }
}

// ============================================================================
// ParseWeaponConfigString - ea: 0x63FD00 (bg_weapons.cpp)
// ============================================================================
// ea: 0x0063FD00
void ParseWeaponConfigString(const char* name, const ConfigString* cfgstr)
{
    if (bg_iNumWeapons >= 92)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\bg_weapons.cpp";
        AeAssert::gCurrentLine = 572;
        AeAssert::gCurrentExpr = "bg_iNumWeapons < (92)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Exceeded maximum weapons"))
            __debugbreak();
    }
    ++bg_iNumWeapons;
    weaponFileInfo_t* inited =
        InitWeaponInfo(bg_iNumWeapons, weaponInfoFields, 334);
    weaponFileInfo_t* pWeap = inited;
    if (*name == 0)
        inited->szInternalName = &emptyString;
    else
        inited->szInternalName = (char*)name;
    inited->internalNameHash = HashString::CalcHash(name);
    if (ParseConfigStringToStruct(
            (unsigned char*)inited, weaponInfoFields, 334, cfgstr, 15,
            BG_ParseWeaponInfoSpecificFieldType, SetConfigString2) != 0)
    {
        weaponFileInfo_t* v4 = bg_weaponInfo[bg_iNumWeapons];
        char* szGunXModel = *(char**)((char*)v4 + 0x14);
        if (v4->type == WEAPTYPE_INTERACT && szGunXModel[0] == 0)
        {
            gInteractArmsWeaponIndex = bg_iNumWeapons;
        }
    }
    else
    {
        bg_weaponInfo[bg_iNumWeapons--] = nullptr;
    }
    TPakId pakId = CurPakId();
    const char* const* namea = material_names;
    void** pDecals = (void**)((char*)pWeap + 0x8EC);
    char buf[512];
    while (namea < &material_names[23])
    {
        sprintf(buf, "%s_%s", pWeap->szInternalName, *namea);
        *pDecals = 0;
        GdbFile gdb;
        GdbFileManager::sInst->GetGdbFile(&gdb, pakId, buf, "decal");
        void* records = gdb.mRecords;
        if (records != nullptr)
        {
            if (*(unsigned int*)records == 0)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
                AeAssert::gCurrentLine = 81;
                AeAssert::gCurrentExpr = "index < mSize";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Bounds check"))
                    __debugbreak();
            }
            void** mList = *(void***)((char*)records + 4);
            if (mList[1] != nullptr)
            {
                tlFixedString v15((const char*)mList[1]);
                mList[1] = cdGetTexture(pakId, v15);
            }
            if (mList[4] != nullptr)
            {
                tlFixedString v13((const char*)mList[4]);
                mList[4] = cdGetTexture(pakId, v13);
            }
            if (mList[7] != nullptr)
            {
                tlFixedString v14((const char*)mList[7]);
                mList[7] = cdGetTexture(pakId, v14);
            }
            *pDecals = mList;
        }
        else
        {
            GdbFile gdb2;
            GdbFileManager::sInst->GetGdbFile(&gdb2, pakId, "m1garand_wood",
                                              "decal");
            if (gdb2.mRecords != nullptr)
                *pDecals = *(void***)((char*)gdb2.mRecords + 4);
        }
        ++pDecals;
        ++namea;
    }
}

// ============================================================================
// BG_ParseWeaponInfoFiles - ea: 0x63FF60 (bg_weapons.cpp)
// ============================================================================
static unsigned int s_none_hash;  // none_hash (game.o BSS 0xF591D4)
static int s_bgpw_init;           // $S30_3 (game.o BSS 0xF591D8)

// ea: 0x0063FF60
void BG_ParseWeaponInfoFiles()
{
    weaponFileInfo_t* inited = InitWeaponInfo(0, weaponInfoFields, 334);
    if ("none"[0] != 0)
        inited->szInternalName = (char*)"none";
    else
        inited->szInternalName = &emptyString;
    if ((s_bgpw_init & 1) == 0)
    {
        s_bgpw_init |= 1;
        s_none_hash = HashString::CalcHash("none");
    }
    inited->internalNameHash = s_none_hash;
    ConfigStringManager* v1 = ConfigStringManager_sInst;
    bg_iNumWeapons = 0;
    bg_iNumSharedAmmoCaps = 0;
    TPakId v2 = CurPakId();
    v1->CallbackSearch(v2, "WEAPONFILE", ParseWeaponConfigString);
}

// ============================================================================
// BG_SetupTransitionTimes - ea: 0x606C30 (bg_weapons.cpp)
// ============================================================================
// ea: 0x00606C30
int BG_SetupTransitionTimes()
{
    int result = bg_iNumWeapons;
    int v1 = 1;
    if (bg_iNumWeapons >= 1)
    {
        weaponFileInfo_t** v2 = bg_weaponInfo;
        do
        {
            weaponFileInfo_t* v3 = v2[v1];
            int iAdsTransInTime = *(int*)((char*)v3 + 0x694);
            if (iAdsTransInTime <= 0)
                v3->fOOPosAnimLength[0] = 0.0033333334f;
            else
                v3->fOOPosAnimLength[0] = 1.0f / (float)iAdsTransInTime;
            int iAdsTransOutTime = *(int*)((char*)v3 + 0x698);
            if (iAdsTransOutTime <= 0)
                v3->fOOPosAnimLength[1] = 0.0020000001f;
            else
                v3->fOOPosAnimLength[1] = 1.0f / (float)iAdsTransOutTime;
            result = bg_iNumWeapons;
            ++v1;
        } while (v1 <= bg_iNumWeapons);
    }
    return result;
}

// ============================================================================
// BG_SetupWeaponAlts - ea: 0x606CC0 (bg_weapons.cpp)
// ============================================================================
// ea: 0x00606CC0
int BG_SetupWeaponAlts()
{
    int v0 = bg_iNumWeapons;
    weaponFileInfo_t** v1 = bg_weaponInfo;
    for (int j = 1; j <= bg_iNumWeapons; ++j)
    {
        v1[j]->iAltWeaponIndex = 0;
        v0 = bg_iNumWeapons;
    }
    int result = 1;
    int i = 1;
    if (v0 >= 1)
    {
        while (1)
        {
            weaponFileInfo_t* v4 = v1[result];
            weaponFileInfo_t* pWeap = v4;
            if (v4->iAltWeaponIndex == 0 && v4->szAltWeaponName[0] != 0
                && v4->iAltWeaponIndex == 0)
            {
                int* p_iAltWeaponIndex = &v4->iAltWeaponIndex;
                while (1)
                {
                    int v5 = 1;
                    if (v0 >= 1)
                    {
                        weaponFileInfo_t* v6;
                        while (1)
                        {
                            v6 = bg_weaponInfo[v5];
                            if (_stricmp(v4->szAltWeaponName,
                                         v6->szInternalName) == 0)
                            {
                                break;
                            }
                            if (++v5 > bg_iNumWeapons)
                                goto LABEL_23;
                        }
                        int slot = v4->slot;
                        *p_iAltWeaponIndex = v5;
                        if (slot != v6->slot)
                        {
                            AeAssert::gCurrentAuthor = AeAssert::JRS;
                            AeAssert::gCurrentFile =
                                "c:\\cod\\code\\game\\bg_weapons.cpp";
                            AeAssert::gCurrentLine = 1063;
                            AeAssert::gCurrentExpr = nullptr;
                            if (!AeAssert::IsIgnored()
                                && AeAssert::Warning(
                                    "weapon '%s' does not have same "
                                    "weaponSlot setting as its alt weapon "
                                    "'%s'",
                                    v4->szInternalName, v6->szInternalName))
                                __debugbreak();
                        }
                        if (v4->bSlotStackable != v6->bSlotStackable)
                        {
                            AeAssert::gCurrentAuthor = AeAssert::JRS;
                            AeAssert::gCurrentFile =
                                "c:\\cod\\code\\game\\bg_weapons.cpp";
                            AeAssert::gCurrentLine = 1067;
                            AeAssert::gCurrentExpr = nullptr;
                            if (!AeAssert::IsIgnored()
                                && AeAssert::Warning(
                                    "weapon '%s' does not have same "
                                    "slotStackable setting as its alt weapon "
                                    "'%s'",
                                    v4->szInternalName, v6->szInternalName))
                                __debugbreak();
                        }
                    }
                LABEL_23:
                    if (*p_iAltWeaponIndex == 0)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::JRS;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\bg_weapons.cpp";
                        AeAssert::gCurrentLine = 1074;
                        AeAssert::gCurrentExpr = nullptr;
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Warning(
                                "could not find altWeapon '%s' for weapon "
                                "'%s'",
                                v4->szAltWeaponName, v4->szInternalName))
                            __debugbreak();
                    }
                    v4 = bg_weaponInfo[v5];
                    p_iAltWeaponIndex = &v4->iAltWeaponIndex;
                    if (v4->iAltWeaponIndex != 0)
                        break;
                    v0 = bg_iNumWeapons;
                }
                if (v4 != pWeap)
                {
                    AeAssert::gCurrentAuthor = AeAssert::JRS;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\bg_weapons.cpp";
                    AeAssert::gCurrentLine = 1078;
                    AeAssert::gCurrentExpr = nullptr;
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Warning(
                            "weapon '%s' has a bad altWeapon '%s'",
                            pWeap->szInternalName, pWeap->szAltWeaponName))
                        __debugbreak();
                }
            }
            v0 = bg_iNumWeapons;
            result = ++i;
            if (i > bg_iNumWeapons)
                break;
            v1 = bg_weaponInfo;
        }
    }
    return result;
}

// ============================================================================
// BG_SetupUseHintStrings - ea: 0x606F30 (bg_weapons.cpp)
// ============================================================================
// ea: 0x00606F30
int BG_SetupUseHintStrings()
{
    int result = bg_iNumWeapons;
    for (int i = 1; i <= bg_iNumWeapons; ++i)
    {
        weaponFileInfo_t* v2 = bg_weaponInfo[i];
        if (v2->szUseHintString[0] != 0
            && G_GetHintStringIndex(&v2->iUseHintStringIndex,
                                    v2->szUseHintString) == 0)
        {
            Com_Error(ERR_DROP,
                      "Too many different hintstring values on weapons. "
                      "Max allowed is %i different strings",
                      32);
        }
        result = bg_iNumWeapons;
    }
    return result;
}

// ============================================================================
// BG_SetupWeaponInfo - ea: 0x640020 (bg_weapons.cpp)
// ============================================================================
static int s_bgswi_init;  // init_0 (game.o BSS 0xF591DC)

// ea: 0x00640020
void BG_SetupWeaponInfo()
{
    s_bgswi_init = 1;
    Com_DPrintf("----------------------\n");
    Com_DPrintf("Game: BG_SetupWeaponInfo\n");
    int iArraySource = 0;
    bg_weaponInfo = (weaponFileInfo_t**)Com_GetWeaponInfoMemory(
        368, &iArraySource);
    if (bg_weaponInfo == nullptr)
        Com_Error(ERR_DROP, "Could not allocate weapon info array");
    memset(bg_szWeapAmmoNames, 0, 368);
    memset(bg_szWeapClipNames, 0, 368);
    bg_iWeapAmmoMaxs[0] = 0;
    bg_szWeapAmmoNames[0] = "none";
    bg_iNumAmmoTypes = 1;
    bg_iWeapClipSizes[0] = 0;
    bg_szWeapClipNames[0] = "none";
    bg_iNumWeapClips = 1;
    BG_ParseWeaponInfoFiles();
    BG_SetupTransitionTimes();
    BG_SetupAmmoIndexes();
    BG_SetupSharedAmmoIndexes();
    BG_SetupClipIndexes();
    BG_FillInWeaponItems();
    BG_SetupWeaponAlts();
    BG_SetupUseHintStrings();
    Com_DPrintf("----------------------\n");
}

// ============================================================================
// PM_UpdateViewAngles - ea: 0x63D2B0 (bg_pmove.cpp)
// ============================================================================
static const float s_colorWhite[4] = { 1.0f, 1.0f, 1.0f, 1.0f };  // colorWhite

// ea: 0x0063D2B0
void PM_UpdateViewAngles(
    PlayerState* ps, usercmd_s* cmd, usercmd_s* oldcmd, int msec,
    void (__cdecl* capsuleTrace)(trace_t*, const math::Position3&,
                                 const math::Position3&, const math::Position3&,
                                 const math::Position3&,
                                 const collision_context_t&))
{
    math::Dir3 groundNormal;
    groundNormal.v = _mm_setr_ps(0.0f, 0.0f, 0.7f, 0.0f);
    typedef void (__cdecl* ProneTracePtr)(
        trace_t*, const math::Position3*, const math::Position3*,
        const math::Position3*, const math::Position3*,
        const collision_context_t&);
    ProneTracePtr proneTrace = (ProneTracePtr)capsuleTrace;
    if (ps->pm_type == 4)
        return;
    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    int eFlags = player->s.eFlags;
    if (((eFlags & 0x100000) == 0 || IsPlayerFullySeatedInVehicle(player))
        && (pml.pWeap == nullptr
            || ((weaponFileInfo_t*)pml.pWeap)->type != 7))
    {
        int pm_type = ps->pm_type;
        if (pm_type != 5)
        {
            if (pm_type >= 6)
            {
                PM_UpdateLean(
                    ps, cmd,
                    (void (__cdecl*)(trace_t*, const math::Position3*,
                                     const math::Position3*,
                                     const math::Position3*,
                                     const math::Position3*,
                                     const collision_context_t*))capsuleTrace);
                return;
            }
            int v10 = (int16_t)((int16_t)ps->delta_angles[0]
                                + (int16_t)cmd->angles[0]);
            float oldYaw = ps->viewangles[1];
            if (v10 > 14500)
            {
                ps->delta_angles[0] = 14500 - cmd->angles[0];
                v10 = 14500;
            }
            else if (v10 < -14500)
            {
                ps->delta_angles[0] = -14500 - cmd->angles[0];
                v10 = -14500;
            }
            ps->viewangles[0] = (float)v10 * 0.0054931641f;
            ps->viewangles[1] =
                (float)(int16_t)((int16_t)ps->delta_angles[1]
                                 + (int16_t)cmd->angles[1])
                * 0.0054931641f;
            ps->viewangles[2] =
                (float)(int16_t)((int16_t)ps->delta_angles[2]
                                 + (int16_t)cmd->angles[2])
                * 0.0054931641f;
            float oldYaw2 = ps->viewangles[1];

            if ((0x100000 & ps->eFlags) != 0)
            {
                if (pm != nullptr
                    && (pm->vehicleViewClamp[0] != 0.0f
                        || pm->vehicleViewClamp[1] != 0.0f
                        || pm->vehicleViewClamp[2] != 0.0f))
                {
                    for (int i = 0; i < 3; ++i)
                    {
                        if (fabsf(pm->vehicleViewClamp[i]) >= 1.0f)
                        {
                            float fLadderFacing =
                                AngleDelta(pm->vehicleAngles[i],
                                           ps->viewangles[i]);
                            if (fabsf(fLadderFacing)
                                > fabsf(pm->vehicleViewClamp[i]))
                            {
                                float v16 =
                                    fLadderFacing <= pm->vehicleViewClamp[i]
                                        ? pm->vehicleViewClamp[i]
                                              + fLadderFacing
                                        : fLadderFacing
                                              - pm->vehicleViewClamp[i];
                                ps->delta_angles[i] +=
                                    (int)(v16 * 182.04445f) & 0xFFFF;
                                float v18 =
                                    v16 <= 0.0f
                                        ? pm->vehicleAngles[i]
                                              + pm->vehicleViewClamp[i]
                                        : pm->vehicleAngles[i]
                                              - pm->vehicleViewClamp[i];
                                ps->viewangles[i] =
                                    AngleNormalize360Accurate(v18);
                            }
                        }
                    }
                }
            }
            else if ((ps->pm_flags & 0x10) != 0
                     && ps->mGroundEntity.mHandle.mVal == 0
                     && bg_ladder_yawcap.integer != 0)
            {
                float ladderYaw = vectoyaw(ps->vLadderVec) + 180.0f;
                float fLadderFacing =
                    AngleDelta(ladderYaw, ps->viewangles[1]);
                float cap = (float)bg_ladder_yawcap.integer;
                if (fLadderFacing > cap || -cap > fLadderFacing)
                {
                    float v23 = fLadderFacing <= cap
                                    ? cap + fLadderFacing
                                    : fLadderFacing - cap;
                    ps->delta_angles[1] +=
                        (int)(v23 * 182.04445f) & 0xFFFF;
                    ps->viewangles[1] = AngleNormalize360Accurate(
                        v23 <= 0.0f ? ladderYaw - cap : ladderYaw + cap);
                }
            }

            if ((dword_106000 & ps->eFlags) == 0)
            {
                int pm_flags = ps->pm_flags;
                if ((pm_flags & 1) != 0
                    || (pm_flags & 0x20) != 0
                        && BG_GetInfoForWeapon(ps->weapon)->weapClass
                               == WEAPCLASS_LMG)
                {
                    float fLadderFacing =
                        AngleDelta(ps->proneDirection, ps->viewangles[1]);
                    int weapon = ps->weapon;
                    float yawcap = (float)bg_prone_yawcap.integer;
                    if (BG_GetInfoForWeapon(weapon)->weapClass
                        == WEAPCLASS_LMG)
                    {
                        yawcap = (float)bg_lmg_yawcap.integer;
                    }
                    int bProneBlocked = 0;
                    if (g_debugProneCheck.integer != 0)
                    {
                        float vForward[3];
                        vForward[0] = ps->origin.v.m128_f32[0];
                        vForward[1] = ps->origin.v.m128_f32[1];
                        vForward[2] =
                            (float)ps->proneViewHeight
                            + ps->origin.v.m128_f32[2];
                        float vEnd[3];
                        AnglesToForward(ps->viewangles, vEnd);
                        float end2[3];
                        end2[0] = vEnd[0] * 18.0f + vForward[0];
                        end2[1] = vEnd[1] * 18.0f + vForward[1];
                        end2[2] = vEnd[2] * 18.0f + vForward[2];
                        G_DebugLine(vForward, end2, s_colorWhite, 1, 1);
                        G_DebugArc(vForward, 16.0f,
                                   ps->proneDirection
                                       - bg_prone_yawcap.value,
                                   ps->proneDirection
                                       + bg_prone_yawcap.value,
                                   s_colorWhite, 1, 1);
                    }
                    if ((ps->pm_flags & 1) != 0
                        && (BG_GetInfoForWeapon(ps->weapon)->weapClass
                                != WEAPCLASS_LMG
                            || (ps->pm_flags & 0x20) == 0)
                        && (fLadderFacing > (yawcap - 5.0f)
                            || (0.0f - (yawcap - 5.0f)) > fLadderFacing
                            || (cmd->forwardmove != 0 || cmd->rightmove != 0)
                                && fLadderFacing != 0.0f))
                    {
                        float v27 = (float)msec * 0.001f * 55.0f;
                        float yaw_cap;
                        if (v27 <= fabsf(fLadderFacing))
                            yaw_cap = fLadderFacing <= 0.0f
                                          ? ps->proneDirection + v27
                                          : ps->proneDirection - v27;
                        else
                            yaw_cap = ps->viewangles[1];
                        int bRetry = 1;
                        if (BG_CheckProneTurned(ps, 0, yaw_cap, proneTrace))
                        {
                            goto LABEL_71;
                        }
                        while (bRetry)
                        {
                            fLadderFacing =
                                AngleDelta(ps->proneDirection, yaw_cap);
                            float v32;
                            if (fabsf(fLadderFacing) <= 1.0f)
                            {
                                bRetry = 0;
                                bProneBlocked = 1;
                                v32 = fLadderFacing;
                            }
                            else
                            {
                                bRetry = 1;
                                v32 = fLadderFacing <= 0.0f ? -1.0f : 1.0f;
                            }
                            yaw_cap =
                                AngleNormalize360Accurate(yaw_cap + v32);
                            if (BG_CheckProneTurned(ps, 0, yaw_cap,
                                                    proneTrace))
                            {
                                goto LABEL_71;
                            }
                        }
                        goto LABEL_72;
LABEL_71:
                        if (BG_CheckProneValid(
                                ps->mClient, &ps->origin, ps->maxs[0],
                                30.0f, ps->viewangles[1], nullptr, nullptr,
                                nullptr, 1,
                                ps->mGroundEntity.mHandle.mVal != 0,
                                &groundNormal, proneTrace, nullptr,
                                nullptr, PCT_CLIENT, 45.0f) != 0
                            && BG_CheckProneValid(
                                   ps->mClient, &ps->origin, ps->maxs[0],
                                   30.0f, yaw_cap, nullptr, nullptr,
                                   nullptr, 1,
                                   ps->mGroundEntity.mHandle.mVal != 0,
                                   &groundNormal, proneTrace, nullptr,
                                   nullptr, PCT_CLIENT, 45.0f) != 0)
                        {
                            ps->proneDirection = yaw_cap;
                        }
                        else
                        {
                            bProneBlocked = 1;
                        }
LABEL_72:
                        ;
                    }
                    fLadderFacing =
                        AngleDelta(ps->proneDirection, ps->viewangles[1]);
                    if (fLadderFacing != 0.0f && (ps->pm_flags & 1) != 0
                        && BG_GetInfoForWeapon(ps->weapon)->weapClass
                               != WEAPCLASS_LMG)
                    {
                        float yaw_cap = ps->proneDirection;
                        int bRetry = 1;
                        while (1)
                        {
                            int v38 = BG_CheckProneValid(
                                ps->mClient, &ps->origin, ps->maxs[0],
                                30.0f, yaw_cap, nullptr, nullptr, nullptr,
                                1, ps->mGroundEntity.mHandle.mVal != 0,
                                &groundNormal, proneTrace, nullptr,
                                nullptr, PCT_CLIENT, 45.0f);
                            if (v38 != 0
                                && BG_CheckProneTurned(ps, 0, yaw_cap,
                                                       proneTrace))
                            {
                                ps->proneDirection = yaw_cap;
                                goto LABEL_92;
                            }
                            if (bRetry == 0)
                                goto LABEL_92;
                            float v39;
                            if (fabsf(fLadderFacing) <= 1.0f)
                            {
                                bRetry = 0;
                                v39 = fLadderFacing;
                            }
                            else
                            {
                                bRetry = 1;
                                v39 = fLadderFacing <= 0.0f ? -1.0f : 1.0f;
                            }
                            ps->delta_angles[1] +=
                                (int)(v39 * 182.04445f) & 0xFFFF;
                            ps->viewangles[1] = AngleNormalize360Accurate(
                                ps->viewangles[1] + v39);
                            fLadderFacing = AngleDelta(
                                ps->proneDirection, ps->viewangles[1]);
                            bProneBlocked = 1;
                            if (v38 == 0)
                            {
                                yaw_cap = AngleNormalize360Accurate(
                                    yaw_cap + fLadderFacing);
                            }
                        }
                    }
LABEL_92:
                    if (bProneBlocked != 0)
                        player->client->mProneBlockedTime = level.time;
                    if (fLadderFacing > yawcap || -yawcap > fLadderFacing)
                    {
                        float v44 = fLadderFacing <= yawcap
                                        ? yawcap + fLadderFacing
                                        : fLadderFacing - yawcap;
                        ps->delta_angles[1] +=
                            (int)(v44 * 182.04445f) & 0xFFFF;
                        ps->viewangles[1] = AngleNormalize360Accurate(
                            v44 <= 0.0f
                                ? ps->proneDirection + yawcap
                                : ps->proneDirection - yawcap);
                    }
                    if (bProneBlocked != 0)
                    {
                        ps->pm_flags |= 0x8000;
                        float v48 = AngleDelta(oldYaw, ps->viewangles[1]);
                        if (fabsf(v48) <= 1.0f)
                        {
                            float v49 =
                                AngleDelta(oldYaw2, ps->viewangles[1]);
                            if (v49 * v48 > 0.0f)
                            {
                                float v50 = v48 * 0.98000002f;
                                ps->viewangles[1] =
                                    AngleNormalize360Accurate(
                                        ps->viewangles[1] + v50);
                                ps->delta_angles[1] +=
                                    (int)(v50 * 182.04445f) & 0xFFFF;
                            }
                        }
                    }
                    fLadderFacing =
                        AngleDelta(ps->proneTorsoPitch, ps->viewangles[0]);
                    if (fLadderFacing > 45.0f || fLadderFacing < -45.0f)
                    {
                        float v51 = fLadderFacing <= 45.0f
                                        ? fLadderFacing + 45.0f
                                        : fLadderFacing - 45.0f;
                        ps->delta_angles[0] +=
                            (int)(v51 * 182.04445f) & 0xFFFF;
                        ps->viewangles[0] = AngleNormalize180Accurate(
                            v51 <= 0.0f
                                ? ps->proneTorsoPitch + 45.0f
                                : ps->proneTorsoPitch - 45.0f);
                    }
                }
            }
            PM_UpdateStickyAim(ps, cmd, oldcmd);
            PM_UpdateMeleeAssistAim(ps, msec);
            if (ps->pm_type != 3 && ps->pm_type != 2 && ps->pm_type != 4)
                PM_UpdateLean(
                    ps, cmd,
                    (void (__cdecl*)(trace_t*, const math::Position3*,
                                     const math::Position3*,
                                     const math::Position3*,
                                     const math::Position3*,
                                     const collision_context_t*))capsuleTrace);
        }
    }
}
