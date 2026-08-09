// ============================================================================
// g_combat.cpp - damage/combat helpers (g.o: g_combat.cpp family, leaf subset)
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

extern "C" int __fpclass(float);

// ea: 0x0044AF10
void handleDeathInvulnerability(Entity* /*ent*/, int /*a2*/, int /*a3*/)
{
    ;
}

static bool IS_NAN(float x) {
    return (__fpclass(x) & 0x297) != 0;
}

// ea: 0x0044AF20
int CheckArmor(Entity* ent, int damage, int dflags)
{
    if (damage != 0 && ent->actor != nullptr)
        return Actor_CheckArmor(ent->actor, damage, dflags);
    else
        return 0;
}

// ea: 0x0044C1B0
void G_MissileLandAngles(Entity* ent, trace_t* trace, math::Position3* vAngles,
                         int bForceAlign)
{
    int v7 = level.previousTime + (int)((level.time - level.previousTime) * trace->fraction);
    BG_EvaluateTrajectory(&ent->s.apos, v7, *vAngles);
    if (trace->normal.v.m128_f32[2] <= 0.1f)
    {
        if (bForceAlign == 0)
        {
            float angle = ((rand() & 0x7F) - 63) + ent->s.apos.trDelta[0];
            ent->s.apos.trDelta[0] = AngleNormalize360(angle);
        }
    }
    else
    {
        float fSurfacePitch = PitchForYawOnNormal(vAngles->v.m128_f32[1], trace->normal.v.m128_f32);
        float fAngleDelta = AngleSubtract(fSurfacePitch, vAngles->v.m128_f32[0]);
        float fAbsAngDelta = (float)fabs(fAngleDelta);
        if (bForceAlign == 0)
        {
            ent->s.apos.trBase[0] = vAngles->v.m128_f32[0];
            ent->s.apos.trBase[1] = vAngles->v.m128_f32[1];
            ent->s.apos.trBase[2] = vAngles->v.m128_f32[2];
            ent->s.apos.trTime = v7;
            float v8;
            if (fAbsAngDelta >= 80.0f)
                v8 = ((float)rand() * 0.0000091552738f + 0.85000002f) * ent->s.apos.trDelta[0];
            else
                v8 = (((float)rand() * 0.0000091552738f + 0.85000002f) * ent->s.apos.trDelta[0]) * -1.0f;
            ent->s.apos.trDelta[0] = v8;
        }
        float tracea = AngleNormalize180(vAngles->v.m128_f32[0]);
        vAngles->v.m128_f32[0] = tracea;
        if (bForceAlign != 0 || fAbsAngDelta < 45.0f)
        {
            if ((float)fabs(tracea) <= 90.0f)
                vAngles->v.m128_f32[0] = AngleNormalize360(fSurfacePitch);
            else
                vAngles->v.m128_f32[0] = AngleNormalize360(fSurfacePitch + 180.0f);
        }
        else if (fAbsAngDelta >= 80.0f)
        {
            vAngles->v.m128_f32[0] = AngleNormalize360(tracea);
        }
        else
        {
            vAngles->v.m128_f32[0] = AngleNormalize360((fAngleDelta * 0.25f) + tracea);
        }
    }
}

// ea: 0x0044C390
void G_LaunchMissile(Entity* ent)
{
    ent->nextthink = level.time + BG_GetInfoForWeapon(ent->s.weapon)->iProjectileDelay;
    ent->think = THINK__G_IncomingMissile;
    if (gpBrocAPI->mBrocExports.mCallbackFireArtilleryShell != nullptr)
        gpBrocAPI->mBrocExports.mCallbackFireArtilleryShell(ent->mHandle.mHandle.mVal);
}

// ea: 0x0044C3F0
void G_IncomingMissile(Entity* ent)
{
    ent->nextthink = level.time + 1100;
    ent->think = THINK__G_DelayMissile;
    PostEffectEventScriptCall(ent, "SHELL_INCOMING", false, PAK_ID_INVALID, false);
}

