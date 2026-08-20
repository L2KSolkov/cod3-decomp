// ============================================================================
// g_client.cpp - client-side game logic (g.o: g_client.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

// .bss @ 0xF641A8 (file-local per-client spot timer, stride 1580 ints)
static int dword_F641A8[4 * 1580];

// Player_GetActivateEnt statics (.bss @ 0xEF3F60 / 0xEF3F68)
static unsigned int sS14_6 = 0;
static unsigned int tag_aim_hash_1 = 0;
static DbLinkedHandle<EntityHandleDb, Entity> touch[0x540];

static int compare_use(const void* a, const void* b)
{
    return (int)(((const useList_t*)a)->score - ((const useList_t*)b)->score);
}

// ea: 0x00473D90
int Player_GetActivateEnt(Entity* pEnt, useList_t* const useList)
{
    Client* client = pEnt->client;
    if ((sS14_6 & 1) == 0)
    {
        sS14_6 |= 1u;
        memset(touch, 0, sizeof(touch));
    }
    float forward[3];
    AnglesToForward(client->ps.viewangles, forward);
    float origin[3];
    origin[0] = pEnt->r.currentOrigin.v.m128_f32[0];
    origin[1] = pEnt->r.currentOrigin.v.m128_f32[1];
    origin[2] = pEnt->r.currentOrigin.v.m128_f32[2]
              + client->ps.viewHeightCurrent;
    math::Position3 mins;
    math::Position3 maxs;
    mins.v.m128_f32[0] = origin[0] - 150.0f;
    mins.v.m128_f32[1] = origin[1] - 150.0f;
    mins.v.m128_f32[2] = origin[2] - 96.0f;
    maxs.v.m128_f32[0] = origin[0] + 150.0f;
    maxs.v.m128_f32[1] = origin[1] + 150.0f;
    maxs.v.m128_f32[2] = origin[2] + 96.0f;
    int num = CM_AreaEntities(mins, maxs, touch, 0x540, 0x204000);
    int curUse = 0;
    int ignoredFullItems = 0;
    float distToUsePoint = 0.0f;
    int entryPoint = 0;
    float delta[3];
    for (int i = 0; i < num; ++i)
    {
        Entity* ent = HandleDbToEnt(touch[i]);
        if (ent == nullptr || pEnt == ent)
            continue;
        if (ent->s.eType != 2 && (ent->r.contents & 0x200000) == 0
            && (ent->actor == nullptr || ent->actor->useable == 0))
            continue;
        float center[3];
        center[0] = (ent->r.absmax.v.m128_f32[0]
                     + ent->r.absmin.v.m128_f32[0]) * 0.5f;
        center[1] = (ent->r.absmax.v.m128_f32[1]
                     + ent->r.absmin.v.m128_f32[1]) * 0.5f;
        center[2] = (ent->r.absmax.v.m128_f32[2]
                     + ent->r.absmin.v.m128_f32[2]) * 0.5f;
        delta[0] = center[0] - origin[0];
        delta[1] = center[1] - origin[1];
        delta[2] = center[2] - origin[2];
        if (center[2] > abovehead_tresh + pEnt->r.currentOrigin.v.m128_f32[2])
            continue;
        if (ent->s.eType == 14 && ent->scr_vehicle != nullptr)
        {
            if (!ent->scr_vehicle->CanUseVehicle(pEnt, distToUsePoint,
                                                 entryPoint))
                continue;
            pEnt->client->mVehicleAnimRoute = entryPoint + 1;
            if (pEnt->client->mVehicleAnimRoute < 0)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PlayerUse.cpp";
                AeAssert::gCurrentLine = 321;
                AeAssert::gCurrentExpr =
                    "pEnt->client->mVehicleAnimRoute >= 0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            pEnt->client->mVehicleEntryPoint = entryPoint;
        }
        else
        {
            float dist = sqrtf(delta[0] * delta[0] + delta[1] * delta[1]
                               + delta[2] * delta[2]);
            if (dist != 0.0f)
            {
                delta[0] /= dist;
                delta[1] /= dist;
                delta[2] /= dist;
            }
            if (dist > 100.0f)
                continue;
            float dot = delta[0] * forward[0] + delta[1] * forward[1]
                      + delta[2] * forward[2];
            if (dot <= 0.0f)
                continue;
            if (ent->s.eType == 14 && dot < 0.38f)
                continue;
            float score = (1.0f - ((dot - 0.76f) * 4.1666665f)) * 200.0f;
            if (ent->mClassNameHash.mHash == hash_const.trigger_use.mHash)
                score -= 200.0f;
            if (ent->s.eType == 2)
            {
                if (BG_CanItemBeGrabbed(&ent->s, &pEnt->client->ps, 0) == 0)
                {
                    score += 10000.0f;
                    ++ignoredFullItems;
                }
            }
            if (ent->mClassNameHash.mHash == hash_const.script_model.mHash)
                score += 5000.0f;
            useList[curUse].ent = ent;
            useList[curUse].score = score + distToUsePoint;
            ++curUse;
        }
    }
    qsort(useList, curUse, 8, compare_use);
    int numValid = curUse - ignoredFullItems;
    for (int i = 0; i < numValid; ++i)
    {
        Entity* ent = useList[i].ent;
        float center[3];
        center[0] = (ent->r.absmax.v.m128_f32[0]
                     + ent->r.absmin.v.m128_f32[0]) * 0.5f;
        center[1] = (ent->r.absmax.v.m128_f32[1]
                     + ent->r.absmin.v.m128_f32[1]) * 0.5f;
        center[2] = (ent->r.absmax.v.m128_f32[2]
                     + ent->r.absmin.v.m128_f32[2]) * 0.5f;
        if (ent->s.eType == 14)
            center[2] = origin[2];
        if (ent->s.eType == 10)
        {
            if ((sS14_6 & 2) == 0)
            {
                sS14_6 |= 2u;
                tag_aim_hash_1 = HashString::CalcHash("tag_aim");
            }
            int bone = SV_DObjGetBoneIndex(ent, tag_aim_hash_1);
            if (bone >= 0)
            {
                G_DObjCalcBone(ent, bone);
                DObjSkelMat* mtx = &SV_DObjGetMatrixArray(ent)[bone];
                if (mtx != nullptr)
                {
                    float axis[3][3];
                    AnglesToAxis(ent->r.currentAngles, axis);
                    DObjSkelMat out;
                    DObjSkel2MatrixMultiply43(mtx, axis, &out);
                    center[0] = out.origin[0];
                    center[1] = out.origin[1];
                    center[2] = out.origin[2];
                }
            }
        }
        bool doTrace = true;
        if (ent->mClassNameHash.mHash == hash_const.trigger_use.mHash)
        {
            if (ent->s.angles2.v.m128_f32[0] < 1.0f)
            {
                float fwd[3];
                fwd[0] = forward[0];
                fwd[1] = forward[1];
                fwd[2] = 0.0f;
                VectorNormalize(fwd);
                float trigFwd[3];
                float trigAngles[3] = { 0.0f,
                                        ent->s.angles2.v.m128_f32[1],
                                        0.0f };
                AnglesToForward(trigAngles, trigFwd);
                float dot1 = fwd[0] * trigFwd[0] + fwd[1] * trigFwd[1]
                           + fwd[2] * trigFwd[2];
                float dirTo[3];
                dirTo[0] = delta[0];
                dirTo[1] = delta[1];
                dirTo[2] = 0.0f;
                VectorNormalize(dirTo);
                float dot2 = fwd[0] * dirTo[0] + fwd[1] * dirTo[1]
                           + fwd[2] * dirTo[2];
                if (!(-dot1 > ent->s.angles2.v.m128_f32[0]
                      && ent->s.angles2.v.m128_f32[0] <= -dot2))
                    doTrace = false;
            }
        }
        if (doTrace)
        {
            collision_context_t context;
            context.pass_entity1.mHandle.mVal = 0;
            context.pass_entity2.mHandle.mVal =
                ent->s.eType == 14 ? 0x200051 : 0x200011;
            context.pass_owner1.mHandle.mVal = 0;
            context.pass_owner2.mHandle.mVal = 0;
            context.contentmask = client->ps.mClient.mHandle.mVal;
            math::Position3 start;
            start.v.m128_f32[0] = origin[0];
            start.v.m128_f32[1] = origin[1];
            start.v.m128_f32[2] = origin[2];
            math::Position3 zero;
            zero.v = _mm_setzero_ps();
            math::Position3 end;
            end.v.m128_f32[0] = center[0];
            end.v.m128_f32[1] = center[1];
            end.v.m128_f32[2] = center[2];
            trace_t tr;
            SV_Trace(&tr, &start, &zero, &zero, &end, &context, 0, 0,
                     nullptr, 0, 0.0f);
            Entity* trEnt = HandleDbToEnt(tr.mEntity);
            if (tr.fraction >= 1.0f || trEnt == ent)
                break;
        }
        useList[i].score += 100000.0f;
    }
    qsort(useList, curUse, 8, compare_use);
    return curUse - ignoredFullItems;
}

