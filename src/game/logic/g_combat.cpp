// ============================================================================
// g_combat.cpp - damage/combat helpers (g.o: g_combat.cpp family, leaf subset)
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <new>
#include <stdlib.h>
#include <string.h>

// Minimal view of RumbleManager (full class in core/core_systems.h).
class RumbleEffectInstanceHandle {
public:
    int mVal;  // +0x00
};
class RumbleManager {
public:
    static RumbleManager* Inst(int instance);  // ?Inst@RumbleManager@@SAPAV1@H@Z (g.o)
    RumbleEffectInstanceHandle Play(const RumbleEffect& effect,
                                    float intensity);
};


extern "C" int __fpclass(float);

// ============================================================================
// collision_context_t ctors - ea: 0x4AEFE0 / 0x4AF030 (collision_context.h)
// ============================================================================
// ea: 0x004AEFE0
collision_context_t::collision_context_t()
{
    this->pass_entity1.mHandle.mVal = 0;
    this->pass_entity2.mHandle.mVal = 0;
    this->pass_owner1.mHandle.mVal = 0;
    this->pass_owner2.mHandle.mVal = 0;
    this->contentmask = -1;
}

// ea: 0x004AF0C0
collision_context_t::collision_context_t(int mask)
{
    this->pass_entity1.mHandle.mVal = 0;
    this->pass_entity2.mHandle.mVal = 0;
    this->pass_owner1.mHandle.mVal = 0;
    this->pass_owner2.mHandle.mVal = 0;
    this->contentmask = mask;
}

// ea: 0x004AF030
collision_context_t::collision_context_t(
    DbLinkedHandle<EntityHandleDb, Entity> handle, int mask)
{
    this->pass_entity1.mHandle.mVal = 0;
    this->pass_entity2.mHandle.mVal = 0;
    this->pass_owner1.mHandle.mVal = 0;
    this->pass_owner2.mHandle.mVal = 0;
    this->pass_entity1 = handle;
    this->contentmask = mask;
}

// ea: 0x004AF070
collision_context_t::collision_context_t(
    DbLinkedHandle<EntityHandleDb, Entity> handle1,
    DbLinkedHandle<EntityHandleDb, Entity> handle2, int mask)
{
    this->pass_entity1.mHandle.mVal = 0;
    this->pass_entity2.mHandle.mVal = 0;
    this->pass_owner1.mHandle.mVal = 0;
    this->pass_owner2.mHandle.mVal = 0;
    this->pass_entity1 = handle1;
    this->pass_entity2 = handle2;
    this->contentmask = mask;
}

// Initialize a collision context supplied by a translation unit that only has
// the verified 24-byte layout view.  This preserves the real host vtable.
extern "C" void CollisionContext_Init(void* storage)
{
    new (storage) collision_context_t();
}

// ea: 0x0044AF10
void handleDeathInvulnerability(Entity* /*ent*/, int /*a2*/, int /*a3*/)
{
    ;
}

// ea: 0x00483560
void Cmd_Kill_f(Entity* ent)
{
    if (g_reloading.integer == 0)
    {
        ent->flags &= ~1u;
        ent->health = 0;
        ent->client->ps.stats[0] = 0;
        player_die(ent, ent, ent, dword_186A0, 25, 0, nullptr, nullptr, HITLOC_NONE);
    }
}

