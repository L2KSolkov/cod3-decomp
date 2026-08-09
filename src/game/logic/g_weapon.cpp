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
