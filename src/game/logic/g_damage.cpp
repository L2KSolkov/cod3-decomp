// ============================================================================
// g_damage.cpp - core damage pipeline (g.o: g_combat.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

// .rdata/@data constants used by G_Damage (verified via disasm)
int damageForceReductionThreshold;  // 0xDD7F40?
int damageForceMax;                 // 0xDD7F44?

// ea: 0x00475590
void G_FinishDamage(Entity* targ, Entity* inflictor, Entity* attacker,
                    const float* dir, const float* point, int damage, int mod,
                    int weapon, hitLocation_t hitLoc)
{
    int v9 = damage;
    Entity* v10 = targ;
    float localdir[3];
    localdir[0] = dir[0];
    localdir[1] = dir[1];
    localdir[2] = dir[2];
    Client* client = targ->client;
    int take = damage;
    if (client != nullptr)
    {
        client->damage_blood += damage;
        client->damage_from[0] = localdir[0];
        client->damage_from[1] = localdir[1];
        client->damage_from[2] = localdir[2];
    }
    if (v9 != 0)
    {
        int v13 = v10->health - v9;
        Entity* v14 = attacker;
        Entity* v15 = attacker;
        v10->health = v13;
        unsigned int mVal = v15->mHandle.mHandle.mVal;
        Broc::entity e;
        e.___u0 = mVal;
        v10->Notify(hash_const.damage, take, &e, &mod, &hitLoc, nullptr);
        int health = v10->health;
        if (health > 0)
        {
            unsigned char pain = v10->pain;
            if (pain != 0)
            {
                if (pain >= 6u)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_combat.cpp";
                    AeAssert::gCurrentLine = 1834;
                    AeAssert::gCurrentExpr = "targ->pain > 0 && targ->pain < PAIN_MAX";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                v10->rotate.v.m128_f32[0] = localdir[0];
                v10->rotate.v.m128_f32[1] = localdir[1];
                v10->rotate.v.m128_f32[2] = localdir[2];
                v10->pos3.v.m128_f32[0] = point[0];
                v10->pos3.v.m128_f32[1] = point[1];
                v10->pos3.v.m128_f32[2] = point[2];
                paintable[v10->pain](v10, v15, damage, point, mod, localdir, hitLoc);
            }
        }
        else
        {
            if (client != nullptr)
                v10->flags |= 0x20u;
            if (health < -999)
                v10->health = -999;
            if (client == nullptr)
                Scr_NotifyFromEnt(v10, hash_const.death, v15);
            unsigned char die = v10->die;
            v10->enemy = v15;
            if (die != 0)
            {
                if (die >= 8u)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_combat.cpp";
                    AeAssert::gCurrentLine = 1823;
                    AeAssert::gCurrentExpr = "targ->die > 0 && targ->die < DIE_MAX";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                dietable[v10->die](v10, inflictor, v15, damage, mod, weapon,
                                   point, localdir, hitLoc);
            }
        }
        if (v10->client != nullptr)
            v10->client->ps.stats[0] = v10->health;
        if (v10->client != nullptr && v15->client != nullptr && v15 != v10)
        {
            int PlayerIndex = EntityManager::sInst->GetPlayerIndex(v15);
            v10->client->ps.mDamageFromPlayers[PlayerIndex] += damage;
        }
        if (v10->actor != nullptr
            && inflictor != nullptr
            && (hitLoc == 2 || hitLoc == 1)
            && (v10->mFlags & 0x10) == 0
            && rand() % 100 <= 10
            && SpawnHelmet(v10, point, dir, damage) != nullptr)
        {
            v10->mFlags |= 0x10u;
        }
    }
}