// ea: 0x004814A0
void Bullet_Fire_Fake(Entity* attacker, float spread, int damage,
                      weaponParms* wp, Entity* weaponEnt, float coneAngleTangent)
{
    float end[3];
    Bullet_Endpos(spread, end, wp);
    Entity* mWorld = weaponEnt;
    if (weaponEnt == nullptr)
        mWorld = EntityManager::sInst->mWorld;
    Bullet_Fire_Fake_Extended(mWorld->mHandle, attacker, wp->muzzleTrace, end,
                              damage, 0, wp, mWorld->mHandle, coneAngleTangent);
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
void G_MissileLandAngles(Entity* ent, trace_t* trace, math::Position3& vAngles,
                         int bForceAlign)
{
    int v7 = level.previousTime + (int)((level.time - level.previousTime) * trace->fraction);
    BG_EvaluateTrajectory(&ent->s.apos, v7, vAngles);
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
        float fSurfacePitch = PitchForYawOnNormal(vAngles.v.m128_f32[1], trace->normal.v.m128_f32);
        float fAngleDelta = AngleSubtract(fSurfacePitch, vAngles.v.m128_f32[0]);
        float fAbsAngDelta = (float)fabs(fAngleDelta);
        if (bForceAlign == 0)
        {
            ent->s.apos.trBase[0] = vAngles.v.m128_f32[0];
            ent->s.apos.trBase[1] = vAngles.v.m128_f32[1];
            ent->s.apos.trBase[2] = vAngles.v.m128_f32[2];
            ent->s.apos.trTime = v7;
            float v8;
            if (fAbsAngDelta >= 80.0f)
                v8 = ((float)rand() * 0.0000091552738f + 0.85000002f) * ent->s.apos.trDelta[0];
            else
                v8 = (((float)rand() * 0.0000091552738f + 0.85000002f) * ent->s.apos.trDelta[0]) * -1.0f;
            ent->s.apos.trDelta[0] = v8;
        }
        float tracea = AngleNormalize180(vAngles.v.m128_f32[0]);
        vAngles.v.m128_f32[0] = tracea;
        if (bForceAlign != 0 || fAbsAngDelta < 45.0f)
        {
            if ((float)fabs(tracea) <= 90.0f)
                vAngles.v.m128_f32[0] = AngleNormalize360(fSurfacePitch);
            else
                vAngles.v.m128_f32[0] = AngleNormalize360(fSurfacePitch + 180.0f);
        }
        else if (fAbsAngDelta >= 80.0f)
        {
            vAngles.v.m128_f32[0] = AngleNormalize360(tracea);
        }
        else
        {
            vAngles.v.m128_f32[0] = AngleNormalize360((fAngleDelta * 0.25f) + tracea);
        }
    }
}

// ea: 0x0044C390
void G_LaunchMissile(Entity* ent, int /*unused*/)
{
    ent->nextthink = level.time + BG_GetInfoForWeapon(ent->s.weapon)->iProjectileDelay;
    ent->think = THINK__G_IncomingMissile;
    if (gpBrocAPI->mBrocExports.mCallbackFireArtilleryShell != nullptr)
        gpBrocAPI->mBrocExports.mCallbackFireArtilleryShell(ent->mHandle.mHandle.mVal);
}

// ea: 0x0044C3F0
void G_IncomingMissile(Entity* ent, int /*unused*/)
{
    ent->nextthink = level.time + 1100;
    ent->think = THINK__G_DelayMissile;
    PostEffectEventScriptCall(ent, "SHELL_INCOMING", false, PAK_ID_INVALID, false);
}

// ea: 0x0044C430
void G_DelayMissile(Entity* ent, int /*unused*/)
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
void G_MissileDie(Entity* self, Entity* inflictor, Entity* /*unused2*/,
                  int /*unused3*/, int /*unused4*/)
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

