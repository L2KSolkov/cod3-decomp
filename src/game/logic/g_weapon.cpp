// ============================================================================
// g_weapon.cpp - weapon fire/mine helpers (g.o: g_weapon.cpp family subset)
// ============================================================================

#include "game/logic/g_local.h"

int Weapon_Mine_Test(Entity* ent, weaponParms* wp, math::Position3* position,
                     math::Dir3* normal);

#include <math.h>
#include <stdlib.h>

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
    if (g_developer->integer != 0
        && g_cheats->integer != 0
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
    int entityList[128];
    math::Position3 mins;
    math::Position3 maxs;
    mins.v.m128_f32[0] = ent->r.absmin.v.m128_f32[0] + wp->forward[0] * 30.0f - 20.0f;
    mins.v.m128_f32[1] = ent->r.absmin.v.m128_f32[1] + wp->forward[1] * 30.0f - 20.0f;
    mins.v.m128_f32[2] = ent->r.absmin.v.m128_f32[2] + wp->forward[2] * 30.0f - 20.0f;
    maxs.v.m128_f32[0] = ent->r.absmax.v.m128_f32[0] + wp->forward[0] * 30.0f + 20.0f;
    maxs.v.m128_f32[1] = ent->r.absmax.v.m128_f32[1] + wp->forward[1] * 30.0f + 20.0f;
    maxs.v.m128_f32[2] = ent->r.absmax.v.m128_f32[2] + wp->forward[2] * 30.0f + 20.0f;
    int v35 = CM_AreaEntities(&mins, &maxs, entityList, 128, 0x4000000);
    math::Position3 probe;
    probe.v.m128_f32[0] = wp->forward[0] * 45.0f + wp->muzzleTrace[0];
    probe.v.m128_f32[1] = wp->forward[1] * 45.0f + wp->muzzleTrace[1];
    probe.v.m128_f32[2] = wp->forward[2] * 45.0f + wp->muzzleTrace[2];
    Entity* found = nullptr;
    for (int v20 = 0; v20 < v35; ++v20)
    {
        Entity* mObject = HandleDbToEnt(*(DbLinkedHandle<EntityHandleDb, Entity>*)&entityList[v20]);
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
void G_BulletFireSpread(const Entity* source, Entity* attacker, weaponParms* wp,
                        int damage, float spread, Entity* weaponEnt,
                        float coneAngleTangent, unsigned int seed)
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