// ea: 0x00483E10
void G_Damage(Entity* targ, Entity* inflictor, Entity* attacker,
              const float* dir, const float* point, int damage, int dflags,
              int mod, hitLocation_t hitLoc, int weapon)
{
    int v11 = mod;
    if (targ != nullptr)
    {
        sentient_s* sentient = targ->sentient;
        if (sentient != nullptr)
            sentient->last_damage_mod = mod;
    }
    if (damage >= 9999 && targ->invulnerability_timeout > level.time)
        targ->takedamage = 1;
    if (targ->takedamage != 0 && g_reloading.integer == 0)
    {
        math::Position3 pos;
        math::Dir3 norm;
        pos.v = _mm_setzero_ps();
        norm.v = _mm_setzero_ps();
        norm.v.m128_f32[2] = 1.0f;
        if (point != nullptr)
            pos.v = *(const __m128*)point;
        if (dir != nullptr)
            norm.v = *(const __m128*)dir;
        if (targ->scr_vehicle == nullptr || G_IsVehicleImmune(targ, v11) == 0)
        {
            Client* client = targ->client;
            if (client != nullptr)
            {
                if (client->ps.pm_type >= 6)
                    return;
                if (!IsLocalPlayer(targ))
                {
                    int v14 = 0;
                    Entity* v15;
                    if (weapon <= 0)
                    {
                        v15 = attacker;
                        if (inflictor != nullptr)
                            v14 = inflictor->s.weapon;
                        else if (attacker == nullptr)
                        {
                            MultiplayerMgr::sInst->PlayerDamage(targ, inflictor, pos, norm,
                                                                v14, (float)damage, (unsigned char)mod,
                                                                dflags, hitLoc);
                            return;
                        }
                    }
                    else
                    {
                        v14 = weapon;
                        v15 = attacker;
                    }
                    if (v15 == nullptr)
                    {
                        MultiplayerMgr::sInst->PlayerDamage(targ, inflictor, pos, norm,
                                                            v14, (float)damage, (unsigned char)mod,
                                                            dflags, hitLoc);
                        return;
                    }
                    if (mod == 32)
                    {
                        if (IsLocalPlayer(targ))
                            MultiplayerMgr::sInst->PlayerDamage(targ, attacker, pos, norm,
                                                                v14, (float)damage, 0x20u,
                                                                dflags, hitLoc);
                        return;
                    }
                    if (v15->scr_vehicle != nullptr)
                    {
                        if (HandleDbToEnt(v15->r.mOwner) != nullptr)
                        {
                            Entity* v16 = HandleDbToEnt(v15->r.mOwner);
                            if (v16->IsLocalPlayer())
                            {
                                Entity* v17 = HandleDbToEnt(v15->r.mOwner);
                                MultiplayerMgr::sInst->PlayerDamage(targ, v17, pos, norm,
                                                                    v14, (float)damage, (unsigned char)mod,
                                                                    dflags, hitLoc);
                            }
                        }
                        return;
                    }
                    if (v15 == inflictor && mod != 27 && mod != 26)
                        return;
                    if (v15->client != nullptr)
                    {
                        if (!attacker->IsLocalPlayer() && mod != 5 && mod != 6)
                            return;
                        if (!IsLocalPlayer(targ))
                        {
                            MultiplayerMgr::sInst->PlayerDamage(targ, attacker, pos, norm,
                                                                v14, (float)damage, (unsigned char)mod,
                                                                dflags, hitLoc);
                            return;
                        }
                    }
                }
                if (!IsLocalPlayer(targ)
                    || (G_IsPlayerInVehicle(targ)
                        && IsPlayerFullySeatedInVehicle(targ)
                        && (mod == 4 || mod == 8 || mod == 10 || mod == 6 || mod == 14
                            || mod == 18 || mod == 27 || !G_CanPlayerBeDamagedInVehicle(targ))))
                {
                    return;
                }
            }
            if (targ->client != nullptr)
            {
                hitLocation_t v19 = hitLoc;
                if (hitLoc > HITLOC_GUN)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_combat.cpp";
                    AeAssert::gCurrentLine = 1322;
                    AeAssert::gCurrentExpr = "(hitLoc >= HITLOC_NONE) && (hitLoc < HITLOC_NUM)";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                if ((targ->s.eFlags & 0x1000) != 0 && v19 == HITLOC_HELMET)
                {
                    v19 = HITLOC_HEAD;
                    hitLoc = HITLOC_HEAD;
                }
                damage = (int)(damage * g_fHitLocDamageMult[v19]);
            }
            if (inflictor == nullptr)
                inflictor = EntityManager::sInst->mWorld;
            Entity* mWorld = attacker;
            if (attacker == nullptr)
            {
                mWorld = EntityManager::sInst->mWorld;
                attacker = mWorld;
                if (mWorld == nullptr)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_combat.cpp";
                    AeAssert::gCurrentLine = 1338;
                    AeAssert::gCurrentExpr = "attacker";
                    if (!AeAssert::IsIgnored())
                    {
                        char* v21 = va("entnum: %d", targ->mHandle.mHandle.mVal);
                        if (AeAssert::Assert(v21))
                            __debugbreak();
                    }
                }
            }
            if (targ->s.eType == 4)
            {
                if (targ->use != 0 && targ->moverState == 0)
                {
                    Scr_NotifyFromEnt(targ, hash_const.trigger, mWorld);
                    unsigned char use = targ->use;
                    if (use == 0 || use >= 0xEu)
                    {
                        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_combat.cpp";
                        AeAssert::gCurrentLine = 1348;
                        AeAssert::gCurrentExpr = "targ->use > 0 && targ->use < USE_MAX";
                        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                            __debugbreak();
                    }
                    usetable[targ->use](targ, inflictor, mWorld);
                }
                return;
            }
            if (targ->s.eType == 7)
            {
                Broc::entity e;
                e.___u0 = mWorld->mHandle.mHandle.mVal;
                targ->Notify(hash_const.damage, damage, &e, &mod, &hitLoc, dir);
                return;
            }
            if (targ->scr_vehicle != nullptr)
            {
                float brocDir[3];
                if (dir != nullptr)
                    VectorNormalize2(dir, brocDir);
                else
                {
                    brocDir[0] = 0.0f;
                    brocDir[1] = 0.0f;
                    brocDir[2] = 1.0f;
                }
                if (mp_friendlyfire.integer == 0
                    && (dflags & 0x20) == 0
                    && targ->r.mOwner.mHandle.mVal != 0
                    && mWorld->sentient != nullptr)
                {
                    Entity* owner = HandleDbToEnt(targ->r.mOwner);
                    Entity* v24;
                    if (mWorld->scr_vehicle != nullptr && mWorld->r.mOwner.mHandle.mVal != 0)
                        v24 = HandleDbToEnt(mWorld->r.mOwner);
                    else
                        v24 = mWorld;
                    Entity* v25 = owner;
                    if (v25 != nullptr)
                    {
                        sentient_s* v26 = v24->sentient;
                        if (v26 != nullptr && cgGlobal.teamGame && v25->sentient->eTeam == v26->eTeam)
                            return;
                    }
                }
                if ((dflags & 0x80u) == 0)
                {
                    unsigned char pain = targ->pain;
                    if (pain != 0)
                    {
                        if (pain >= 6u)
                        {
                            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_combat.cpp";
                            AeAssert::gCurrentLine = 1462;
                            AeAssert::gCurrentExpr = "targ->pain > 0 && targ->pain < PAIN_MAX";
                            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                                __debugbreak();
                        }
                        if (dir != nullptr)
                        {
                            if (targ->scr_vehicle == nullptr)
                            {
                                targ->rotate.v.m128_f32[0] = brocDir[0];
                                targ->rotate.v.m128_f32[1] = brocDir[1];
                                targ->rotate.v.m128_f32[2] = brocDir[2];
                            }
                            targ->pos3.v.m128_f32[0] = point[0];
                            targ->pos3.v.m128_f32[1] = point[1];
                            targ->pos3.v.m128_f32[2] = point[2];
                        }
                        else
                        {
                            if (targ->scr_vehicle == nullptr)
                            {
                                targ->rotate.v.m128_f32[2] = 0.0f;
                                targ->rotate.v.m128_f32[1] = 0.0f;
                                targ->rotate.v.m128_f32[0] = 0.0f;
                            }
                            targ->pos3.v.m128_f32[2] = 0.0f;
                            targ->pos3.v.m128_f32[1] = 0.0f;
                            targ->pos3.v.m128_f32[0] = 0.0f;
                        }
                        paintable[targ->pain](targ, mWorld, damage, point, mod, brocDir, hitLoc);
                    }
                    if (mWorld != nullptr)
                    {
                        if (mWorld->IsLocalPlayer() && (mod == 1 || mod == 2))
                            dword_F63D1C[1580 * mWorld->GetPlayerIndex()] = level.time;
                        Entity* v29;
                        if (mWorld->client != nullptr)
                        {
                            v29 = mWorld;
                        }
                        else
                        {
                            if (mWorld->scr_vehicle == nullptr)
                                goto label_121;
                            if (HandleDbToEnt(mWorld->r.mOwner) == nullptr)
                                return;
                            v29 = HandleDbToEnt(mWorld->r.mOwner);
                        }
                        if (!v29->IsLocalPlayer())
                            return;
                    }
label_121:
                    Entity* v30 = HandleDbToEnt(targ->r.mOwner);
                    if (v30 == nullptr || !v30->IsLocalPlayer())
                    {
                        MultiplayerMgr::sInst->VehicleDamage(targ, mWorld, pos, norm,
                                                             (float)damage, weapon,
                                                             (unsigned char)mod, dflags);
                        return;
                    }
                }
                float v31 = Scr_Vehicle_DamageScale(targ, attacker, inflictor, point, mod);
                int v32 = (int)(v31 * damage);
                Destructible* mValue = targ->mDestructible.mValue;
                damage = v32;
                if (mValue != nullptr && mod != 31)
                {
                    bool v34 = targ->health - v32 <= 0;
                    int v35 = v32;
                    if (!v34 && v32 > damageForceReductionThreshold)
                        v35 = damageForceReductionThreshold - (int)((v32 - damageForceReductionThreshold) * -0.25f);
                    if (v35 > damageForceMax)
                        v35 = damageForceMax;
                    ValidatePakId((TPakId)targ->mDestructible.mPakId);
                    Destructible::DoDamage(mValue, targ, v35, &pos, (const float*)&norm, mod, false);
                }
                int v36 = targ->health - v32;
                targ->health = v36;
                Broc::entity e2;
                e2.___u0 = attacker->mHandle.mHandle.mVal;
                targ->Notify(hash_const.damage, damage, &e2, &mod, &hitLoc, nullptr);
                int health = targ->health;
                if (health <= 0 && v32 + health > 0)
                {
                    Entity* v38 = attacker;
                    if (attacker->scr_vehicle != nullptr)
                        v38 = HandleDbToEnt(attacker->r.mOwner);
                    if (v38 == EntityManager::sInst->mWorld)
                        v38 = nullptr;
                    MultiplayerMgr::sInst->VehicleDeath(targ, v38, weapon, mod);
                }
                return;
            }
            if (mWorld->scr_vehicle != nullptr)
            {
                Entity* owner2 = HandleDbToEnt(mWorld->r.mOwner);
                if (owner2 != nullptr)
                    attacker = owner2;
            }
            Client* v40 = targ->client;
            if (v40 == nullptr || v40->noclip == 0)
            {
                float brocDir[3];
                if (dir != nullptr)
                    VectorNormalize2(dir, brocDir);
                else
                {
                    dflags |= 8u;
                    brocDir[0] = 0.0f;
                    brocDir[1] = 0.0f;
                    brocDir[2] = 1.0f;
                }
                float v41 = 0.050000001f;
                if (v40 != nullptr)
                {
                    int pm_flags = v40->ps.pm_flags;
                    if ((pm_flags & 1) != 0)
                        v41 = 0.0099999998f;
                    else if ((pm_flags & 2) != 0)
                        v41 = 0.025f;
                }
                int v43 = damage;
                int v44 = mod;
                int v45 = (int)(damage * v41);
                if (mod == 3 || mod == 4 || mod == 7 || mod == 8 || mod == 9 || mod == 10
                    || mod == 5 || mod == 6 || mod == 17 || mod == 18 || mod == 27)
                {
                    if (v45 > 60)
                        v45 = 60;
                }
                else
                {
                    v45 = 0;
                    if (targ->s.eType == 3)
                        return;
                }
                if ((targ->flags & 0x20) != 0)
                    v45 = 0;
                if ((dflags & 8) == 0 && v45 != 0)
                {
                    Client* v46 = targ->client;
                    if (v46 != nullptr)
                    {
                        float v47 = (v45 * g_knockback.value) * 0.0040000002f;
                        float v48 = v47 * brocDir[1];
                        float v49 = v47 * brocDir[2];
                        v46->ps.velocity.v.m128_f32[0] += (v45 * g_knockback.value) * 0.0040000002f * brocDir[0];
                        v46->ps.velocity.v.m128_f32[1] += v48;
                        v46->ps.velocity.v.m128_f32[2] += v49;
                        if (targ == attacker && (v44 == 9 || v44 == 10 || v44 == 5 || v44 == 6 || v44 == 3 || v44 == 4))
                            v46->ps.velocity.v.m128_f32[2] *= 0.25f;
                        Client* v50 = targ->client;
                        if (v50->ps.pm_time == 0)
                        {
                            int v51 = 2 * v45;
                            if (2 * v45 >= 50)
                            {
                                if (v51 > 200)
                                    v51 = 200;
                            }
                            else
                            {
                                v51 = 50;
                            }
                            v50->ps.pm_time = v51;
                            v50->ps.pm_flags |= 0x200u;
                        }
                    }
                }
                if ((targ->flags & 1) == 0)
                {
                    if (v43 < 1)
                        damage = 1;
                    actor_s* actor = targ->actor;
                    if (actor != nullptr && actor->eState[actor->iStateLevel] == AIS_WOUNDED)
                        damage = 300;
                    int v53 = damage;
                    int v54 = CheckArmor(targ, damage, dflags);
                    int v55 = v53 - v54;
                    if (g_debugDamage.integer != 0)
                        G_Printf("client:%i health:%i damage:%i armor:%i\n",
                                 targ->mHandle.mHandle.mVal, targ->health, v55, v54);
                    int v56 = weapon;
                    if (weapon <= 0)
                        v56 = inflictor->s.weapon;
                    if (v55 != 0 && targ->client != nullptr && gpBrocAPI->mBrocExports.mCallbackPlayerDamage != nullptr)
                    {
                        unsigned int mVal = attacker->mHandle.mHandle.mVal;
                        float hitPos[3] = { pos.v.m128_f32[0], pos.v.m128_f32[1], pos.v.m128_f32[2] };
                        gpBrocAPI->mBrocExports.mCallbackPlayerDamage(
                            targ->mHandle.mHandle.mVal, inflictor->mHandle.mHandle.mVal,
                            mVal, brocDir, hitPos, v55, mod, v56, hitLoc);
                    }
                    else
                    {
                        G_FinishDamage(targ, inflictor, attacker, brocDir, (const float*)&pos,
                                       v55, mod, v56, hitLoc);
                    }
                }
            }
        }
    }
}

