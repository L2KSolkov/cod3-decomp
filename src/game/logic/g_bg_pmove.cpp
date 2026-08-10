// ============================================================================
// g_bg_pmove.cpp - game.o bg_pmove/bg_misc/bg_weapons helpers
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

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
extern vmCvar_t bg_nofatigue;  // ?bg_nofatigue@@3UvmCvar_t@@A (game.o)
extern vmCvar_t g_gravity;     // ?g_gravity@@3UvmCvar_t@@A
extern weaponFileInfo_t** bg_weaponInfo;  // ?bg_weaponInfo@@3PAPAUweaponFileInfo_t@@A (game.o)
extern const char** pEventNamesList;      // ?pEventNamesList@@3PAPBDA (game.o)
extern const char* szWeapTypeNames[9];    // ?szWeapTypeNames@@3PAPBDA (game.o)

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