// ea: 0x0048E1E0
void ClientThink_real(Entity* ent)
{
    Client* client = ent->client;
    if (client->pers.connected != 2 /* CON_CONNECTED */)
        return;
    usercmd_s* cmd = &client->pers.cmd;
    if (cmd->serverTime > level.time + 200)
        cmd->serverTime = level.time + 200;
    if (cmd->serverTime < level.time - 1000)
        cmd->serverTime = level.time - 1000;
    int delta = cmd->serverTime - client->ps.commandTime;
    if (delta < 1)
    {
        if (delta >= -100)
            return;
        client->ps.commandTime = cmd->serverTime - 100;
    }
    if (pmove_msec.integer < 8)
        Cvar_Set("pmove_msec", "8");
    else if (pmove_msec.integer > 33)
        Cvar_Set("pmove_msec", "33");
    if (pmove_fixed.integer != 0 || client->pers.pmoveFixed != 0)
        cmd->serverTime = pmove_msec.integer
                          * ((cmd->serverTime + pmove_msec.integer - 1)
                             / pmove_msec.integer);
    switch (client->pers.playerState)
    {
    case 2:
        client->oldbuttons = client->buttons;
        client->buttons = client->pers.cmd.buttons;
        return;
    case 1:
        SpectatorThink(ent, &client->pers.cmd);
        return;
    case 4:
    case 5:
        if (Entity_IsInRagdoll(ent))
            return;
        break;
    default:
        break;
    }
    client->ps.pm_flags = client->bFrozen
                              ? (client->ps.pm_flags | 0x4000)
                              : (client->ps.pm_flags & ~0x4000);
    if (client->noclip != 0)
        client->ps.pm_type = 2;
    else if (client->ufo != 0)
        client->ps.pm_type = 3;
    else if (client->ps.stats[0] > 0)
        client->ps.pm_type = ent->tagInfo != nullptr;
    else
        client->ps.pm_type = (ent->tagInfo != nullptr) + 6;
    int oldEventSequence = client->ps.event.eventSequence;
    client->ps.gravity = g_gravity.integer;
    client->ps.speed = g_speed.integer;
    client->currentAimSpreadScale =
        client->ps.aimSpreadScale * 0.0039215689f;
    pmove_t pm;
    memset(&pm, 0, 0x150);
    pm.ps = (PlayerState*)client;
    pm.cmd = *cmd;
    pm.oldcmd = client->pers.oldcmd;
    pm.tracemask = (client->pers.playerState < 6)
                       ? ent->clipmask
                       : 0x820011;
    pm.debugLevel = g_debugMove.integer;
    pm.pmove_fixed =
        pmove_fixed.integer | client->pers.pmoveFixed;
    pm.pmove_msec = pmove_msec.integer;
    pm.trace = g_TraceCapsule;
    pm.boxtrace = g_TraceCapsule;
    pm.capsuletrace = g_TraceCapsule;
    pm.pointcontents = SV_PointContents;
    collision_context_t pmCtx;
    pmCtx.pass_entity1 = ent->mHandle;
    pmCtx.pass_entity2.mHandle.mVal = 0;
    pmCtx.pass_owner1.mHandle.mVal = 0;
    pmCtx.pass_owner2.mHandle.mVal = 0;
    pmCtx.contentmask = pm.tracemask;
    client->oldOrigin.v.m128_f32[0] = client->ps.origin.v.m128_f32[0];
    client->oldOrigin.v.m128_f32[1] = client->ps.origin.v.m128_f32[1];
    client->oldOrigin.v.m128_f32[2] = client->ps.origin.v.m128_f32[2];
    if (TestFPS::sInst->mTesting)
    {
        TestFPS::sInst->PositionCamera(&pm);
    }
    else if (Entity_has_zone_collision(ent)
             || client->pers.playerState == 2
             || client->pers.playerState == 1)
    {
        math::Position3 oldPos = client->ps.origin;
        if (g_freeze_movement != 0)
        {
            pm.cmd.forwardmove = 0;
            pm.cmd.rightmove = 0;
            pm.cmd.buttons = 0;
            pm.vehicleAngles[0] = 0.0f;
            pm.vehicleAngles[1] = 0.0f;
            pm.vehicleAngles[2] = 0.0f;
        }
        Pmove(&pm, false);
        math::Position3 newPos = client->ps.origin;
        bool moved = pm.vehicleAngles[0] != 0.0f
                  || pm.vehicleAngles[1] != 0.0f
                  || pm.vehicleAngles[2] != 0.0f;
        if (moved)
        {
            if (client->ps.origin.v.m128_f32[0] == oldPos.v.m128_f32[0]
                && client->ps.origin.v.m128_f32[1] == oldPos.v.m128_f32[1]
                && client->ps.origin.v.m128_f32[2] == oldPos.v.m128_f32[2])
            {
                if (client->pers.playerState != 2)
                    push_in_world(pm, 15.1f, pmCtx);
            }
        }
        if (client->pers.playerState != 2)
        {
            client->ps.origin = oldPos;
            float dx = client->ps.origin.v.m128_f32[0]
                     - newPos.v.m128_f32[0];
            float dy = client->ps.origin.v.m128_f32[1]
                     - newPos.v.m128_f32[1];
            float dz = client->ps.origin.v.m128_f32[2]
                     - newPos.v.m128_f32[2];
            float dist2 = dx * dx + dy * dy + dz * dz;
            if (dist2 > radius_2 * radius_2
                && (client->ps.eFlags & 0x100000) == 0)
            {
                client->ps.origin.v.m128_f32[2] += 35.0f;
                newPos.v.m128_f32[2] += 35.0f;
                if (tunnel_test(pm, radius_2,
                                *(math::Position3*)
                                    &client->ps.origin.v.m128_f32[0],
                                *(math::Position3*)&newPos.v.m128_f32[0]))
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\g_active.cpp";
                    AeAssert::gCurrentLine = 935;
                    AeAssert::gCurrentExpr = "0";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(
                               "tunneling outside of resolve_collision\n"))
                        __debugbreak();
                    client->ps.origin = oldPos;
                }
            }
        }
    }
    ent->r.svFlags = (client->ps.eFlags & 0x10) != 0
                         ? (ent->r.svFlags | 0x200)
                         : (ent->r.svFlags & ~0x200);
    Entity* ground = HandleDbToEnt(client->ps.mGroundEntity);
    if (ground != nullptr && ground->client != nullptr
        && sqrtf(client->ps.velocity.v.m128_f32[0]
                     * client->ps.velocity.v.m128_f32[0]
                 + client->ps.velocity.v.m128_f32[1]
                       * client->ps.velocity.v.m128_f32[1]
                 + client->ps.velocity.v.m128_f32[2]
                       * client->ps.velocity.v.m128_f32[2])
               < 200.0f)
    {
        float r = random();
        client->ps.velocity.v.m128_f32[0] += (r + r - 1.0f) * 100.0f;
        r = random();
        client->ps.velocity.v.m128_f32[1] += (r + r - 1.0f) * 100.0f;
        client->ps.velocity.v.m128_f32[2] += 200.0f;
    }
    if (client->ps.event.eventSequence != oldEventSequence)
        ent->r.eventTime = level.time;
    BG_PlayerStateToEntityStateExtrapolate(&client->ps, &ent->s,
                                           client->ps.commandTime, 1);
    if ((client->ps.eFlags & 0x100000) == 0)
    {
        ent->r.currentOrigin.v.m128_f32[0] = ent->s.pos.trBase[0];
        ent->r.currentOrigin.v.m128_f32[1] = ent->s.pos.trBase[1];
        ent->r.currentOrigin.v.m128_f32[2] = ent->s.pos.trBase[2];
    }
    ent->r.mins = pm.mins;
    ent->r.maxs = pm.maxs;
    ClientEvents(ent, (int)oldEventSequence);
    g_LinkEntity(ent);
    if (client->ps.origin.v.m128_f32[0] != client->oldOrigin.v.m128_f32[0]
        || client->ps.origin.v.m128_f32[1] != client->oldOrigin.v.m128_f32[1]
        || client->ps.origin.v.m128_f32[2] != client->oldOrigin.v.m128_f32[2])
    {
        Sentient_InvalidateNearestNode(ent->sentient);
    }
    if (client->noclip == 0 && client->ufo == 0
        && client->ps.pm_type < 6 && cls.state != 5 /* CA_MAP_RESTART */)
    {
        G_TouchTriggersAndVehicles(ent, &client->ps.origin, &pmCtx);
    }
    if ((client->ps.eFlags & 0x100000) != 0
        && IsPlayerFullySeatedInVehicle(ent))
    {
        ent->r.currentOrigin.v.m128_f32[0] =
            client->ps.origin.v.m128_f32[0];
        ent->r.currentOrigin.v.m128_f32[1] =
            client->ps.origin.v.m128_f32[1];
        ent->r.currentOrigin.v.m128_f32[2] =
            client->ps.origin.v.m128_f32[2];
        ent->r.currentAngles.v.m128_f32[0] = 0.0f;
        ent->r.currentAngles.v.m128_f32[1] =
            client->ps.viewangles[1];
    }
    ClientImpacts(ent, &pm);
    if (level.time >= client->mFootStepsSurface[1] + 500)
        client->mFootStepsSurface[0] = level.time;
    if (client->ps.event.eventSequence != oldEventSequence)
        ent->r.eventTime = level.time;
    int oldButtons = client->buttons;
    client->oldbuttons = oldButtons;
    client->buttons = cmd->buttons;
    client->latched_buttons = cmd->buttons & ~oldButtons;
    client->fGunPitch = cmd->gunPitch;
    client->fGunYaw = cmd->gunYaw;
    client->fGunXOfs = cmd->gunXOfs;
    client->fGunYOfs = cmd->gunYOfs;
    client->fGunZOfs = cmd->gunZOfs;
    if (client->pers.playerState < 6)
    {
        Client_ClaimNode(ent);
        Player_UpdateActivate(ent);
    }
}

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
    CalcMuzzlePoint(ent, muzzle);
    math::Position3 end;
    end.v.m128_f32[0] = muzzle.v.m128_f32[0] + wp.forward[0] * 3000.0f;
    end.v.m128_f32[1] = muzzle.v.m128_f32[1] + wp.forward[1] * 3000.0f;
    end.v.m128_f32[2] = muzzle.v.m128_f32[2] + wp.forward[2] * 3000.0f;
    collision_context_t context;
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
        G_SetOrigin(pEnt, trace.endpos);
    }
    else
    {
        G_SetOrigin(pEnt, pEnt->r.currentOrigin);
    }
}