// ea: 0x00484AC0
int G_RadiusDamage(const float* origin, Entity* inflictor, Entity* attacker,
                   float fInnerDamage, float fOuterDamage, float radius,
                   Entity* ignore, int mod)
{
    DbLinkedHandle<EntityHandleDb, Entity> entityList[256];
    memset(entityList, 0, sizeof(entityList));
    int points = 0;
    if (attacker == nullptr)
        return 0;
    if (radius < 1.0f)
        radius = 1.0f;
    math::Position3 mins;
    math::Position3 maxs;
    mins.v.m128_f32[0] = origin[0] - (radius * 1.4142135f);
    maxs.v.m128_f32[0] = (radius * 1.4142135f) + origin[0];
    mins.v.m128_f32[1] = origin[1] - (radius * 1.4142135f);
    maxs.v.m128_f32[1] = origin[1] + (radius * 1.4142135f);
    mins.v.m128_f32[2] = origin[2] - (radius * 1.4142135f);
    maxs.v.m128_f32[2] = origin[2] + (radius * 1.4142135f);
    int v11 = CM_AreaEntities(mins, maxs, entityList, 256, -1);
    int v12 = 0;
    int numListedEntities = 0;
    float damage;
    if (v11 > 0)
    {
        do
        {
            unsigned int v13 = entityList[v12].mHandle.mVal;
            unsigned int v14 = v13 & 0xFFF;
            Entity* mObject = nullptr;
            if (v14 < 0x540 && v13 >> 12 == EntityHandleDb::sInst.mElements[v14].mKey)
                mObject = EntityHandleDb::sInst.mElements[v14].mObject;
            if (v14 >= 0x540 || v13 >> 12 != EntityHandleDb::sInst.mElements[v14].mKey || mObject == nullptr)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_combat.cpp";
                AeAssert::gCurrentLine = 2020;
                AeAssert::gCurrentExpr = "ent";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("NULL entity found"))
                    __debugbreak();
                goto next_entity;
            }
            if (mObject != ignore && mObject->takedamage != 0)
            {
                Entity* v16 = inflictor;
                if (inflictor != nullptr)
                {
                    tagInfo_t* tagInfo = inflictor->tagInfo;
                    if (tagInfo != nullptr && tagInfo->parent == mObject)
                    {
                        damage = fInnerDamage;
                        goto apply_damage;
                    }
                }
                float dir[3];
                if (mObject->r.bmodel != nullptr)
                {
                    for (int i = 0; i < 3; ++i)
                    {
                        float v21 = mObject->r.absmin.v.m128_f32[i];
                        float v22;
                        if (v21 <= origin[i])
                        {
                            if (origin[i] <= mObject->r.absmax.v.m128_f32[i])
                                v22 = 0.0f;
                            else
                                v22 = origin[i] - mObject->r.absmax.v.m128_f32[i];
                        }
                        else
                        {
                            v22 = v21 - origin[i];
                        }
                        dir[i] = v22;
                    }
                }
                else
                {
                    dir[0] = mObject->r.currentOrigin.v.m128_f32[0] - origin[0];
                    dir[1] = mObject->r.currentOrigin.v.m128_f32[1] - origin[1];
                    dir[2] = mObject->r.currentOrigin.v.m128_f32[2] - origin[2];
                }
                float v33 = (float)sqrt(dir[2] * dir[2] + dir[1] * dir[1] + dir[0] * dir[0]);
                if (v33 < radius && (mObject->client == nullptr || level.bPlayerIgnoreRadiusDamage == 0))
                {
                    damage = ((1.0f - (v33 / radius)) * (fInnerDamage - fOuterDamage)) + fOuterDamage;
                    if (CanDamage(mObject, origin, v16) != 0)
                        goto apply_damage;
                }
                goto next_entity;
apply_damage:
                if (LogAccuracyHit(mObject, attacker) != 0)
                    points = 1;
                float point[3];
                point[0] = origin[0];
                point[1] = origin[1];
                point[2] = origin[2];
                float dist[3];
                if (mObject->actor != nullptr)
                {
                    dist[0] = mObject->r.currentOrigin.v.m128_f32[0] - origin[0];
                    dist[1] = mObject->r.currentOrigin.v.m128_f32[1] - origin[1];
                    dist[2] = (mObject->r.currentOrigin.v.m128_f32[2] - origin[2]) + 50.0f;
                }
                else
                {
                    float clamped[3];
                    clamped[0] = origin[0];
                    clamped[1] = origin[1];
                    clamped[2] = origin[2];
                    for (int i = 0; i < 3; ++i)
                    {
                        if (clamped[i] < mObject->r.absmin.v.m128_f32[i])
                            clamped[i] = mObject->r.absmin.v.m128_f32[i];
                        else if (clamped[i] > mObject->r.absmax.v.m128_f32[i])
                            clamped[i] = mObject->r.absmax.v.m128_f32[i];
                    }
                    float center[3];
                    if (mObject->r.bmodel != nullptr)
                    {
                        center[0] = (mObject->r.bmodel->center.v.m128_f32[0] + mObject->r.bmodel->center.v.m128_f32[0]) * 0.5f;
                        center[1] = (mObject->r.bmodel->center.v.m128_f32[1] + mObject->r.bmodel->center.v.m128_f32[1]) * 0.5f;
                        center[2] = (mObject->r.bmodel->center.v.m128_f32[2] + mObject->r.bmodel->center.v.m128_f32[2]) * 0.5f;
                    }
                    else
                    {
                        center[0] = (mObject->r.absmin.v.m128_f32[0] + mObject->r.absmax.v.m128_f32[0]) * 0.5f;
                        center[1] = (mObject->r.absmin.v.m128_f32[1] + mObject->r.absmax.v.m128_f32[1]) * 0.5f;
                        center[2] = (mObject->r.absmin.v.m128_f32[2] + mObject->r.absmax.v.m128_f32[2]) * 0.5f;
                    }
                    dist[0] = center[0] - origin[0];
                    dist[1] = center[1] - origin[1];
                    dist[2] = center[2] - origin[2];
                    point[0] = clamped[0];
                    point[1] = clamped[1];
                    point[2] = clamped[2];
                }
                G_Damage(mObject, inflictor, attacker, dist, point,
                         (int)damage, 9, mod, HITLOC_NONE, -1);
            }
next_entity:
            v12 = numListedEntities + 1;
            numListedEntities = v12;
        } while (v12 < v11);
    }
    return points;
}

