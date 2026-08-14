// ============================================================================
// touch.cpp - mp_actors.o G_TouchTriggersAndVehicles + Actor_Grenade_CheckToss
// ============================================================================

#include "game/logic/g_local.h"

#include <string.h>

extern level_locals_t level;           // ?level@@3Ulevel_locals_t@@A @ 0xEC9650

// touch query scratch state (mp_actors.o data)
static TouchEntityData tData;      // ?tData@@3UTouchEntityData@@A @ 0xF995B0
static int s_tDataInitGuard;       // $S16_5 @ 0xF99850
static float touch_range[3] = { 0.0f, 0.0f, 0.0f };  // 0xE37CC4

// ea: 0x007861B0
void G_TouchTriggersAndVehicles(Entity* pEnt,
                                const math::Position3& origin,
                                const collision_context_t& context)
{
    if ((s_tDataInitGuard & 1) == 0)
    {
        s_tDataInitGuard |= 1;
        memset(tData.touch, 0, sizeof(tData.touch));
    }
    tData.mins.v.m128_f32[0] = origin.v.m128_f32[0] - touch_range[0];
    tData.mins.v.m128_f32[1] = origin.v.m128_f32[1] - touch_range[1];
    tData.mins.v.m128_f32[2] = origin.v.m128_f32[2] - touch_range[2];
    tData.maxs.v.m128_f32[0] = origin.v.m128_f32[0] + touch_range[0];
    tData.maxs.v.m128_f32[1] = origin.v.m128_f32[1] + touch_range[1];
    tData.maxs.v.m128_f32[2] = origin.v.m128_f32[2] + touch_range[2];
    tData.num = CM_AreaEntities(
        tData.mins, tData.maxs, tData.touch, 128,
        0x800000 | GetEntityTouchTriggerType(pEnt));
    if (tData.num != 0)
    {
        tData.mins.v.m128_f32[0] =
            pEnt->r.mins.v.m128_f32[0] + origin.v.m128_f32[0];
        tData.mins.v.m128_f32[1] =
            pEnt->r.mins.v.m128_f32[1] + origin.v.m128_f32[1];
        tData.mins.v.m128_f32[2] =
            pEnt->r.mins.v.m128_f32[2] + origin.v.m128_f32[2];
        tData.maxs.v.m128_f32[0] =
            pEnt->r.maxs.v.m128_f32[0] + origin.v.m128_f32[0];
        tData.maxs.v.m128_f32[1] =
            pEnt->r.maxs.v.m128_f32[1] + origin.v.m128_f32[1];
        tData.maxs.v.m128_f32[2] =
            pEnt->r.maxs.v.m128_f32[2] + origin.v.m128_f32[2];
        G_DoTouchTriggers(pEnt, &origin, &tData,
                          (collision_context_t*)&context);
        G_TouchVehicles(pEnt, &origin, &tData,
                        (collision_context_t*)&context);
    }
}

// ea: 0x0077C630
int __fastcall Actor_Grenade_CheckToss(actor_s* pSelf,
                                       const float* const vOffset,
                                       const float* const vForward,
                                       Broc::string method,
                                       float* const vPosOut,
                                       float* const vVelOut,
                                       int bRechecking)
{
    (void)pSelf;
    (void)vOffset;
    (void)vForward;
    (void)vPosOut;
    (void)vVelOut;
    (void)bRechecking;
    method.~string();
    return 0;
}