// ea: 0x00454D00
void Player_UpdateFriendlyOverlay(Entity* pEnt)
{
    Entity* pLookatEnt = pEnt->client->pLookatEnt;
    if (pLookatEnt == nullptr)
    {
        SV_SetConfigstring(13, va("%s", "none"));
        return;
    }
    actor_s* actor = pLookatEnt->actor;
    if (actor == nullptr
        || actor->Physics.bIsAlive == 0
        || actor->mProperName.mBlock == nullptr
        || (actor->mProperName.mBlock + 1) == nullptr
        || ((char*)&(actor->mProperName.mBlock + 1)->mBuff)[0] == 0)
    {
        scr_vehicle_t* scr_vehicle = pLookatEnt->scr_vehicle;
        if (scr_vehicle != nullptr
            && pLookatEnt->health > 0
            && scr_vehicle->mProperName.mBlock != nullptr
            && (scr_vehicle->mProperName.mBlock + 1) != nullptr
            && ((char*)&(scr_vehicle->mProperName.mBlock + 1)->mBuff)[0] != 0)
        {
            Broc::string::Block* mBlock = scr_vehicle->mProperName.mBlock;
            const char* v16 = (const char*)&mBlock[1];
            if (mBlock == nullptr)
                v16 = defaultFileName;
            SV_SetConfigstring(13, va("%s", v16));
            if (pEnt->client == nullptr)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PlayerUse.cpp";
                AeAssert::gCurrentLine = 502;
                AeAssert::gCurrentExpr = "pEnt->client";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            if (pLookatEnt->maxHealth == 0)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PlayerUse.cpp";
                AeAssert::gCurrentLine = 503;
                AeAssert::gCurrentExpr = "traceEnt->maxHealth";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            float colorFraca = pLookatEnt->health / pLookatEnt->maxHealth;
            if (colorFraca > 1.0f)
                colorFraca = 1.0f;
            SV_SetConfigstring(14, va("%f", colorFraca));
            vehicle_info_t* v19 = s_vehicleInfos[pLookatEnt->scr_vehicle->infoIdx];
            const char* nameOverlay = v19->nameOverlay;
            if (nameOverlay[0] == 0)
            {
                const char* v22 = va("%s", "none");
                SV_SetConfigstring(15, v22);
            }
            else
            {
                SV_SetConfigstring(15, va("%s", nameOverlay));
            }
            return;
        }
        if (pLookatEnt->health <= 0)
            return;
        Broc::string::Block* v23 = pLookatEnt->mGroupName.mBlock;
        if (v23 == nullptr)
            return;
        Broc::string::Block* v24 = v23 + 1;
        if (v24 == nullptr
            || ((char*)&v24->mBuff)[0] == 0
            || pLookatEnt->team.mBlock == nullptr
            || pLookatEnt->team.GetBuff()[1] != 'l')
            return;
        Broc::string::Block* v25 = pLookatEnt->mGroupName.mBlock;
        const char* v26 = v25 != nullptr ? (const char*)&v25[1] : defaultFileName;
        SV_SetConfigstring(13, va("%s", v26));
        if (pEnt->client == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PlayerUse.cpp";
            AeAssert::gCurrentLine = 523;
            AeAssert::gCurrentExpr = "pEnt->client";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        if (pLookatEnt->maxHealth == 0)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PlayerUse.cpp";
            AeAssert::gCurrentLine = 524;
            AeAssert::gCurrentExpr = "traceEnt->maxHealth";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        float colorFracb = pLookatEnt->health / pLookatEnt->maxHealth;
        if (colorFracb > 1.0f)
            colorFracb = 1.0f;
        SV_SetConfigstring(14, va("%f", colorFracb));
        weaponFileInfo_t* InfoForWeapon =
            BG_GetInfoForWeapon((int)pLookatEnt->mHintString);
        const char* v22;
        if (InfoForWeapon == nullptr
            || InfoForWeapon->szOverlayName == nullptr
            || *InfoForWeapon->szOverlayName == 0)
            v22 = va("%s", defaultFileName);
        else
            v22 = va("%s", InfoForWeapon->szOverlayName);
        SV_SetConfigstring(15, v22);
        return;
    }
    Broc::string::Block* v5 = actor->mProperName.mBlock;
    const char* v6 = v5 != nullptr ? (const char*)&v5[1] : defaultFileName;
    SV_SetConfigstring(13, va("%s", v6));
    if (pEnt->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PlayerUse.cpp";
        AeAssert::gCurrentLine = 486;
        AeAssert::gCurrentExpr = "pEnt->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pLookatEnt->maxHealth == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PlayerUse.cpp";
        AeAssert::gCurrentLine = 487;
        AeAssert::gCurrentExpr = "traceEnt->maxHealth";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float colorFrac = pLookatEnt->health / pLookatEnt->maxHealth;
    if (colorFrac > 1.0f)
        colorFrac = 1.0f;
    SV_SetConfigstring(14, va("%f", colorFrac));
    unsigned char WeaponIndexForName =
        BG_GetWeaponIndexForName(pLookatEnt->actor->mWeaponName);
    weaponFileInfo_t* v10 = BG_GetInfoForWeapon(WeaponIndexForName);
    SV_SetConfigstring(15, va("%s", v10->szOverlayName));
}

// ea: 0x00491F10
void ClientSpawn(Entity* ent, const float* origin, const float* angles,
                 bool stopPhysics, bool isRevive)
{
    bool v5 = stopPhysics;
    Client* client = ent->client;
    client->pers.connected = 2 /* CON_CONNECTED */;
    unsigned int physicsFlags = 0x400000 & ent->flags;
    if (v5)
    {
        StopPhysics(ent);
        if (ent->mDObj != nullptr)
            G_DObjCalcPose(ent);
    }
    float spawn_origin[3] = {origin[0], origin[1], origin[2]};
    float spawn_angles[3] = {angles[0], angles[1], angles[2]};
    if (!v5 && !EntityManager::sInst->IsLocalPlayer(ent))
    {
        spawn_origin[0] = ent->r.currentOrigin.v.m128_f32[0];
        spawn_origin[1] = ent->r.currentOrigin.v.m128_f32[1];
        spawn_origin[2] = ent->r.currentOrigin.v.m128_f32[2];
        spawn_angles[0] = ent->r.currentAngles.v.m128_f32[0];
        spawn_angles[1] = ent->r.currentAngles.v.m128_f32[1];
        spawn_angles[2] = ent->r.currentAngles.v.m128_f32[2];
    }
    int eFlags = ent->client->ps.eFlags;
    XModel* saveViewmodel = client->ps.viewmodel.mValue;
    TPakId v38 = (TPakId)client->ps.viewmodel.mPakId;
    int v10 = ~eFlags & 8;
    extern void Client_Clear(Client* client, bool clearPersistentAlso,
                             bool clearWeapons);
    Client_Clear(client, false, false);
    if (isRevive != 0 && EntityManager::sInst->IsLocalPlayer(ent))
    {
        int lastWeapon = client->ps.lastWeapon;
        client->ps.weapon = lastWeapon;
        weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(lastWeapon);
        if (InfoForWeapon == nullptr
            || (InfoForWeapon->slot != 1 /* WEAPSLOT_PRIMARY */
                && InfoForWeapon->slot != 2 /* WEAPSLOT_PRIMARYB */))
        {
            int PlayerIndex = EntityManager::sInst->GetPlayerIndex(ent);
            if (CG_SelectFirstWeaponInSlotWithLocalIndex(1, 0, PlayerIndex) == 0)
            {
                int v14 = EntityManager::sInst->GetPlayerIndex(ent);
                CG_SelectFirstWeaponInSlotWithLocalIndex(1, 1, v14);
            }
            client->ps.weapon = cg_aWeaponSelect[EntityManager::sInst->GetPlayerIndex(ent)];
        }
    }
    ent->r.mOwner.mHandle.mVal = 0;
    g_femanager.mDontDrawHud = false;
    int maxHealth = client->pers.maxHealth;
    client->ps.viewmodel.mValue = saveViewmodel;
    client->ps.viewmodel.mPakId = v38;
    client->ps.stats[2] = maxHealth;
    client->ps.mClient.mHandle.mVal = ent->mHandle.mHandle.mVal;
    client->ps.eFlags = v10 | 0x10;
    ent->r.svFlags |= 0x208;
    if (EntityManager::sInst->IsLocalPlayer(ent))
    {
        int v21 = EntityManager::sInst->GetPlayerIndex(ent);
        cl_aADS[v21] = 1;
        CG_ResetLowHealthOverlay(v21);
        cl_stance_ss[v21] = 0;
    }
    ent->s.mGroundEntity.mHandle.mVal = 0;
    ent->takedamage = 1;
    ent->mClassName = str_const.player;
    ent->mClassNameHash.mHash = HashString::CalcHash(ent->mClassName.GetBuff());
    ent->r.contents = 0x2000000;
    ent->clipmask = 42008593;
    ent->die = 3;
    ent->flags = 49152;
    if (!stopPhysics)
        ent->flags = physicsFlags | 0xC000;
    UpdateEntityHash(ent);
    ent->r.mins.v.m128_f32[0] = playerMins.v.m128_f32[0];
    ent->r.mins.v.m128_f32[1] = playerMins.v.m128_f32[1];
    ent->r.mins.v.m128_f32[2] = playerMins.v.m128_f32[2];
    ent->r.maxs.v.m128_f32[0] = playerMaxs.v.m128_f32[0];
    ent->r.maxs.v.m128_f32[1] = playerMaxs.v.m128_f32[1];
    ent->r.maxs.v.m128_f32[2] = playerMaxs.v.m128_f32[2];
    client->ps.mins[0] = ent->r.mins.v.m128_f32[0];
    client->ps.mins[1] = ent->r.mins.v.m128_f32[1];
    client->ps.mins[2] = ent->r.mins.v.m128_f32[2];
    client->ps.maxs[0] = ent->r.maxs.v.m128_f32[0];
    client->ps.maxs[1] = ent->r.maxs.v.m128_f32[1];
    client->ps.maxs[2] = ent->r.maxs.v.m128_f32[2];
    client->ps.proneViewHeight = bg_viewheight_prone.integer;
    client->ps.crouchViewHeight = bg_viewheight_crouched.integer;
    client->ps.standViewHeight = bg_viewheight_standing.integer;
    client->ps.deadViewHeight = 8;
    client->ps.sprintSpeedScale = 1.6f;
    client->ps.viewHeightCurrent = 0.0f;
    client->ps.viewHeightLerpPosAdj = 0.0f;
    client->ps.proneSpeedScale = 0.15000001f;
    client->ps.viewHeightTarget = bg_viewheight_standing.integer;
    client->ps.viewHeightLerpTime = 0;
    client->ps.walkSpeedScale = 0.40000001f;
    client->ps.runSpeedScale = 1.0f;
    client->ps.crouchSpeedScale = 0.64999998f;
    client->ps.strafeSpeedScale = 1.0f;
    client->ps.backSpeedScale = 1.0f;
    client->ps.leanSpeedScale = 0.40000001f;
    client->ps.friction = 1.0f;
    client->ps.fatigueScale = 1.0f;
    ent->client->ps.spectatorClient = -1;
    client->ps.stats[0] = 0;
    ent->health = 0;
    G_SetOrigin(ent, spawn_origin);
    client->ps.origin.v.m128_f32[0] = spawn_origin[0];
    client->ps.origin.v.m128_f32[1] = spawn_origin[1];
    client->ps.origin.v.m128_f32[2] = spawn_origin[2];
    client->ps.pm_flags = client->ps.pm_flags & 0xFFFFF7DF | 0x800;
    SV_GetUsercmd(client - level.clients, &ent->client->pers.cmd);
    SetClientViewAngle(ent, spawn_angles);
    g_LinkEntity(ent);
    client->ps.pm_time = 100;
    client->ps.pm_flags = client->ps.pm_flags | 0x200;
    client->respawnTime = level.time;
    client->latched_buttons = 0;
    client->invulnerableEnabled = true;
    client->prevLinkAngles[0] = 0.0f;
    client->prevLinkAngles[1] = 0.0f;
    client->prevLinkAngles[2] = 0.0f;
    client->linkAnglesFrac[0] = 0.0f;
    client->linkAnglesFrac[1] = 0.0f;
    client->linkAnglesFrac[2] = 0.0f;
    client->ps.commandTime = level.time - 100;
    ent->client->pers.cmd.serverTime = level.time;
    ClientThink(ent->mHandle);
    BG_PlayerStateToEntityState(&client->ps, &ent->s, 1);
    if (IS_NAN(ent->r.currentOrigin.v.m128_f32[0])
        || IS_NAN(ent->r.currentOrigin.v.m128_f32[1])
        || IS_NAN(ent->r.currentOrigin.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_client.cpp";
        AeAssert::gCurrentLine = 1192;
        AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    ent->r.currentOrigin.v.m128_f32[0] = ent->client->ps.origin.v.m128_f32[0];
    ent->r.currentOrigin.v.m128_f32[1] = ent->client->ps.origin.v.m128_f32[1];
    ent->r.currentOrigin.v.m128_f32[2] = ent->client->ps.origin.v.m128_f32[2];
    g_LinkEntity(ent);
    ClientEndFrame(ent, ServerTime::sInst.mTickMSec);
    BG_PlayerStateToEntityState(&client->ps, &ent->s, 1);
}

// ea: 0x0048DE60
void ClientEvents(Entity* ent, int oldEventSequence)
{
    Client* client = ent->client;
    int eventSequence = client->ps.event.eventSequence;
    if ((int)oldEventSequence < eventSequence - 4)
        oldEventSequence = (eventSequence - 4);
    unsigned char v6 = (unsigned char)(int)oldEventSequence;
    int iTeamFlags = 0;
    int vSentientPos[3];
    vSentientPos[2] = oldEventSequence;
    if ((int)oldEventSequence >= eventSequence)
        return;
    while (1)
    {
        int v7 = v6 & 3;
        int v8 = client->ps.event.events[v7];
        int v9 = ent->client->ps.event.eventParms[v7];
        if (v8 < 139 || v8 >= 162)
        {
            switch (v8)
            {
            case 186: case 187: case 189: case 197: case 198: case 199:
                FireWeapon(ent);
                break;
            case 193:
                FireWeaponMelee(ent);
                break;
            case 196:
                Spotting(ent);
                break;
            case 222:
            {
                Client* v12 = ent->client;
                if (v12 != nullptr && (ent->flags & 1) == 0)
                {
                    ent->health = 0;
                    v12->ps.stats[0] = 0;
                    player_die(ent, ent, ent, dword_186A0, 25,
                               ent->s.weapon, nullptr, nullptr, HITLOC_NONE);
                }
                break;
            }
            default:
            {
                Client* v13 = ent->client;
                if ((v13->ps.pm_flags & 2) == 0
                    && (v13->pers.cmd.buttons & 8) == 0
                    && ((v8 >= 1 && v8 <= 46) || (v8 >= 93 && v8 <= 138)))
                {
                    sentient_s* sentient = ent->sentient;
                    if (sentient != nullptr)
                    {
                        team_t v15 = Sentient_EnemyTeam(sentient->eTeam);
                        if (v15 != 0 /* TEAM_FREE */)
                        {
                            float v25[3];
                            Sentient_GetOrigin(ent->sentient, v25);
                            int v27 = 1 << v15;
                            if (v8 < 24 || v8 >= 47)
                            {
                                j_nullsub_17(ent, 12 /* AI_EV_FOOTSTEP */, v27,
                                             (math::Position3*)v25, 0.0f);
                            }
                            else
                            {
                                j_nullsub_17(ent, 13 /* AI_EV_FOOTSTEP_LITE */,
                                             v27, (math::Position3*)v25, 0.0f);
                            }
                        }
                    }
                }
                break;
            }
            }
        }
        else
        {
            if (ent->s.eType != 1)
                return;
            float v10;
            if (v9 < 100)
            {
                v10 = v9 * 0.0099999998f;
                if (v10 == 0.0f)
                    goto next_event;
            }
            else
            {
                v10 = 1.1f;
            }
            Client* v11 = ent->client;
            int damageAmount = (int)(ent->client->ps.stats[2] * v10);
            if (v11 == nullptr)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_active.cpp";
                AeAssert::gCurrentLine = 468;
                AeAssert::gCurrentExpr = "ent->client";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("this must be a player"))
                    __debugbreak();
            }
            ent->client->pain_debounce_time = level.time + 200;
            G_Damage(ent, nullptr, nullptr, nullptr, nullptr, damageAmount, 0,
                     24, HITLOC_NONE, -1);
        }
next_event:
        v6 = (unsigned char)(vSentientPos[2] + 1);
        vSentientPos[2] = vSentientPos[2] + 1;
        if (vSentientPos[2] >= ent->client->ps.event.eventSequence)
            return;
        client = ent->client;
    }
}

// ea: 0x0048EBF0
int game_vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4,
                int arg5, int arg6)
{
    (void)arg4; (void)arg5; (void)arg6;
    int v5 = 0;
    switch (command)
    {
    case 0:
        return G_InitGame(arg0, arg1, arg2, arg3);
    case 1:
        G_ShutdownGame(arg0);
        return 0;
    case 2:
        return (int)ClientConnect(*(DbLinkedHandle<EntityHandleDb, Entity>*)arg0);
    case 3:
        ClientBegin(*(DbLinkedHandle<EntityHandleDb, Entity>*)arg0);
        return 0;
    case 4:
        ClientDisconnect(*(DbLinkedHandle<EntityHandleDb, Entity>*)arg0);
        return 0;
    case 5:
        ClientCommand(*(DbLinkedHandle<EntityHandleDb, Entity>*)arg0);
        return 0;
    case 6:
        ClientThink(*(DbLinkedHandle<EntityHandleDb, Entity>*)arg0);
        return 0;
    case 7:
        return GetFollowPlayerState(arg0, (PlayerState*)arg1);
    case 8:
        G_LoadLevel(arg0);
        return 0;
    case 9:
        G_CheckLoadGame(arg0, arg1);
        return 0;
    case 11:
        G_RunPreFrame(arg0);
        return 0;
    case 12:
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_main.cpp";
        AeAssert::gCurrentLine = 740;
        AeAssert::gCurrentExpr = nullptr;
        if (AeAssert::IsIgnored()
            || !AeAssert::Warning("G_RunFrame should now be called directly - don't use VM"))
            return 0;
        __debugbreak();
        return 0;
    }
    case 13:
        return ConsoleCommand();
    case 14:
        return 0;
    case 18:
        return level.snapTime;
    case 19:
    {
        Entity* v7 = HandleDbToEnt(*(DbLinkedHandle<EntityHandleDb, Entity>*)arg0);
        G_DObjCalcPose(v7);
        return 0;
    }
    case 20:
        return level.time;
    case 22:
        G_SendClientMessages();
        return 0;
    default:
        v5 = -1;
        return v5;
    }
}
// ea: 0x00482C10
void Player_UpdateCursorHints(Entity* ent)
{
    if (ent->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PlayerUse.cpp";
        AeAssert::gCurrentLine = 573;
        AeAssert::gCurrentExpr = "ent->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    PlayerState* p_ps = &ent->client->ps;
    p_ps->serverCursorHint = 0;
    p_ps->serverCursorHintVal = 0;
    p_ps->serverCursorHintTrace.mEntity.mHandle.mVal = 0;
    gGrenadeCanBePickedUp = false;
    if (ent->health <= 0)
        return;
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(p_ps->weapon);
    if (InfoForWeapon->type != 1 /* WEAPTYPE_GRENADE */
        || (*(int*)((char*)InfoForWeapon + 0x6EC)) == 0
        || (p_ps->grenadeTimeLeft >= InfoForWeapon->iFuseTime)
        || p_ps->grenadeTimeLeft == 0
        || Com_BitCheck(p_ps->weapons, p_ps->weapon) == 0)
    {
        Player_UpdateFriendlyOverlay(ent);
        if (ent->active == 0 && g_reloading.integer != 4)
        {
            p_ps->serverCursorHint = 0;
            p_ps->serverCursorHintVal = 0;
            int hintType = 0;
            int hintVal = 0;
            p_ps->serverCursorHintString = -1;
            int hintString = -1;
            int weapClass = BG_GetInfoForWeapon(ent->client->ps.weapon)->weapClass;
            Client* client = ent->client;
            if ((client->ps.pm_flags & 0x20) != 0 && weapClass == 14 /* WEAPCLASS_LMG */)
            {
            }
            else if ((0x100000 & client->ps.eFlags) != 0)
            {
                Entity* owner = HandleDbToEnt(ent->r.mOwner);
                if (owner == nullptr || owner->scr_vehicle == nullptr)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PlayerUse.cpp";
                    AeAssert::gCurrentLine = 620;
                    AeAssert::gCurrentExpr = "*ent->r.mOwner && ent->r.mOwner->scr_vehicle";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("bad vehicle entity"))
                        __debugbreak();
                }
                if (ent->client->ps.vehPos != 7
                    || owner->scr_vehicle->mMantleTime != 0)
                {
                    if (weapClass == 14 /* WEAPCLASS_LMG */
                        && BG_AllowPlayerWeaponAtVehiclePos(
                            ent->client->ps.vehType, ent->client->ps.vehPos))
                        p_ps->serverCursorHint = 12;
                }
                else
                {
                    int v8 = p_ps->weaponslots[4];
                    p_ps->serverCursorHint = 2;
                    static unsigned int mantleJumpOffHintString =
                        HashString::CalcHash("MPGAME_TANK_MANTLE_JUMPOFF");
                    if (v8 != 0)
                    {
                        weaponFileInfo_t* v9 = BG_GetInfoForWeapon(v8);
                        if (v9 != nullptr && v9->bCanMantle != 0)
                        {
                            static unsigned int mantleGrenadeThrowHintString =
                                HashString::CalcHash("MPGAME_TANK_MANTLE_GRENADE");
                            p_ps->serverCursorHintString =
                                mantleGrenadeThrowHintString;
                        }
                        else
                        {
                            p_ps->serverCursorHintString =
                                mantleJumpOffHintString;
                        }
                    }
                    else
                    {
                        p_ps->serverCursorHintString = mantleJumpOffHintString;
                    }
                }
            }
            else if (!EntityManager::sInst->IsLocalPlayer(ent)
                     || InteractionController::Inst(
                            EntityManager::sInst->GetPlayerIndex(ent)) == nullptr)
            {
                useList_t useList[1344];
                int numUsable = Player_GetActivateEnt(ent, useList);
                if (weapClass == 14 /* WEAPCLASS_LMG */)
                {
                    float point[3];
                    point[0] = 0.0f;
                    point[1] = ent->client->ps.viewangles[1];
                    point[2] = 0.0f;
                    float forward[3];
                    AnglesToForward(point, forward);
                    point[0] = forward[0] * 25.0f
                               + ent->r.currentOrigin.v.m128_f32[0];
                    point[1] = forward[1] * 25.0f
                               + ent->r.currentOrigin.v.m128_f32[1];
                    point[2] = forward[2] * 25.0f
                               + ent->r.currentOrigin.v.m128_f32[2] + 1.0f;
                    int crouch = 0;
                    if (G_CheckPointInsideTriggerMount(ent, point, &crouch) != 0)
                    {
                        hintType = 12;
                        hintVal = crouch;
                        if (BG_GetInfoForWeapon(ent->client->ps.weapon)
                                ->szUseHintString[0]
                            != 0)
                            hintString = BG_GetInfoForWeapon(
                                             ent->client->ps.weapon)
                                             ->iUseHintStringIndex;
                    }
                    p_ps->serverCursorHint = hintType;
                    p_ps->serverCursorHintVal = hintVal;
                    p_ps->serverCursorHintString = hintString;
                }
                if (numUsable != 0)
                {
                    int hasString = 0;
                    for (int i = 0; i < numUsable; ++i)
                    {
                        Entity* v13 = useList[i].ent;
                        p_ps->serverCursorHintTrace.mEntity.mHandle.mVal =
                            v13->mHandle.mHandle.mVal;
                        if (v13 == EntityManager::sInst->mWorld)
                        {
                            int dmgFlags;
                            if ((p_ps->serverCursorHintTrace.surfaceFlags & 8)
                                    != 0
                                && (p_ps->pm_flags & 0x10) == 0)
                                dmgFlags = 14;
                            else
                                dmgFlags = hintType;
                            p_ps->serverCursorHintVal = hintVal;
                            p_ps->serverCursorHint = dmgFlags;
                            if (hasString == 0)
                                p_ps->serverCursorHintString = hintString;
                            if (dmgFlags == 0)
                                p_ps->serverCursorHintTrace.mEntity.mHandle.mVal = 0;
                            return;
                        }
                        if (v13->actor != nullptr)
                        {
                            sentient_s* sentient = v13->sentient;
                            int dmgFlags = hintType;
                            if (sentient != nullptr
                                && (sentient->eTeam == TEAM_ALLIES
                                    || sentient->eTeam == TEAM_NEUTRAL))
                                dmgFlags = 16;
                            if (v13->actor->iUseHintString >= 0)
                                hintString = v13->actor->iUseHintString;
                            p_ps->serverCursorHintVal = hintVal;
                            p_ps->serverCursorHint = dmgFlags;
                            if (hasString == 0)
                                p_ps->serverCursorHintString = hintString;
                            if (dmgFlags == 0)
                                p_ps->serverCursorHintTrace.mEntity.mHandle.mVal = 0;
                            return;
                        }
                        if (v13->s.eType == 0)
                        {
                            if (v13->mClassNameHash.mHash
                                == hash_const.trigger_use.mHash)
                            {
                                hintType = v13->s.dmgFlags;
                                if (hintType != 0)
                                {
                                    unsigned char scale = v13->s.scale;
                                    if (scale != 0xFF)
                                        hintString = scale;
                                }
                            }
                        }
                        else if (v13->s.eType == 10)
                        {
                            if (G_IsTurretUsable(v13, ent) != 0)
                            {
                                hintType = 11;
                                if (BG_GetInfoForWeapon(v13->s.weapon)
                                        ->szUseHintString[0]
                                    != 0)
                                    hintString = BG_GetInfoForWeapon(
                                                     v13->s.weapon)
                                                     ->iUseHintStringIndex;
                            }
                        }
                        else if (v13->s.eType == 14)
                        {
                            if (v13->scr_vehicle->CanMantleVehicle(ent))
                            {
                                if (G_IsVehicleUsable(v13, ent, true) != 0)
                                {
                                    hintType = 6;
                                    hintString =
                                        v13->scr_vehicle->GetMantleHintStringIndex();
                                }
                            }
                            else if (G_IsVehicleUsable(v13, ent, true) != 0)
                            {
                                int mVehicleEntryPoint =
                                    ent->client->mVehicleEntryPoint;
                                int v23 =
                                    G_EntryPointSeatAssociation(v13, mVehicleEntryPoint);
                                if (v23 != 0)
                                {
                                    int v24 = v23 - 1;
                                    if (v24 == 1)
                                        hintType = 3;
                                    else if (v24 == 0)
                                        hintType = 5;
                                }
                                else
                                {
                                    hintType = 4;
                                }
                                hintString =
                                    v13->scr_vehicle->GetEntryHintStringIndex(
                                        v13, mVehicleEntryPoint);
                            }
                        }
                        else if (v13->s.eType == 7)
                        {
                            hintType = 7;
                        }
                        else if (v13->s.eType == 2)
                        {
                            const gitem_s* v26 = v13->item;
                            switch (v26->giType)
                            {
                            case 1 /* IT_WEAPON */:
                            {
                                int giTag = v26->giTag;
                                hintType = Com_BitCheck(ent->client->ps.weapons,
                                                        giTag)
                                               ? giTag + 144
                                               : giTag + 16;
                                break;
                            }
                            case 2 /* IT_AMMO */:
                                hintType = v26->giTag + 144;
                                break;
                            case 3: case 5:
                                hintType = 13;
                                break;
                            case 4:
                                hintType = 145;
                                break;
                            case 6:
                                hintType = 7;
                                break;
                            case 7:
                                hintType = v13->count + 273;
                                break;
                            default:
                                break;
                            }
                        }
                        else if (v13->s.eType == 4)
                        {
                            unsigned int mHash = v13->mClassNameHash.mHash;
                            if (mHash == hash_const.func_door_rotating.mHash
                                && v13->moverState != 7
                                && v13->moverState != 8)
                                continue;
                            if (mHash == hash_const.func_door.mHash
                                && v13->moverState != 0
                                && v13->moverState != 1)
                                continue;
                            hintType = 9;
                            if (v13->key != 0)
                                hintType = 10;
                        }
                        int dmgFlags = hintType;
                        if (v13->s.dmgFlags > 0 && hintType != 0)
                            dmgFlags = v13->s.dmgFlags;
                        if (v13->mHintString != 0)
                        {
                            p_ps->serverCursorHintString = v13->mHintString;
                            hasString = 1;
                        }
                        p_ps->serverCursorHintVal = hintVal;
                        p_ps->serverCursorHint = dmgFlags;
                        if (hasString == 0)
                            p_ps->serverCursorHintString = hintString;
                        if (dmgFlags == 0)
                            p_ps->serverCursorHintTrace.mEntity.mHandle.mVal = 0;
                        return;
                    }
                }
            }
        }
    }
}

