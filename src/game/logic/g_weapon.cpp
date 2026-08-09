// ============================================================================
// g_weapon.cpp - weapon fire/mine helpers (g.o: g_weapon.cpp family subset)
// ============================================================================

#include "game/logic/g_local.h"

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