// ea: 0x0046A1C0
bool Prop_SetupCollmap(Entity* ent)
{
    ValidatePakId((TPakId)ent->mModel.mPakId);
    if (ent->mModel.mValue == nullptr)
        return 0;
    ValidatePakId((TPakId)ent->mModel.mPakId);
    if (ent->mModel.mValue->name.mStr == nullptr)
        return 0;
    ValidatePakId((TPakId)ent->mModel.mPakId);
    if (strstr(ent->mModel.mValue->name.mStr, "sbmodel") != nullptr)
        return 0;
    ent->s.brushmodel = 0;
    SV_SetBrushModel(ent);
    ent->r.contents = 0x200001;
    return 1;
}

// ea: 0x0046E270
bool G_CanPlayerBeDamagedInVehicle(Entity* player)
{
    Client* client = player->client;
    if (client == nullptr)
        return false;
    int eFlags = client->ps.eFlags;
    if ((0x100000 & eFlags) == 0 || (0x400000 & eFlags) != 0)
        return false;
    Entity* v4 = HandleDbToEnt(player->r.mOwner);
    if (v4 == nullptr)
        return false;
    scr_vehicle_t* scr_vehicle = v4->scr_vehicle;
    if (scr_vehicle == nullptr)
        return false;
    return !IsPlayerFullySeatedInVehicle(player)
        || s_vehicleInfos[scr_vehicle->infoIdx]->type != 2
        || player->client->ps.vehPos != 0;
}

