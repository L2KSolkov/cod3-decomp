// ============================================================================
// g_weapon.cpp - weapon fire/mine helpers (g.o: g_weapon.cpp family subset)
// ============================================================================

#include "game/logic/g_local.h"

int Weapon_Mine_Test(Entity* ent, weaponParms* wp, math::Position3* position,
                     math::Dir3* normal);

// ea: 0x0077C4D0 (mp_actors.o)
int Actor_Grenade_IsValidTrajectory(actor_s* pSelf, const float* vFrom,
                                    const float* vVelocity,
                                    const float* vGoal)
{
    (void)pSelf; (void)vFrom; (void)vVelocity; (void)vGoal;
    return 0;
}

#include <math.h>
#include <stdlib.h>

static Entity* EntFromHandle(unsigned int h)
{
    unsigned int idx = h & 0xFFF;
    if (idx < 0x540
        && h >> 12 == EntityHandleDb::sInst.mElements[idx].mKey)
        return EntityHandleDb::sInst.mElements[idx].mObject;
    return nullptr;
}

// ea: 0x0044B130
void Fill_Clip(PlayerState* ps, int weapon)
{
    int v2 = BG_AmmoForWeapon(weapon);
    int v3 = BG_ClipForWeapon(weapon);
    if (weapon > 0 && weapon <= BG_GetNumWeapons())
    {
        int inclip = ps->ammoclip[v3];
        int v4 = BG_GetAmmoClipSize(v3) - inclip;
        int v5 = ps->ammo[v2];
        if (v4 > v5)
            v4 = ps->ammo[v2];
        if (v4 != 0)
        {
            ps->ammo[v2] = v5 - v4;
            ps->ammoclip[v3] += v4;
        }
    }
}

// ea: 0x0044B9F0
void G_CheckReloadStatus()
{
    ;
}

// ea: 0x004835B0
void Cmd_DropWeapon_f(Entity* pSelf)
{
    if (g_developer.integer != 0
        && g_cheats.integer != 0
        && Drop_Weapon(pSelf, pSelf->s.weapon, nullptr) != nullptr)
    {
        G_AddEvent(pSelf, 174, 0);
    }
}

// ea: 0x004720E0
void Weapon_Revive(Entity* ent, int /*grenType*/, weaponParms* wp)
{
    Entity* traceEnt = nullptr;
    if (Weapon_Revive_Test(ent, wp, &traceEnt))
        MultiplayerMgr::sInst->AttemptToRevivePlayer(traceEnt, ent);
    Scr_Notify(ent, hash_const.fireSpecial, 0);
}

// ea: 0x004818A0
Entity* weapon_mine_fire(Entity* ent, int weapon, weaponParms* wp)
{
    Entity* v4 = nullptr;
    float v6[3];
    math::Position3 position;
    if (Weapon_Mine_Test(ent, wp, (math::Position3*)v6, (math::Dir3*)&position.v.m128_f32[1]))
    {
        v4 = fire_mine(ent, v6, &position.v.m128_f32[1], weapon);
        MultiplayerMgr::MPEntityHandle v8;
        MultiplayerMgr::sInst->RegisterDroppedItem(kItemTypeMines, v4, ent, 0);
        MultiplayerMgr::sInst->FireMissile(weapon, *(math::Position3*)v6,
                                           *(math::Dir3*)&position.v.m128_f32[1], v8);
    }
    return v4;
}

// ea: 0x00453620
void CalcMuzzlePoints(Entity* ent, weaponParms* wp)
{
    Client* client = ent->client;
    float tmp[3];
    tmp[0] = client->ps.viewangles[0];
    tmp[1] = client->ps.viewangles[1];
    tmp[2] = client->ps.viewangles[2];
    if (EntityManager::sInst->IsLocalPlayer(ent))
    {
        Client* v4 = ent->client;
        tmp[0] = v4->fGunPitch;
        tmp[1] = v4->fGunYaw;
    }
    AngleVectors(tmp, wp->forward, wp->right, wp->up);
    math::Position3 muzzlePoint;
    CalcMuzzlePoint(ent, &muzzlePoint);
    wp->muzzleTrace[0] = muzzlePoint.v.m128_f32[0];
    wp->muzzleTrace[1] = muzzlePoint.v.m128_f32[1];
    wp->muzzleTrace[2] = muzzlePoint.v.m128_f32[2];
}

// ea: 0x0044C6B0
void Die_MineDamaged(Entity* mine)
{
    mine->think = THINK__G_ExplodeMissile;
    mine->nextthink = level.time + 10;
    mine->touch = 0;
    mine->die = 0;
}

// ea: 0x004513E0
void* Com_GetWeaponInfoMemory(int iSize, int* piParsed)
{
    extern void* Com_GetWeaponInfoMemory(int iSize, int* piParsed, int iSource);
    return Com_GetWeaponInfoMemory(iSize, piParsed, 1);
}

// ea: 0x00451400
void Com_FreeWeaponInfoMemory(int bRestart)
{
    extern void Com_FreeWeaponInfoMemory(int iSource, int bRestart);
    Com_FreeWeaponInfoMemory(1, bRestart);
}

// ea: 0x004533C0
void Weapon_ItemHealth_Fire(Entity* /*ent*/, int /*grenType*/, weaponParms* wp)
{
    weaponFileInfo_t* pWeapInfo = wp->pWeapInfo;
    float iProjectileSpeed = (float)pWeapInfo->iProjectileSpeed;
    float v5 = wp->forward[1] * iProjectileSpeed;
    float v6 = (wp->forward[2] * iProjectileSpeed) + (float)pWeapInfo->iProjectileSpeedUp;
    float vTossDir[3];
    vTossDir[0] = iProjectileSpeed * wp->forward[0];
    vTossDir[1] = v5;
    vTossDir[2] = v6;
    VectorNormalize(vTossDir);
}

// ea: 0x004534F0
void CalcMuzzlePoint(Entity* ent, math::Position3* muzzlePoint)
{
    if ((0x100000 & ent->client->ps.eFlags) != 0)
    {
        muzzlePoint->v.m128_f32[0] = dword_F63C70[1580 * currCl];
        muzzlePoint->v.m128_f32[1] = dword_F63C70[1580 * currCl + 1];
        muzzlePoint->v.m128_f32[2] = dword_F63C70[1580 * currCl + 2];
    }
    else
    {
        muzzlePoint->v.m128_f32[0] = ent->r.currentOrigin.v.m128_f32[0];
        muzzlePoint->v.m128_f32[1] = ent->r.currentOrigin.v.m128_f32[1];
        muzzlePoint->v.m128_f32[2] = ent->r.currentOrigin.v.m128_f32[2];
        muzzlePoint->v.m128_f32[2] = ent->client->ps.viewHeightCurrent + muzzlePoint->v.m128_f32[2];
    }
    float tmp[3];
    tmp[0] = muzzlePoint->v.m128_f32[0];
    tmp[1] = muzzlePoint->v.m128_f32[1];
    tmp[2] = muzzlePoint->v.m128_f32[2];
    G_AddLean(ent, tmp);
    muzzlePoint->v.m128_f32[0] = tmp[0];
    muzzlePoint->v.m128_f32[1] = tmp[1];
    muzzlePoint->v.m128_f32[2] = tmp[2];
    muzzlePoint->v.m128_f32[0] = ent->client->fGunXOfs + muzzlePoint->v.m128_f32[0];
    muzzlePoint->v.m128_f32[1] = ent->client->fGunYOfs + muzzlePoint->v.m128_f32[1];
    muzzlePoint->v.m128_f32[2] = ent->client->fGunZOfs + muzzlePoint->v.m128_f32[2];
}

// ea: 0x004536F0
int ADS_IS_ACTIVE()
{
    return !View::IsSplitScreen();
}

// ea: 0x00454AF0
void UseLiveGrenade(Entity* /*ent*/, Entity* /*other*/, Entity* /*activator*/)
{
    ;
}

// ea: 0x00469720
void Touch_Mine(Entity* mine, Entity* toucher)
{
    Client* client = toucher->client;
    if (client == nullptr)
        goto explode;
    unsigned int v3 = toucher->s.mGroundEntity.mHandle.mVal & 0xFFF;
    if (v3 < 0x540
        && toucher->s.mGroundEntity.mHandle.mVal >> 12 == EntityHandleDb::sInst.mElements[v3].mKey
        && EntityHandleDb::sInst.mElements[v3].mObject != nullptr
        && (client->ps.pm_flags & 1) == 0)
    {
        if (!EntityManager::sInst->IsLocalPlayer(toucher))
        {
            G_FreeEntity(mine, 0);
            return;
        }
        goto explode;
    }
    return;
explode:
    PostEffectEventScriptCall(mine, "minefield_click", false, PAK_ID_INVALID, false);
    mine->think = THINK__G_ExplodeMissile;
    mine->nextthink = irand(0, 500) + level.time + 250;
    mine->touch = 0;
    mine->die = 0;
}

// ea: 0x004697F0
void Touch_Mine_Not_Owner(Entity* mine, Entity* toucher)
{
    unsigned int v3 = mine->r.mOwner.mHandle.mVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v3 < 0x540 && mine->r.mOwner.mHandle.mVal >> 12 == EntityHandleDb::sInst.mElements[v3].mKey)
        mObject = EntityHandleDb::sInst.mElements[v3].mObject;
    if (toucher != mObject)
        Touch_Mine(mine, toucher);
}

// ea: 0x00469840
void Think_EnableMine(Entity* ent)
{
    unsigned int v1 = ent->r.mOwner.mHandle.mVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v1 < 0x540 && ent->r.mOwner.mHandle.mVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey)
        mObject = EntityHandleDb::sInst.mElements[v1].mObject;
    if (BG_PlayerTouchesMine(&mObject->client->ps, &ent->s, level.time))
    {
        ent->nextthink = level.time + 10;
    }
    else
    {
        ent->think = THINK__NULL;
        ent->touch = 11;
    }
}

// ea: 0x0045F980
int Weapon_Mine_Test(Entity* ent, weaponParms* wp, math::Position3* position, math::Dir3* normal)
{
    math::Position3 start;
    start.v.m128_f32[0] = (wp->forward[0] * delta) + wp->muzzleTrace[0];
    start.v.m128_f32[1] = (wp->forward[1] * delta) + wp->muzzleTrace[1];
    start.v.m128_f32[2] = (wp->forward[2] * delta) + wp->muzzleTrace[2];
    collision_context_t context;
    context.__vftable = nullptr;
    context.pass_entity1.mHandle.mVal = 0;
    context.pass_entity2.mHandle.mVal = ent->mHandle.mHandle.mVal;
    context.contentmask = 0;
    math::Position3 end = start;
    end.v.m128_f32[2] += 1.0f;
    trace_t tr;
    tr.surfaceFlags = 0;
    tr.contents = 0;
    math::Position3 zeroMins;
    math::Position3 zeroMaxs;
    zeroMins.v = _mm_setzero_ps();
    zeroMaxs.v = _mm_setzero_ps();
    SV_Trace(&tr, &start, &zeroMins, &zeroMaxs, &end, &context, 0, 1,
             bulletPriorityMap, 1, 0.02f);
    if (tr.normal.v.m128_f32[1] == 1.0f)
        return 0;
    if ((0x100000 & ent->client->ps.eFlags) != 0)
        return 0;
    int surf = (int)tr.normal.v.m128_f32[2] & 0x1F00000;
    if (surf != 0x1000000 && surf != 0xA00000 && surf != 0xB00000
        && surf != 0x1300000 && surf != 0x1200000 && surf != 0x1300000)
    {
        return 0;
    }
    if (position != nullptr)
    {
        position->v.m128_f32[0] = tr.endpos.v.m128_f32[0];
        position->v.m128_f32[1] = tr.endpos.v.m128_f32[1];
        position->v.m128_f32[2] = tr.endpos.v.m128_f32[2];
        position->v.m128_f32[3] = tr.endpos.v.m128_f32[3];
    }
    if (normal != nullptr)
    {
        normal->v.m128_f32[0] = tr.endpos.v.m128_f32[1];
        normal->v.m128_f32[1] = tr.endpos.v.m128_f32[2];
        normal->v.m128_f32[2] = tr.endpos.v.m128_f32[3];
        normal->v.m128_f32[3] = tr.normal.v.m128_f32[0];
    }
    return 1;
}

