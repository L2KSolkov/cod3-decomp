// ============================================================================
// g_bg_pmove.cpp - game.o bg_pmove/bg_misc/bg_weapons helpers
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

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