// ea: 0x00483760
void PlayerDead(Entity* self, Entity* inflictor, Entity* attacker, int damage,
                int meansOfDeath, int weapon, const float* position,
                const float* dir, hitLocation_t hitLoc)
{
    Entity* mWorld = inflictor;
    if (inflictor == nullptr)
    {
        mWorld = attacker;
        if (attacker == nullptr)
            mWorld = EntityManager::sInst->mWorld;
    }
    Entity* v10 = self;
    self->client->ps.pm_type = 7 - (self->client->ps.pm_type != 1);
    if (v10->health > 0)
        v10->health = 0;
    if (v10->health < -999)
        v10->health = -999;
    v10->client->ps.stats[0] = v10->health;
    if (v10->client->ps.grenadeTimeLeft != 0)
    {
        float launchvel[3];
        launchvel[0] = (rand() * 0.000061035156f - 1.0f) * 160.0f;
        launchvel[1] = (rand() * 0.000061035156f - 1.0f) * 160.0f;
        launchvel[2] = (rand() * 0.000030517578f) * 160.0f;
        float vOrigin[3];
        vOrigin[0] = v10->r.currentOrigin.v.m128_f32[0];
        vOrigin[1] = v10->r.currentOrigin.v.m128_f32[1];
        vOrigin[2] = v10->r.currentOrigin.v.m128_f32[2] + 40.0f;
        fire_grenade(v10, vOrigin, launchvel, v10->s.weapon,
                     v10->client->ps.grenadeTimeLeft);
        v10->s.weapon = 0;
        v10->client->ps.weapon = 0;
    }
    v10->client->ps.viewangles[0] = v10->r.currentAngles.v.m128_f32[0];
    v10->client->ps.viewangles[1] = v10->r.currentAngles.v.m128_f32[1];
    v10->client->ps.viewangles[2] = v10->r.currentAngles.v.m128_f32[2];
    int oldWeapon = v10->s.weapon;
    v10->takedamage = 1;
    v10->r.contents = 0x4000000;
    v10->s.weapon = 0;
    v10->client->ps.weapon = 0;
    v10->client->mVehicleAnimMoving = false;
    v10->r.currentAngles.v.m128_f32[0] = 0.0f;
    v10->r.currentAngles.v.m128_f32[2] = 0.0f;
    v10->s.loopSound = 0;
    v10->r.maxs.v.m128_f32[2] = 16.0f;
    if (v10->r.mins.v.m128_f32[2] > 16.0f)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_combat.cpp";
        AeAssert::gCurrentLine = 638;
        AeAssert::gCurrentExpr = "self->r.maxs[2] >= self->r.mins[2]";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    v10->client->respawnTime = level.time + 1700;
    v10->die = 0;
    G_DObjUpdate(v10, false);
    if (v10->mDObj != nullptr)
        G_DObjCalcPose(v10);
    SentientApplyPhysicsDamage(v10, mWorld, damage, meansOfDeath, position,
                               dir, hitLoc, weapon);
    v10->s.weapon = oldWeapon;
    v10->client->ps.weapon = oldWeapon;
    if (EntityManager::sInst->IsLocalPlayer(v10))
        g_femanager.mDontDrawHud = false;
    if (EntityManager::sInst->IsLocalPlayer(v10))
    {
        int mServerClientIndex = v10->client->mServerClientIndex;
        float vOrigin[3];
        Sentient_GetOrigin(v10->sentient, vOrigin);
        if (meansOfDeath == 25)
        {
            if (oldWeapon != 0)
                goto shock_branch_a;
        }
        else if (meansOfDeath == 3 || meansOfDeath == 4 || meansOfDeath == 7
                 || meansOfDeath == 8 || meansOfDeath == 9
                 || meansOfDeath == 10 || meansOfDeath == 5
                 || meansOfDeath == 6 || meansOfDeath == 17
                 || meansOfDeath == 18 || meansOfDeath == 27)
        {
        shock_branch_a:
            if ((0x100000 & v10->client->ps.eFlags) == 0
                || v10->client->ps.vehType != 2
                || v10->client->ps.vehPos == 0)
            {
                Broc::string shock("default");
                gpBrocAPI->mBrocExports.mShellShock(
                    v10->mHandle.mHandle.mVal, shock, 3.0f);
                CG_StartShakeCamera(1.0f, 800, vOrigin, 2000.0f,
                                    mServerClientIndex);
                dword_F64018[1580 * mServerClientIndex] =
                    (int)(cgGlobal.time + cg_redFlashTime.value);
            }
            void* rumbleMgr = RumbleManager::Inst(mServerClientIndex);
            if (rumbleMgr != nullptr)
            {
                RumbleEffect effect;
                effect.mRumbleDataArray[0].enabled = true;
                effect.mRumbleDataArray[0].delay = 0.0f;
                effect.mRumbleDataArray[0].intensity = 1.0f;
                effect.mRumbleDataArray[0].ramp_up_duration = 0.0f;
                effect.mRumbleDataArray[0].steady_duration = 1.0f;
                effect.mRumbleDataArray[0].ramp_down_duration = 0.0f;
                effect.mRumbleDataArray[1].enabled = true;
                effect.mRumbleDataArray[1].delay = 0.1f;
                effect.mRumbleDataArray[1].intensity = 1.0f;
                effect.mRumbleDataArray[1].ramp_up_duration = 0.5f;
                effect.mRumbleDataArray[1].steady_duration = 0.5f;
                effect.mRumbleDataArray[1].ramp_down_duration = 0.2f;
                ((RumbleManager*)rumbleMgr)->Play(effect, 1.0f);
            }
        }
        else
        {
            if ((0x100000 & v10->client->ps.eFlags) == 0
                || v10->client->ps.vehType != 2
                || v10->client->ps.vehPos == 0)
            {
                Broc::string shock("default");
                gpBrocAPI->mBrocExports.mShellShock(
                    v10->mHandle.mHandle.mVal, shock, 2.0f);
                CG_StartShakeCamera(1.0f, 800, vOrigin, 2000.0f,
                                    mServerClientIndex);
                dword_F64018[1580 * mServerClientIndex] =
                    (int)(cgGlobal.time + cg_redFlashTime.value);
            }
            void* rumbleMgr = RumbleManager::Inst(mServerClientIndex);
            if (rumbleMgr != nullptr)
            {
                RumbleEffect effect;
                effect.mRumbleDataArray[0].enabled = true;
                RumbleEffect_SetIntensity(&effect, kRumbleLEFT, 0.7f);
                effect.mRumbleDataArray[0].steady_duration = 0.2f;
                effect.mRumbleDataArray[0].delay = 0.0f;
                effect.mRumbleDataArray[1].enabled = true;
                RumbleEffect_SetIntensity(&effect, kRumbleRIGHT, 0.7f);
                effect.mRumbleDataArray[1].steady_duration = 0.2f;
                effect.mRumbleDataArray[1].delay = 0.0f;
                effect.mRumbleDataArray[1].ramp_up_duration = 0.2f;
                effect.mRumbleDataArray[1].ramp_down_duration = 0.2f;
                ((RumbleManager*)rumbleMgr)->Play(effect, 1.0f);
            }
        }
    }
    g_LinkEntity(v10);
    Broc::string msg("INGAME_PLAYER_DIED");
    gpBrocAPI->mBrocExports.mMissionFailed(&msg);
}

