// ============================================================================
// g_client.cpp - client-side game logic (g.o: g_client.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

// .bss @ 0xF641A8 (file-local per-client spot timer, stride 1580 ints)
static int dword_F641A8[4 * 1580];

// ea: 0x00448BB0
bool Player_CheckFriendlyFireUse(PlayerState* /*ps*/)
{
    return true;
}

// ea: 0x00449F50
void respawn(Entity* /*ent*/)
{
    ;
}

// ea: 0x00448BD0
void G_SetClientSound(Entity* ent)
{
    ent->s.loopSound = 0;
}

// ea: 0x00448E40
void G_RunClient(Entity* /*ent*/)
{
    ;
}

// ea: 0x00448F00
int ClientInactivityTimer(Entity* /*ent*/)
{
    return 1;
}

// ea: 0x00448F10
int ClientSpectatorInactivityTimer(Entity* /*ent*/)
{
    return 1;
}

// ea: 0x00472130
void Spotting(Entity* ent)
{
    int* spotTime = &dword_F641A8[1580 * currCl];
    if (*spotTime + 1000 > level.time)
        return;
    weaponParms wp;
    wp.pWeapInfo = BG_GetInfoForWeapon(ent->s.weapon);
    AngleVectors(ent->client->ps.viewangles, wp.forward, wp.right, wp.up);
    math::Position3 muzzle;
    CalcMuzzlePoint(ent, &muzzle);
    math::Position3 end;
    end.v.m128_f32[0] = muzzle.v.m128_f32[0] + wp.forward[0] * 3000.0f;
    end.v.m128_f32[1] = muzzle.v.m128_f32[1] + wp.forward[1] * 3000.0f;
    end.v.m128_f32[2] = muzzle.v.m128_f32[2] + wp.forward[2] * 3000.0f;
    collision_context_t context;
    context.__vftable = nullptr;
    context.pass_entity1.mHandle.mVal = ent->mHandle.mHandle.mVal;
    context.pass_entity2.mHandle.mVal = 0;
    context.pass_owner1.mHandle.mVal = 0;
    context.pass_owner2.mHandle.mVal = 0;
    context.contentmask = 0x2802033;
    math::Position3 zeroA;
    math::Position3 zeroB;
    zeroA.v = _mm_setzero_ps();
    zeroB.v = _mm_setzero_ps();
    trace_t trace;
    SV_Trace(&trace, &muzzle, &zeroA, &zeroB, &end, &context, 0, 1,
             bulletPriorityMap, 1, 0.02f);
    unsigned int handleVal = trace.mEntity.mHandle.mVal;
    unsigned int idx = handleVal & 0xFFF;
    if (idx >= 0x540)
        return;
    if ((handleVal >> 12) != EntityHandleDb::sInst.mElements[idx].mKey)
        return;
    Entity* mObject = EntityHandleDb::sInst.mElements[idx].mObject;
    if (mObject == nullptr)
        return;
    Client* client = mObject->client;
    if (client == nullptr && mObject->scr_vehicle == nullptr)
        return;
    Entity* v11 = mObject;
    if (client != nullptr && (0x100000 & client->ps.eFlags) != 0)
    {
        Entity* v12 = HandleDbToEnt(mObject->r.mOwner);
        if (v12 != nullptr && v12->scr_vehicle != nullptr)
            v11 = v12;
    }
    MultiplayerMgr::sInst->SpotEntity(v11);
    *spotTime = level.time;
}