// ea: 0x0044C430
void G_DelayMissile(Entity* ent)
{
    ent->nextthink = level.time + 30000;
    ent->think = THINK__G_ExplodeMissile;
    float v1 = ent->r.currentOrigin.v.m128_f32[0];
    float v2 = ent->r.currentOrigin.v.m128_f32[1];
    float v3 = ent->r.currentOrigin.v.m128_f32[2] + 1000.0f;
    ent->s.pos.trBase[0] = v1;
    ent->s.pos.trBase[1] = v2;
    ent->s.pos.trBase[2] = v3;
    ent->r.currentOrigin.v.m128_f32[0] = v1;
    ent->r.currentOrigin.v.m128_f32[1] = v2;
    ent->r.currentOrigin.v.m128_f32[2] = v3;
    ent->s.pos.trType = TR_LINEAR;
    ent->s.pos.trTime = level.time - 50;
}

// ea: 0x0044C4B0
void G_MissileDie(Entity* self, Entity* inflictor)
{
    if (inflictor != self)
    {
        self->takedamage = 0;
        self->think = THINK__G_ExplodeMissile;
        self->nextthink = level.time + 10;
    }
}

// ea: 0x0044C4E0
void G_PredictBounceMissile(const Entity* ent, trajectory_t* pos, trace_t* trace, int time)
{
    float v14[3];
    math::Position3 origin;
    BG_EvaluateTrajectory(pos, time, *reinterpret_cast<math::Position3*>(v14));
    BG_EvaluateTrajectoryDelta(pos, time, origin.v.m128_f32);
    float v5 = origin.v.m128_f32[2];
    float v6 = origin.v.m128_f32[1];
    float v7 = (((trace->normal.v.m128_f32[0] * origin.v.m128_f32[0])
                 + (v5 * trace->normal.v.m128_f32[2]))
                + (v6 * trace->normal.v.m128_f32[1]))
               * -2.0f;
    float v8 = (v7 * trace->normal.v.m128_f32[0]) + origin.v.m128_f32[0];
    pos->trDelta[0] = v8;
    pos->trDelta[1] = (v7 * trace->normal.v.m128_f32[1]) + v6;
    pos->trDelta[2] = (v7 * trace->normal.v.m128_f32[2]) + v5;
    if ((ent->s.eFlags & 0x2000000) == 0)
        goto label_5;
    pos->trDelta[0] = v8 * 0.5f;
    float velocity0 = v8 * 0.5f;
    float v9 = pos->trDelta[1] * 0.5f;
    pos->trDelta[1] = v9;
    float velocity1 = v9;
    float v10 = pos->trDelta[2] * 0.5f;
    pos->trDelta[2] = v10;
    float velocity2 = v10;
    if (trace->normal.v.m128_f32[2] <= 0.69999999f)
        goto label_5;
    if (sqrt(velocity2 * velocity2 + velocity1 * velocity1 + velocity0 * velocity0) < 20.0f)
    {
        memcpy(pos->trBase, trace, sizeof(pos->trBase));
        pos->trType = TR_STATIONARY;
        pos->trTime = 0;
        pos->trDuration = 0;
        pos->trDelta[1] = 0.0f;
        pos->trDelta[0] = 0.0f;
    }
    else
    {
label_5:
        float v11 = trace->normal.v.m128_f32[2] * 0.1f;
        if (v11 > 0.0f)
            v11 = 0.0f;
        float v12 = v14[0] + (trace->normal.v.m128_f32[0] * 0.1f);
        pos->trBase[1] = v14[1] + (trace->normal.v.m128_f32[1] * 0.1f);
        float v13 = v14[2];
        pos->trTime = time;
        pos->trBase[0] = v12;
        pos->trBase[2] = v13 + v11;
    }
}