// ea: 0x00485FF0
void G_ExplodeMissile(Entity* ent, int msec)
{
    unsigned int mVal = ent->parentHandle.mHandle.mVal;
    Entity* mObject = nullptr;
    unsigned int v5 = mVal & 0xFFF;
    trace_t trace;
    trace.surfaceFlags = 0;
    trace.contents = 0;
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(ent->s.weapon);
    if (v5 < 0x540 && mVal >> 12 == EntityHandleDb::sInst.mElements[v5].mKey)
        mObject = EntityHandleDb::sInst.mElements[v5].mObject;
    Entity* owner = mObject;
    if (mObject != nullptr)
    {
        if (!EntityManager::sInst->IsLocalPlayer(mObject)
            && mObject->client != nullptr
            && InfoForWeapon->iTriggerRadius == 0)
        {
            G_FreeEntity(ent, msec);
            return;
        }
        if (mObject->s.eType == 10 && mObject->r.mOwner.mHandle.mVal != 0)
            owner = HandleDbToEnt(mObject->r.mOwner);
    }
    if (InfoForWeapon->slot != WEAPSLOT_SMOKE_GRENADE || ent->s.mGroundEntity.mHandle.mVal != 0)
    {
        ent->nextthink = 0;
        Entity* activator = ent->activator;
        if (activator != nullptr && activator->actor != nullptr && (ent->r.svFlags & 1) != 0)
        {
            static unsigned int sInit = 0;
            static unsigned int GRENADE_RETURN_HAND_TAG_hash;
            if ((sInit & 1) == 0)
            {
                sInit |= 1u;
                GRENADE_RETURN_HAND_TAG_hash = HashString::CalcHash("tag_weapon_right");
            }
            DObjSkelMat tagMat;
            G_DObjGetWorldTagMatrix(ent->activator, GRENADE_RETURN_HAND_TAG_hash, &tagMat);
            G_SetOrigin(ent, &tagMat.origin[0]);
            ent->r.svFlags &= ~1u;
            G_EntDetach(ent->activator, InfoForWeapon->szWorldModel, "tag_weapon_right");
        }
        else
        {
            math::Position3 pos;
            BG_EvaluateTrajectory(&ent->s.pos, level.time, pos);
            G_SetOrigin(ent, &pos);
        }
        j_nullsub_120(ent);
        int eFlags = ent->s.eFlags;
        int svFlags = ent->r.svFlags;
        float v10 = ent->r.currentOrigin.v.m128_f32[0];
        ent->flags |= 0x400u;
        int clipmask = ent->clipmask;
        ent->s.eFlags = eFlags | 0x80;
        ent->s.eType = 0;
        ent->r.contents = 0;
        ent->r.svFlags = svFlags | 0x20;
        collision_context_t context;
        context.__vftable = nullptr;
        context.pass_entity1.mHandle.mVal = 0;
        context.pass_entity2.mHandle.mVal = clipmask & 0xFDFFFFEE | 0x11;
        math::Position3 end;
        end.v.m128_f32[0] = v10;
        end.v.m128_f32[1] = ent->r.currentOrigin.v.m128_f32[1];
        end.v.m128_f32[2] = ent->r.currentOrigin.v.m128_f32[2] - 16.0f;
        context.contentmask = (int)(ent->r.currentOrigin.v.m128_f32[2] + 2.0f);
        math::Position3 zeroA;
        math::Position3 zeroB;
        zeroA.v = _mm_setzero_ps();
        zeroB.v = _mm_setzero_ps();
        math::Position3 start;
        start.v.m128_f32[0] = v10;
        start.v.m128_f32[1] = ent->r.currentOrigin.v.m128_f32[1];
        start.v.m128_f32[2] = ent->r.currentOrigin.v.m128_f32[2];
        SV_Trace(&trace, &start, &zeroB, &zeroA, &end, &context, 0, 0, nullptr, 0, 0.0f);
        unsigned char v16 = DirToByte(trace.normal.v.m128_f32);
        G_AddEvent(ent, 210, v16);
        unsigned char projExplosion = InfoForWeapon->projExplosion;
        ent->s.scale = projExplosion;
        collision_context_t ctx2(32);
        if (SV_PointContents(ent->r.currentOrigin, ctx2) != 0)
            ent->s.surfType = 20;
        else
            ent->s.surfType = ((int)trace.normal.v.m128_f32[2] >> 20) & 0x1F;
        float v18 = trace.endpos.v.m128_f32[0];
        float v19 = trace.endpos.v.m128_f32[1];
        ent->think = THINK__G_FreeEntity;
        ent->nextthink = level.time + 60000;
        math::Position3 origin;
        origin.v.m128_f32[0] = v18;
        origin.v.m128_f32[1] = v19;
        origin.v.m128_f32[2] = trace.endpos.v.m128_f32[2];
        G_SetOrigin(ent, &origin);
        Entity::SetLerpOrigin(&ent->s, &origin);
        Entity* v21 = HandleDbToEnt(ent->r.mOwner);
        MultiplayerMgr::sInst->ProjectileExplosion(ent, ent->s.weapon, origin,
                                                   trace.normal, ent->s.surfType, v21);
        weaponFileInfo_t* v22 = InfoForWeapon;
        if (InfoForWeapon->slot == WEAPSLOT_SMOKE_GRENADE || InfoForWeapon->iExplosionInnerDamage != 0)
        {
            collision_context_t ctx3;
            ctx3.__vftable = nullptr;
            ctx3.pass_entity1.mHandle.mVal = 0;
            ctx3.pass_entity2.mHandle.mVal = ent->mHandle.mHandle.mVal;
            ctx3.contentmask = 0;
            math::Position3 end2;
            end2.v.m128_f32[0] = ent->r.currentOrigin.v.m128_f32[0];
            end2.v.m128_f32[1] = ent->r.currentOrigin.v.m128_f32[1];
            end2.v.m128_f32[2] = ent->r.currentOrigin.v.m128_f32[2] + 10.0f;
            math::Position3 zeroC;
            math::Position3 zeroD;
            zeroC.v = _mm_setzero_ps();
            zeroD.v = _mm_setzero_ps();
            SV_Trace(&trace, &ent->r.currentOrigin, &zeroD, &zeroC, &end2, &ctx3, 0, 0, nullptr, 0, 0.0f);
            G_RadiusDamage(origin.v.m128_f32, ent, owner,
                           (float)InfoForWeapon->iExplosionInnerDamage,
                           (float)InfoForWeapon->iExplosionOuterDamage,
                           (float)InfoForWeapon->iExplosionRadius,
                           ent, ent->splashMethodOfDeath);
            v22 = InfoForWeapon;
        }
        int key = ent->key;
        if (key > 0 && v22->slot != WEAPSLOT_SMOKE_GRENADE)
            EffectEventSys::sInst->StopEffect(key, false);
        SV_UnlinkEntity(ent);
    }
    else
    {
        if (ent->key == 0)
            ent->key = PostEffectEventWeapon(ent, InfoForWeapon->szInternalName,
                                             kActionEI_MELEE_PLAYER_LOSING | kActionWEAPON_FIRE_3RD);
        ent->nextthink = level.time + 3000;
    }
}