// ea: 0x004598D0
int G_PredictMissile(const Entity* ent, int duration, float* endPos,
                     int allowBounce, int* timeAtRest)
{
    Entity backupEnt(PAK_ID_INVALID);
    trajectory_t pos;
    memcpy(&pos, &ent->s.pos, sizeof(pos));
    math::Position3 origin;
    BG_EvaluateTrajectory(&pos, level.time, origin);
    *timeAtRest = ent->nextthink;
    int i;
    for (i = level.time + 100; i < duration + level.time; i += 100)
    {
        math::Position3 end;
        BG_EvaluateTrajectory(&pos, i, end);
        trace_t trace;
        G_MissileTrace(&trace, origin, end, ent->r.mOwner, ent->clipmask,
                       bulletPriorityMap);
        origin.v = trace.endpos.v;
        if (trace.allsolid != 0)
            return 0;
        float fraction = trace.fraction;
        if (fraction == 1.0f
            || (fraction < 1.0f && trace.normal.v.m128_f32[2] > 0.7f))
        {
            math::Position3 end2;
            end2.v.m128_f32[0] = origin.v.m128_f32[0];
            end2.v.m128_f32[1] = origin.v.m128_f32[1];
            end2.v.m128_f32[2] = origin.v.m128_f32[2] - 1.5f;
            G_MissileTrace(&trace, origin, end2, ent->r.mOwner, ent->clipmask,
                           bulletPriorityMap);
            fraction = trace.fraction;
            if (fraction != 1.0f)
            {
                pos.trBase[2] += (trace.endpos.v.m128_f32[2] + 1.5f)
                                 - origin.v.m128_f32[2];
                origin.v.m128_f32[0] = trace.endpos.v.m128_f32[0];
                origin.v.m128_f32[1] = trace.endpos.v.m128_f32[1];
                origin.v.m128_f32[2] = trace.endpos.v.m128_f32[2] + 1.5f;
            }
        }
        if ((trace.surfaceFlags & 0x10) != 0)
            return 0;
        if (allowBounce != 0 && (ent->s.eFlags & 0x3000000) != 0)
        {
            G_PredictBounceMissile(ent, &pos, &trace,
                                   i - (int)(fraction * -100.0f) - 100);
            pos.trTime = i;
            if (pos.trType != 0)
                continue;
        }
        *timeAtRest = i;
        break;
    }
    endPos[0] = origin.v.m128_f32[0];
    endPos[1] = origin.v.m128_f32[1];
    endPos[2] = origin.v.m128_f32[2];
    if (allowBounce != 0 && (ent->s.eFlags & 0x3000000) != 0)
        return ent->nextthink;
    return i;
}