// ea: 0x0044C8C0
void Static_Pain(Entity* ent)
{
    bool v2 = (ent->spawnflags & 4) == 0;
    float enta = (float)level.time;
    if (v2)
    {
        if (enta > ((float)(rand() % 1000) + ent->delay + ent->wait + 500.0f))
            ent->wait = (float)level.time;
    }
    else if (enta > ((float)(rand() % 1000) + ent->delay + ent->wait + 500.0f))
    {
        ent->wait = (float)level.time;
        float temp[3];
        temp[0] = ent->r.currentOrigin.v.m128_f32[0];
        temp[1] = ent->r.currentOrigin.v.m128_f32[1];
        temp[2] = ent->r.currentOrigin.v.m128_f32[2];
        if (IS_NAN(temp[0]) || IS_NAN(ent->r.currentOrigin.v.m128_f32[1])
            || IS_NAN(ent->r.currentOrigin.v.m128_f32[2]))
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
            AeAssert::gCurrentLine = 2062;
            AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        ent->r.currentOrigin.v.m128_f32[0] = ent->pos3.v.m128_f32[0];
        ent->r.currentOrigin.v.m128_f32[1] = ent->pos3.v.m128_f32[1];
        ent->r.currentOrigin.v.m128_f32[2] = ent->pos3.v.m128_f32[2];
        ent->r.currentOrigin.v.m128_f32[0] = temp[0];
        ent->r.currentOrigin.v.m128_f32[1] = temp[1];
        ent->r.currentOrigin.v.m128_f32[2] = temp[2];
    }
}

// ea: 0x0044D2D0
void G_SetupProps()
{
    ;
}

// ea: 0x0044F1F0
int G_IsVehicleImmune(Entity* ent, int mod)
{
    vehicle_info_t* v2 = s_vehicleInfos[ent->scr_vehicle->infoIdx];
    float bulletDamage;
    switch (mod)
    {
    case 1:
    case 2:
        bulletDamage = v2->bulletDamage;
        if (bulletDamage > 0.0049999999f)
            return 0;
        return 1;
    case 3:
    case 4:
        bulletDamage = v2->grenadeDamage;
        if (bulletDamage > 0.0049999999f)
            return 0;
        return 1;
    case 5:
    case 6:
        bulletDamage = v2->mineDamage;
        if (bulletDamage > 0.0049999999f)
            return 0;
        return 1;
    case 9:
    case 10:
    case 17:
    case 18:
        bulletDamage = v2->projectileDamage;
        if (bulletDamage > 0.0049999999f)
            return 0;
        return 1;
    case 27:
    case 31:
    case 32:
        return 0;
    default:
        return 1;
    }
}