Entity* g_path_owner;  // 0xF51E40?

// ea: 0x0048BDE0
void G_RunMissile(Entity* ent, int msec)
{
    trace_t tr;
    tr.surfaceFlags = 0;
    tr.contents = 0;
    if (g_debugGrenades.integer != 0)
    {
        weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(ent->s.weapon);
        // debug rendering (RGBA color = 1.0 alpha)
        float col[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        DebugRender::RenderSphere(&ent->r.currentOrigin,
                                  (1.0f - ((float)(g_player_maxhealth.integer - ent->health)
                                            / (float)g_player_maxhealth.integer)) * 1.0f,
                                  col);
        if (InfoForWeapon->iExplosionRadius != 0)
        {
            float col2[4] = { 1.0f, 0.0f, 0.1f, 1.0f };
            DebugRender::RenderBox(&ent->r.absmin, &ent->r.absmax, col2);
            float col3[4] = { 1.0f, 0.0f, 0.30000001f, 1.0f };
            DebugRender::RenderSphere(&ent->r.currentOrigin, (float)InfoForWeapon->iExplosionRadius, col3);
        }
    }
    if (ent->methodOfDeath == 3)
    {
        if (level.time - ent->timestamp >= 500)
        {
            j_nullsub_17(ent, AI_EV_GRENADE_PING, -1, &ent->r.currentOrigin, 0.0f);
            ent->timestamp = level.time;
        }
    }
    else
    {
        j_nullsub_17(ent, AI_EV_PROJECTILE_PING, -1, &ent->r.currentOrigin, 0.0f);
    }
    if (IS_NAN(ent->r.currentOrigin.v.m128_f32[0])
        || IS_NAN(ent->r.currentOrigin.v.m128_f32[1])
        || IS_NAN(ent->r.currentOrigin.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_missile.cpp";
        AeAssert::gCurrentLine = 823;
        AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    math::Position3 vOldOrigin;
    float vReflect[3];
    math::Position3 origin;
    float dir[3];
    collision_context_t ctx;
    if (ent->tagInfo != nullptr)
        goto label_19;
    BG_EvaluateTrajectory(&ent->s.pos, level.time, *reinterpret_cast<math::Position3*>(vReflect));
    origin.v.m128_f32[0] = vReflect[0] - ent->r.currentOrigin.v.m128_f32[0];
    origin.v.m128_f32[1] = vReflect[1] - ent->r.currentOrigin.v.m128_f32[1];
    origin.v.m128_f32[2] = vReflect[2] - ent->r.currentOrigin.v.m128_f32[2];
    if ((ent->s.eFlags & 0x20000000) != 0 && ent->s.apos.trType != TR_STATIONARY)
    {
        vectoangles(origin.v.m128_f32, ent->s.apos.trBase);
        ent->s.apos.trBase[0] = ent->s.apos.trBase[0] + 90.0f;
    }
    if (VectorNormalize(origin.v.m128_f32) < 0.0049999999f)
    {
label_19:
        G_RunThink(ent, msec);
        return;
    }
    if (level.MissleOnlyActiveForTime != 0.0f && (level.time - ent->timestamp) >= level.MissleOnlyActiveForTime)
        goto label_71;
    float v10 = (float)fabs(ent->s.pos.trDelta[2]);
    float v33[3];
    int v35 = 0;
    Entity* touch = nullptr;
    ctx.__vftable = nullptr;
    ctx.pass_entity1.mHandle.mVal = 0;
    ctx.pass_entity2.mHandle.mVal = 34;
    int contents;
    if (v10 <= 30.0f || SV_PointContents(ent->r.currentOrigin, ctx) != 0)
    {
        unsigned int mVal = ent->r.mOwner.mHandle.mVal;
        int clipmask = ent->clipmask;
        collision_context_t ctx2;
        ctx2.__vftable = nullptr;
        ctx2.pass_entity1.mHandle.mVal = mVal;
        ctx2.pass_entity2.mHandle.mVal = clipmask;
        ctx2.contentmask = 0;
        g_LocationalTrace((trace_t*)v33, &ent->r.currentOrigin,
                          (const math::Position3*)vReflect, &ctx2,
                          bulletPriorityMap, 0.0f);
    }
    else
    {
        G_MissileTrace((trace_t*)v33, &ent->r.currentOrigin,
                       (const math::Position3*)vReflect, ent->r.mOwner,
                       ent->clipmask | 0x22, bulletPriorityMap);
    }
    if (((int)tr.normal.v.m128_f32[2] & 0x1F00000) == 0x1400000
        || ((int)tr.normal.v.m128_f32[2] & 0x1F00000) == 0x800000)
    {
        float norm[3];
        VectorNormalize2(ent->s.pos.trDelta, norm);
        float v39 = norm[2];
        if (v39 < 0.0f)
            v39 = v39 * -1.0f;
        Entity* v15 = G_TempEntity(ent->r.currentOrigin.v.m128_f32, 203);
        v15->s.eventParm = DirToByte(&tr.endpos.v.m128_f32[1]);
        v15->s.scale = DirToByte(norm);
        v15->s.surfType = ((int)tr.normal.v.m128_f32[2] >> 20) & 0x1F;
        v15->s.weapon = ent->s.weapon;
        v15->s.mOtherEntity.mHandle.mVal = ent->mHandle.mHandle.mVal;
        G_MissileTrace((trace_t*)v33, &ent->r.currentOrigin,
                       (const math::Position3*)vReflect, ent->r.mOwner,
                       ent->clipmask, bulletPriorityMap);
    }
    if (ent->methodOfDeath == 3
        && tr.mEntity.mHandle.mVal != 0
        && (HandleDbToEnt(tr.mEntity)->flags & 0x80000) != 0)
    {
        Entity* v16 = HandleDbToEnt(tr.mEntity);
        int v17 = v16->r.contents;
        v16->r.contents = 0;
        G_MissileTrace((trace_t*)v33, &ent->r.currentOrigin,
                       (const math::Position3*)vReflect, ent->r.mOwner,
                       ent->clipmask, bulletPriorityMap);
        v16->r.contents = v17;
    }
    if (g_debugBullets.integer >= 5)
    {
        Entity* v19 = G_TempEntity(ent->r.currentOrigin.v.m128_f32, 214);
        memcpy(&v19->s.origin2, v33, 12);
    }
    if (IS_NAN(ent->r.currentOrigin.v.m128_f32[0])
        || IS_NAN(ent->r.currentOrigin.v.m128_f32[1])
        || IS_NAN(ent->r.currentOrigin.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_missile.cpp";
        AeAssert::gCurrentLine = 921;
        AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    unsigned char v20 = (unsigned char)(tr.mEntity.mHandle.mVal >> 8);
    ent->r.currentOrigin.v.m128_f32[0] = v33[0];
    ent->r.currentOrigin.v.m128_f32[1] = v33[1];
    ent->r.currentOrigin.v.m128_f32[2] = v33[2];
    float v21;
    if (v20 != 0)
    {
        v21 = 0.0f;
        tr.normal.v.m128_f32[1] = 0.0f;
    }
    else
    {
        v21 = tr.normal.v.m128_f32[1];
    }
    if ((ent->s.eFlags & 0x3800000) != 0
        && (v21 == 1.0f || (v21 < 1.0f && tr.endpos.v.m128_f32[3] > 0.69999999f)))
    {
        math::Position3 start;
        start.v.m128_f32[0] = ent->r.currentOrigin.v.m128_f32[0];
        start.v.m128_f32[1] = ent->r.currentOrigin.v.m128_f32[1];
        start.v.m128_f32[2] = ent->r.currentOrigin.v.m128_f32[2] + 1.5f;
        math::Position3 end;
        end.v.m128_f32[0] = ent->r.currentOrigin.v.m128_f32[0];
        end.v.m128_f32[1] = ent->r.currentOrigin.v.m128_f32[1];
        end.v.m128_f32[2] = ent->r.currentOrigin.v.m128_f32[2] - 1.5f;
        G_MissileTrace((trace_t*)v33, &start, &end, ent->r.mOwner,
                       ent->clipmask, bulletPriorityMap);
        if (tr.normal.v.m128_f32[1] != 1.0f)
        {
            ent->s.pos.trBase[2] = ((v33[2] + 1.5f) - ent->r.currentOrigin.v.m128_f32[2]) + ent->s.pos.trBase[2];
            ent->r.currentOrigin.v.m128_f32[0] = v33[0];
            ent->r.currentOrigin.v.m128_f32[1] = v33[1];
            ent->r.currentOrigin.v.m128_f32[2] = v33[2] + 1.5f;
        }
    }
    g_LinkEntity(ent);
    if (ent->methodOfDeath == 3)
    {
        weaponFileInfo_t* v29 = BG_GetInfoForWeapon(ent->s.weapon);
        G_GrenadeTouchTriggerDamage(ent, &ent->r.currentOrigin, &ent->r.currentOrigin,
                                    v29->iExplosionInnerDamage, ent->methodOfDeath);
    }
    if (tr.normal.v.m128_f32[1] == 1.0f || ((int)tr.normal.v.m128_f32[3] & 2) != 0)
    {
        if (sqrt(ent->s.pos.trDelta[0] * ent->s.pos.trDelta[0]
                 + ent->s.pos.trDelta[1] * ent->s.pos.trDelta[1]
                 + ent->s.pos.trDelta[2] * ent->s.pos.trDelta[2]) != 0.0f)
            ent->s.mGroundEntity.mHandle.mVal = 0;
        goto label_71;
    }
    if (((int)tr.normal.v.m128_f32[2] & 4) != 0)
    {
        if (ent->mDObj != nullptr)
            ent->SetAlwaysRender(true);
label_71:
        G_RunThink(ent, msec);
        return;
    }
    if (((int)tr.normal.v.m128_f32[2] & 0x10) != 0)
    {
        G_FreeEntity(ent, msec);
        return;
    }
    g_path_owner = ent;
    float oldOrigin[3] = { ent->r.currentOrigin.v.m128_f32[0],
                           ent->r.currentOrigin.v.m128_f32[1],
                           ent->r.currentOrigin.v.m128_f32[2] };
    G_MissileImpact(ent, (trace_t*)v33, origin.v.m128_f32, oldOrigin);
    g_path_owner = nullptr;
    if (ent->s.eType == 3)
        goto label_71;
}