// ea: 0x0045F3B0
float Bullet_Endpos(float spread, float* end, const weaponParms* wp,
                    float randomA, float randomB)
{
    float v6 = 16384.0f;
    if (wp->pWeapInfo->ammoType != 5 /* WEAPAMMOTYPE_UMG */)
        v6 = 8192.0f;
    if (IS_NAN(spread))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 549;
        AeAssert::gCurrentExpr = "!IS_NAN(spread)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    double v7 = tan(spread * 3.1415927f * 0.0055555557f);
    gTanAimConeSpread = (float)v7;
    float fAimOffset = (float)(v7 * v6);
    if (IS_NAN(fAimOffset))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 557;
        AeAssert::gCurrentExpr = "!IS_NAN(fAimOffset)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    float psin;
    FastSinCos((randomA * 360.0f) * 3.1415927f * 0.0055555557f, &psin, &randomA);
    randomA = randomA * randomB * fAimOffset;
    randomB = (psin * randomB) * fAimOffset;
    if (IS_NAN(randomA))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 563;
        AeAssert::gCurrentExpr = "!IS_NAN(r)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    if (IS_NAN(randomB))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 564;
        AeAssert::gCurrentExpr = "!IS_NAN(u)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    if (IS_NAN(wp->muzzleTrace[0])
        || IS_NAN(wp->muzzleTrace[1])
        || IS_NAN(wp->muzzleTrace[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 565;
        AeAssert::gCurrentExpr = "!IS_NAN((wp->muzzleTrace)[0]) && !IS_NAN((wp->muzzleTrace)[1]) && !IS_NAN((wp->muzzleTrace)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(wp->forward[0])
        || IS_NAN(wp->forward[1])
        || IS_NAN(wp->forward[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 566;
        AeAssert::gCurrentExpr = "!IS_NAN((wp->forward)[0]) && !IS_NAN((wp->forward)[1]) && !IS_NAN((wp->forward)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(wp->right[0])
        || IS_NAN(wp->right[1])
        || IS_NAN(wp->right[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 567;
        AeAssert::gCurrentExpr = "!IS_NAN((wp->right)[0]) && !IS_NAN((wp->right)[1]) && !IS_NAN((wp->right)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(wp->up[0])
        || IS_NAN(wp->up[1])
        || IS_NAN(wp->up[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 568;
        AeAssert::gCurrentExpr = "!IS_NAN((wp->up)[0]) && !IS_NAN((wp->up)[1]) && !IS_NAN((wp->up)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    end[0] = (wp->forward[0] * v6) + wp->muzzleTrace[0];
    end[1] = (wp->forward[1] * v6) + wp->muzzleTrace[1];
    end[2] = (wp->forward[2] * v6) + wp->muzzleTrace[2];
    if (IS_NAN(end[0]) || IS_NAN(end[1]) || IS_NAN(end[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 572;
        AeAssert::gCurrentExpr = "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    end[0] = (wp->right[0] * randomA) + end[0];
    end[1] = (wp->right[1] * randomA) + end[1];
    end[2] = (wp->right[2] * randomA) + end[2];
    end[0] = (wp->up[0] * randomB) + end[0];
    end[1] = (wp->up[1] * randomB) + end[1];
    end[2] = (wp->up[2] * randomB) + end[2];
    if (IS_NAN(end[0]) || IS_NAN(end[1]) || IS_NAN(end[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 577;
        AeAssert::gCurrentExpr = "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    return fAimOffset;
}

// ea: 0x0048AA50
void FireWeaponMelee(Entity* ent)
{
    if ((0x106000 & ent->client->ps.eFlags) == 0 || ent->active == 0)
    {
        int weapon = ent->s.weapon;
        ent->invulnerability_timeout = 0;
        weaponParms wp;
        wp.pWeapInfo = BG_GetInfoForWeapon(weapon);
        AngleVectors(ent->client->ps.viewangles, wp.forward, wp.right, wp.up);
        math::Position3 muzzlePoint;
        CalcMuzzlePoint(ent, &muzzlePoint);
        wp.muzzleTrace[0] = muzzlePoint.v.m128_f32[0];
        wp.muzzleTrace[1] = muzzlePoint.v.m128_f32[1];
        wp.muzzleTrace[2] = muzzlePoint.v.m128_f32[2];
        Weapon_Melee(ent, &wp);
    }
}

// ea: 0x00471BA0
bool Weapon_Revive_Test(Entity* ent, weaponParms* wp, Entity** traceEnt)
{
    Client* client = ent->client;
    AngleVectors(client->ps.viewangles, wp->forward, wp->right, wp->up);
    math::Position3 muzzle;
    CalcMuzzlePoint(ent, &muzzle);
    math::Position3 end;
    end.v.m128_f32[0] = wp->forward[0] * 100.0f + muzzle.v.m128_f32[0];
    end.v.m128_f32[1] = wp->forward[1] * 100.0f + muzzle.v.m128_f32[1];
    end.v.m128_f32[2] = wp->forward[2] * 100.0f + muzzle.v.m128_f32[2];
    wp->muzzleTrace[0] = muzzle.v.m128_f32[0];
    wp->muzzleTrace[1] = muzzle.v.m128_f32[1];
    wp->muzzleTrace[2] = muzzle.v.m128_f32[2];
    DbLinkedHandle<EntityHandleDb, Entity> entityList[128];
    math::Position3 mins;
    math::Position3 maxs;
    mins.v.m128_f32[0] = ent->r.absmin.v.m128_f32[0] + wp->forward[0] * 30.0f - 20.0f;
    mins.v.m128_f32[1] = ent->r.absmin.v.m128_f32[1] + wp->forward[1] * 30.0f - 20.0f;
    mins.v.m128_f32[2] = ent->r.absmin.v.m128_f32[2] + wp->forward[2] * 30.0f - 20.0f;
    maxs.v.m128_f32[0] = ent->r.absmax.v.m128_f32[0] + wp->forward[0] * 30.0f + 20.0f;
    maxs.v.m128_f32[1] = ent->r.absmax.v.m128_f32[1] + wp->forward[1] * 30.0f + 20.0f;
    maxs.v.m128_f32[2] = ent->r.absmax.v.m128_f32[2] + wp->forward[2] * 30.0f + 20.0f;
    int v35 = CM_AreaEntities(mins, maxs, entityList, 128, 0x4000000);
    math::Position3 probe;
    probe.v.m128_f32[0] = wp->forward[0] * 45.0f + wp->muzzleTrace[0];
    probe.v.m128_f32[1] = wp->forward[1] * 45.0f + wp->muzzleTrace[1];
    probe.v.m128_f32[2] = wp->forward[2] * 45.0f + wp->muzzleTrace[2];
    Entity* found = nullptr;
    for (int v20 = 0; v20 < v35; ++v20)
    {
        Entity* mObject = HandleDbToEnt(entityList[v20]);
        if (mObject == nullptr)
            continue;
        if ((mObject->client == nullptr && mObject->actor == nullptr) || ent == mObject)
            continue;
        __m128 clamped = _mm_min_ps(_mm_max_ps(probe.v, mObject->r.absmin.v),
                                    mObject->r.absmax.v);
        float dx = probe.v.m128_f32[0] - clamped.m128_f32[0];
        float dy = probe.v.m128_f32[1] - clamped.m128_f32[1];
        float dz = probe.v.m128_f32[2] - clamped.m128_f32[2];
        if (radius * radius > dx * dx + dy * dy + dz * dz)
        {
            found = mObject;
            break;
        }
    }
    collision_context_t context;
    context.__vftable = nullptr;
    context.pass_entity1.mHandle.mVal = ent->mHandle.mHandle.mVal;
    context.pass_entity2.mHandle.mVal = 0;
    context.pass_owner1.mHandle.mVal = 0;
    context.pass_owner2.mHandle.mVal = 0;
    context.contentmask = 0x4000000;
    math::Position3 zeroA;
    math::Position3 zeroB;
    zeroA.v = _mm_setzero_ps();
    zeroB.v = _mm_setzero_ps();
    trace_t trace;
    if (found != nullptr)
    {
        math::Position3 closest;
        closest.v = _mm_min_ps(_mm_max_ps(probe.v, found->r.absmin.v),
                               found->r.absmax.v);
        SV_Trace(&trace, &muzzle, &zeroA, &zeroB, &closest, &context, 0, 1,
                 bulletPriorityMap, 1, 0.0f);
        if (trace.mEntity.mHandle.mVal == found->mHandle.mHandle.mVal
            || trace.fraction == 1.0f)
        {
            trace.fraction = 0.5f;
            trace.endpos.v = closest.v;
            trace.mEntity.mHandle.mVal = found->mHandle.mHandle.mVal;
        }
    }
    else
    {
        SV_Trace(&trace, &muzzle, &zeroA, &zeroB, &end, &context, 0, 1,
                 bulletPriorityMap, 1, 0.0f);
    }
    unsigned int v27 = trace.mEntity.mHandle.mVal;
    Entity* v30 = HandleDbToEnt(*(DbLinkedHandle<EntityHandleDb, Entity>*)&v27);
    if (v30 != nullptr
        && v30->client != nullptr
        && v30->sentient != nullptr
        && ent->sentient != nullptr
        && v30->client->pers.playerState == 4
        && v30->sentient->eTeam == ent->sentient->eTeam)
    {
        if (traceEnt != nullptr)
            *traceEnt = v30;
        return true;
    }
    ent->client->mMedicNobodyToReviveTime = level.time;
    return false;
}

// ea: 0x0047B500
Entity* fire_rocket(Entity* self, float* start, float* dir, float lifetime)
{
    VectorNormalize(dir);
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(self->s.weapon);
    TPakId mPakId = (TPakId)self->mPakId;
    if (mPakId == PAK_ID_INVALID)
        mPakId = CurPakId();
    Entity* v6 = G_Spawn(mPakId);
    v6->mClassName = str_const.rocket;
    v6->mClassNameHash.mHash = HashString::CalcHash(v6->mClassName.GetBuff());
    v6->think = THINK__G_ExplodeMissile;
    v6->s.eType = 3;
    v6->r.svFlags = 160;
    v6->nextthink = (int)(lifetime * 1000.0f) + level.time;
    v6->s.eFlags |= 0x8000;
    v6->s.weapon = self->s.weapon;
    v6->timestamp = level.time;
    v6->r.mOwner.mHandle.mVal = self->mHandle.mHandle.mVal;
    v6->parentHandle.mHandle.mVal = self->mHandle.mHandle.mVal;
    v6->damage = InfoForWeapon->iDamage;
    v6->methodOfDeath = 9;
    v6->splashMethodOfDeath = 10;
    v6->clipmask = 41951377;
    v6->s.pos.trType = TR_LINEAR;
    v6->s.pos.trTime = level.time - 50;
    v6->s.pos.trBase[0] = start[0];
    v6->s.pos.trBase[1] = start[1];
    v6->s.pos.trBase[2] = start[2];
    v6->s.pos.trDelta[0] = InfoForWeapon->iProjectileSpeed * dir[0];
    v6->s.pos.trDelta[1] = InfoForWeapon->iProjectileSpeed * dir[1];
    v6->s.pos.trDelta[2] = InfoForWeapon->iProjectileSpeed * dir[2];
    if (IS_NAN(v6->r.currentOrigin.v.m128_f32[0])
        || IS_NAN(v6->r.currentOrigin.v.m128_f32[1])
        || IS_NAN(v6->r.currentOrigin.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_missile.cpp";
        AeAssert::gCurrentLine = 1596;
        AeAssert::gCurrentExpr = "!IS_NAN((bolt->r.currentOrigin)[0]) && !IS_NAN((bolt->r.currentOrigin)[1]) && !IS_NAN((bolt->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    v6->r.currentOrigin.v.m128_f32[0] = start[0];
    v6->r.currentOrigin.v.m128_f32[1] = start[1];
    v6->r.currentOrigin.v.m128_f32[2] = start[2];
    vectoangles(dir, v6->r.currentAngles.v.m128_f32);
    G_SetAngle(v6, &v6->r.currentAngles);
    ValidatePakId((TPakId)v6->mModel.mPakId);
    if (v6->mModel.mValue != nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_missile.cpp";
        AeAssert::gCurrentLine = 1602;
        AeAssert::gCurrentExpr = "!bolt->mModel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    TPakId v13 = (TPakId)v6->mPakId;
    if (v13 == PAK_ID_INVALID)
        v13 = CurPakId();
    v6->mModel = XModelManager::sInst->GetXModel(v13, InfoForWeapon->szProjectileModel);
    if (InfoForWeapon->type == 2 /* WEAPTYPE_PROJECTILE */)
        v6->key = PostEffectEventWeapon(v6, InfoForWeapon->szInternalName,
                                        0x40 /* kActionEI_MELEE_PLAYER_LOSING|kActionWEAPON_FIRE_3RD */).mVal;
    if (self->scr_vehicle == nullptr)
        goto label_26;
    Entity* mObject = HandleDbToEnt(self->r.mOwner);
    if (mObject == nullptr)
        goto label_26;
    if (mObject->sentient != nullptr)
    {
        HandleDbToEnt(self->r.mOwner)->sentient->lastShotTime = cgGlobal.time;
        G_DObjUpdate(v6, false);
        return v6;
    }
label_26:
    if (self->sentient != nullptr)
        self->sentient->lastShotTime = cgGlobal.time;
    G_DObjUpdate(v6, false);
    return v6;
}

// ea: 0x0047B0E0
Entity* fire_mine(Entity* self, float* position, float* dir, int weapon)
{
    Entity* v6 = G_Spawn(CurPakId());
    v6->think = THINK__Think_GeneralLink;
    v6->nextthink = level.time + 10;
    v6->s.eType = 3;
    v6->r.svFlags = 160;
    v6->s.weapon = weapon;
    v6->r.mOwner.mHandle.mVal = self->mHandle.mHandle.mVal;
    v6->parentHandle.mHandle.mVal = self->mHandle.mHandle.mVal;
    v6->touch = 12;
    v6->die = 6;
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(weapon);
    float iTriggerRadius = (float)InfoForWeapon->iTriggerRadius;
    v6->r.mins.v.m128_f32[0] = -iTriggerRadius;
    v6->r.mins.v.m128_f32[1] = -iTriggerRadius;
    v6->r.mins.v.m128_f32[2] = -iTriggerRadius;
    v6->r.maxs.v.m128_f32[0] = iTriggerRadius;
    v6->r.maxs.v.m128_f32[1] = iTriggerRadius;
    v6->r.maxs.v.m128_f32[2] = iTriggerRadius;
    v6->mClassName = "mine";
    v6->mClassNameHash.mHash = HashString::CalcHash(v6->mClassName.GetBuff());
    v6->damage = InfoForWeapon->iDamage;
    v6->methodOfDeath = 5;
    v6->splashMethodOfDeath = 6;
    v6->s.eFlags = (InfoForWeapon->bNoTumble != 0 ? 0x20000000 : 0) | v6->s.eFlags;
    v6->clipmask = 41951377;
    v6->r.contents = 1073741832;
    v6->takedamage = 1;
    v6->health = 20;
    v6->s.pos.trType = TR_STATIONARY;
    v6->s.pos.trTime = level.time;
    v6->s.pos.trBase[0] = position[0];
    v6->s.pos.trBase[1] = position[1];
    v6->s.pos.trBase[2] = position[2];
    v6->s.apos.trType = TR_STATIONARY;
    v6->timestamp = level.time;
    v6->s.apos.trTime = level.time;
    vectoangles(dir, v6->r.currentAngles.v.m128_f32);
    v6->r.currentAngles.v.m128_f32[0] += 90.0f;
    G_SetAngle(v6, &v6->r.currentAngles);
    if (IS_NAN(v6->r.currentOrigin.v.m128_f32[0])
        || IS_NAN(v6->r.currentOrigin.v.m128_f32[1])
        || IS_NAN(v6->r.currentOrigin.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_missile.cpp";
        AeAssert::gCurrentLine = 1533;
        AeAssert::gCurrentExpr = "!IS_NAN((bolt->r.currentOrigin)[0]) && !IS_NAN((bolt->r.currentOrigin)[1]) && !IS_NAN((bolt->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    v6->r.currentOrigin.v.m128_f32[0] = position[0];
    v6->r.currentOrigin.v.m128_f32[1] = position[1];
    v6->r.currentOrigin.v.m128_f32[2] = position[2];
    v6->r.currentAngles.v.m128_f32[0] = v6->s.apos.trBase[0];
    v6->r.currentAngles.v.m128_f32[1] = v6->s.apos.trBase[1];
    v6->r.currentAngles.v.m128_f32[2] = v6->s.apos.trBase[2];
    ValidatePakId((TPakId)v6->mModel.mPakId);
    if (v6->mModel.mValue != nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_missile.cpp";
        AeAssert::gCurrentLine = 1537;
        AeAssert::gCurrentExpr = "!bolt->mModel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    TPakId v16 = (TPakId)v6->mPakId;
    if (v16 == PAK_ID_INVALID)
        v16 = CurPakId();
    v6->mModel = XModelManager::sInst->GetXModel(v16, InfoForWeapon->szProjectileModel);
    ValidatePakId((TPakId)v6->mModel.mPakId);
    if (v6->mModel.mValue == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_missile.cpp";
        AeAssert::gCurrentLine = 1539;
        AeAssert::gCurrentExpr = "bolt->mModel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    G_DObjUpdate(v6, false);
    g_LinkEntity(v6);
    return v6;
}

// ea: 0x0047B8A0
Entity* fire_artillery(Entity* i_Self, float* i_StrikePoint, int i_Delay)
{
    int weapon = i_Self->s.weapon;
    float dir[3] = { 0.0f, 0.0f, -1.0f };
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(weapon);
    TPakId mPakId = (TPakId)i_Self->mPakId;
    if (mPakId == PAK_ID_INVALID)
        mPakId = CurPakId();
    Entity* v7 = G_Spawn(mPakId);
    v7->mClassName = str_const.rocket;
    v7->mClassNameHash.mHash = HashString::CalcHash(v7->mClassName.GetBuff());
    v7->think = THINK__G_LaunchMissile;
    v7->s.eType = 3;
    v7->r.svFlags = 160;
    v7->nextthink = (level.time - InfoForWeapon->iProjectileDelay) + i_Delay + 250;
    v7->s.eFlags |= 0x8000;
    v7->s.weapon = i_Self->s.weapon;
    v7->r.mOwner.mHandle.mVal = i_Self->mHandle.mHandle.mVal;
    v7->parentHandle.mHandle.mVal = i_Self->mHandle.mHandle.mVal;
    v7->damage = InfoForWeapon->iDamage;
    v7->methodOfDeath = 17;
    v7->splashMethodOfDeath = 18;
    v7->clipmask = 41951377;
    v7->s.pos.trType = TR_STATIONARY;
    v7->s.pos.trBase[0] = i_StrikePoint[0];
    v7->s.pos.trBase[1] = i_StrikePoint[1];
    v7->s.pos.trBase[2] = i_StrikePoint[2];
    v7->s.pos.trDelta[0] = InfoForWeapon->iProjectileSpeed * dir[0];
    v7->s.pos.trDelta[1] = InfoForWeapon->iProjectileSpeed * dir[1];
    v7->s.pos.trDelta[2] = InfoForWeapon->iProjectileSpeed * dir[2];
    v7->r.currentOrigin.v.m128_f32[0] = i_StrikePoint[0];
    v7->r.currentOrigin.v.m128_f32[1] = i_StrikePoint[1];
    v7->r.currentOrigin.v.m128_f32[2] = i_StrikePoint[2];
    vectoangles(dir, v7->r.currentAngles.v.m128_f32);
    G_SetAngle(v7, &v7->r.currentAngles);
    if (InfoForWeapon->szProjectileModel[0] != 0)
    {
        ValidatePakId((TPakId)v7->mModel.mPakId);
        if (v7->mModel.mValue != nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_missile.cpp";
            AeAssert::gCurrentLine = 1682;
            AeAssert::gCurrentExpr = "!bolt->mModel";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        TPakId v14 = (TPakId)v7->mPakId;
        if (v14 == PAK_ID_INVALID)
            v14 = CurPakId();
        v7->mModel = XModelManager::sInst->GetXModel(v14, InfoForWeapon->szProjectileModel);
        ValidatePakId((TPakId)v7->mModel.mPakId);
        if (v7->mModel.mValue == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_missile.cpp";
            AeAssert::gCurrentLine = 1684;
            AeAssert::gCurrentExpr = "bolt->mModel";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
    }
    G_DObjUpdate(v7, false);
    return v7;
}

// ea: 0x00481B60
void Weapon_Artillery_Fire(Entity* ent, float spread, weaponParms* wp,
                           float lifetime)
{
    float dir[3];
    dir[0] = tan(spread * 3.1415927f * 0.0055555557f) * 16.0f;
    gunrandom(&dir[2], &dir[1]);
    float v5 = wp->forward[1] * 16.0f;
    float v6 = wp->forward[0] * 16.0f;
    float v7 = wp->forward[2] * 16.0f;
    dir[2] = dir[2] * dir[0];
    float v9 = (wp->right[0] * dir[2]) + v6;
    float v10 = (wp->right[1] * dir[2]) + v5;
    float v11 = wp->right[2] * dir[2];
    float launchpos[3];
    launchpos[0] = (wp->up[0] * (dir[1] * dir[0])) + v9;
    launchpos[1] = (wp->up[1] * (dir[1] * dir[0])) + v10;
    launchpos[2] = (wp->up[2] * (dir[1] * dir[0])) + (v11 + v7);
    dir[1] = dir[1] * dir[0];
    VectorNormalize(launchpos);
    float start[3] = { wp->muzzleTrace[0], wp->muzzleTrace[1],
                       wp->muzzleTrace[2] };
    fire_rocket(ent, start, launchpos, lifetime)->s.pos.trType = TR_GRAVITY;
    Client* client = ent->client;
    if (client != nullptr)
    {
        client->ps.velocity.v.m128_f32[0] -= wp->forward[0] * 64.0f;
        ent->client->ps.velocity.v.m128_f32[1] -= wp->forward[1] * 64.0f;
        ent->client->ps.velocity.v.m128_f32[2] -= wp->forward[2] * 64.0f;
    }
    math::Position3 v17;
    v17.v.m128_f32[0] = start[0];
    v17.v.m128_f32[1] = start[1];
    v17.v.m128_f32[2] = start[2];
    v17.v.m128_f32[3] = 0.0f;
    math::Dir3 v16;
    v16.v.m128_f32[0] = launchpos[0];
    v16.v.m128_f32[1] = launchpos[1];
    v16.v.m128_f32[2] = launchpos[2];
    v16.v.m128_f32[3] = 0.0f;
    if (ent->s.eType == 14)
    {
        MultiplayerMgr::sInst->VehicleFireMissile(ent, ent->s.weapon, &v17, &v16);
    }
    else
    {
        MultiplayerMgr::sInst->FireMissile(ent->s.weapon, v17, v16,
                                           MultiplayerMgr::MPEntityHandle());
    }
}

// ea: 0x0045FE90
void Weapon_ArtilleryStrike_Fire(Entity* ent, float spread, weaponParms* wp)
{
    float angle[5];
    Bullet_Endpos(spread, &angle[2], wp, 0.0f, 0.0f);
    if ((ent->client->ps.pm_flags & 1) == 0)
        wp->muzzleTrace[2] += 16.0f;
    float v9, v10, v11;
    if (EntityManager::sInst->IsLocalPlayer(ent))
    {
        int v4 = 1580 * EntityManager::sInst->GetPlayerIndex(ent);
        float v5 = wp->forward[0];
        float v6 = wp->forward[1];
        float v7 = wp->forward[2];
        wp->muzzleTrace[0] = dword_F63C70[v4];
        wp->muzzleTrace[1] = dword_F63C70[v4 + 1];
        float v8 = dword_F63C70[v4 + 2];
        v9 = (v5 * 8192.0f) + wp->muzzleTrace[0];
        v10 = (v6 * 8192.0f) + wp->muzzleTrace[1];
        wp->muzzleTrace[2] = v8;
        v11 = (v7 * 8192.0f) + v8;
    }
    else
    {
        v11 = angle[4];
        v10 = angle[3];
        v9 = angle[2];
    }
    math::Position3 end;
    end.v.m128_f32[0] = v9;
    end.v.m128_f32[1] = v10;
    end.v.m128_f32[2] = v11;
    end.v.m128_f32[3] = 0.0f;
    math::Position3 start;
    start.v.m128_f32[0] = wp->muzzleTrace[0];
    start.v.m128_f32[1] = wp->muzzleTrace[1];
    start.v.m128_f32[2] = wp->muzzleTrace[2];
    start.v.m128_f32[3] = 0.0f;
    collision_context_t context;
    context.__vftable = nullptr;
    context.pass_entity1.mHandle.mVal = 0;
    context.pass_entity2.mHandle.mVal = 41951283;
    context.pass_owner1.mHandle.mVal = 0;
    context.pass_owner2.mHandle.mVal = 0;
    context.contentmask = 0x2802033;
    math::Position3 zeroA;
    math::Position3 zeroB;
    zeroA.v = _mm_setzero_ps();
    zeroB.v = _mm_setzero_ps();
    trace_t trace;
    SV_Trace(&trace, &start, &zeroA, &zeroB, &end, &context, 0, 1,
             bulletPriorityMap, 1, 0.0f);
    if (trace.fraction >= 1.0f
        || (trace.surfaceFlags & 4) != 0)
    {
        int WeaponForInfo = BG_GetWeaponForInfo(wp->pWeapInfo);
        Add_Ammo(ent, WeaponForInfo, 1, 0);
        return;
    }
    float dist2 = 0.0f;
    float dx = trace.endpos.v.m128_f32[0] - ent->r.currentOrigin.v.m128_f32[0];
    float dy = trace.endpos.v.m128_f32[1] - ent->r.currentOrigin.v.m128_f32[1];
    float dz = trace.endpos.v.m128_f32[2] - ent->r.currentOrigin.v.m128_f32[2];
    dist2 = dx * dx + dy * dy + dz * dz;
    if (dist2 <= 202500.0f)
    {
        int WeaponForInfo = BG_GetWeaponForInfo(wp->pWeapInfo);
        Add_Ammo(ent, WeaponForInfo, 1, 0);
        return;
    }
    int angleSeed = irand(0, 360);
    float forward[3];
    AnglesToForward((const float*)&angleSeed, forward);
    weaponFileInfo_t* pWeapInfo = wp->pWeapInfo;
    float v17 = (rand() * 0.000061035156f - 1.0f)
                * (pWeapInfo->iProjectileRadius * 0.2f);
    math::Position3 position;
    position.v.m128_f32[0] = (v17 * forward[0]) + trace.endpos.v.m128_f32[0];
    position.v.m128_f32[1] = (v17 * forward[1]) + trace.endpos.v.m128_f32[1];
    position.v.m128_f32[2] = (v17 * forward[2]) + trace.endpos.v.m128_f32[2];
    position.v.m128_f32[3] = 0.0f;
    MultiplayerMgr::sInst->FireArtillery(ent, pWeapInfo->index, &position,
                                         level.time, false);
}

// ea: 0x0048DAE0
void FireWeapon(Entity* ent)
{
    if (((0x106000 & ent->client->ps.eFlags) == 0 || ent->active == 0)
        && EntityManager::sInst->IsLocalPlayer(ent))
    {
        int weapon = ent->s.weapon;
        ent->invulnerability_timeout = 0;
        weaponParms wp;
        wp.pWeapInfo = BG_GetInfoForWeapon(weapon);
        CalcMuzzlePoints(ent, &wp);
        Client* client = ent->client;
        float aimSpreadScale = client->currentAimSpreadScale;
        bool bAds = client->ps.fWeaponPosFrac == 1.0f;
        float MinSpreadForWeapon = BG_GetMinSpreadForWeapon(
            &client->ps, ent->s.weapon, level.time, bAds);
        int type = wp.pWeapInfo->type;
        aimSpreadScale = (*(float*)((char*)wp.pWeapInfo + 0x674)
                          - MinSpreadForWeapon)
                             * aimSpreadScale
                         + MinSpreadForWeapon;
        switch (type)
        {
        case 0:
        {
            float coneAngleTangent = BG_GetConeAngleForWeapon(
                &ent->client->ps, ent->s.weapon, level.time, bAds);
            Bullet_Fire(ent, aimSpreadScale, wp.pWeapInfo->iDamage, &wp, ent,
                        coneAngleTangent);
            ent->client->ps.mLastFireWeaponTime = level.time;
            ent->client->ps.mLastFireWeapon = ent->s.weapon;
            break;
        }
        case 1:
        {
            Client* v6 = ent->client;
            if ((0x100000 & v6->ps.eFlags) == 0 || v6->ps.vehPos != 7)
                goto launcher;
            Entity* v8 = HandleDbToEnt(ent->r.mOwner);
            if (v8 == nullptr || v8->scr_vehicle == nullptr)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
                AeAssert::gCurrentLine = 2278;
                AeAssert::gCurrentExpr = "vehicle && vehicle->scr_vehicle";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Player is not correctly attached to vehicle."))
                    __debugbreak();
            }
            if (v8->scr_vehicle->mMantleTime == 0)
                MultiplayerMgr::sInst->VehicleMantled(v8, ent);
            break;
        }
        case 2:
launcher:
            if (wp.pWeapInfo->weapClass == 5 /* WEAPCLASS_GRENADE */)
                weapon_grenadelauncher_fire(ent, ent->s.weapon, &wp);
            else
                Weapon_RocketLauncher_Fire(ent, aimSpreadScale, &wp, 10.0f,
                                           true);
            break;
        case 3:
            Weapon_ArtilleryStrike_Fire(ent, aimSpreadScale, &wp);
            break;
        case 4:
            switch (wp.pWeapInfo->weapClass)
            {
            case 1 /* WEAPCLASS_HEALTH */:
                Weapon_ItemHealth_Fire(ent, ent->s.weapon, &wp);
                break;
            case 2 /* WEAPCLASS_AMMO */:
                Weapon_ItemAmmo_Fire(ent, ent->s.weapon, &wp);
                break;
            case 3 /* WEAPCLASS_REVIVE */:
                Weapon_Revive(ent, ent->s.weapon, &wp);
                break;
            default:
                break;
            }
            break;
        case 5:
            return;
        case 7:
            if (weapon_mine_fire(ent, ent->s.weapon, &wp) == nullptr)
            {
                Add_Ammo(ent, ent->s.weapon, 1, 0);
                if (gpBrocAPI->mBrocExports.mCallbackMineFailed != nullptr)
                    gpBrocAPI->mBrocExports.mCallbackMineFailed(
                        ent->mHandle.mHandle.mVal);
            }
            break;
        case 8:
            if (gpBrocAPI->mBrocExports.mCallbackDropFlag != nullptr)
            {
                MultiplayerMgr::sInst->AnimEvent(20);
                gpBrocAPI->mBrocExports.mCallbackDropFlag(
                    ent->mHandle.mHandle.mVal);
            }
            break;
        default:
            G_Error("Unknown weapon type %i for %s\n", type,
                    wp.pWeapInfo->szInternalName);
            break;
        }
    }
}

// ea: 0x0048D730
void G_BulletFireSpread(const Entity* source, Entity* attacker,
                        const weaponParms* wp, int damage, float spread,
                        Entity* weaponEnt, float coneAngleTangent, int seed)
{
    if (source == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 628;
        AeAssert::gCurrentExpr = "source";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("G_BulletFireSpread: No source entity"))
            __debugbreak();
    }
    if (attacker == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 629;
        AeAssert::gCurrentExpr = "attacker";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("G_BulletFireSpread: No attacker entity"))
            __debugbreak();
    }
    if (wp == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 630;
        AeAssert::gCurrentExpr = "wp";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("G_BulletFireSpread: No weapon params"))
            __debugbreak();
    }
    if (wp->pWeapInfo == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 631;
        AeAssert::gCurrentExpr = "wp->pWeapInfo";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("G_BulletFireSpread: No no weapon info"))
            __debugbreak();
    }
    float start[3] = { wp->muzzleTrace[0], wp->muzzleTrace[1],
                       wp->muzzleTrace[2] };
    bdRandomState rng;
    bdRandom_setSeed(&rng, seed);
    for (int i = 0; i < wp->pWeapInfo->iShotCount; ++i)
    {
        float randomA = (float)bdRandom_nextUInt(&rng) * 4.6566129e-10f;
        float randomB = (float)bdRandom_nextUInt(&rng) * 4.6566129e-10f;
        float end[3];
        Bullet_Endpos(spread, end, wp, randomA, randomB);
        unsigned int mVal = weaponEnt != nullptr
                                ? weaponEnt->mHandle.mHandle.mVal
                                : EntityManager::sInst->mWorld->mHandle.mHandle.mVal;
        Bullet_Fire_Extended(*(DbLinkedHandle<EntityHandleDb, Entity>*)&mVal,
                             attacker, start, end, damage, 0, wp,
                             *(DbLinkedHandle<EntityHandleDb, Entity>*)&mVal,
                             coneAngleTangent);
    }
}

// ea: 0x0045FB40
void Weapon_ItemAmmo_Fire(Entity* ent, int grenType, weaponParms* wp)
{
    weaponFileInfo_t* pWeapInfo = wp->pWeapInfo;
    float iProjectileSpeedUp = (float)pWeapInfo->iProjectileSpeedUp;
    float iProjectileSpeed = (float)pWeapInfo->iProjectileSpeed;
    float vTossDir[3];
    vTossDir[0] = iProjectileSpeed * wp->forward[0];
    vTossDir[1] = iProjectileSpeed * wp->forward[1];
    vTossDir[2] = (iProjectileSpeed * wp->forward[2]) + iProjectileSpeedUp;
    float tossPos[3] = {vTossDir[0], vTossDir[1], vTossDir[2]};
    VectorNormalize(tossPos);
    Client* client = ent->client;
    float v14 = (client->ps.velocity.v.m128_f32[0] * tossPos[0])
                + (client->ps.velocity.v.m128_f32[1] * tossPos[1])
                + (client->ps.velocity.v.m128_f32[2] * tossPos[2]);
    vTossDir[0] += v14 * tossPos[0];
    vTossDir[1] += v14 * tossPos[1];
    vTossDir[2] += v14 * tossPos[2];
    math::Position3 tossPos3;
    tossPos3.v.m128_f32[0] = wp->muzzleTrace[0] + wp->forward[0] * 20.0f;
    tossPos3.v.m128_f32[1] = wp->muzzleTrace[1] + wp->forward[1] * 20.0f;
    tossPos3.v.m128_f32[2] = wp->muzzleTrace[2] + wp->forward[2] * 20.0f;
    tossPos3.v.m128_f32[3] = 0.0f;
    float maxs[3] = {-1.0f, -1.0f, 2.0f};
    float mins[3] = {1.0f, 1.0f, 2.0f};
    collision_context_t context;
    context.__vftable = nullptr;
    context.pass_entity1.mHandle.mVal = 0;
    context.pass_entity2.mHandle.mVal = 41951377;
    context.pass_owner1.mHandle.mVal = ent->mHandle.mHandle.mVal;
    context.pass_owner2.mHandle.mVal = 0;
    context.contentmask = 0;
    float startZ = ent->r.currentOrigin.v.m128_f32[2]
                   + ent->r.maxs.v.m128_f32[2];
    if (context.contentmask > startZ - 4.0f)
        context.contentmask = (int)(startZ - 4.0f);
    math::Position3 start;
    start.v.m128_f32[0] = ent->r.currentOrigin.v.m128_f32[0];
    start.v.m128_f32[1] = ent->r.currentOrigin.v.m128_f32[1];
    start.v.m128_f32[2] = ent->r.currentOrigin.v.m128_f32[2];
    start.v.m128_f32[3] = 0.0f;
    trace_t trace;
    SV_Trace(&trace, &start, (math::Position3*)mins, (math::Position3*)maxs,
             &tossPos3, &context, 0, 0, nullptr, 0, 0.0f);
    math::Position3 vTossVel;
    vTossVel.v = _mm_setzero_ps();
    vTossVel.v.m128_f32[0] = vTossDir[0];
    vTossVel.v.m128_f32[1] = vTossDir[1];
    vTossVel.v.m128_f32[2] = vTossDir[2];
    math::Position3 vTossAngles;
    vTossAngles.v.m128_f32[0] = ent->r.currentAngles.v.m128_f32[0];
    vTossAngles.v.m128_f32[1] = ent->r.currentAngles.v.m128_f32[1];
    vTossAngles.v.m128_f32[2] = ent->r.currentAngles.v.m128_f32[2];
    math::Position3 vTossOrigin;
    vTossOrigin.v = _mm_setzero_ps();
    vTossOrigin.v.m128_f32[0] = trace.endpos.v.m128_f32[0];
    vTossOrigin.v.m128_f32[1] = trace.endpos.v.m128_f32[1];
    vTossOrigin.v.m128_f32[2] = trace.endpos.v.m128_f32[2];
    MultiplayerMgr::MPEntityHandle v29;
    MultiplayerMgr::sInst->GetNextDroppedItemID(&v29, 1 /* kItemTypeSupport */,
                                                ent);
    MultiplayerMgr::sInst->DropItem(1, &vTossOrigin,
                                    (const math::Dir3*)&vTossAngles,
                                    (const math::Dir3*)&vTossVel,
                                    v29.mVal, false, -1);
}

// ea: 0x0047A760
Entity* fire_grenade(Entity* self, float* start, float* dir, int grenadeWPID,
                     int time)
{
    Entity* v9 = G_Spawn(CurPakId());
    Entity* v8 = self;
    if (self != nullptr && self->client != nullptr
        && self->client->ps.grenadeTimeLeft != 0)
    {
        v9->nextthink = level.time + self->client->ps.grenadeTimeLeft;
        v8->client->ps.grenadeTimeLeft = 0;
    }
    else
    {
        v9->nextthink = time + level.time;
    }
    v9->think = THINK__G_ExplodeMissile;
    v9->s.eType = 3;
    v9->r.svFlags = 160;
    v9->s.weapon = grenadeWPID;
    unsigned int mVal = v8 != nullptr ? v8->mHandle.mHandle.mVal : 0;
    v9->r.mOwner.mHandle.mVal = mVal;
    v9->parentHandle.mHandle.mVal = mVal;
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(grenadeWPID);
    Broc::string* className =
        InfoForWeapon->slot == 5 /* WEAPSLOT_SMOKE_GRENADE */
            ? &str_const.smoke_grenade
            : &str_const.grenade;
    v9->mClassName = *className;
    v9->mClassNameHash.mHash = HashString::CalcHash(v9->mClassName.GetBuff());
    v9->damage = InfoForWeapon->iDamage;
    v9->methodOfDeath = 4 * (InfoForWeapon->slot == 5) + 3;
    v9->splashMethodOfDeath = 4 * (InfoForWeapon->slot == 5) + 4;
    v9->s.eFlags = InfoForWeapon->bNoBounce != 0 ? 0x800000 : 50331648;
    v9->s.pos.trType = TR_GRAVITY;
    v9->clipmask = 41951377;
    v9->s.eFlags = v9->s.eFlags
                   | (-(InfoForWeapon->bNoTumble != 0) & 0x20000000);
    v9->s.pos.trTime = level.time;
    v9->s.pos.trBase[0] = start[0];
    v9->s.pos.trBase[1] = start[1];
    v9->s.pos.trBase[2] = start[2];
    v9->s.pos.trDelta[0] = dir[0];
    v9->s.pos.trDelta[1] = dir[1];
    v9->s.pos.trDelta[2] = dir[2];
    v9->timestamp = level.time;
    v9->s.apos.trType = TR_LINEAR;
    v9->s.apos.trTime = level.time;
    vectoangles(dir, v9->s.apos.trBase);
    if (InfoForWeapon->bNoTumble == 0
        && InfoForWeapon->type == 1 /* WEAPTYPE_GRENADE */)
    {
        double v20 = AngleNormalize360(v9->s.apos.trBase[0] - 120.0f);
        v9->s.apos.trBase[0] = (float)v20;
        flrand(-45.0f, 45.0f);
        v9->s.apos.trDelta[0] = (float)(v20 + 720.0);
    }
    v9->s.apos.trDelta[1] = 0.0f;
    if (InfoForWeapon->bNoTumble != 0)
    {
        v9->s.apos.trBase[0] = 0.0f;
    }
    else
    {
        float a1 = flrand(-45.0f, 45.0f);
        v9->s.apos.trDelta[2] = a1 + 360.0f;
    }
    if (IS_NAN(v9->r.currentOrigin.v.m128_f32[0])
        || IS_NAN(v9->r.currentOrigin.v.m128_f32[1])
        || IS_NAN(v9->r.currentOrigin.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_missile.cpp";
        AeAssert::gCurrentLine = 1337;
        AeAssert::gCurrentExpr = "!IS_NAN((bolt->r.currentOrigin)[0]) && !IS_NAN((bolt->r.currentOrigin)[1]) && !IS_NAN((bolt->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    v9->r.currentOrigin.v.m128_f32[0] = start[0];
    v9->r.currentOrigin.v.m128_f32[1] = start[1];
    v9->r.currentOrigin.v.m128_f32[2] = start[2];
    v9->r.currentAngles.v.m128_f32[0] = v9->s.apos.trBase[0];
    v9->r.currentAngles.v.m128_f32[1] = v9->s.apos.trBase[1];
    v9->r.currentAngles.v.m128_f32[2] = v9->s.apos.trBase[2];
    v9->key = PostEffectEventWeapon(v9, InfoForWeapon->szInternalName,
                                    0x40).mVal;
    ValidatePakId((TPakId)v9->mModel.mPakId);
    if (v9->mModel.mValue != nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_missile.cpp";
        AeAssert::gCurrentLine = 1344;
        AeAssert::gCurrentExpr = "!bolt->mModel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    TPakId mPakId = (TPakId)v9->mPakId;
    if (mPakId == PAK_ID_INVALID)
        mPakId = CurPakId();
    v9->mModel = XModelManager::sInst->GetXModel(
        mPakId, InfoForWeapon->szProjectileModel);
    ValidatePakId((TPakId)v9->mModel.mPakId);
    if (v9->mModel.mValue == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_missile.cpp";
        AeAssert::gCurrentLine = 1346;
        AeAssert::gCurrentExpr = "bolt->mModel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    G_DObjUpdate(v9, false);
    g_femanager.IGO->AddActiveGrenade(v9);
    return v9;
}

// ea: 0x0047ABC0
Entity* fire_rifle_grenade(Entity* self, const float* target, int grenadeWPID,
                           bool checkTrajectory)
{
    TPakId mPakId = (TPakId)self->mPakId;
    if (mPakId == PAK_ID_INVALID)
        mPakId = CurPakId();
    Entity* v6 = G_Spawn(mPakId);
    unsigned char WeaponIndexForName =
        BG_GetWeaponIndexForName(self->actor->mWeaponName);
    weaponParms wp;
    wp.pWeapInfo = BG_GetInfoForWeapon(WeaponIndexForName);
    j_nullsub_37(self->actor, &wp);
    float start[3] = {wp.muzzleTrace[0], wp.muzzleTrace[1], wp.muzzleTrace[2]};
    float time = VectorDistance(start, target) * 0.001f;
    int v9 = Cvar_VariableIntegerValue("g_gravity");
    float dir[3];
    dir[0] = (target[0] - start[0]) * (1.0f / time);
    dir[1] = (target[1] - start[1]) * (1.0f / time);
    float drop = -v9 * (time * time) * 0.5f;
    dir[2] = ((target[2] - start[2]) - drop) * (1.0f / time);
    if (!checkTrajectory
        || Actor_Grenade_IsValidTrajectory(self->actor, start, dir,
                                           target) != 0)
    {
        Client* client = self->client;
        if (client != nullptr && client->ps.grenadeTimeLeft != 0)
        {
            v6->nextthink = level.time + client->ps.grenadeTimeLeft;
            self->client->ps.grenadeTimeLeft = 0;
        }
        else
        {
            v6->nextthink = (int)(time * 1000.0f) + level.time;
        }
        v6->think = THINK__G_ExplodeMissile;
        v6->s.eType = 3;
        v6->r.svFlags = 160;
        v6->s.weapon = grenadeWPID;
        v6->r.mOwner.mHandle.mVal = self->mHandle.mHandle.mVal;
        v6->parentHandle.mHandle.mVal = self->mHandle.mHandle.mVal;
        weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(grenadeWPID);
        v6->mClassName = str_const.rocket;
        v6->mClassNameHash.mHash = HashString::CalcHash(v6->mClassName.GetBuff());
        v6->damage = InfoForWeapon->iDamage;
        v6->methodOfDeath = InfoForWeapon->slot == 7 /* WEAPSLOT_PISTOL */;
        v6->splashMethodOfDeath =
            InfoForWeapon->slot == 4 /* WEAPSLOT_GRENADE */;
        v6->clipmask = 41951377;
        v6->s.pos.trType = TR_GRAVITY;
        v6->s.eFlags |= 0x8000;
        v6->s.pos.trTime = level.time;
        v6->s.pos.trBase[0] = start[0];
        v6->s.pos.trBase[1] = start[1];
        v6->s.pos.trBase[2] = start[2];
        v6->s.pos.trDelta[0] = dir[0];
        v6->s.pos.trDelta[1] = dir[1];
        v6->s.pos.trDelta[2] = dir[2];
        v6->timestamp = level.time;
        v6->s.apos.trType = TR_LINEAR;
        v6->s.apos.trTime = level.time;
        vectoangles(dir, v6->s.apos.trBase);
        if (InfoForWeapon->type == 1 /* WEAPTYPE_GRENADE */)
        {
            double v17 = AngleNormalize360(v6->s.apos.trBase[0] - 120.0f);
            v6->s.apos.trBase[0] = (float)v17;
            flrand(-45.0f, 45.0f);
            v6->s.apos.trDelta[0] = (float)(v17 + 720.0);
        }
        v6->s.apos.trDelta[1] = 0.0f;
        float a1 = flrand(-45.0f, 45.0f);
        v6->s.apos.trDelta[2] = a1 + 360.0f;
        if (IS_NAN(v6->r.currentOrigin.v.m128_f32[0])
            || IS_NAN(v6->r.currentOrigin.v.m128_f32[1])
            || IS_NAN(v6->r.currentOrigin.v.m128_f32[2]))
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_missile.cpp";
            AeAssert::gCurrentLine = 1452;
            AeAssert::gCurrentExpr = "!IS_NAN((bolt->r.currentOrigin)[0]) && !IS_NAN((bolt->r.currentOrigin)[1]) && !IS_NAN((bolt->r.currentOrigin)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        v6->r.currentOrigin.v.m128_f32[0] = start[0];
        v6->r.currentOrigin.v.m128_f32[1] = start[1];
        v6->r.currentOrigin.v.m128_f32[2] = start[2];
        v6->r.currentAngles.v.m128_f32[0] = v6->s.apos.trBase[0];
        v6->r.currentAngles.v.m128_f32[1] = v6->s.apos.trBase[1];
        v6->r.currentAngles.v.m128_f32[2] = v6->s.apos.trBase[2];
        v6->key = PostEffectEventWeapon(v6, InfoForWeapon->szInternalName,
                                        0x40).mVal;
        ValidatePakId((TPakId)v6->mModel.mPakId);
        if (v6->mModel.mValue != nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_missile.cpp";
            AeAssert::gCurrentLine = 1459;
            AeAssert::gCurrentExpr = "!bolt->mModel";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        TPakId v18 = (TPakId)v6->mPakId;
        if (v18 == PAK_ID_INVALID)
            v18 = CurPakId();
        v6->mModel = XModelManager::sInst->GetXModel(
            v18, InfoForWeapon->szProjectileModel);
        ValidatePakId((TPakId)v6->mModel.mPakId);
        if (v6->mModel.mValue == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_missile.cpp";
            AeAssert::gCurrentLine = 1461;
            AeAssert::gCurrentExpr = "bolt->mModel";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        G_DObjUpdate(v6, false);
        return v6;
    }
    return nullptr;
}

// ea: 0x004891C0
void Weapon_Melee(Entity* ent, weaponParms* wp)
{
    int weapon = ent->s.weapon;
    int damage = BG_GetInfoForWeapon(weapon)->iMeleeDamage;
    if (level.time < ent->invulnerability_timeout)
        ent->invulnerability_timeout = 0;
    math::Position3 end;
    end.v.m128_f32[0] = (wp->forward[0] * 72.0f) + wp->muzzleTrace[0];
    end.v.m128_f32[1] = (wp->forward[1] * 72.0f) + wp->muzzleTrace[1];
    end.v.m128_f32[2] = (wp->forward[2] * 72.0f) + wp->muzzleTrace[2];
    end.v.m128_f32[3] = 0.0f;
    collision_context_t context;
    context.__vftable = (collision_context_t_vtbl*)0x00CD8F6C;
    context.pass_entity1 = ent->mHandle;
    context.pass_entity2.mHandle.mVal = 0;
    context.pass_owner1.mHandle.mVal = 0;
    context.pass_owner2.mHandle.mVal = 0;
    context.contentmask = 0x2802033;
    trace_t tr;
    unsigned int hitHandle = 0;
    if ((g_debugBullets.integer & 1) != 0 && ent->actor != nullptr
        || g_debugBullets.integer >= 5)
    {
        Entity* v12 = G_TempEntity(wp->muzzleTrace, 214);
        v12->s.origin2.v.m128_f32[0] = end.v.m128_f32[0];
        v12->s.origin2.v.m128_f32[1] = end.v.m128_f32[1];
        v12->s.origin2.v.m128_f32[2] = end.v.m128_f32[2];
        if (ent->sentient != nullptr && ent->sentient->eTeam == TEAM_AXIS)
            v12->s.dmgFlags = 1;
    }
    if (ent->client == nullptr)
    {
        math::Position3 start;
        start.v.m128_f32[0] = wp->muzzleTrace[0];
        start.v.m128_f32[1] = wp->muzzleTrace[1];
        start.v.m128_f32[2] = wp->muzzleTrace[2];
        start.v.m128_f32[3] = 0.0f;
        g_LocationalTrace(&tr, &start, &end, &context, bulletPriorityMap, 0.0f);
        hitHandle = tr.mEntity.mHandle.mVal;
    }
    else
    {
        TouchEntityData entities;
        memset(&entities, 0, sizeof(entities));
        entities.mins = ent->r.absmin;
        entities.maxs = ent->r.absmax;
        math::Position3 end;
        end.v.m128_f32[0] = (wp->forward[0] * 45.0f) + wp->muzzleTrace[0];
        end.v.m128_f32[1] = (wp->forward[1] * 45.0f) + wp->muzzleTrace[1];
        end.v.m128_f32[2] = (wp->forward[2] * 45.0f) + wp->muzzleTrace[2];
        end.v.m128_f32[3] = 0.0f;
        math::Position3 boxMins;
        boxMins.v.m128_f32[0] =
            ent->r.absmin.v.m128_f32[0] + wp->forward[0] * 30.0f - 20.0f;
        boxMins.v.m128_f32[1] =
            ent->r.absmin.v.m128_f32[1] + wp->forward[1] * 30.0f - 20.0f;
        boxMins.v.m128_f32[2] =
            ent->r.absmin.v.m128_f32[2] + wp->forward[2] * 30.0f - 20.0f;
        math::Position3 boxMaxs;
        boxMaxs.v.m128_f32[0] =
            ent->r.absmax.v.m128_f32[0] + wp->forward[0] * 30.0f + 20.0f;
        boxMaxs.v.m128_f32[1] =
            ent->r.absmax.v.m128_f32[1] + wp->forward[1] * 30.0f + 20.0f;
        boxMaxs.v.m128_f32[2] =
            ent->r.absmax.v.m128_f32[2] + wp->forward[2] * 30.0f + 20.0f;
        int num = CM_AreaEntities(boxMins, boxMaxs, entities.touch, 128,
                                  0x2000000);
        Entity* mObject = nullptr;
        int i;
        for (i = 0; i < num; ++i)
        {
            Entity* ent2 = HandleDbToEnt(entities.touch[i]);
            if (ent2 != nullptr
                && (ent2->client != nullptr || ent2->actor != nullptr)
                && ent != ent2)
            {
                math::Position3 closest;
                closest.v.m128_f32[0] = end.v.m128_f32[0];
                if (closest.v.m128_f32[0] < ent2->r.absmin.v.m128_f32[0])
                    closest.v.m128_f32[0] = ent2->r.absmin.v.m128_f32[0];
                if (closest.v.m128_f32[0] > ent2->r.absmax.v.m128_f32[0])
                    closest.v.m128_f32[0] = ent2->r.absmax.v.m128_f32[0];
                closest.v.m128_f32[1] = end.v.m128_f32[1];
                if (closest.v.m128_f32[1] < ent2->r.absmin.v.m128_f32[1])
                    closest.v.m128_f32[1] = ent2->r.absmin.v.m128_f32[1];
                if (closest.v.m128_f32[1] > ent2->r.absmax.v.m128_f32[1])
                    closest.v.m128_f32[1] = ent2->r.absmax.v.m128_f32[1];
                closest.v.m128_f32[2] = end.v.m128_f32[2];
                if (closest.v.m128_f32[2] < ent2->r.absmin.v.m128_f32[2])
                    closest.v.m128_f32[2] = ent2->r.absmin.v.m128_f32[2];
                if (closest.v.m128_f32[2] > ent2->r.absmax.v.m128_f32[2])
                    closest.v.m128_f32[2] = ent2->r.absmax.v.m128_f32[2];
                float dx = end.v.m128_f32[0] - closest.v.m128_f32[0];
                float dy = end.v.m128_f32[1] - closest.v.m128_f32[1];
                float dz = end.v.m128_f32[2] - closest.v.m128_f32[2];
                if (radius_1 * radius_1 > dx * dx + dy * dy + dz * dz)
                {
                    mObject = ent2;
                    break;
                }
            }
        }
        if (mObject != nullptr)
        {
            math::Position3 start;
            start.v.m128_f32[0] = wp->muzzleTrace[0];
            start.v.m128_f32[1] = wp->muzzleTrace[1];
            start.v.m128_f32[2] = wp->muzzleTrace[2];
            start.v.m128_f32[3] = 0.0f;
            math::Position3 closest;
            closest.v.m128_f32[0] = end.v.m128_f32[0];
            if (closest.v.m128_f32[0] < mObject->r.absmin.v.m128_f32[0])
                closest.v.m128_f32[0] = mObject->r.absmin.v.m128_f32[0];
            if (closest.v.m128_f32[0] > mObject->r.absmax.v.m128_f32[0])
                closest.v.m128_f32[0] = mObject->r.absmax.v.m128_f32[0];
            closest.v.m128_f32[1] = end.v.m128_f32[1];
            if (closest.v.m128_f32[1] < mObject->r.absmin.v.m128_f32[1])
                closest.v.m128_f32[1] = mObject->r.absmin.v.m128_f32[1];
            if (closest.v.m128_f32[1] > mObject->r.absmax.v.m128_f32[1])
                closest.v.m128_f32[1] = mObject->r.absmax.v.m128_f32[1];
            closest.v.m128_f32[2] = end.v.m128_f32[2];
            if (closest.v.m128_f32[2] < mObject->r.absmin.v.m128_f32[2])
                closest.v.m128_f32[2] = mObject->r.absmin.v.m128_f32[2];
            if (closest.v.m128_f32[2] > mObject->r.absmax.v.m128_f32[2])
                closest.v.m128_f32[2] = mObject->r.absmax.v.m128_f32[2];
            math::Position3 zero;
            zero.v = _mm_setzero_ps();
            SV_Trace(&tr, &start, &zero, &zero, &closest, &context, 0, 1,
                     bulletPriorityMap, 1, 0.0f);
            hitHandle = tr.mEntity.mHandle.mVal;
            if (hitHandle == mObject->mHandle.mHandle.mVal
                || tr.fraction == 1.0f)
            {
                tr.fraction = 0.5f;
                tr.endpos = closest;
                hitHandle = mObject->mHandle.mHandle.mVal;
            }
        }
        else
        {
            math::Position3 start;
            start.v.m128_f32[0] = wp->muzzleTrace[0];
            start.v.m128_f32[1] = wp->muzzleTrace[1];
            start.v.m128_f32[2] = wp->muzzleTrace[2];
            start.v.m128_f32[3] = 0.0f;
            math::Position3 zero;
            zero.v = _mm_setzero_ps();
            SV_Trace(&tr, &start, &zero, &zero, &end, &context, 0, 1,
                     bulletPriorityMap, 1, 0.0f);
            hitHandle = tr.mEntity.mHandle.mVal;
        }
    }
    Entity* hitEnt = EntFromHandle(hitHandle);
    if (hitEnt == nullptr || hitEnt->actor == nullptr
        || !Actor_IsMeleeInteractable(hitEnt->actor)
        || !CheckActorInteraction(*hitEnt, "interacttest"))
    {
        math::Position3 muzzlePos;
        muzzlePos.v.m128_f32[0] = wp->muzzleTrace[0];
        muzzlePos.v.m128_f32[1] = wp->muzzleTrace[1];
        muzzlePos.v.m128_f32[2] = wp->muzzleTrace[2];
        muzzlePos.v.m128_f32[3] = 0.0f;
        G_CheckHitTriggerDamage(ent, &muzzlePos, &tr.endpos, damage, 11);
        if ((((unsigned char*)&tr.normal.v.m128_f32[2])[0] & 0x10) != 0
            || tr.normal.v.m128_f32[1] == 1.0f)
            goto melee_miss;
        Entity* v58 = EntFromHandle(hitHandle);
        if (v58 == nullptr)
            goto melee_miss;
        Entity* v40 =
            (v58->client != nullptr || v58->actor != nullptr)
                ? G_TempEntity(&tr.endpos.v.m128_f32[0], 194)
                : G_TempEntity(&tr.endpos.v.m128_f32[0], 195);
        v40->s.mOtherEntity.mHandle.mVal = hitHandle;
        v40->s.eventParm = DirToByte(&tr.normal.v.m128_f32[0]);
        v40->s.weapon = ent->s.weapon;
        if (hitHandle == 0)
            goto melee_miss;
        if (v58->takedamage != 0)
        {
            if (v58->scr_vehicle == nullptr)
            {
                G_Damage(v58, ent, ent, wp->forward,
                         &tr.endpos.v.m128_f32[0], damage, 0, 11,
                         (hitLocation_t)(intptr_t)tr.shader, -1);
            }
            Weapon_MeleeHitShock(v58);
            int surfaceType = (tr.surfaceFlags >> 20) & 0xFFFFFF1F;
            const math::Dir3 dir = native_to_cdl_dir3(wp->forward);
            MultiplayerMgr::sInst->MeleeHit(v58, ent, tr.endpos, dir,
                                            (unsigned char)surfaceType,
                                            (short)damage, 11, (int)tr.shader);
        }
        else
        {
            math::Position3 zeroPos;
            math::Dir3 zeroDir;
            zeroPos.v = _mm_setzero_ps();
            zeroDir.v = _mm_setzero_ps();
            MultiplayerMgr::sInst->MeleeHit(nullptr, ent, zeroPos, zeroDir, 0,
                                            0, 0, 0);
            if (ent->sentient == nullptr || v58->sentient == nullptr
                || ent->sentient->eTeam != v58->sentient->eTeam)
            {
                Broc::entity e;
                e.___u0 = ent->mHandle.mHandle.mVal;
                int d = 0;
                int mod = 11;
                int hitloc = 0;
                v58->Notify(hash_const.damage, d, e, mod, hitloc);
            }
        }
        return;
    }
    return;
melee_miss:
    math::Position3 zeroPos;
    math::Dir3 zeroDir;
    zeroPos.v = _mm_setzero_ps();
    zeroDir.v = _mm_setzero_ps();
    MultiplayerMgr::sInst->MeleeHit(nullptr, ent, zeroPos, zeroDir, 0, 0, 0,
                                    0);
}

// ea: 0x004712A0
void Bullet_Fire_Fake_Extended(
    DbLinkedHandle<EntityHandleDb, Entity> sourceEntity, Entity* attacker,
    float* start, const float* end, int damage, int recursion, weaponParms* wp,
    DbLinkedHandle<EntityHandleDb, Entity> weaponEntity,
    float coneAngleTangent)
{
    if (attacker == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 1372;
        AeAssert::gCurrentExpr = "attacker";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (recursion > 12)
    {
        Com_Printf("Bullet_Fire_Fake_Extended: Too many resursions, bullet "
                   "aborted\n");
        return;
    }
    collision_context_t context;
    context.__vftable = (collision_context_t_vtbl*)0x00CD8F6C;
    context.pass_entity1 = sourceEntity;
    context.pass_entity2.mHandle.mVal = 0x2802033;
    context.pass_owner1.mHandle.mVal = 0;
    context.pass_owner2.mHandle.mVal = 0;
    context.contentmask = 0;
    trace_t tr;
    memset(&tr, 0, sizeof(tr));
    tr.check_decal = true;
    tr.decal_radius = decal_radius;
    unsigned char* prioMap = wp->pWeapInfo->bRifleBullet != 0
                                 ? riflePriorityMap
                                 : bulletPriorityMap;
    math::Position3 s;
    s.v.m128_f32[0] = start[0];
    s.v.m128_f32[1] = start[1];
    s.v.m128_f32[2] = start[2];
    s.v.m128_f32[3] = 0.0f;
    math::Position3 e;
    e.v.m128_f32[0] = end[0];
    e.v.m128_f32[1] = end[1];
    e.v.m128_f32[2] = end[2];
    e.v.m128_f32[3] = 0.0f;
    g_LocationalTrace(&tr, &s, &e, &context, prioMap, coneAngleTangent);
    if (g_debugBullets.integer > 0)
    {
        CL_AddDebugLine(start, &tr.endpos.v.m128_f32[0], colorGreen, 1, 20,
                        1, 0);
        math::Position3 dbg2;
        dbg2.v.m128_f32[0] =
            tr.endpos.v.m128_f32[0] + tr.normal.v.m128_f32[0] * 20.0f;
        dbg2.v.m128_f32[1] =
            tr.endpos.v.m128_f32[1] + tr.normal.v.m128_f32[1] * 20.0f;
        dbg2.v.m128_f32[2] =
            tr.endpos.v.m128_f32[2] + tr.normal.v.m128_f32[2] * 20.0f;
        CL_AddDebugLine(&tr.endpos.v.m128_f32[0], &dbg2.v.m128_f32[0],
                        colorBlue, 1, 20, 1, 0);
    }
    if (((g_debugBullets.integer & 1) != 0 && attacker->actor != nullptr)
        || g_debugBullets.integer >= 5)
    {
        Entity* v13 = G_TempEntity(start, 214);
        v13->s.origin2.v.m128_f32[0] = tr.endpos.v.m128_f32[0];
        v13->s.origin2.v.m128_f32[1] = tr.endpos.v.m128_f32[1];
        v13->s.origin2.v.m128_f32[2] = tr.endpos.v.m128_f32[2];
        if (attacker->sentient != nullptr
            && attacker->sentient->eTeam == TEAM_AXIS)
            v13->s.dmgFlags = 1;
    }
    Entity* mObject = EntFromHandle(tr.surfaceFlags);
    if (tr.normal.v.m128_f32[1] >= 1.0f)
    {
        math::Position3 pstart;
        pstart.v.m128_f32[0] = start[0];
        pstart.v.m128_f32[1] = start[1];
        pstart.v.m128_f32[2] = start[2];
        pstart.v.m128_f32[3] = 0.0f;
        CG_EventSpawnTracer(&pstart, &e, wp->pWeapInfo->index);
    }
    else
    {
        if (g_debugBullets.integer <= -2)
        {
            float dbgStart[3];
            dbgStart[0] = mObject->r.currentOrigin.v.m128_f32[0]
                        + mObject->r.mins.v.m128_f32[0];
            dbgStart[1] = mObject->r.currentOrigin.v.m128_f32[1]
                        + mObject->r.mins.v.m128_f32[1];
            dbgStart[2] = mObject->r.currentOrigin.v.m128_f32[2]
                        + mObject->r.mins.v.m128_f32[2];
            Entity* v22 = G_TempEntity(dbgStart, 214);
            v22->s.origin2.v.m128_f32[0] = mObject->r.currentOrigin.v.m128_f32[0]
                                         + mObject->r.maxs.v.m128_f32[0];
            v22->s.origin2.v.m128_f32[1] = mObject->r.currentOrigin.v.m128_f32[1]
                                         + mObject->r.maxs.v.m128_f32[1];
            v22->s.origin2.v.m128_f32[2] = mObject->r.currentOrigin.v.m128_f32[2]
                                         + mObject->r.maxs.v.m128_f32[2];
            v22->s.dmgFlags = 2;
        }
        float dir[3];
        dir[0] = end[0] - start[0];
        dir[1] = end[1] - start[1];
        dir[2] = end[2] - start[2];
        VectorNormalize(dir);
        float v23 = (tr.normal.v.m128_f32[0] * dir[0]
                     + tr.normal.v.m128_f32[1] * dir[1]
                     + tr.normal.v.m128_f32[2] * dir[2])
                    * -2.0f;
        dir[0] = tr.normal.v.m128_f32[0] * v23 + dir[0];
        dir[1] = tr.normal.v.m128_f32[1] * v23 + dir[1];
        dir[2] = tr.normal.v.m128_f32[2] * v23 + dir[2];
        if (((int)tr.normal.v.m128_f32[2] & 4) == 0)
        {
            if (mObject == nullptr)
            {
                int surfType = ((int)tr.normal.v.m128_f32[2] >> 20) & 0x1F;
                CG_BulletHitEvent(attacker, &tr.endpos,
                                  &tr.normal.v.m128_f32[0],
                                  wp->pWeapInfo->index, surfType, mObject);
                if (tr.partGroup != 0
                    && tr.surfaceFlags
                           == EntityManager::sInst->mWorld->mHandle.mHandle
                                  .mVal)
                {
                    gdDecal* decal =
                        wp->pWeapInfo->pDecals[surfType];
                    if (decal != nullptr)
                    {
                        float color[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
                        float angle = rand() * 0.000095876727f;
                        void* tex = decal->level1_cg_texture;
                        if (tex != nullptr)
                        {
                            bool isHighPriority = attacker->s.eType == 1;
                            float decalRadius =
                                (rand() * 0.000009155552842799158f + 1.0f)
                                * decal->level1_radius;
                            ((DynamicDecalMgr*)DynamicDecalMgr::sInst)
                                ->Add(tex, 0.1f, true, 100, tr.endpos,
                                      *(math::Position3*)&tr.normal,
                                      decalRadius, angle, color,
                                      isHighPriority);
                        }
                    }
                }
            }
            else
            {
                sentient_s* sentient = mObject->sentient;
                if (sentient == nullptr || attacker->actor == nullptr
                    || sentient->eTeam != attacker->sentient->eTeam)
                {
                    if ((mObject->client != nullptr
                         || (sentient != nullptr
                             && tr.normal.v.m128_f32[2] == 0.0f))
                        && mObject->takedamage != 0)
                    {
                        tr.normal.v.m128_f32[2] = (float)0x700000u;
                    }
                    int surfType =
                        ((int)tr.normal.v.m128_f32[2] >> 20) & 0x1F;
                    CG_BulletHitEvent(attacker, &tr.endpos,
                                      &tr.normal.v.m128_f32[0],
                                      wp->pWeapInfo->index, surfType,
                                      mObject);
                }
            }
        }
        else
        {
            CG_EventSpawnTracer(&s, &tr.endpos, wp->pWeapInfo->index);
        }
    }
    char passThrough = 0;
    if (mObject != nullptr)
    {
        ValidatePakId((TPakId)mObject->mModel.mPakId);
        if (mObject->mModel.mValue != nullptr)
        {
            ValidatePakId((TPakId)mObject->mModel.mPakId);
            if ((mObject->mModel.mValue->contents & 0x12) != 0)
                passThrough = 1;
        }
    }
    if (((int)tr.normal.v.m128_f32[3] & 0x12) != 0 || passThrough != 0)
    {
        float dir[3];
        dir[0] = end[0] - start[0];
        dir[1] = end[1] - start[1];
        dir[2] = end[2] - start[2];
        VectorNormalize(dir);
        float v37 = tr.normal.v.m128_f32[0] * dir[0]
                  + tr.normal.v.m128_f32[1] * dir[1]
                  + tr.normal.v.m128_f32[2] * dir[2];
        float v38 = 0.0f;
        if (-v37 >= 0.125f)
            v38 = 0.25f / -v37;
        start[0] = tr.endpos.v.m128_f32[0] + v38 * dir[0];
        start[1] = tr.endpos.v.m128_f32[1] + v38 * dir[1];
        start[2] = tr.endpos.v.m128_f32[2] + v38 * dir[2];
        Bullet_Fire_Fake_Extended(sourceEntity, attacker, start, end, damage,
                                  recursion + 1, wp, weaponEntity, 0.0f);
    }
    else if (mObject != nullptr && mObject->takedamage != 0)
    {
        actor_s* actor = attacker->actor;
        if (actor != nullptr && attacker->tagInfo != nullptr
            && attacker->tagInfo->parent == mObject)
        {
            G_DPrintf("^3AI Shooting through vehicle\n");
            DbLinkedHandle<EntityHandleDb, Entity> src;
            src.mHandle.mVal = mObject->mHandle.mHandle.mVal;
            Bullet_Fire_Fake_Extended(src, attacker,
                                      &tr.endpos.v.m128_f32[0], end, damage,
                                      recursion + 1, wp, weaponEntity, 0.0f);
        }
        else
        {
            sentient_s* sentient = mObject->sentient;
            if (sentient != nullptr && actor != nullptr
                && sentient->eTeam == attacker->sentient->eTeam)
            {
                G_DPrintf("^3Ignoring AI shooting teammate\n");
                DbLinkedHandle<EntityHandleDb, Entity> src;
                src.mHandle.mVal = mObject->mHandle.mHandle.mVal;
                Bullet_Fire_Fake_Extended(
                    src, attacker, &tr.endpos.v.m128_f32[0], end, damage,
                    recursion + 1, wp, weaponEntity, 0.0f);
            }
        }
    }
}

// ea: 0x00489A70
void Bullet_Fire_Extended(
    DbLinkedHandle<EntityHandleDb, Entity> sourceEntity, Entity* attacker,
    float* start, const float* end, int damage, int recursion,
    const weaponParms* wp,
    DbLinkedHandle<EntityHandleDb, Entity> weaponEntity,
    float coneAngleTangent)
{
    Entity* sourceEnt = EntFromHandle(sourceEntity.mHandle.mVal);
    trace_t tr;
    memset(&tr, 0, sizeof(tr));
    int dflags = 0;
    int sourceMod = 1;
    if (attacker == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_weapon.cpp";
        AeAssert::gCurrentLine = 790;
        AeAssert::gCurrentExpr = "attacker";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (recursion > 12)
    {
        Com_Printf("Bullet_Fire_Extended: Too many resursions, bullet "
                   "aborted\n");
        return;
    }
    weaponFileInfo_t* pWeapInfo = wp->pWeapInfo;
    if (pWeapInfo->bRifleBullet != 0)
    {
        sourceMod = 2;
        dflags = 64;
    }
    unsigned int passEnt = sourceEntity.mHandle.mVal;
    Client* attackerClient = attacker->client;
    if (attackerClient != nullptr
        && (attackerClient->ps.eFlags & 0x100000) != 0
        && attackerClient->ps.vehPos != 0)
        passEnt = 0;
    collision_context_t context;
    context.__vftable = (collision_context_t_vtbl*)0x00CD8F6C;
    context.pass_entity1.mHandle.mVal = passEnt;
    context.pass_entity2.mHandle.mVal = 0x2802033;
    context.pass_owner1.mHandle.mVal = 0;
    context.pass_owner2.mHandle.mVal = 0;
    context.contentmask = 0;
    tr.check_decal = true;
    tr.decal_radius = decal_radius_0;
    unsigned char* prioMap = pWeapInfo->bRifleBullet != 0
                                 ? riflePriorityMap
                                 : bulletPriorityMap;
    math::Position3 s;
    s.v.m128_f32[0] = start[0];
    s.v.m128_f32[1] = start[1];
    s.v.m128_f32[2] = start[2];
    s.v.m128_f32[3] = 0.0f;
    math::Position3 e;
    e.v.m128_f32[0] = end[0];
    e.v.m128_f32[1] = end[1];
    e.v.m128_f32[2] = end[2];
    e.v.m128_f32[3] = 0.0f;
    g_LocationalTrace(&tr, &s, &e, &context, prioMap, coneAngleTangent);
    int debugBullets = g_debugBullets.integer;
    if (g_debugBullets.integer > 0)
    {
        int duration = 20;
        if (g_debugBullets.integer > 3)
            duration = 600;
        CL_AddDebugLine(start, &tr.endpos.v.m128_f32[0], colorGreen, 1,
                        duration, 1, 0);
        math::Position3 dbg2;
        dbg2.v.m128_f32[0] =
            tr.endpos.v.m128_f32[0] + tr.normal.v.m128_f32[0] * 20.0f;
        dbg2.v.m128_f32[1] =
            tr.endpos.v.m128_f32[1] + tr.normal.v.m128_f32[1] * 20.0f;
        dbg2.v.m128_f32[2] =
            tr.endpos.v.m128_f32[2] + tr.normal.v.m128_f32[2] * 20.0f;
        CL_AddDebugLine(&tr.endpos.v.m128_f32[0], &dbg2.v.m128_f32[0],
                        colorBlue, 1, duration, 1, 0);
        float dx = tr.endpos.v.m128_f32[0] - start[0];
        float dy = tr.endpos.v.m128_f32[1] - start[1];
        float dz = tr.endpos.v.m128_f32[2] - start[2];
        float dir[3] = { -wp->forward[0], -wp->forward[1], -wp->forward[2] };
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);
        if (g_debugBullets.integer > 1 && gTanAimConeSpread > 0.0f)
        {
            float center[3];
            center[0] = wp->forward[0] * (dist - 4.0f) + start[0];
            center[1] = wp->forward[1] * (dist - 4.0f) + start[1];
            center[2] = wp->forward[2] * (dist - 4.0f) + start[2];
            G_DebugCircleEx(center, dist * gTanAimConeSpread, dir, colorBlue,
                            0, duration);
            float center2[3];
            center2[0] = dir[0] * 4.0f + tr.endpos.v.m128_f32[0];
            center2[1] = dir[1] * 4.0f + tr.endpos.v.m128_f32[1];
            center2[2] = dir[2] * 4.0f + tr.endpos.v.m128_f32[2];
            G_DebugCircleEx(center2, dist * coneAngleTangent, dir, colorRed,
                            0, duration);
            debugBullets = g_debugBullets.integer;
        }
    }
    if (((debugBullets & 1) != 0 && attacker->actor != nullptr)
        || debugBullets >= 5)
    {
        Entity* v22 = G_TempEntity(start, 214);
        v22->s.origin2.v.m128_f32[0] = tr.endpos.v.m128_f32[0];
        v22->s.origin2.v.m128_f32[1] = tr.endpos.v.m128_f32[1];
        v22->s.origin2.v.m128_f32[2] = tr.endpos.v.m128_f32[2];
        if (attacker->sentient != nullptr
            && attacker->sentient->eTeam == TEAM_AXIS)
            v22->s.dmgFlags = 1;
    }
    math::Position3 startPos;
    startPos.v.m128_f32[0] = start[0];
    startPos.v.m128_f32[1] = start[1];
    startPos.v.m128_f32[2] = start[2];
    startPos.v.m128_f32[3] = 0.0f;
    G_CheckHitTriggerDamage(attacker, &startPos, &tr.endpos, damage,
                            sourceMod);
    Entity* hitEnt = EntFromHandle(tr.surfaceFlags);
    if (tr.normal.v.m128_f32[1] < 1.0f)
    {
        if (g_debugBullets.integer <= -2)
        {
            float dbgStart[3];
            dbgStart[0] = hitEnt->r.currentOrigin.v.m128_f32[0]
                        + hitEnt->r.mins.v.m128_f32[0];
            dbgStart[1] = hitEnt->r.currentOrigin.v.m128_f32[1]
                        + hitEnt->r.mins.v.m128_f32[1];
            dbgStart[2] = hitEnt->r.currentOrigin.v.m128_f32[2]
                        + hitEnt->r.mins.v.m128_f32[2];
            Entity* v32 = G_TempEntity(dbgStart, 214);
            v32->s.origin2.v.m128_f32[0] = hitEnt->r.currentOrigin.v.m128_f32[0]
                                         + hitEnt->r.maxs.v.m128_f32[0];
            v32->s.origin2.v.m128_f32[1] = hitEnt->r.currentOrigin.v.m128_f32[1]
                                         + hitEnt->r.maxs.v.m128_f32[1];
            v32->s.origin2.v.m128_f32[2] = hitEnt->r.currentOrigin.v.m128_f32[2]
                                         + hitEnt->r.maxs.v.m128_f32[2];
            v32->s.dmgFlags = 2;
        }
        float dir[3];
        dir[0] = end[0] - start[0];
        dir[1] = end[1] - start[1];
        dir[2] = end[2] - start[2];
        VectorNormalize(dir);
        float v33 = (tr.normal.v.m128_f32[0] * dir[0]
                     + tr.normal.v.m128_f32[1] * dir[1]
                     + tr.normal.v.m128_f32[2] * dir[2])
                    * -2.0f;
        dir[0] = tr.normal.v.m128_f32[0] * v33 + dir[0];
        dir[1] = tr.normal.v.m128_f32[1] * v33 + dir[1];
        dir[2] = tr.normal.v.m128_f32[2] * v33 + dir[2];
        if (((int)tr.normal.v.m128_f32[2] & 4) == 0)
        {
            if (hitEnt == nullptr)
            {
                goto hit_event;
            }
            sentient_s* sentient = hitEnt->sentient;
            if (sentient == nullptr || attacker->actor == nullptr
                || sentient->eTeam != attacker->sentient->eTeam)
            {
                Client* hitClient = hitEnt->client;
                if (hitClient != nullptr
                    || (sentient != nullptr
                        && tr.normal.v.m128_f32[2] == 0.0f))
                {
                    if ((hitEnt->r.contents & 0x4000000) != 0)
                    {
                        float fwd[3];
                        fwd[0] = end[0] - start[0];
                        fwd[1] = end[1] - start[1];
                        fwd[2] = end[2] - start[2];
                        VectorNormalize(fwd);
                        bool localHit = tr.shader != nullptr;
                        math::Dir3 cdlDir = native_to_cdl_dir3(fwd);
                        ApplyPhysics(hitEnt, &tr.endpos, &cdlDir, 20.0f,
                                     localHit, HITLOC_TORSO_UPR);
                    }
                    else if (hitEnt->takedamage != 0)
                    {
                        if (hitClient == nullptr
                            || G_CanPlayerBeDamagedInVehicle(hitEnt))
                        {
                            tr.normal.v.m128_f32[2] = (float)0x700000u;
                        }
                    }
                }
                goto hit_event;
            }
        }
        math::Position3 tracerStart;
        tracerStart.v.m128_f32[0] = start[0];
        tracerStart.v.m128_f32[1] = start[1];
        tracerStart.v.m128_f32[2] = start[2];
        tracerStart.v.m128_f32[3] = 0.0f;
        CG_EventSpawnTracer(&tracerStart, &tr.endpos,
                            wp->pWeapInfo->index);
        goto after_hit;
    }
    {
        math::Position3 tracerStart;
        tracerStart.v.m128_f32[0] = start[0];
        tracerStart.v.m128_f32[1] = start[1];
        tracerStart.v.m128_f32[2] = start[2];
        tracerStart.v.m128_f32[3] = 0.0f;
        CG_EventSpawnTracer(&tracerStart, &e, wp->pWeapInfo->index);
        Entity* weaponEnt = HandleDbToEnt(weaponEntity);
        if (weaponEnt != nullptr && weaponEnt->scr_vehicle == nullptr
            && wp->pWeapInfo->weapClass != 17)
        {
            math::Position3 tmp;
            tmp = native_to_cdl_pos3((float*)end);
            const math::Position3* pos = &tmp;
            math::Dir3 zero;
            zero.v = _mm_setzero_ps();
            MultiplayerMgr::sInst->BulletHit(*pos, zero, 0,
                                             (unsigned char)wp->pWeapInfo
                                                 ->index,
                                             nullptr);
        }
        goto after_hit;
    }
hit_event:
    if (HandleDbToEnt(weaponEntity) != nullptr
        && HandleDbToEnt(weaponEntity)->scr_vehicle == nullptr
        && wp->pWeapInfo->weapClass != 17)
    {
        math::Dir3 normal;
        normal.v = tr.normal.v;
        MultiplayerMgr::sInst->BulletHit(
            tr.endpos, normal,
            (unsigned char)(((int)tr.normal.v.m128_f32[2] >> 20) & 0x1F),
            (unsigned char)wp->pWeapInfo->index, hitEnt);
    }
    if (hitEnt == nullptr || hitEnt->client == nullptr)
    {
        Entity* eventEnt = attacker;
        if (sourceEnt != nullptr && sourceEnt->scr_vehicle == nullptr)
            eventEnt = sourceEnt;
        CG_BulletHitEvent(
            eventEnt, &tr.endpos, &tr.normal.v.m128_f32[0],
            wp->pWeapInfo->index,
            ((int)tr.normal.v.m128_f32[2] >> 20) & 0x1F, hitEnt);
        if (tr.partGroup != 0
            && tr.surfaceFlags
                   == EntityManager::sInst->mWorld->mHandle.mHandle.mVal)
        {
            gdDecal* decal =
                wp->pWeapInfo->pDecals[((int)tr.normal.v.m128_f32[2] >> 20)
                                       & 0x1F];
            if (decal != nullptr)
            {
                float color[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
                float angle = rand() * 0.000095876727f;
                void* tex = decal->level1_cg_texture;
                if (tex != nullptr)
                {
                    bool isHighPriority = attacker->s.eType == 1;
                    float decalRadius =
                        (rand() * 0.000009155552842799158f + 1.0f)
                        * decal->level1_radius;
                    ((DynamicDecalMgr*)DynamicDecalMgr::sInst)
                        ->Add(tex, 0.1f, true, 100, tr.endpos,
                              *(math::Position3*)&tr.normal, decalRadius,
                              angle, color, isHighPriority);
                }
            }
        }
    }
after_hit:
    if (hitEnt != nullptr && hitEnt->takedamage != 0)
    {
        if (wp->pWeapInfo->weapClass != WEAPCLASS_LMG
            || (attacker->client != nullptr
                && (attacker->client->ps.pm_flags & 0x20) != 0))
        {
            // fallthrough (no damage falloff for LMG in bipod state)
        }
        else
        {
            float dist1 = VectorDistance(
                start, &tr.endpos.v.m128_f32[0]);
            damage = (int)Damage_Falloff(
                dist1, (float)damage,
                (float)wp->pWeapInfo->iMinDamagePercent,
                (float)wp->pWeapInfo->iDamageInnerRadius,
                (float)wp->pWeapInfo->iDamageOuterRadius);
        }
        if (wp->pWeapInfo->weapClass != WEAPCLASS_LMG)
        {
            float dist2 = VectorDistance(start,
                                         &tr.endpos.v.m128_f32[0]);
            damage = (int)Damage_Falloff(
                dist2, (float)damage,
                (float)wp->pWeapInfo->iMinDamagePercent,
                (float)wp->pWeapInfo->iDamageInnerRadius,
                (float)wp->pWeapInfo->iDamageOuterRadius);
        }
        if (attacker->actor != nullptr && attacker->tagInfo != nullptr
            && attacker->tagInfo->parent == hitEnt)
        {
            G_DPrintf("^3AI Shooting through vehicle\n");
            DbLinkedHandle<EntityHandleDb, Entity> src;
            src.mHandle.mVal = hitEnt->mHandle.mHandle.mVal;
            Bullet_Fire_Extended(src, attacker, &tr.endpos.v.m128_f32[0],
                                 end, damage, recursion + 1, wp,
                                 weaponEntity, 0.0f);
            return;
        }
        Client* attackerClient2 = attacker->client;
        if (attackerClient2 != nullptr)
        {
            if ((attackerClient2->ps.eFlags & 0x100000) != 0
                && attackerClient2->ps.vehPos != 0
                && hitEnt->mHandle.mHandle.mVal
                       == attacker->r.mOwner.mHandle.mVal)
            {
                damage = (int)(damage * 0.40000001f);
            }
            Client* hitClient = hitEnt->client;
            if (hitClient != nullptr
                && (attackerClient2->ps.eFlags & 0x100000) != 0
                && (hitClient->ps.eFlags & 0x100000) != 0
                && attackerClient2->ps.vehPos != 0
                && hitEnt->r.mOwner.mHandle.mVal
                       == attacker->r.mOwner.mHandle.mVal)
            {
                damage = (int)(damage * 0.1f);
            }
        }
        if (hitEnt->client != nullptr)
        {
            if (hitEnt == EntFromHandle(sourceEntity.mHandle.mVal))
            {
                float newStart[3];
                newStart[0] =
                    wp->forward[0] * 8.0f + tr.endpos.v.m128_f32[0];
                newStart[1] =
                    wp->forward[1] * 8.0f + tr.endpos.v.m128_f32[1];
                newStart[2] =
                    wp->forward[2] * 8.0f + tr.endpos.v.m128_f32[2];
                Bullet_Fire_Extended(sourceEntity, attacker, newStart, end,
                                     damage, recursion + 1, wp, weaponEntity,
                                     0.0f);
                return;
            }
            Entity* sourceEnt2 = EntFromHandle(sourceEntity.mHandle.mVal);
            if (mp_friendlyfire.integer != 0 || sourceEnt2 == nullptr
                || sourceEnt2->sentient == nullptr
                || hitEnt->sentient == nullptr || !cgGlobal.teamGame
                || sourceEnt2->sentient->eTeam != hitEnt->sentient->eTeam)
            {
                CG_BulletHitClientEvent(sourceEntity.mHandle.mVal, &tr.endpos,
                                        (float*)wp->forward, 7,
                                        wp->pWeapInfo->index);
                if (attacker->IsLocalPlayer()
                    && (sourceMod == 1 || sourceMod == 2))
                {
                    dword_F63D1C[1580 * attacker->GetPlayerIndex()] =
                        level.time;
                }
                const math::Dir3 cdlDir =
                    native_to_cdl_dir3(wp->forward);
                MultiplayerMgr::sInst->BulletHitPlayer(
                    hitEnt, attacker, tr.endpos, cdlDir, 7,
                    (unsigned char)wp->pWeapInfo->index, (short)damage,
                    (unsigned char)dflags, (unsigned char)sourceMod,
                    (int)tr.shader);
            }
        }
        Entity* inflictor = HandleDbToEnt(weaponEntity);
        G_Damage(hitEnt, inflictor, attacker, wp->forward,
                 &tr.endpos.v.m128_f32[0], damage, dflags, sourceMod,
                 (hitLocation_t)(intptr_t)tr.shader, wp->pWeapInfo->index);
        if (hitEnt->sentient != nullptr && (dflags & 0x40) != 0
            && damage / 2 > 0)
        {
            DbLinkedHandle<EntityHandleDb, Entity> src;
            src.mHandle.mVal = hitEnt->mHandle.mHandle.mVal;
            float newStart[3];
            newStart[0] = tr.endpos.v.m128_f32[0];
            newStart[1] = tr.endpos.v.m128_f32[1];
            newStart[2] = tr.endpos.v.m128_f32[2];
            Bullet_Fire_Extended(src, attacker, newStart, end, damage / 2,
                                 recursion + 1, wp, weaponEntity, 0.0f);
        }
    }
    char passThrough = 0;
    if (hitEnt != nullptr)
    {
        ValidatePakId((TPakId)hitEnt->mModel.mPakId);
        if (hitEnt->mModel.mValue != nullptr)
        {
            ValidatePakId((TPakId)hitEnt->mModel.mPakId);
            if ((hitEnt->mModel.mValue->contents & 0x12) != 0)
                passThrough = 1;
        }
    }
    if (((int)tr.normal.v.m128_f32[3] & 0x12) != 0 || passThrough != 0)
    {
        float dir[3];
        dir[0] = end[0] - start[0];
        dir[1] = end[1] - start[1];
        dir[2] = end[2] - start[2];
        VectorNormalize(dir);
        float v62 = tr.normal.v.m128_f32[0] * dir[0]
                  + tr.normal.v.m128_f32[1] * dir[1]
                  + tr.normal.v.m128_f32[2] * dir[2];
        float v63 = 0.0f;
        if (-v62 >= 0.125f)
            v63 = 0.25f / -v62;
        start[0] = tr.endpos.v.m128_f32[0] + v63 * dir[0];
        start[1] = tr.endpos.v.m128_f32[1] + v63 * dir[1];
        start[2] = tr.endpos.v.m128_f32[2] + v63 * dir[2];
        Bullet_Fire_Extended(sourceEntity, attacker, start, end, damage,
                             recursion + 1, wp, weaponEntity, 0.0f);
    }
}