// ea: 0x004664A0
void Player_UpdateLookAtEntity(Entity* pEnt)
{
    Client* client = pEnt->client;
    if (client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PlayerUse.cpp";
        AeAssert::gCurrentLine = 1046;
        AeAssert::gCurrentExpr = "pEnt->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Entity* pLookatEnt = pEnt->client->pLookatEnt;
    if (pLookatEnt == nullptr || pLookatEnt->actor == nullptr)
        pEnt->sentient->mDesiredChainPos.mValue = 0;
    pEnt->client->iLookatEntLastTime = 0;
    weaponParms weapParms;
    if (pEnt->active != 0)
    {
        Entity* mObject = HandleDbToEnt(pEnt->client->ps.mViewLockedEntity);
        if (Turret_FillWeaponParms(mObject, pEnt, &weapParms, 0) == 0)
            return;
    }
    else
    {
        CalcMuzzlePoints(pEnt, &weapParms);
    }
    unsigned char* v6 = riflePriorityMap;
    if (pEnt->client->ps.weapon == 0
        || BG_GetInfoForWeapon(pEnt->client->ps.weapon)->bRifleBullet == 0)
        v6 = bulletPriorityMap;
    math::Position3 start;
    math::Position3 end;
    collision_context_t context;
    context.pass_entity1.mHandle.mVal = 0;
    context.pass_entity2.mHandle.mVal = 578822145;
    context.pass_owner1.mHandle.mVal = 0;
    context.pass_owner2.mHandle.mVal = 0;
    context.contentmask = 0x2802033;
    trace_t trace;
    if ((0x100000 & pEnt->client->ps.eFlags) != 0)
    {
        int idx = 1580 * EntityManager::sInst->GetPlayerIndex(pEnt);
        start.v.m128_f32[0] = dword_F63C70[idx];
        start.v.m128_f32[1] = dword_F63C70[idx + 1];
        start.v.m128_f32[2] = dword_F63C70[idx + 2];
        start.v.m128_f32[3] = 0.0f;
        float forward[3];
        AnglesToForward(pEnt->client->ps.viewangles, forward);
        end.v.m128_f32[0] = forward[0] * 8192.0f + start.v.m128_f32[0];
        end.v.m128_f32[1] = forward[1] * 8192.0f + start.v.m128_f32[1];
        end.v.m128_f32[2] = forward[2] * 8192.0f + start.v.m128_f32[2];
        end.v.m128_f32[3] = 0.0f;
        context.pass_entity1.mHandle.mVal = pEnt->r.mOwner.mHandle.mVal;
        g_LocationalTrace(&trace, start, end, context, v6, 0.0f);
    }
    else
    {
        start.v.m128_f32[0] = weapParms.up[0];
        start.v.m128_f32[1] = weapParms.up[1];
        start.v.m128_f32[2] = weapParms.up[2];
        start.v.m128_f32[3] = 0.0f;
        end.v.m128_f32[0] = weapParms.forward[0] * 8192.0f + start.v.m128_f32[0];
        end.v.m128_f32[1] = weapParms.forward[1] * 8192.0f + start.v.m128_f32[1];
        end.v.m128_f32[2] = weapParms.forward[2] * 8192.0f + start.v.m128_f32[2];
        end.v.m128_f32[3] = 0.0f;
        context.pass_entity1.mHandle.mVal = pEnt->mHandle.mHandle.mVal;
        g_LocationalTrace(&trace, start, end, context, v6, 0.0f);
    }
    pEnt->client->fLastTraceDist = trace.normal.v.m128_f32[1] * 8192.0f;
    if (trace.mEntity.mHandle.mVal
        == EntityManager::sInst->mWorld->mHandle.mHandle.mVal)
        return;
    Entity* v11 = HandleDbToEnt(trace.mEntity);
    if (v11 == nullptr)
        return;
    int weapon = pEnt->client->ps.weapon;
    float fTraceDist = trace.normal.v.m128_f32[1] * 8192.0f;
    if ((weapon == 0
         || (BG_GetInfoForWeapon(weapon)->iDamageOuterRadius) < 0
         || v11->sentient == nullptr
         || v11->sentient->eTeam == pEnt->sentient->eTeam
         || BG_GetInfoForWeapon(weapon)->iDamageOuterRadius > fTraceDist)
        && SmokeGrenadeMgr_EntityCanSeeEntity(SmokeGrenadeMgr::sInst, pEnt,
                                              v11, 0.40000001f))
    {
        pEnt->client->pLookatEnt = v11;
        pEnt->client->iLookatEntLastTime = level.time;
    }
    if ((v11->flags & 0x2000000) != 0)
    {
        if (v11->team.mBlock != nullptr
            && (v11->team.mBlock + 1) != nullptr
            && ((char*)&(v11->team.mBlock + 1)->mBuff)[0] != 0
            && v11->team.GetBuff()[1] == 'l')
            pEnt->client->pLookatEnt = v11;
        else if (v11->team.mBlock != nullptr
                 && (v11->team.mBlock + 1) != nullptr
                 && ((char*)&(v11->team.mBlock + 1)->mBuff)[0] != 0
                 && v11->team.GetBuff()[1] == 'x')
            pEnt->client->pLookatEnt = v11;
        goto trigger_check;
    }
    if ((v11->r.contents & 0x4000) != 0)
    {
        if ((trace.normal.v.m128_f32[2] != 0.0f)
            && (v11->sentient == nullptr
                || (~(1 << Sentient_EnemyTeam(pEnt->sentient->eTeam))
                    & (1 << v11->sentient->eTeam)) != 0))
        {
            if (g_femanager.mDontDrawHud)
                pEnt->client->pLookatEnt = nullptr;
            else
                pEnt->client->pLookatEnt = v11;
        }
    }
    else if (v11->scr_vehicle == nullptr)
    {
    trigger_check:
        if ((gTriggerLookAtOverride < 0.0f
             || (trace.normal.v.m128_f32[1] * 8192.0f)
                    <= gTriggerLookAtOverride)
            && v11->mClassNameHash.mHash == hash_const.trigger_lookat.mHash)
        {
            pEnt->client->pLookatEnt = v11;
            G_Trigger(v11, pEnt);
        }
        return;
    }
    else if (!g_femanager.mDontDrawHud)
    {
        pEnt->client->pLookatEnt = v11;
    }
}

// ea: 0x00491B60
void ClientEndFrame(Entity* ent, int msec)
{
    Client* client = ent->client;
    int playerState = client->pers.playerState;
    if (playerState == 2)
    {
        IntermissionClientEndFrame(ent);
        return;
    }
    if (playerState == 1)
    {
        SpectatorClientEndFrame(ent);
        return;
    }
    if ((client->ps.eFlags & 0x6000) != 0)
    {
        if (client->ps.mViewLockedEntity.mHandle.mVal == 0)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_active.cpp";
            AeAssert::gCurrentLine = 1230;
            AeAssert::gCurrentExpr = "ent->client->ps.mViewLockedEntity != TEntityHandle::NullHandle()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        Entity* v5 = HandleDbToEnt(ent->client->ps.mViewLockedEntity);
        turret_think_client(v5);
    }
    ent->r.svFlags = ent->r.svFlags & 0xFFFFFFF6 | 8;
    ent->takedamage = level.time >= ent->invulnerability_timeout;
    ent->client->ps.pm_flags |= 0x80000;
    G_SetClientContents(ent);
    Client* v8 = ent->client;
    if (v8->mVehicleNoWeaponTime != 0 && v8->mVehicleNoWeaponTime < level.time)
    {
        v8->mVehicleNoWeaponTime = 0;
        G_DObjUpdate(ent, false);
    }
    G_VehicleClientThink(msec);
    if (ent->client->ps.eFlags == 0 && ent->client->ps.eFlags == 0)
    {
        if (ent->tagInfo != nullptr)
        {
            ent->client->ps.pm_type = ent->client->ps.stats[0] > 0 ? 1 : 7;
            G_LinkClient(ent);
        }
        else
        {
            if (ent->client->ps.pm_type == 1 || ent->client->ps.pm_type == 7)
            {
                --ent->client->ps.pm_type;
            }
            else
            {
                ent->client->ps.pm_time = 0;
                ent->client->prevLinkAngles[0] = 0.0f;
                ent->client->prevLinkAngles[1] = 0.0f;
            }
        }
    }
    if (ent->sentient == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_active.cpp";
        AeAssert::gCurrentLine = 1305;
        AeAssert::gCurrentExpr = "ent->sentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Sentient_UpdateActualChainPos(ent->sentient);
    if (ent->client->ps.commandTime != 0
        && level.time > 500
        && EntityManager::sInst->IsLocalPlayer(ent))
    {
        Player_UpdateLookAtEntity(ent);
        Player_UpdateCursorHints(ent);
    }
    P_DamageFeedback(ent);
    int eFlags = ent->s.eFlags;
    if (level.time - ent->client->lastCmdTime <= 1000)
        ent->s.eFlags = eFlags & 0xFFFFF7FF;
    else
        ent->s.eFlags = eFlags | 0x800;
    ent->client->ps.stats[0] = ent->health;
    ent->s.loopSound = 0;
    BG_PlayerStateToEntityStateExtrapolate(&ent->client->ps, &ent->s,
                                           ent->client->ps.commandTime, 1);
    if (BG_GetInfoForWeapon(ent->client->ps.weapon)->type == 1 /* WEAPTYPE_GRENADE */)
    {
        int grenadeTimeLeft = ent->client->ps.grenadeTimeLeft;
        if (grenadeTimeLeft != 0 && grenadeTimeLeft < 3000)
            j_nullsub_17(ent, 0x10 /* AI_EV_GRENADE_COOK */, 0,
                         (math::Position3*)&ent->client->ps.origin, 0.0f);
    }
    float viewPos[3];
    viewPos[0] = ent->client->ps.origin.v.m128_f32[0];
    viewPos[1] = ent->client->ps.origin.v.m128_f32[1];
    viewPos[2] = ent->client->ps.origin.v.m128_f32[2]
                 + ent->client->ps.viewHeightCurrent;
    G_AddLean(ent, viewPos);
    ent->client->ps.iCompassFriendInfo =
        G_GetNonPVSFriendlyInfo(viewPos,
            ent->client->hLastCompassFriendlyInfoEnt.mHandle.mVal);
    if (ent->client->ps.iCompassFriendInfo != 0)
    {
        Entity* actor = G_GetFriendlyIndexActor(ent->client->ps.iCompassFriendInfo & 0x3F);
        ent->client->hLastCompassFriendlyInfoEnt.mHandle.mVal =
            actor != nullptr ? actor->mHandle.mHandle.mVal : 0;
    }
    else
    {
        ent->client->hLastCompassFriendlyInfoEnt.mHandle.mVal = 0;
    }
    ent->client->ps.iCompassTankInfo = 0;
    if (ent->client->ps.iCompassTankInfo != 0)
    {
        DbLinkedHandle<EntityHandleDb, Entity> result =
            G_GetTankEntNum(ent->client->ps.iCompassTankInfo & 0x3F);
        ent->client->hLastCompassTankInfoEnt.mHandle.mVal = result.mHandle.mVal;
    }
    else
    {
        ent->client->hLastCompassTankInfoEnt.mHandle.mVal = 0;
    }
}
