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