// ea: 0x0044C8C0
void Static_Pain(Entity* ent, Entity* /*unused1*/, int /*unused2*/,
                 const float* const /*unused3*/, int /*unused4*/,
                 const float* const /*unused5*/, hitLocation_t /*unused6*/)
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
bool Bullet_ShouldGoThroughFriend(const Entity* /*attacker*/,
                                  const Entity* /*victim*/)
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
void BodySink(Entity* ent, int /*unused*/)
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

// G_HitLocStrcpy - ea: 0x0044AF00 (release no-op export)
void G_HitLocStrcpy(unsigned char* out, const char* in)
{
    (void)out;
    (void)in;
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
    ConfigStringManager* v0 = ConfigStringManager::sInst;
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
void G_MissileTrace(trace_t* results, const math::Position3& start,
                    const math::Position3& end,
                    DbLinkedHandle<EntityHandleDb, Entity> passEntity,
                    int contentmask, unsigned char* priorityMap)
{
    collision_context_t v7;
    v7.pass_entity1 = passEntity;
    v7.pass_entity2.mHandle.mVal = 0;
    v7.contentmask = contentmask;
    g_LocationalTrace(results, start, end, v7, priorityMap, 0.0f);
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
            results->normal.v = _mm_sub_ps(start.v, end.v);
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
            G_SetOrigin(ent, ent->r.currentOrigin);
            math::Position3 vAngles;
            G_MissileLandAngles(ent, trace, vAngles, 1);
            G_SetAngle(ent, vAngles);
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
    G_MissileLandAngles(ent, trace, vAngles, 0);
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

// ea: 0x00456BB0
int CanDamage(Entity* targ, const float* origin, Entity* inflictor)
{
    float v30 = 15.0f;
    float dest[5][3];
    float halfHeight;
    float v11;
    if (inflictor == nullptr || inflictor->tagInfo == nullptr
        || inflictor->tagInfo->parent != targ)
    {
        sentient_s* sentient = targ->sentient;
        if (sentient != nullptr)
        {
            if (targ->client != nullptr)
                v30 = 8.0f;
            Sentient_GetEyePosition(sentient, dest[4]);
            float v24 = (dest[4][2] - targ->r.currentOrigin.v.m128_f32[2]) * 0.5f;
            float traceEnd[4];
            traceEnd[1] = origin[0] - targ->r.currentOrigin.v.m128_f32[0];
            traceEnd[2] = origin[1] - targ->r.currentOrigin.v.m128_f32[1];
            traceEnd[3] = 0.0f;
            VectorNormalize(&traceEnd[1]);
            float v6 = dest[4][2] + targ->r.currentOrigin.v.m128_f32[2];
            float v7 = (targ->r.currentOrigin.v.m128_f32[0] + dest[4][0]) * 0.5f;
            float v26 = (targ->r.currentOrigin.v.m128_f32[1] + dest[4][1]) * 0.5f;
            float v8 = v6 * 0.5f;
            float v9 = (traceEnd[3] * v30) + v8;
            halfHeight = v8;
            dest[1][2] = v9 - v24;
            dest[0][2] = v9 + v24;
            dest[0][0] = (-traceEnd[2] * v30) + v7;
            dest[0][1] = (traceEnd[1] * v30) + v26;
            dest[1][0] = dest[0][0];
            dest[1][1] = dest[0][1];
            float v10 = (-v30 * traceEnd[3]) + v8;
            v11 = (-v30 * traceEnd[1]) + v26;
            dest[2][0] = (-v30 * -traceEnd[2]) + v7;
            dest[2][2] = v10 + v24;
            dest[3][0] = dest[2][0];
            dest[3][1] = v11;
            dest[3][2] = v10 - v24;
        }
        else
        {
            float v12 = (targ->r.absmax.v.m128_f32[2] + targ->r.absmin.v.m128_f32[2]) * 0.5f;
            float v13 = (targ->r.absmax.v.m128_f32[1] + targ->r.absmin.v.m128_f32[1]) * 0.5f;
            float v25 = (targ->r.absmax.v.m128_f32[0] + targ->r.absmin.v.m128_f32[0]) * 0.5f;
            halfHeight = v12;
            dest[0][2] = v12;
            v11 = v13 + 15.0f;
            dest[1][2] = v12;
            dest[2][2] = v12;
            dest[3][2] = v12;
            dest[0][0] = v25 + 15.0f;
            dest[0][1] = v13 + 15.0f;
            dest[1][0] = v25 + 15.0f;
            dest[1][1] = v13 - 15.0f;
            dest[2][0] = v25 - 15.0f;
            dest[3][0] = v25 - 15.0f;
            dest[3][1] = v13 - 15.0f;
        }
        dest[2][1] = v11;
        collision_context_t context;
        context.pass_entity1.mHandle.mVal = targ->mHandle.mHandle.mVal;
        context.pass_entity2.mHandle.mVal = 0;
        context.pass_owner1.mHandle.mVal = 0;
        context.pass_owner2.mHandle.mVal = 0;
        context.contentmask = 41951377;
        math::Position3 zeroMins;
        math::Position3 zeroMaxs;
        zeroMins.v = _mm_setzero_ps();
        zeroMaxs.v = _mm_setzero_ps();
        for (int i = 0; i < 5; ++i)
        {
            math::Position3 start;
            start.v.m128_f32[0] = dest[i][0];
            start.v.m128_f32[1] = dest[i][1];
            start.v.m128_f32[2] = dest[i][2];
            math::Position3 end;
            end.v.m128_f32[0] = origin[0];
            end.v.m128_f32[1] = origin[1];
            end.v.m128_f32[2] = origin[2];
            trace_t trace;
            SV_Trace(&trace, &start, &zeroMins, &zeroMaxs, &end, &context, 0, 1,
                     bulletPriorityMap, 1, 0.0f);
            dest[4][0] = trace.endpos.v.m128_f32[0];
            dest[4][1] = trace.endpos.v.m128_f32[1];
            dest[4][2] = trace.endpos.v.m128_f32[2];
            if (trace.fraction == 1.0f || VectorDistance(origin, dest[4]) < 2.0f)
                break;
            if (i + 1 >= 5)
                return 0;
        }
    }
    return 1;
}

// CollisionDesc layout twin (core_systems.h cannot be included here)
struct LocalCollisionDesc {
    math::Position3 coord;   // +0x00
    math::Position3 normal;  // +0x10
    int             material; // +0x20
};

// ea: 0x00456520
void SentientApplyPhysicsDamage(Entity* pSelf, Entity* pInflictor, int iDamage,
                                int iMod, const float* vPosition,
                                const float* vDir, hitLocation_t hitLoc,
                                int iWeapon)
{
    float force = ((0x400000 & pSelf->flags) != 0) ? 0.5f : 1.0f;
    switch (iMod)
    {
    case 3: case 4: case 5: case 6: case 9: case 10: case 11:
    case 17: case 18: case 20: case 32:
    {
        float vdir[4];
        vdir[0] = vDir[0];
        vdir[1] = vDir[1];
        vdir[2] = vDir[2];
        vdir[3] = 0.0f;
        float v12;
        if (vdir[2] >= 0.2f)
        {
            if (vdir[2] <= 0.64999998f)
                goto skip_updir;
            vdir[0] += (vdir[0] >= 0.0f ? 1 : -1) * 0.2f;
            vdir[1] += (vdir[1] >= 0.0f ? 1 : -1) * 0.2f;
            v12 = vdir[2] * 0.89999998f;
        }
        else
        {
            v12 = vdir[2] + 0.5f;
        }
        vdir[2] = v12;
skip_updir:
        float v11;
        if (iMod == 11)
        {
            float v15 = iDamage * 2.2f;
            v11 = 70.0f;
            if (v15 >= 70.0f)
            {
                v11 = 150.0f;
                if (v15 <= v11)
                    v11 = v15;
            }
        }
        else if (iMod != 32 && iMod != 20)
        {
            if (iMod == 3 || iMod == 4 || iMod == 7 || iMod == 8 || iMod == 9
                || iMod == 10 || iMod == 5 || iMod == 6 || iMod == 17
                || iMod == 18 || iMod == 27)
            {
                float v15 = iDamage * 1.4f;
                v11 = 100.0f;
                if (v15 < 100.0f)
                    goto done_cap;
                v11 = 220.0f;
                if (v15 <= v11)
                    v11 = v15;
            }
            else
            {
                float v15 = iDamage * 1.4f;
                v11 = 80.0f;
                if (v15 < 80.0f)
                    goto done_cap;
                v11 = 170.0f;
                if (v15 <= v11)
                    v11 = v15;
            }
        }
        else
        {
            v11 = 0.0f;
        }
done_cap:
        force = v11 * force;
        math::Position3 hitp;
        hitp.v = _mm_setzero_ps();
        math::Dir3 hitd;
        hitd.v.m128_f32[0] = vdir[0];
        hitd.v.m128_f32[1] = vdir[1];
        hitd.v.m128_f32[2] = vdir[2];
        hitd.v.m128_f32[3] = 0.0f;
        ApplyPhysics(pSelf, &hitp, &hitd, force, false, HITLOC_TORSO_UPR);
        if ((vdir[2] * force) > 100.0f)
        {
            float v16 = vdir[0] * force;
            float v17 = vdir[1] * force;
            float v18 = vdir[2] * force;
            if (sqrt(v16 * v16 + v17 * v17 + v18 * v18) > 180.0f)
                PostEffectEventScriptCall(pSelf, "PLAYER_DEATH_FLYING", false,
                                          PAK_ID_INVALID, false);
        }
        return;
    }
    case 25:
    {
        math::Position3 hitp;
        hitp.v = pSelf->r.currentOrigin.v;
        float vdir[4];
        vdir[0] = vDir[0];
        vdir[1] = vDir[1];
        vdir[2] = vDir[2];
        vdir[3] = 0.0f;
        weaponFileInfo_t* info = BG_GetInfoForWeapon(iWeapon);
        float v27;
        if (info->weapClass == 5 /* WEAPCLASS_GRENADE */)
        {
            math::Position3 pos;
            pos.v = pSelf->r.currentOrigin.v;
            LocalCollisionDesc cd;
            cd.coord.v = pos.v;
            cd.normal.v = _mm_setzero_ps();
            cd.normal.v.m128_f32[2] = 1.0f;
            cd.material = (int)pSelf->s.surfType;
            PostEffectEventProjExplode(pSelf, "fraggrenade",
                                       (const CollisionDesc*)&cd);
            v27 = 100.0f;
            AnglesToForward(pSelf->r.currentAngles.v.m128_f32, vdir);
            vdir[0] = -vdir[0];
            vdir[1] = -vdir[1];
            vdir[2] = -vdir[2];
            *(unsigned int*)&vdir[3] = 0x40000000;
        }
        else
        {
            v27 = (float)iDamage;
            if (iDamage == 0)
            {
                AnglesToForward(pSelf->r.currentAngles.v.m128_f32, vdir);
                vdir[3] = 1.0f;
            }
        }
        float fforce;
        if (v27 >= 0.0f)
        {
            fforce = 100.0f;
            if (v27 <= 100.0f)
                fforce = v27;
        }
        else
        {
            fforce = 0.0f;
        }
        math::Dir3 hitd;
        hitd.v.m128_f32[0] = vdir[0];
        hitd.v.m128_f32[1] = vdir[1];
        hitd.v.m128_f32[2] = vdir[2];
        hitd.v.m128_f32[3] = vdir[3];
        ApplyPhysics(pSelf, &hitp, &hitd, fforce, true, HITLOC_TORSO_UPR);
        return;
    }
    default:
    {
        weaponFileInfo_t* InfoForWeapon = iWeapon > 0 ? BG_GetInfoForWeapon(iWeapon) : nullptr;
        bool v30 = InfoForWeapon != nullptr && InfoForWeapon->weapClass == 17;
        float v31 = vDir[2] >= 0.2f ? 0.1f : 0.40000001f;
        if (v30)
            v31 = v31 + 0.1f;
        float vdir[4];
        vdir[0] = vDir[0];
        vdir[1] = vDir[1];
        vdir[2] = v31 + vDir[2];
        vdir[3] = 0.0f;
        math::Position3 hitp;
        hitp.v.m128_f32[0] = vPosition[0];
        hitp.v.m128_f32[1] = vPosition[1];
        hitp.v.m128_f32[2] = vPosition[2];
        hitp.v.m128_f32[3] = 0.0f;
        if (v30)
        {
            float dx = pSelf->r.currentOrigin.v.m128_f32[0] - pInflictor->r.currentOrigin.v.m128_f32[0];
            float dy = pSelf->r.currentOrigin.v.m128_f32[1] - pInflictor->r.currentOrigin.v.m128_f32[1];
            float dz = pSelf->r.currentOrigin.v.m128_f32[2] - pInflictor->r.currentOrigin.v.m128_f32[2];
            float dist2 = dx * dx + dy * dy + dz * dz;
            float v35 = dist2 > max_dist2 ? max_dist2 : dist2;
            float v36 = (1.0f - (v35 / max_dist2)) * max_intensity;
            float v37 = 30.0f;
            if (v36 >= 30.0f)
            {
                v37 = 110.0f;
                if (v36 <= 110.0f)
                    v37 = v36;
            }
            force = (v37 / InfoForWeapon->iShotCount) * force;
            math::Position3 p1;
            p1.v = _mm_setzero_ps();
            math::Dir3 d;
            d.v.m128_f32[0] = vdir[0];
            d.v.m128_f32[1] = vdir[1];
            d.v.m128_f32[2] = vdir[2];
            d.v.m128_f32[3] = 0.0f;
            ApplyPhysics(pSelf, &p1, &d, force, true, HITLOC_TORSO_LWR);
            ApplyPhysics(pSelf, &p1, &d, force * 0.69999999f, true, hitLoc);
        }
        else
        {
            float v38 = iDamage * 0.5f;
            if (v38 < 20.0f)
                v38 = 20.0f;
            else if (v38 > 30.0f)
                v38 = 30.0f;
            math::Dir3 d;
            d.v.m128_f32[0] = vdir[0];
            d.v.m128_f32[1] = vdir[1];
            d.v.m128_f32[2] = vdir[2];
            d.v.m128_f32[3] = 0.0f;
            ApplyPhysics(pSelf, &hitp, &d, v38 * force, false, hitLoc);
        }
        return;
    }
    }
}
