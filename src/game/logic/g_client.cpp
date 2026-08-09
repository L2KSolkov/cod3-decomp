// ============================================================================
// g_client.cpp - client-side game logic (g.o: g_client.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

#include <string.h>

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

// ea: 0x00449BC0
void SetClientViewAngle(Entity* ent, const float* angle)
{
    float newAngle[3] = { angle[0], angle[1], angle[2] };
    Client* client = ent->client;
    if ((client->ps.pm_flags & 1) != 0 && (client->ps.eFlags & 0x6000) == 0)
    {
        float fDeltab = AngleDelta(client->ps.proneDirection, newAngle[1]);
        float fDelta = AngleNormalize180(fDeltab);
        if (fDelta > 45.0f || fDelta < -45.0f)
        {
            float v4 = fDelta > 45.0f ? fDelta - 45.0f : fDelta + 45.0f;
            ent->client->ps.delta_angles[1] += (int)(v4 * 182.04445f) & 0xFFFF;
            float v5 = v4 <= 0.0f ? ent->client->ps.proneDirection + 45.0f
                                  : ent->client->ps.proneDirection - 45.0f;
            newAngle[1] = AngleNormalize360(v5);
        }
        float fDeltac = AngleDelta(ent->client->ps.proneTorsoPitch, newAngle[0]);
        float fDeltaa = AngleNormalize180(fDeltac);
        if (fDeltaa > 45.0f || fDeltaa < -15.0f)
        {
            float v6 = fDeltaa > 45.0f ? fDeltaa - 45.0f : fDeltaa + 15.0f;
            ent->client->ps.delta_angles[0] += (int)(v6 * 182.04445f) & 0xFFFF;
            float v7 = v6 <= 0.0f ? ent->client->ps.proneTorsoPitch + 15.0f
                                  : ent->client->ps.proneTorsoPitch - 45.0f;
            newAngle[0] = AngleNormalize180(v7);
        }
    }
    for (int i = 0; i < 3; ++i)
        ent->client->ps.delta_angles[i] =
            ((int)(newAngle[i] * 182.04445f) & 0xFFFF) - ent->client->pers.cmd.angles[i];
    ent->r.currentAngles.v.m128_f32[0] = newAngle[0];
    ent->r.currentAngles.v.m128_f32[1] = newAngle[1];
    ent->r.currentAngles.v.m128_f32[2] = newAngle[2];
    memcpy(ent->client->ps.viewangles, &ent->r.currentAngles,
           sizeof(ent->client->ps.viewangles));
}

// ea: 0x00455780
void G_FinishSetupSpawnPoint(Entity* pEnt, int msec)
{
    if (pEnt != nullptr && pEnt->cell_index < 0)
    {
        math::Position3 pos;
        pos.v = pEnt->r.currentOrigin.v;
        pEnt->cell_index = (int16_t)R_CellForPoint(&pos);
        if (pEnt->cell_index < 0)
        {
            pos.v.m128_f32[2] += 20.0f;
            pEnt->cell_index = (int16_t)R_CellForPoint(&pos);
            if (pEnt->cell_index < 0)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)10;  // JSV
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_client.cpp";
                AeAssert::gCurrentLine = 53;
                AeAssert::gCurrentExpr = "pEnt->cell_index >= 0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("spawn point out of the world"))
                    __debugbreak();
            }
        }
    }
    if (Entity_has_zone_collision(pEnt))
    {
        collision_context_t context;
        context.__vftable = nullptr;
        context.pass_entity1.mHandle.mVal = pEnt->mHandle.mHandle.mVal;
        context.pass_entity2.mHandle.mVal = 0;
        context.pass_owner1.mHandle.mVal = 0;
        context.pass_owner2.mHandle.mVal = 0;
        context.contentmask = 0x2810011;
        math::Position3 start;
        math::Position3 end;
        start.v.m128_f32[0] = pEnt->r.currentOrigin.v.m128_f32[0];
        start.v.m128_f32[1] = pEnt->r.currentOrigin.v.m128_f32[1];
        start.v.m128_f32[2] = pEnt->r.currentOrigin.v.m128_f32[2] + 128.0f;
        end.v = pEnt->r.currentOrigin.v;
        trace_t trace;
        SV_Trace(&trace, &start, &playerMins, &playerMaxs, &end, &context, 1, 0,
                 nullptr, 0, 0.0f);
        start.v.m128_f32[0] = trace.endpos.v.m128_f32[0];
        start.v.m128_f32[1] = trace.endpos.v.m128_f32[1];
        start.v.m128_f32[2] = trace.endpos.v.m128_f32[2];
        end.v.m128_f32[0] = trace.endpos.v.m128_f32[0];
        end.v.m128_f32[1] = trace.endpos.v.m128_f32[1];
        end.v.m128_f32[2] = trace.endpos.v.m128_f32[2] - 256.0f;
        SV_Trace(&trace, &start, &playerMins, &playerMaxs, &end, &context, 1, 0,
                 nullptr, 0, 0.0f);
        pEnt->s.mGroundEntity.mHandle.mVal = trace.mEntity.mHandle.mVal;
        start.v = trace.endpos.v;
        end.v = trace.endpos.v;
        SV_Trace(&trace, &start, &playerMins, &playerMaxs, &end, &context, 1, 0,
                 nullptr, 0, 0.0f);
        if (trace.allsolid != 0)
        {
            Com_Printf("WARNING: Spawn point entity %i is in solid at (%i, %i, %i)\n",
                       pEnt->mHandle.mHandle.mVal,
                       (int)pEnt->r.currentOrigin.v.m128_f32[0],
                       (int)pEnt->r.currentOrigin.v.m128_f32[1],
                       (int)pEnt->r.currentOrigin.v.m128_f32[2]);
        }
        G_SetOrigin(pEnt, (const math::Position3*)&trace.endpos);
    }
    else
    {
        G_SetOrigin(pEnt, &pEnt->r.currentOrigin);
    }
}