// ea: 0x00452D70
float Bullet_Endpos(float spread, float* end, const weaponParms* wp)
{
    float v3 = 16384.0f;
    const weaponParms* v4 = wp;
    if (wp->pWeapInfo->iProjectileDelay != 5)
        v3 = 8192.0f;
    if (IS_NAN(spread))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 499;
        AeAssert::gCurrentExpr = "!IS_NAN(spread)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    float v5 = (float)tan(spread * 3.1415927f * 0.0055555557f);
    gTanAimConeSpread = v5;
    float v6 = v5 * v3;
    float fAimOffset = v6;
    if (IS_NAN(v6))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 507;
        AeAssert::gCurrentExpr = "!IS_NAN(fAimOffset)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    float r, u;
    gunrandom(&r, &u);
    u = u * fAimOffset;
    r = r * fAimOffset;
    if (IS_NAN(r))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 513;
        AeAssert::gCurrentExpr = "!IS_NAN(r)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    if (IS_NAN(u))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 514;
        AeAssert::gCurrentExpr = "!IS_NAN(u)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    if (IS_NAN(v4->muzzleTrace[0]) || IS_NAN(v4->muzzleTrace[1]) || IS_NAN(v4->muzzleTrace[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 515;
        AeAssert::gCurrentExpr = "!IS_NAN((wp->muzzleTrace)[0]) && !IS_NAN((wp->muzzleTrace)[1]) && !IS_NAN((wp->muzzleTrace)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(v4->forward[0]) || IS_NAN(v4->forward[1]) || IS_NAN(v4->forward[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 516;
        AeAssert::gCurrentExpr = "!IS_NAN((wp->forward)[0]) && !IS_NAN((wp->forward)[1]) && !IS_NAN((wp->forward)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(v4->right[0]) || IS_NAN(v4->right[1]) || IS_NAN(v4->right[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 517;
        AeAssert::gCurrentExpr = "!IS_NAN((wp->right)[0]) && !IS_NAN((wp->right)[1]) && !IS_NAN((wp->right)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(v4->up[0]) || IS_NAN(v4->up[1]) || IS_NAN(v4->up[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 518;
        AeAssert::gCurrentExpr = "!IS_NAN((wp->up)[0]) && !IS_NAN((wp->up)[1]) && !IS_NAN((wp->up)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    end[0] = (v4->forward[0] * v3) + v4->muzzleTrace[0];
    float v7 = end[0];
    end[1] = (v4->forward[1] * v3) + v4->muzzleTrace[1];
    end[2] = (v4->forward[2] * v3) + v4->muzzleTrace[2];
    if (IS_NAN(v7) || IS_NAN(end[1]) || IS_NAN(end[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 522;
        AeAssert::gCurrentExpr = "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    float v8 = r;
    end[0] = (v4->right[0] * r) + end[0];
    end[1] = (v4->right[1] * v8) + end[1];
    float v9 = (v4->right[2] * v8) + end[2];
    end[2] = v9;
    end[0] = (v4->up[0] * u) + end[0];
    float v11 = end[0];
    end[1] = (v4->up[1] * u) + end[1];
    end[2] = (v4->up[2] * u) + end[2];
    if (IS_NAN(v11) || IS_NAN(end[1]) || IS_NAN(end[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 527;
        AeAssert::gCurrentExpr = "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    return fAimOffset;
}

// ea: 0x00453310
float Damage_Falloff(float fDistance, float fDamage, float fMinDamagePercent,
                     int iInnerRadius, int iOuterRadius)
{
    float v5 = fDamage;
    if (fDistance > iInnerRadius)
    {
        if (fDistance <= iOuterRadius)
            v5 = ((((1.0f - ((fDistance - iInnerRadius) / (iOuterRadius - iInnerRadius)))
                    * (100.0f - fMinDamagePercent))
                   + fMinDamagePercent)
                  * fDamage)
                 * 0.0099999998f;
        else
            v5 = (fMinDamagePercent * 0.0099999998f) * fDamage;
        fDamage = v5;
    }
    if (v5 < 0.0f)
        return 0.0f;
    return fDamage;
}

// ea: 0x004533B0
bool Bullet_ShouldGoThroughFriend()
{
    return false;
}

// ea: 0x00453420
int LogAccuracyHit(Entity* target, Entity* attacker)
{
    if (target == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 2131;
        AeAssert::gCurrentExpr = "target";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (attacker == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 2132;
        AeAssert::gCurrentExpr = "attacker";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Client* client = target->client;
    return target->takedamage != 0
        && target != attacker
        && client != nullptr
        && attacker->client != nullptr
        && client->ps.stats[0] > 0;
}

// ea: 0x004551C0
void P_DamageFeedback(Entity* player)
{
    Client* client = player->client;
    if (client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_active.cpp";
        AeAssert::gCurrentLine = 55;
        AeAssert::gCurrentExpr = "client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (client->ps.pm_type < 6)
    {
        int damage_blood = client->damage_blood;
        if (damage_blood > 0)
        {
            int maxHealth = client->pers.maxHealth;
            if (maxHealth > 0)
            {
                int v4 = 100 * damage_blood / maxHealth;
                int v5 = v4;
                if (v4 >= 1)
                {
                    if (v4 > 127)
                        v5 = 127;
                }
                else
                {
                    v5 = 1;
                }
                float v6 = v5 + client->ps.aimSpreadScale;
                client->ps.aimSpreadScale = v6;
                if (v6 > 255.0f)
                    client->ps.aimSpreadScale = 255.0f;
                int v7;
                if (client->damage_fromWorld != 0)
                {
                    v7 = 255;
                    client->ps.event.damagePitch = 255;
                    client->damage_fromWorld = 0;
                }
                else
                {
                    float angles[3];
                    vectoangles(client->damage_from, angles);
                    client->ps.event.damagePitch = (int)(angles[0] * 0.71111113f);
                    v7 = (int)(angles[1] * 0.71111113f);
                }
                client->ps.event.damageYaw = v7;
                if (level.time > player->client->pain_debounce_time && (player->flags & 1) == 0)
                {
                    int v8 = (int)(((float)client->ps.stats[0] / client->ps.stats[2]) * 100.0f);
                    if (v8 >= 0)
                    {
                        if (v8 > 100)
                            v8 = 100;
                    }
                    else
                    {
                        v8 = 0;
                    }
                    G_AddEvent(player, 216, v8);
                    player->client->pain_debounce_time = level.time + 700;
                }
                int v9 = client->ps.event.damageEvent + 1;
                client->ps.event.damageCount = v5;
                client->ps.event.damageEvent = v9;
                client->damage_blood = 0;
            }
        }
    }
}

// ea: 0x00455A40
void BodySink(Entity* ent)
{
    if (level.time - ent->timestamp <= 6500)
    {
        float v1 = ent->s.pos.trBase[2] - 1.0f;
        ent->nextthink = level.time + 1;
        ent->s.pos.trBase[2] = v1;
    }
    else
    {
        SV_UnlinkEntity(ent);
        ent->physicsObject = 0;
    }
}

// ea: 0x00455A90
void G_UpdateHeadHitEnt(Entity* pSelf)
{
    Client* client = pSelf->client;
    float vOrg[3];
    vOrg[0] = pSelf->r.currentOrigin.v.m128_f32[0];
    vOrg[1] = pSelf->r.currentOrigin.v.m128_f32[1];
    vOrg[2] = pSelf->r.currentOrigin.v.m128_f32[2];
    vOrg[2] = client->ps.viewHeightCurrent + vOrg[2];
    G_AddLean(pSelf, vOrg);
    Entity* pHitHitEnt = pSelf->client->pHitHitEnt;
    if (pHitHitEnt == nullptr)
    {
        pSelf->client->pHitHitEnt = G_Spawn(PAK_ID_INVALID);
        pHitHitEnt = pSelf->client->pHitHitEnt;
        pHitHitEnt->r.mins.v.m128_f32[0] = -8.0f;
        pHitHitEnt->r.mins.v.m128_f32[1] = -8.0f;
        pHitHitEnt->r.mins.v.m128_f32[2] = -8.0f;
        pHitHitEnt->r.maxs.v.m128_f32[0] = 8.0f;
        pHitHitEnt->r.maxs.v.m128_f32[1] = 8.0f;
        pHitHitEnt->r.maxs.v.m128_f32[2] = 8.0f;
        pHitHitEnt->r.contents = 8320;
        pHitHitEnt->r.mOwner.mHandle.mVal = pSelf->mHandle.mHandle.mVal;
        pHitHitEnt->pain = 2;
        pHitHitEnt->die = 2;
        pHitHitEnt->health = 100000;
        pHitHitEnt->takedamage = 1;
    }
    G_SetOrigin(pHitHitEnt, vOrg);
    g_LinkEntity(pHitHitEnt);
}

// ea: 0x00455BB0
void G_RemoveHeadHitEnt(Entity* pSelf)
{
    Entity* pHitHitEnt = pSelf->client->pHitHitEnt;
    if (pHitHitEnt != nullptr)
    {
        pHitHitEnt->takedamage = 0;
        pHitHitEnt->r.contents = 0;
        pHitHitEnt->r.mOwner.mHandle.mVal = 0;
        pHitHitEnt->nextthink = level.time + 1;
        pHitHitEnt->think = THINK__G_FreeEntity;
        pSelf->client->pHitHitEnt = nullptr;
    }
}

// ea: 0x00456330
void ParseHitLocDmgTableEntry(const char* name, const ConfigString* cfgstr)
{
    int v2 = 0;
    const hitLoc* v3 = g_hitLocs;
    cspField_t hitLocDmgFields[19];
    do
    {
        const char* mName = v3->mName;
        g_fHitLocDamageMult[v2] = 1.0f;
        hitLocDmgFields[v2].szName = mName;
        hitLocDmgFields[v2].iOffset = v2 * 4;
        hitLocDmgFields[v2].iFieldType = 6;
        g_HitLocConstNames[v2] = HashString::CalcHash(mName);
        ++v3;
        ++v2;
    } while (v2 < 19);
    dword_EA53C8 = 0;
    if (ParseConfigStringToStruct((unsigned char*)g_fHitLocDamageMult, hitLocDmgFields,
                                  19, cfgstr, 0, nullptr, G_HitLocStrcpy) == 0)
        G_Error("Error parsing hitloc damage table %s\n", name);
}

// ea: 0x004563E0
void G_ParseHitLocDmgTable()
{
    ConfigStringManager* v0 = ConfigStringManager_sInst;
    TPakId v1 = CurPakId();
    v0->CallbackSearch(v1, "MPLOCDMGTABLE", ParseHitLocDmgTableEntry);
}

// ea: 0x00456B80
void Corpse_Die(Entity* pSelf, Entity* pInflictor, Entity* pAttacker,
                int iDamage, int iMod, int iWeapon,
                const float* vPosition, const float* vDir, hitLocation_t hitLoc)
{
    SentientApplyPhysicsDamage(pSelf, pInflictor, iDamage, iMod, vPosition, vDir, hitLoc, iWeapon);
}

// ea: 0x00459800
void G_MissileTrace(trace_t* results, const math::Position3* start,
                    const math::Position3* end,
                    DbLinkedHandle<EntityHandleDb, Entity> passEntity,
                    int contentmask, unsigned char* priorityMap)
{
    collision_context_t v7;
    v7.pass_entity1 = passEntity;
    v7.pass_entity2.mHandle.mVal = 0;
    v7.contentmask = contentmask;
    v7.__vftable = nullptr;
    g_LocationalTrace(results, start, end, &v7, priorityMap, 0.0f);
    if (results->startsolid != 0)
    {
        if ((results->contents & 0x800) != 0)
        {
            results->startsolid = 0;
            results->fraction = 1.0f;
        }
        else
        {
            results->fraction = 0.0f;
            results->normal.v = _mm_sub_ps(start->v, end->v);
        }
    }
}

// ea: 0x0048B250
void HeadHitEnt_Pain(Entity* pSelf, Entity* pAttacker, int iDamage,
                     const float* vPoint, int iMod, const float* vDir,
                     hitLocation_t hitLoc)
{
    unsigned int mVal = pSelf->r.mOwner.mHandle.mVal;
    pSelf->health = 100000;
    unsigned int v8 = mVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v8 < 0x540 && mVal >> 12 == EntityHandleDb::sInst.mElements[v8].mKey)
        mObject = EntityHandleDb::sInst.mElements[v8].mObject;
    if (mObject->takedamage != 0)
        G_Damage(mObject, pAttacker, pAttacker, vDir, vPoint, iDamage, 0, iMod, hitLoc, -1);
}

// ea: 0x0048B2C0
void HeadHitEnt_Die(Entity* self, Entity* inflictor, Entity* attacker,
                    int damage, int meansOfDeath, int iWeapon,
                    const float* position, const float* vDir, hitLocation_t hitLoc)
{
    HeadHitEnt_Pain(self, attacker, damage, position, meansOfDeath, vDir, hitLoc);
}

// ea: 0x00469360
int G_BounceMissile(Entity* ent, trace_t* trace)
{
    int v19 = SV_PointContents(ent->r.currentOrigin, *new collision_context_t(32));
    float vDelta[3];
    BG_EvaluateTrajectoryDelta(&ent->s.pos,
                               level.previousTime + (int)((level.time - level.previousTime) * trace->fraction),
                               vDelta);
    float v3 = vDelta[2];
    float v4 = vDelta[1];
    float v5 = (((trace->normal.v.m128_f32[0] * vDelta[0]) + (trace->normal.v.m128_f32[2] * vDelta[2]))
                + (trace->normal.v.m128_f32[1] * vDelta[1]))
               * -2.0f;
    ent->s.pos.trDelta[0] = (trace->normal.v.m128_f32[0] * v5) + vDelta[0];
    ent->s.pos.trDelta[1] = (v5 * trace->normal.v.m128_f32[1]) + v4;
    ent->s.pos.trDelta[2] = (v5 * trace->normal.v.m128_f32[2]) + v3;
    if (trace->normal.v.m128_f32[2] > 0.7f)
        ent->s.mGroundEntity = trace->mEntity;
    if ((ent->s.eFlags & 0x2000000) != 0)
    {
        float v6;
        if (v19 != 0 || (trace->contents & 0x2000000) != 0)
            v6 = 0.125f;
        else
            v6 = 0.5f;
        ent->s.pos.trDelta[0] = ent->s.pos.trDelta[0] * v6;
        ent->s.pos.trDelta[1] = ent->s.pos.trDelta[1] * v6;
        ent->s.pos.trDelta[2] = ent->s.pos.trDelta[2] * v6;
        if (trace->normal.v.m128_f32[2] > 0.7f
            && sqrt(ent->s.pos.trDelta[2] * ent->s.pos.trDelta[2]
                    + ent->s.pos.trDelta[1] * ent->s.pos.trDelta[1]
                    + ent->s.pos.trDelta[0] * ent->s.pos.trDelta[0]) < 20.0f)
        {
            G_SetOrigin(ent, &ent->r.currentOrigin);
            math::Position3 vAngles;
            G_MissileLandAngles(ent, trace, &vAngles, 1);
            G_SetAngle(ent, &vAngles);
            return 0;
        }
    }
    float off[3];
    off[0] = trace->normal.v.m128_f32[0] * 0.1f;
    off[1] = trace->normal.v.m128_f32[1] * 0.1f;
    off[2] = trace->normal.v.m128_f32[2] * 0.1f;
    if (off[2] > 0.0f)
        off[2] = 0.0f;
    ent->r.currentOrigin.v.m128_f32[0] = off[0] + ent->r.currentOrigin.v.m128_f32[0];
    ent->r.currentOrigin.v.m128_f32[1] = ent->r.currentOrigin.v.m128_f32[1] + off[1];
    ent->r.currentOrigin.v.m128_f32[2] = off[2] + ent->r.currentOrigin.v.m128_f32[2];
    memcpy(ent->s.pos.trBase, &ent->r.currentOrigin, sizeof(ent->s.pos.trBase));
    ent->s.pos.trTime = level.time;
    math::Position3 vAngles;
    G_MissileLandAngles(ent, trace, &vAngles, 0);
    ent->s.apos.trBase[0] = vAngles.v.m128_f32[0];
    ent->s.apos.trBase[1] = vAngles.v.m128_f32[1];
    ent->s.apos.trBase[2] = vAngles.v.m128_f32[2];
    ent->s.apos.trTime = level.time;
    unsigned int v8 = trace->mEntity.mHandle.mVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v8 < 0x540 && trace->mEntity.mHandle.mVal >> 12 == EntityHandleDb::sInst.mElements[v8].mKey)
        mObject = EntityHandleDb::sInst.mElements[v8].mObject;
    j_nullsub_64(ent, mObject);
    if (v19 != 0)
        return 0;
    float v10 = ent->s.pos.trDelta[2] - vDelta[2];
    float v11 = ent->s.pos.trDelta[1] - vDelta[1];
    float v12 = v10 * v10 + v11 * v11;
    float v13 = ent->s.pos.trDelta[0] - vDelta[0];
    return sqrt(v12 + v13 * v13) > 100.0f;
}
