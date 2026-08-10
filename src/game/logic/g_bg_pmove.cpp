// ============================================================================
// g_bg_pmove.cpp - game.o bg_pmove/bg_misc/bg_weapons helpers
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <stdio.h>
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
