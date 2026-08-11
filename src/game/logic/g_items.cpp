// ============================================================================
// g_items.cpp - item spawning/pickup/bounce (g.o: g_items.cpp family)
// ============================================================================

#include "game/logic/g_local.h"
#include "core/PoolAllocator.h"

// ea: 0x00457A10
void Use_Item(Entity* ent, Entity* /*other*/, Entity* /*activator*/)
{
    RespawnItem(ent);
}

// ea: 0x0048B4E0
void Touch_Item_Auto(Entity* ent, Entity* other, int bTouched)
{
    if (bTouched == 0
        || other->client == nullptr
        || other->client->bDisableAutoPickup == 0)
    {
        ent->active = 1;
        Touch_Item(ent, other, bTouched);
    }
}

#include <math.h>
#include <stdlib.h>
#include <string.h>

extern "C" int __fpclass(float);

static bool IS_NAN(float x) {
    return (__fpclass(x) & 0x297) != 0;
}

// ea: 0x0044AA60
int G_canPickupMelee(Entity* ent)
{
    Client* client = ent->client;
    if (client == nullptr)
        return 0;
    if (ent->s.weapon == 0)
        return 1;
    if (client->ps.weaponstate == 5)
        return 0;
    return BG_GetInfoForWeapon(client->pers.cmd.weapon)->bTwoHanded == 0;
}

// ea: 0x0044B560
int ClearRegisteredItems()
{
    memset(itemRegistered, 0, 137 * sizeof(int));
    itemRegistered[0] = 1;
    return 0;
}

// ea: 0x0044B580
void SaveRegisteredItems()
{
    int v0 = 0;
    int v1 = 0;
    int v2 = 0;
    int v3 = 0;
    char string[260];
    level.bRegisterItems = 0;
    do
    {
        if (itemRegistered[v3] != 0)
            v1 += 1 << v2;
        if (++v2 == 4)
        {
            string[v0++] = (char)(v1 + (v1 >= 10 ? 87 : 48));
            v1 = 0;
            v2 = 0;
        }
        ++v3;
    } while (v3 < 137);
    if (v2 != 0)
        string[v0++] = (char)(v1 + (v1 >= 10 ? 87 : 48));
    string[v0] = 0;
    SV_SetConfigstring(8, string);
}

// ea: 0x0044B630
void RegisterItem(unsigned int iItemIndex, int bUpdateCS)
{
    if (iItemIndex > 0x88)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_items.cpp";
        AeAssert::gCurrentLine = 1535;
        AeAssert::gCurrentExpr = "(iItemIndex >= 0) && (iItemIndex < bg_numItems)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (itemRegistered[iItemIndex] == 0)
    {
        itemType_t giType = bg_itemlist[iItemIndex].giType;
        level.newAssetLoaded = 1;
        itemRegistered[iItemIndex] = 1;
        if (giType == IT_WEAPON)
        {
            int iAltWeaponIndex = iItemIndex;
            do
            {
                itemRegistered[iAltWeaponIndex] = 1;
                iAltWeaponIndex = BG_GetInfoForWeapon(iAltWeaponIndex)->iAltWeaponIndex;
            } while (iAltWeaponIndex != 0 && iAltWeaponIndex != (int)iItemIndex);
        }
        if (bUpdateCS != 0)
            level.bRegisterItems = 1;
    }
}

// ea: 0x0044B6F0
int IsItemRegistered(unsigned int iItemIndex)
{
    if (iItemIndex > 0x88)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_items.cpp";
        AeAssert::gCurrentLine = 1596;
        AeAssert::gCurrentExpr = "(iItemIndex >= 0) && (iItemIndex < bg_numItems)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return itemRegistered[iItemIndex];
}

// ea: 0x00457760
void RespawnItem(Entity* ent)
{
    Entity* teammaster = ent;
    Broc::string::Block* mBlock = ent->team.mBlock;
    if (mBlock != nullptr)
    {
        Broc::string::Block* v3 = mBlock + 1;
        if (v3 != nullptr && v3->mBuff != nullptr && v3->mBuff[0] != 0)
        {
            if (ent->teammaster == nullptr)
                G_Error("RespawnItem: bad teammaster");
            teammaster = ent->teammaster;
            int v4 = 0;
            for (Entity* i = teammaster; i != nullptr; ++v4)
                i = i->teamchain;
            int v6 = rand() % v4;
            if (v6 > 0)
            {
                do
                {
                    --v6;
                    teammaster = teammaster->teamchain;
                } while (v6 != 0);
            }
        }
    }
    teammaster->r.svFlags &= ~1;
    teammaster->flags &= ~0x400;
    teammaster->r.contents = 1075838984;
    SV_LinkEntity(teammaster);
    teammaster->nextthink = 0;
}

// ea: 0x00476400
void G_SpawnItem(Entity* ent, const gitem_s* item)
{
    static unsigned int sInit = 0;
    static unsigned int random_hash;
    static unsigned int wait_hash;
    static unsigned int noise_hash;
    if ((sInit & 1) == 0)
    {
        sInit |= 1u;
        random_hash = HashString::CalcHash("random");
    }
    if ((sInit & 2) == 0)
    {
        sInit |= 2u;
        wait_hash = HashString::CalcHash("wait");
    }
    if ((sInit & 4) == 0)
    {
        sInit |= 4u;
        noise_hash = HashString::CalcHash("noise");
    }
    Entity* v2 = ent;
    G_SpawnFloat(random_hash, 0.0f, &ent->random);
    G_SpawnFloat(wait_hash, 0.0f, &v2->wait);
    const gitem_s* v3 = item;
    RegisterItem((unsigned int)(item - bg_itemlist), 0);
    v2->item = v3;
    G_SetModel(v2, v3->world_model[0], PAK_ID_INVALID, 0);
    const char* noiseName;
    if (G_SpawnString(noise_hash, nullptr, &noiseName) != 0)
        v2->noise_index = G_SoundAliasIndex(noiseName);
    bool v4 = v3->giType == IT_WEAPON;
    v2->r.mins.v.m128_f32[0] = -1.0f;
    v2->r.mins.v.m128_f32[1] = -1.0f;
    float v5;
    if (v4)
    {
        v2->r.mins.v.m128_f32[2] = -1.0f;
        v5 = 1065353216.0f;
        v2->r.maxs.v.m128_f32[0] = 1.0f;
        v2->r.maxs.v.m128_f32[1] = 1.0f;
    }
    else
    {
        v2->r.mins.v.m128_f32[2] = 0.0f;
        v2->r.maxs.v.m128_f32[0] = 1.0f;
        v2->r.maxs.v.m128_f32[1] = 1.0f;
        v5 = 2.0f;
    }
    v2->r.maxs.v.m128_f32[2] = v5;
    v2->s.eFlags |= 0x10;
    v2->r.svFlags |= 0x200u;
    const gitem_s* v7 = v2->item;
    int v8 = (int)(v7 - bg_itemlist);
    v2->r.contents = 1075839240;
    v2->touch = 7;
    v2->s.eType = 2;
    v2->s.brushmodel = v8;
    G_DObjUpdate(v2, false);
    v2->flags |= 0x8000;
    v2->use = 8;
    if (level.spawning != 0)
    {
        G_SetAngle(v2, &v2->r.currentAngles);
        v2->nextthink = level.time + 200;
        v2->think = THINK__FinishSpawningItem;
    }
    else
    {
        if ((v2->spawnflags & 1) == 0)
        {
            v2->s.mGroundEntity.mHandle.mVal = 0;
            if (v3->giType == IT_WEAPON)
                v2->r.currentAngles.v.m128_f32[2] += 90.0f;
        }
        G_SetAngle(v2, &v2->r.currentAngles);
        G_SetOrigin(v2, &v2->r.currentOrigin);
        g_LinkEntity(v2);
    }
}

// ea: 0x00475A70
Entity* LaunchItem(TPakId pakId, const gitem_s* item, float* origin,
                   float* angles, float* velocity, Entity* owner)
{
    Entity* v6 = G_Spawn(PAK_ID_INVALID);
    const gitem_s* v7 = item;
    Entity* v8 = v6;
    int v9 = (int)(item - bg_itemlist);
    v8->s.eType = 2;
    v8->s.brushmodel = v9;
    v8->mClassName = v7->classname;
    v8->mClassNameHash.mHash = HashString(v8->mClassName).mHash;
    v8->item = v7;
    bool v10 = v7->giType == IT_WEAPON;
    v8->r.mins.v.m128_f32[0] = -1.0f;
    v8->r.mins.v.m128_f32[1] = -1.0f;
    float v11;
    if (v10)
    {
        v8->r.mins.v.m128_f32[2] = -1.0f;
        v11 = 1.0f;
        v8->r.maxs.v.m128_f32[0] = 1.0f;
        v8->r.maxs.v.m128_f32[1] = 1.0f;
    }
    else
    {
        v8->r.mins.v.m128_f32[2] = 0.0f;
        v8->r.maxs.v.m128_f32[0] = 1.0f;
        v8->r.maxs.v.m128_f32[1] = 1.0f;
        v11 = 2.0f;
    }
    Entity* v12 = owner;
    v8->r.maxs.v.m128_f32[2] = v11;
    v8->s.eFlags |= 0x10;
    v8->r.svFlags |= 0x200u;
    v8->r.contents = 1075839240;
    if (v12 != nullptr)
        v8->r.mOwner.mHandle.mVal = v12->mHandle.mHandle.mVal;
    else
        v8->r.mOwner.mHandle.mVal = 0;
    G_SetModel(v8, v7->world_model[0], pakId, 0);
    G_DObjUpdate(v8, false);
    v8->touch = 7;
    const float* v14 = angles;
    if (v12 != nullptr && v12->client != nullptr && v12->health <= 0)
        angles[1] = angles[1] + 90.0f;
    G_SetAngle(v8, v14);
    G_SetOrigin(v8, origin);
    v8->s.pos.trType = TR_GRAVITY;
    v8->s.pos.trTime = level.time;
    v8->s.pos.trDelta[0] = velocity[0];
    v8->s.pos.trDelta[1] = velocity[1];
    v8->s.pos.trDelta[2] = velocity[2];
    if (IS_NAN(v8->s.pos.trDelta[0]) || IS_NAN(v8->s.pos.trDelta[1]) || IS_NAN(v8->s.pos.trDelta[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_items.cpp";
        AeAssert::gCurrentLine = 1138;
        AeAssert::gCurrentExpr = "!IS_NAN((dropped->s.pos.trDelta)[0]) && !IS_NAN((dropped->s.pos.trDelta)[1]) && !IS_NAN((dropped->s.pos.trDelta)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    v8->flags = 64;
    g_LinkEntity(v8);
    if (v7->giType == IT_WEAPON)
    {
        v8->think = THINK__G_FreeEntity;
        v8->nextthink = level.time + 30000;
    }
    return v8;
}

// ea: 0x00475D20
Entity* Drop_Item(Entity* ent, const gitem_s* item, float angle, int novelocity)
{
    float vPos[3];
    float angles[3];
    float velocity[3];
    angles[1] = ent->r.currentAngles.v.m128_f32[1] + angle;
    float v4 = 0.0f;
    angles[0] = 0.0f;
    angles[2] = 0.0f;
    if (novelocity != 0)
    {
        velocity[1] = 0.0f;
        velocity[0] = 0.0f;
        v4 = 0.0f;
    }
    else
    {
        AnglesToForward(angles, velocity);
        velocity[0] *= 100.0f;
        velocity[1] *= 100.0f;
        v4 = ((((float)rand() * 0.000061035156f) - 1.0f) * 50.0f) + velocity[2] * 100.0f + 200.0f;
    }
    velocity[2] = v4;
    vPos[0] = ent->r.currentOrigin.v.m128_f32[0];
    vPos[1] = ent->r.currentOrigin.v.m128_f32[1];
    vPos[2] = ((ent->r.maxs.v.m128_f32[2] - ent->r.mins.v.m128_f32[2]) * 0.5f) + ent->r.currentOrigin.v.m128_f32[2];
    return LaunchItem(CurPakId(), item, vPos, angles, velocity, ent);
}

static bool s_pszTagHashInit = false;
static unsigned int pszTag_hash;

// ea: 0x00475E40
Entity* Drop_Weapon(Entity* pEnt, int iWeaponIndex, const char* pszTag)
{
    gitem_s* item = &bg_itemlist[iWeaponIndex];
    if (item->giType != IT_WEAPON)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_items.cpp";
        AeAssert::gCurrentLine = 1235;
        AeAssert::gCurrentExpr = "pWeapItem->giType == IT_WEAPON";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    weaponFileInfo_t* info = BG_GetInfoForWeapon(iWeaponIndex);
    if (info->bDoNotDrop != 0 || info->type == WEAPTYPE_ITEM)
        return nullptr;
    if (info->slot != WEAPSLOT_PRIMARY && info->slot != WEAPSLOT_PRIMARYB)
        return nullptr;
    bool novelocity = false;
    if (pEnt != nullptr && pEnt->client != nullptr)
    {
        int playerState = pEnt->client->pers.playerState;
        if (playerState == 4 || playerState == 5)
            novelocity = true;
    }
    Entity* pDrop = Drop_Item(pEnt, item, 0.0f, novelocity);
    int iAmmoIndex = BG_AmmoForWeapon(iWeaponIndex);
    int iClipIndex = BG_ClipForWeapon(iWeaponIndex);
    int ammo = 0;
    int clip = 0;
    if (pEnt->client != nullptr)
    {
        ammo = pEnt->client->ps.ammo[iAmmoIndex];
        pEnt->client->ps.ammo[iAmmoIndex] = 0;
        clip = pEnt->client->ps.ammoclip[iClipIndex];
        pEnt->client->ps.ammoclip[iClipIndex] = 0;
        BG_TakePlayerWeapon(&pEnt->client->ps, iWeaponIndex);
        if (pEnt->client->pers.playerClass == 2
            && pEnt->client->ps.weaponslots[1] == iWeaponIndex
            && pEnt->client->ps.weaponslots[9] != 0)
        {
            BG_TakePlayerWeapon(&pEnt->client->ps,
                                pEnt->client->ps.weaponslots[9]);
        }
    }
    else
    {
        BG_GetRandomAmmoCounts(ammo, clip, iWeaponIndex);
    }
    pDrop->count = ammo;
    pDrop->count2 = clip;
    if (ammo == 0)
        pDrop->count = -1;
    if (clip == 0)
        pDrop->count2 = -1;
    if (pszTag != nullptr)
    {
        if (!s_pszTagHashInit)
        {
            s_pszTagHashInit = true;
            pszTag_hash = HashString::CalcHash(pszTag);
        }
        DObjSkelMat mat;
        float angles[3];
        float yaw;
        if (G_DObjGetWorldTagMatrix(pEnt, pszTag_hash, &mat) != 0)
        {
            math::Position3 start;
            start.v.m128_f32[0] = pEnt->r.currentOrigin.v.m128_f32[0]
                                + (pEnt->r.maxs.v.m128_f32[0]
                                   + pEnt->r.mins.v.m128_f32[0]) * 0.5f;
            start.v.m128_f32[1] = pEnt->r.currentOrigin.v.m128_f32[1]
                                + (pEnt->r.maxs.v.m128_f32[1]
                                   + pEnt->r.mins.v.m128_f32[1]) * 0.5f;
            start.v.m128_f32[2] = pEnt->r.currentOrigin.v.m128_f32[2]
                                + (pEnt->r.maxs.v.m128_f32[2]
                                   + pEnt->r.mins.v.m128_f32[2]) * 0.5f;
            collision_context_t context(pEnt->mHandle, 1041);
            trace_t trace;
            math::Position3 tmp;
            const math::Position3* end = native_to_cdl_pos3(&tmp, &mat.origin[0]);
            g_TraceCapsule(&trace, start, pDrop->r.mins, pDrop->r.maxs,
                           *end, context);
            pDrop->s.pos.trBase[0] = trace.endpos.v.m128_f32[0];
            pDrop->s.pos.trBase[1] = trace.endpos.v.m128_f32[1];
            pDrop->s.pos.trBase[2] = trace.endpos.v.m128_f32[2];
            if (IS_NAN(pDrop->r.currentOrigin.v.m128_f32[0])
                || IS_NAN(pDrop->r.currentOrigin.v.m128_f32[1])
                || IS_NAN(pDrop->r.currentOrigin.v.m128_f32[2]))
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_items.cpp";
                AeAssert::gCurrentLine = 1316;
                AeAssert::gCurrentExpr =
                    "!IS_NAN((pDrop->r.currentOrigin)[0]) && "
                    "!IS_NAN((pDrop->r.currentOrigin)[1]) && "
                    "!IS_NAN((pDrop->r.currentOrigin)[2])";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                    __debugbreak();
            }
            pDrop->r.currentOrigin.v.m128_f32[0] = trace.endpos.v.m128_f32[0];
            pDrop->r.currentOrigin.v.m128_f32[1] = trace.endpos.v.m128_f32[1];
            pDrop->r.currentOrigin.v.m128_f32[2] = trace.endpos.v.m128_f32[2];
            pDrop->s.pos.trTime = level.time;
            Axis4ToAngles(mat.axis, angles);
            yaw = angles[2];
        }
        else
        {
            angles[0] = pEnt->r.currentAngles.v.m128_f32[0];
            angles[1] = pEnt->r.currentAngles.v.m128_f32[1];
            yaw = pEnt->r.currentAngles.v.m128_f32[2];
        }
        angles[2] = yaw + 90.0f;
        G_SetAngle(pDrop, &pEnt->r.currentAngles);
        pDrop->s.apos.trType = TR_LINEAR;
        pDrop->s.apos.trTime = level.time;
        pDrop->s.apos.trDelta[0] = ((rand() * 0.000061035156f) - 1.0f) * 50.0f;
        pDrop->s.apos.trDelta[1] = ((rand() * 0.000061035156f) - 1.0f) * 40.0f;
        pDrop->s.apos.trDelta[2] = ((rand() * 0.000061035156f) - 1.0f) * 60.0f;
        if (IS_NAN(pDrop->s.pos.trDelta[0])
            || IS_NAN(pDrop->s.pos.trDelta[1])
            || IS_NAN(pDrop->s.pos.trDelta[2]))
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_items.cpp";
            AeAssert::gCurrentLine = 1334;
            AeAssert::gCurrentExpr =
                "!IS_NAN((pDrop->s.pos.trDelta)[0]) && "
                "!IS_NAN((pDrop->s.pos.trDelta)[1]) && "
                "!IS_NAN((pDrop->s.pos.trDelta)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
    }
    pDrop->think = THINK__G_FreeEntity;
    pDrop->nextthink = level.time + 30000;
    return pDrop;
}

// ea: 0x00475910
int Pickup_Health(Entity* ent, Entity* other)
{
    int quantity = ent->item->quantity;
    Client* client;
    int v5, v6;
    if (quantity == 5 || quantity == 100)
    {
        client = other->client;
        v5 = client->ps.stats[2];
        v6 = 2 * v5;
    }
    else
    {
        client = other->client;
        v5 = client->ps.stats[2];
        v6 = v5;
    }
    int other2 = ent->count;
    if (other2 == 0)
        other2 = quantity;
    int health = other->health;
    int v8 = health - (int)((v5 * other2) * -0.0099999998f);
    other->health = v8;
    if (v8 <= v6)
    {
        int v9 = client->ps.stats[2];
        int v10 = (100 * v8) / v9;
        if (v10 >= 1)
        {
            if (v10 > 100)
                v10 = 100;
        }
        else
        {
            v10 = 1;
        }
        int v11 = (100 * health) / v9;
        if (v11 < 1)
            v11 = 1;
        int v12 = other2 + v11;
        if (v12 > 100)
            v12 = 100;
        if (v10 != v12)
        {
            unsigned int v13 = (unsigned int)((1374389535LL * v12 * v9) >> 32) >> 5;
            other->health = (int)(v13 + (v13 >> 31));
        }
    }
    else
    {
        other->health = v6;
    }
    client->ps.stats[0] = other->health;
    Scr_Notify(ent, hash_const.trigger, 1u);
    PostEffectEventScriptCall(ent, "HEALTH_PICKUP", false, PAK_ID_INVALID, false);
    if (other->client != nullptr)
        Scr_Notify(other, hash_const.pickup, 1u);
    return -1;
}

// ea: 0x00475890
int Pickup_Ammo(Entity* ent, Entity* other, int bTouched)
{
    int count = ent->count;
    if (count == 0)
        count = ent->item->quantity;
    int result = Add_Ammo(other, ent->item->giTag, count, 0);
    if (result != 0)
    {
        unsigned int e = other->mHandle.mHandle.mVal;
        ent->Notify(hash_const.trigger, &e);
        if (other->client != nullptr)
            Scr_Notify(other, hash_const.pickup, 1);
        return (ent->spawnflags & 8) != 0 ? 40 : -1;
    }
    return result;
}

// ea: 0x0044B3B0
int Pickup_Weapon_Ammo(Entity* ent, Entity* other)
{
    if (gpBrocAPI->mBrocExports.mCallbackGiveAmmoPack != nullptr)
    {
        gpBrocAPI->mBrocExports.mCallbackGiveAmmoPack(
            other->mHandle.mHandle.mVal, ent->count);
    }
    return -1;
}

// ea: 0x0044B3F0
int Pickup_Kit(Entity* ent, Entity* other, int bTouched)
{
    if (gpBrocAPI->mBrocExports.mCallbackPickupKit != nullptr)
    {
        if (other->client == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_items.cpp";
            AeAssert::gCurrentLine = 287;
            AeAssert::gCurrentExpr = "other->client";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        int count = ent->count;
        if (other->client->pers.playerClass == count)
            return 0;
        MultiplayerMgr::MPEntityHandle handle =
            MultiplayerMgr::sInst->FindDroppedItemID(kItemTypeMax, ent,
                                                     nullptr);
        if (handle.mVal == 0)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_items.cpp";
            AeAssert::gCurrentLine = 295;
            AeAssert::gCurrentExpr = "handle.IsAssigned()";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Pickup_Weapon could not find the "
                                    "network entity from the dropped weapon"))
                __debugbreak();
            if (handle.mVal == 0)
            {
                MultiplayerMgr::MPEntityHandle v6;
                MultiplayerMgr::sInst->GetNextDroppedItemID(
                    &v6, kItemTypeMax, nullptr);
                handle.mVal = v6.mVal;
            }
        }
        MultiplayerMgr::sInst->RegisterDroppedItem(kItemTypeMax, ent, other,
                                                   handle.mVal & 0x7FF);
        MultiplayerMgr::sInst->SwapKit(other->client->pers.playerClass,
                                       handle.mVal);
        gpBrocAPI->mBrocExports.mCallbackPickupKit(
            other->mHandle.mHandle.mVal, count);
    }
    return -1;
}

// ea: 0x00484F20
int Pickup_Weapon(Entity* ent, Entity* other, int* piMakeNoise, int bTouched)
{
    int giTag = ent->item->giTag;
    weaponFileInfo_t* pWeap = BG_GetInfoForWeapon(giTag);
    if (pWeap->slot == WEAPSLOT_PISTOL)
    {
        const char* ammoTypeName = BG_GetAmmoTypeName(giTag);
        Entity* player = EntityManager::sInst->GetPlayer(currCl);
        const char* v9 =
            BG_GetAmmoTypeName(player->client->ps.weaponslots[3]);
        if (_strnicmp(v9, ammoTypeName, strlen(ammoTypeName)) == 0)
        {
            giTag = EntityManager::sInst->GetPlayer(currCl)
                        ->client->ps.weaponslots[3];
            pWeap = BG_GetInfoForWeapon(giTag);
        }
    }
    int quantity;
    int count = ent->count;
    if (count < 0)
    {
        quantity = 0;
        goto clip_stage;
    }
    if (count == 0)
    {
        int iDropAmmoMax = pWeap->iDropAmmoMax;
        int iDropAmmoMin = pWeap->iDropAmmoMin;
        if (iDropAmmoMax < iDropAmmoMin)
        {
            int tmp = iDropAmmoMax;
            iDropAmmoMax = iDropAmmoMin;
            iDropAmmoMin = tmp;
        }
        if (iDropAmmoMax != 0)
        {
            if (iDropAmmoMax < 0)
            {
                ent->count = 0;
                goto ammo_clamp;
            }
        }
        else if (iDropAmmoMin == 0)
        {
            int maxClip = BG_GetAmmoClipSize(BG_ClipForWeapon(giTag)) - 1;
            ent->count = (int)((random() + 1.0f) * maxClip * 0.5f + 0.5f) + 1;
            goto ammo_clamp;
        }
        if (iDropAmmoMax < iDropAmmoMin)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_items.cpp";
            AeAssert::gCurrentLine = 379;
            AeAssert::gCurrentExpr = "iMax >= iMin";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        int v17 = (iDropAmmoMax == iDropAmmoMin)
                      ? iDropAmmoMin
                      : iDropAmmoMin + rand() % (iDropAmmoMax - iDropAmmoMin);
        ent->count = v17;
        if (v17 <= 0)
            ent->count = 0;
    }
ammo_clamp:
    if (ent->count > BG_GetAmmoTypeMax(BG_AmmoForWeapon(giTag)))
        ent->count = BG_GetAmmoTypeMax(BG_AmmoForWeapon(giTag));
    quantity = ent->count;
clip_stage:
    int iClipAmmo;
    if (ent->count2 >= 0)
    {
        if (ent->count2 == 0)
        {
            if (ent->count >= 0)
            {
                int clipSize = BG_GetAmmoClipSize(BG_ClipForWeapon(giTag));
                ent->count2 = clipSize;
                if (clipSize > ent->count)
                    ent->count2 = ent->count;
                ent->count -= ent->count2;
                quantity = ent->count;
            }
            else
            {
                ent->count2 = 0;
            }
        }
        if (ent->count2 > BG_GetAmmoClipSize(BG_ClipForWeapon(giTag)))
            ent->count2 = BG_GetAmmoClipSize(BG_ClipForWeapon(giTag));
        iClipAmmo = ent->count2;
    }
    else
    {
        iClipAmmo = 0;
    }
    if (Com_BitCheck(other->client->ps.weapons, giTag) != 0)
    {
        *piMakeNoise = 173;
        quantity += iClipAmmo;
        if (g_gameskill->integer != 2 && g_gameskill->integer != 3)
        {
            int maxAmmo = BG_GetAmmoTypeMax(BG_AmmoForWeapon(giTag));
            int curTotal =
                other->client->ps.ammoclip[BG_AmmoForWeapon(giTag)]
                + other->client->ps.ammo[BG_AmmoForWeapon(giTag)];
            int dropMax = BG_GetInfoForWeapon(giTag)->iDropAmmoMax;
            int dropMin = BG_GetInfoForWeapon(giTag)->iDropAmmoMin;
            float v55 = 0.1f;
            float v56 = 0.7f;
            if (g_gameskill->integer == 0)
            {
                v55 = 0.3f;
                v56 = 0.9f;
            }
            float v57 = 1.0f
                        - ((curTotal / (float)maxAmmo - v55) / (v56 - v55));
            if (v57 < 0.0f)
                v57 = 0.0f;
            else if (v57 > 1.0f)
                v57 = 1.0f;
            quantity = (int)((dropMax - dropMin) * v57) + dropMin;
        }
        int v58 = Add_Ammo(other, giTag, quantity, 0);
        if (v58 == quantity)
            goto pickup_done;
        ent->count -= v58;
        if (ent->count <= 0)
        {
            ent->count2 += ent->count;
            ent->count = -1;
            if (ent->count2 <= 0)
                ent->count2 = -1;
        }
        if ((ent->count <= 0 && ent->count2 <= 0)
            || g_weaponAmmoPools.integer == 0)
            goto pickup_done;
        return 0;
    }
    if (other->client->ps.weapon != 0
        && Com_BitCheck(other->client->ps.weapons,
                        other->client->ps.weapon) != 0
        && BG_IsPlayerWeaponInSlot(&other->client->ps,
                                   other->client->ps.weapon, 1)
               == WEAPSLOT_NONE)
    {
        weaponFileInfo_t* v28 =
            BG_GetInfoForWeapon(other->client->ps.weapon);
            if (BG_GetStackSlotForWeapon(&other->client->ps,
                                         other->client->ps.weapon,
                                         (weapSlot_t)v28->slot) == 0
            && BG_GetEmptySlotForWeapon(&other->client->ps, giTag) == 0)
        {
            Com_Printf("WARNING: cannot swap out a debug weapon (can result "
                       "from too many weapons given to the player)\n");
            return 0;
        }
    }
    if (BG_GetEmptySlotForWeapon(&other->client->ps, giTag) == 0)
    {
        weaponFileInfo_t* v30 =
            BG_GetInfoForWeapon(other->client->ps.weapon);
        if (BG_GetStackSlotForWeapon(&other->client->ps, giTag,
                                     (weapSlot_t)v30->slot) == 0)
        {
            int iWeap = other->client->ps.weaponslots[2];
            Entity* pDropped = Drop_Weapon(other, iWeap, nullptr);
            if (pDropped == nullptr)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_items.cpp";
                AeAssert::gCurrentLine = 500;
                AeAssert::gCurrentExpr = "pDropped";
                if (!AeAssert::IsIgnored())
                {
                    const char* v34 = va(
                        "weapon=%s iWeap=%s", pWeap->szInternalName,
                        BG_GetInfoForWeapon(iWeap)->szInternalName);
                    if (AeAssert::Assert(v34))
                        return 0;
                }
                return 0;
            }
            float vPos[3];
            vPos[0] = ent->r.currentOrigin.v.m128_f32[0];
            vPos[1] = ent->r.currentOrigin.v.m128_f32[1];
            vPos[2] = ent->r.currentOrigin.v.m128_f32[2];
            G_SetOrigin(pDropped, vPos);
            G_SetAngle(pDropped, &ent->r.currentAngles);
            g_LinkEntity(pDropped);
            if (iWeap != 0)
            {
                pDropped->r.mOwner = ent->r.mOwner;
                Entity* owner = HandleDbToEnt(ent->r.mOwner);
                MultiplayerMgr::MPEntityHandle handle =
                    MultiplayerMgr::sInst->FindDroppedItemID(
                        kItemTypeWeapons, ent, owner);
                if (handle.mVal == 0)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\g_items.cpp";
                    AeAssert::gCurrentLine = 547;
                    AeAssert::gCurrentExpr = "handle.IsAssigned()";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Pickup_Weapon could not find the "
                                            "network entity from the dropped "
                                            "weapon"))
                        __debugbreak();
                    if (handle.mVal == 0)
                    {
                        MultiplayerMgr::MPEntityHandle v85;
                        Entity* owner2 = HandleDbToEnt(ent->r.mOwner);
                        MultiplayerMgr::sInst->GetNextDroppedItemID(
                            &v85, kItemTypeWeapons, owner2);
                        handle.mVal = v85.mVal;
                    }
                }
                Entity* owner3 = HandleDbToEnt(ent->r.mOwner);
                MultiplayerMgr::sInst->RegisterDroppedItem(
                    kItemTypeWeapons, pDropped, owner3,
                    handle.mVal & 0x7FF);
                MultiplayerMgr::sInst->SwapWeapon(
                    iWeap, handle.mVal, pDropped->count2, pDropped->count);
            }
            else
            {
                Entity* owner = HandleDbToEnt(ent->r.mOwner);
                MultiplayerMgr::sInst->RegisterDroppedItem(kItemTypeWeapons,
                                                           pDropped, owner);
            }
        }
    }
    if (BG_GetEmptySlotForWeapon(&other->client->ps, giTag) == 0)
    {
        weaponFileInfo_t* v40 =
            BG_GetInfoForWeapon(other->client->ps.weapon);
        if (BG_GetStackSlotForWeapon(&other->client->ps, giTag,
                                     (weapSlot_t)v40->slot) == 0
            && !AeAssert::IsIgnored())
        {
            int iWeap = other->client->ps.weaponslots[4] != 0
                            ? Com_BitCheck(other->client->ps.weapons,
                                           other->client->ps.weaponslots[4])
                            : -1;
            const char* handle = other->client->ps.weaponslots[4] != 0
                                     ? BG_GetInfoForWeapon(
                                           other->client->ps.weaponslots[4])
                                           ->szInternalName
                                     : "none";
            int v85 = other->client->ps.weaponslots[3] != 0
                          ? Com_BitCheck(other->client->ps.weapons,
                                         other->client->ps.weaponslots[3])
                          : -1;
            const char* szInternalName =
                other->client->ps.weaponslots[3] != 0
                    ? BG_GetInfoForWeapon(
                          other->client->ps.weaponslots[3])->szInternalName
                    : "none";
            int sa_max_drop_ammo =
                other->client->ps.weaponslots[2] != 0
                    ? Com_BitCheck(other->client->ps.weapons,
                                   other->client->ps.weaponslots[2])
                    : -1;
            const char* sa_max_ammo =
                other->client->ps.weaponslots[2] != 0
                    ? BG_GetInfoForWeapon(
                          other->client->ps.weaponslots[2])->szInternalName
                    : "none";
            int v44 = other->client->ps.weaponslots[1] != 0
                          ? Com_BitCheck(other->client->ps.weapons,
                                         other->client->ps.weaponslots[1])
                          : -1;
            const char* v46 = other->client->ps.weaponslots[1] != 0
                                  ? BG_GetInfoForWeapon(
                                        other->client->ps.weaponslots[1])
                                        ->szInternalName
                                  : "none";
            int v47 =
                Com_BitCheck(other->client->ps.weapons, giTag);
            const char* v48 = va(
                "weapon=%s, owned=%i slot1='%s'(%i) slot2='%s'(%i) "
                "slot3='%s'(%i) slot4='%s'(%i)",
                pWeap->szInternalName, v47, v46, v44, sa_max_ammo,
                sa_max_drop_ammo, szInternalName, v85, handle, iWeap);
            if (AeAssert::Assert(v48))
                __debugbreak();
        }
    }
    BG_GivePlayerWeapon(&other->client->ps, giTag);
    if (bTouched == 0 && other->IsLocalPlayer()
        && pWeap->pickupWithoutSelect == 0)
    {
        BG_SelectWeaponIndex(giTag, other->GetPlayerIndex());
    }
    if (pWeap->weapClass == WEAPCLASS_GRENADE)
    {
        quantity += iClipAmmo;
    }
    else if (iClipAmmo >= 0)
    {
        int clipSize = BG_GetAmmoClipSize(BG_ClipForWeapon(giTag));
        if (iClipAmmo > clipSize)
        {
            quantity += iClipAmmo - clipSize;
            iClipAmmo = BG_GetAmmoClipSize(BG_ClipForWeapon(giTag));
        }
        other->client->ps.ammoclip[BG_ClipForWeapon(giTag)] = iClipAmmo;
    }
    Add_Ammo(other, giTag, quantity, iClipAmmo == -1);
pickup_done:
    Scr_Notify(ent, hash_const.trigger, 2);
    if (other->client != nullptr)
        Scr_Notify(other, hash_const.pickup, 2);
    if ((ent->spawnflags & 8) == 0)
        return -1;
    ent->count = 0;
    ent->count2 = 0;
    return g_weaponRespawn.integer;
}

// ea: 0x004859C0
void Touch_Item(Entity* ent, Entity* other, int bTouched)
{
    if (ent->active == 0)
        return;
    ent->active = 0;
    Client* client = other->client;
    if (client == nullptr || other->health < 1
        || (client->ps.eFlags & 0x100000) != 0 || !other->IsLocalPlayer()
        || (ent->s.pos.trType == TR_GRAVITY
            && ent->r.mOwner.mHandle.mVal == other->mHandle.mHandle.mVal))
    {
        return;
    }
    if (BG_CanItemBeGrabbed(&ent->s, &other->client->ps, bTouched) != 0)
    {
        int makenoise = 171;
        int v24 = 0;
        switch (ent->item->giType)
        {
        case IT_WEAPON:
            v24 = Pickup_Weapon(ent, other, &makenoise, bTouched);
            goto pickup_done;
        case IT_AMMO:
            v24 = Pickup_Ammo(ent, other, bTouched);
            goto pickup_done;
        case IT_WEAPON_HEALTH:
        case IT_HEALTH:
            v24 = Pickup_Health(ent, other);
            goto pickup_done;
        case IT_WEAPON_AMMO:
            v24 = Pickup_Weapon_Ammo(ent, other);
            goto pickup_done;
        case IT_KIT:
            if (ent->count == client->pers.playerClass)
            {
                DbLinkedHandle<EntityHandleDb, Entity> h;
                h.mHandle.mVal = other->mHandle.mHandle.mVal;
                SV_GameSendServerCommand(h,
                                         va("gm \"MPGAME_PICKUP_KIT_SAME\""));
            }
            else if (client->ps.ctf_has_flag != 0)
            {
                DbLinkedHandle<EntityHandleDb, Entity> h;
                h.mHandle.mVal = other->mHandle.mHandle.mVal;
                SV_GameSendServerCommand(h,
                                         va("gm \"MPGAME_PICKUP_KIT_FLAG\""));
            }
            else
            {
                v24 = Pickup_Kit(ent, other, bTouched);
            pickup_done:
                if (v24 != 0)
                {
                    G_DPrintf("Item: %i %s\n", other->mHandle.mHandle.mVal,
                              ent->item->classname);
                    if (ent->noise_index != 0)
                        makenoise = 172;
                    G_AddEvent(other, makenoise, ent->s.brushmodel);
                    if (ent->wait == -1.0f)
                    {
                        ent->flags |= 0x400;
                        ent->r.svFlags |= 1;
                        ent->s.eFlags |= 0x80;
                        ent->r.contents = 0;
                        ent->r.eventType |= 2;
                    }
                    else
                    {
                        int wait = (int)ent->wait;
                        if (ent->wait != 0.0f)
                            wait = (int)ent->wait;
                        if (ent->random != 0.0f)
                        {
                            float r = random();
                            wait += (int)((r + r - 1.0f) * ent->random);
                            if (wait < 1)
                                wait = 1;
                        }
                        if ((ent->flags & 0x40) != 0)
                            ent->r.eventType |= 1;
                        ent->flags |= 0x400;
                        ent->r.svFlags |= 1;
                        ent->r.contents = 0;
                        if (wait > 0)
                        {
                            ent->nextthink = level.time + 1000 * wait;
                            ent->think = THINK__multi_wait;
                        }
                        else
                        {
                            ent->nextthink = 0;
                            ent->think = THINK__NULL;
                        }
                        if (wait == -1)
                        {
                            int droppedType =
                                MultiplayerMgr::sInst->GetDroppedItemType(
                                    ent->item->giType);
                            Entity* owner = HandleDbToEnt(ent->r.mOwner);
                            MultiplayerMgr::MPEntityHandle netIndex =
                                MultiplayerMgr::sInst->FindDroppedItemID(
                                    droppedType, ent, owner);
                            if (netIndex.mVal != 0)
                            {
                                MultiplayerMgr::sInst->PickupItem(
                                    netIndex.mVal, droppedType, other, false);
                            }
                            void* mem = Task::sAllocator->Allocate(0x1C, false);
                            EntityDeathTask* task =
                                mem ? new (mem) EntityDeathTask(ent->mHandle)
                                    : nullptr;
                            TaskSys::sInst->PostTask(task);
                        }
                        else
                        {
                            g_LinkEntity(ent);
                        }
                    }
                }
            }
            break;
        default:
            return;
        }
    }
    else if (bTouched == 0)
    {
        const gitem_s* item = ent->item;
        if (item->giType == IT_WEAPON)
        {
            int giTag = item->giTag;
            if (Com_BitCheck(other->client->ps.weapons, giTag) != 0)
            {
                weaponFileInfo_t* info = BG_GetInfoForWeapon(giTag);
                DbLinkedHandle<EntityHandleDb, Entity> h;
                h.mHandle.mVal = other->mHandle.mHandle.mVal;
                SV_GameSendServerCommand(
                    h, va("gm \"GAME_PICKUP_CANTCARRYMOREAMMO%s\"",
                          info->szDisplayName));
            }
            else
            {
                const char* msg = nullptr;
                switch (BG_GetInfoForWeapon(giTag)->slot)
                {
                case WEAPSLOT_PRIMARY:
                case WEAPSLOT_PRIMARYB:
                    msg = "gm \"GAME_CANT_GET_PRIMARY_WEAP_MESSAGE\"";
                    break;
                case WEAPSLOT_PISTOL:
                    msg = "gm \"GAME_CANT_GET_PISTOL_WEAP_MESSAGE\"";
                    break;
                case WEAPSLOT_GRENADE:
                    msg = "gm \"GAME_CANT_GET_GRENADE_WEAP_MESSAGE\"";
                    break;
                case WEAPSLOT_SMOKE_GRENADE:
                    msg = "gm \"GAME_CANT_GET_SMOKE_GRENADE_WEAP_MESSAGE\"";
                    break;
                case WEAPSLOT_SPECIAL:
                    msg = "gm \"GAME_CANT_GET_SPECIAL_WEAP_MESSAGE\"";
                    break;
                default:
                    return;
                }
                DbLinkedHandle<EntityHandleDb, Entity> h;
                h.mHandle.mVal = other->mHandle.mHandle.mVal;
                SV_GameSendServerCommand(h, va(msg));
            }
        }
    }
}

// ea: 0x004750B0
Entity* SpawnHelmet(Entity* self, const float* hitP, const float* hitDir)
{
    if (EntityManager::sInst->GetPlayer(currCl) == self)
        return nullptr;
    Broc::string helmetName;
    if ((self->flags & 0x2000000) == 0 && self->actor != nullptr)
        helmetName = self->actor->mPopedHelmetName;
    else
        helmetName = self->mTarget;
    if (helmetName.is_empty())
        return nullptr;
    static unsigned int sHelmetHashInit = 0;
    static unsigned int helmetHash = 0;
    if ((sHelmetHashInit & 1) == 0)
    {
        sHelmetHashInit |= 1u;
        helmetHash = HashString::CalcHash("Bip01 Helmet");
    }
    int boneIndex = SV_DObjGetBoneIndex(self, helmetHash);
    if (boneIndex < 0)
        return nullptr;
    TPakId pakId = (TPakId)self->mPakId;
    if (pakId == PAK_ID_INVALID)
        pakId = CurPakId();
    Entity* v9 = G_Spawn(pakId);
    if (v9 == nullptr || HandleDbToEnt(v9->mHandle) == nullptr)
        return nullptr;
    v9->mClassName = helmetName;
    HashString hs(v9->mClassName);
    v9->mClassNameHash = hs;
    v9->spawnflags = 0;
    const char* modelName = v9->mClassName.GetBuff();
    G_SetModel(v9, modelName, (TPakId)v9->mPakId, 0);
    v9->s.brushmodel = 0;
    SV_SetBrushModel(v9);
    DObjSkelMat mat;
    G_DObjGetWorldBoneIndexMatrix(self, boneIndex, &mat);
    float origin[3] = { mat.origin[0], mat.origin[1], mat.origin[2] };
    G_SetOrigin(v9, origin);
    float angles[3] = { self->r.currentAngles.v.m128_f32[0],
                        self->r.currentAngles.v.m128_f32[1] + 90.0f,
                        self->r.currentAngles.v.m128_f32[2] };
    G_SetAngle(v9, angles);
    G_DObjUpdate(v9, false);
    g_LinkEntity(v9);

    math::Position3 center;
    if (v9->r.bmodel != nullptr)
    {
        const math::Position3* bmin =
            (const math::Position3*)((const char*)v9->r.bmodel + 0x30);
        const math::Position3* bmax =
            (const math::Position3*)((const char*)v9->r.bmodel + 0x40);
        center.v.m128_f32[0] = v9->r.currentOrigin.v.m128_f32[0]
                             + (bmin->v.m128_f32[0]
                                + bmax->v.m128_f32[0]) * 0.5f;
        center.v.m128_f32[1] = v9->r.currentOrigin.v.m128_f32[1]
                             + (bmin->v.m128_f32[1]
                                + bmax->v.m128_f32[1]) * 0.5f;
        center.v.m128_f32[2] = v9->r.currentOrigin.v.m128_f32[2]
                             + (bmin->v.m128_f32[2]
                                + bmax->v.m128_f32[2]) * 0.5f;
    }
    else
    {
        center.v.m128_f32[0] =
            (v9->r.absmin.v.m128_f32[0] + v9->r.absmax.v.m128_f32[0]) * 0.5f;
        center.v.m128_f32[1] =
            (v9->r.absmin.v.m128_f32[1] + v9->r.absmax.v.m128_f32[1]) * 0.5f;
        center.v.m128_f32[2] =
            (v9->r.absmin.v.m128_f32[2] + v9->r.absmax.v.m128_f32[2]) * 0.5f;
    }
    math::Position3 hitp;
    hitp.v.m128_f32[0] = (hitP[0] + center.v.m128_f32[0]) * 0.5f;
    hitp.v.m128_f32[1] = (hitP[1] + center.v.m128_f32[1]) * 0.5f;
    hitp.v.m128_f32[2] = (hitP[2] + center.v.m128_f32[2]) * 0.5f;
    math::Dir3 hitd;
    hitd.v.m128_f32[0] = hitDir[0];
    hitd.v.m128_f32[1] = hitDir[1];
    float vert = fabsf(hitDir[2]) * 5.0f;
    if (vert < 1.5f)
        vert = 1.5f;
    else if (vert > 2.2f)
        vert = 2.2f;
    hitd.v.m128_f32[2] = vert;
    hitd.v.m128_f32[3] = 0.0f;
    float force = ((rand() % 100) * 0.01f + 1.0f) * 2.0f;
    if (force < 2.0f)
        force = 2.0f;
    else if (force > 4.0f)
        force = 4.0f;
    ApplyPhysics(v9, &hitp, &hitd, force, false, HITLOC_TORSO_UPR);

    v9->think = THINK__G_FreeEntity;
    v9->nextthink = level.time + timeToAdd;
    IVPointer<Destructible> d =
        ((DestructibleBankManager*)DestructibleBankManager::sInst)
            ->GetDestructible((TPakId)v9->mPakId, "global");
    if (d.mValue != nullptr)
    {
        v9->mDestructible.mValue = d.mValue;
        v9->mDestructible.mPakId = d.mPakId;
        v9->takedamage = 1;
        IVPointer<PhysData> pd =
            ((PhysDataBankManager*)PhysDataBankManager::sInst)
                ->GetPhysData((TPakId)v9->mPakId, "helmet_pop");
        if (pd.mValue != nullptr)
        {
            ValidatePakId((TPakId)pd.mPakId);
            pd.mValue->mMass = helmetMass;
            ValidatePakId((TPakId)pd.mPakId);
            pd.mValue->mFric = helmetFriction;
            ValidatePakId((TPakId)pd.mPakId);
            pd.mValue->mBounce = helmetBounce;
            v9->mDObj->mPhysData.mValue = pd.mValue;
            v9->mDObj->mPhysData.mPakId = pd.mPakId;
        }
    }
    SoundDevice::sInst->PlaySound("helmetpop", v9->mHandle, true, true, hitp,
                                  hitd, -1.0f, -1.0f, -1.0f, -1.0f);
    self->mFlags &= ~0x10u;
    return v9;
}

// ea: 0x00457A30
void G_BounceItem(Entity* ent, trace_t* trace)
{
    if (bg_itemlist[ent->s.brushmodel].giType == IT_WEAPON)
    {
        PostEffectEventScriptCall(ent, "LAND_WEAPON", false, PAK_ID_INVALID, false);
    }
    else if (bg_itemlist[ent->s.brushmodel].giType == IT_WEAPON_AMMO)
    {
        PostEffectEventScriptCall(ent, "LAND_AMMOPACK", false, PAK_ID_INVALID, false);
    }
    float v17[3];
    BG_EvaluateTrajectoryDelta(&ent->s.pos,
                               level.previousTime + (int)((level.time - level.previousTime) * trace->fraction),
                               v17);
    float v3 = v17[2];
    float v4 = v17[1];
    float v5 = (((trace->normal.v.m128_f32[0] * v17[0]) + (trace->normal.v.m128_f32[2] * v17[2]))
                + (trace->normal.v.m128_f32[1] * v17[1]))
               * -2.0f;
    ent->s.pos.trDelta[0] = (v5 * trace->normal.v.m128_f32[0]) + v17[0];
    float v11 = ent->s.pos.trDelta[0];
    ent->s.pos.trDelta[1] = (v5 * trace->normal.v.m128_f32[1]) + v4;
    ent->s.pos.trDelta[2] = (v5 * trace->normal.v.m128_f32[2]) + v3;
    if (IS_NAN(v11) || IS_NAN(ent->s.pos.trDelta[1]) || IS_NAN(ent->s.pos.trDelta[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_items.cpp";
        AeAssert::gCurrentLine = 1709;
        AeAssert::gCurrentExpr = "!IS_NAN((ent->s.pos.trDelta)[0]) && !IS_NAN((ent->s.pos.trDelta)[1]) && !IS_NAN((ent->s.pos.trDelta)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    float v6 = ent->s.pos.trDelta[0] * 0.0f;
    ent->s.pos.trDelta[0] = ent->s.pos.trDelta[0] * 0.0f;
    ent->s.pos.trDelta[1] = ent->s.pos.trDelta[1] * 0.0f;
    ent->s.pos.trDelta[2] = ent->s.pos.trDelta[2] * 0.0f;
    if (IS_NAN(v6) || IS_NAN(ent->s.pos.trDelta[1]) || IS_NAN(ent->s.pos.trDelta[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_items.cpp";
        AeAssert::gCurrentLine = 1714;
        AeAssert::gCurrentExpr = "!IS_NAN((ent->s.pos.trDelta)[0]) && !IS_NAN((ent->s.pos.trDelta)[1]) && !IS_NAN((ent->s.pos.trDelta)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (trace->startsolid != 0)
    {
        unsigned int mVal = ent->mHandle.mHandle.mVal;
        ent->s.pos.trDelta[2] = 0.0f;
        ent->s.pos.trDelta[1] = 0.0f;
        ent->s.pos.trDelta[0] = 0.0f;
        math::Position3 vStart;
        vStart.v.m128_f32[0] = ent->r.currentOrigin.v.m128_f32[0];
        vStart.v.m128_f32[1] = ent->r.currentOrigin.v.m128_f32[1];
        vStart.v.m128_f32[2] = ent->r.currentOrigin.v.m128_f32[2] - 128.0f;
        collision_context_t context;
        context.__vftable = nullptr;
        context.pass_entity1.mHandle.mVal = 0;
        context.pass_entity2.mHandle.mVal = 1041;
        context.contentmask = (int)(ent->r.currentOrigin.v.m128_f32[2] + 64.0f);
        if ((ent->s.eFlags & 0x10) != 0)
        {
            SV_Trace(trace, &ent->r.currentOrigin, &ent->r.mins, &ent->r.maxs,
                     &vStart, &context, 1, 0, nullptr, 0, 0.0f);
        }
        else
        {
            SV_Trace(trace, &ent->r.currentOrigin, &ent->r.mins, &ent->r.maxs,
                     &vStart, &context, 0, 0, nullptr, 0, 0.0f);
        }
        if (trace->startsolid != 0)
            trace->endpos = ent->r.currentOrigin;
    }
    if (trace->normal.v.m128_f32[2] <= 0.0f || ent->s.pos.trDelta[2] >= 40.0f)
    {
        ent->r.currentOrigin.v.m128_f32[0] += trace->normal.v.m128_f32[0];
        ent->r.currentOrigin.v.m128_f32[1] += trace->normal.v.m128_f32[1];
        ent->r.currentOrigin.v.m128_f32[2] += trace->normal.v.m128_f32[2];
        memcpy(ent->s.pos.trBase, &ent->r.currentOrigin, sizeof(ent->s.pos.trBase));
        ent->s.pos.trTime = level.time;
    }
    else if (ent->s.eType == 13)
    {
        G_SetOrigin(ent, &trace->endpos);
        ent->s.mGroundEntity.mHandle.mVal = trace->mEntity.mHandle.mVal;
        j_nullsub_74(ent, 0);
        g_LinkEntity(ent);
    }
    else
    {
        trace->endpos.v.m128_f32[2] = ((float)rand() * 0.000015258789f) + 0.5f + trace->endpos.v.m128_f32[2];
        G_SetOrigin(ent, &trace->endpos);
        ent->s.mGroundEntity.mHandle.mVal = trace->mEntity.mHandle.mVal;
        float normal[3];
        normal[0] = trace->normal.v.m128_f32[0];
        normal[1] = trace->normal.v.m128_f32[1];
        normal[2] = trace->normal.v.m128_f32[2];
        float fwd[3];
        float cross1[3];
        float cross2[3];
        AnglesToForward(ent->r.currentAngles.v.m128_f32, fwd);
        CrossProduct(normal, fwd, cross1);
        CrossProduct(cross1, normal, cross2);
        float ang[3];
        AxisToAngles((const float(*)[3])cross2, ang);
        if (bg_itemlist[ent->s.brushmodel].giType == IT_WEAPON)
            ang[0] += 90.0f;
        G_SetAngle(ent, ang);
        g_LinkEntity(ent);
        ent->s.pos.trType = TR_STATIONARY;
        memcpy(ent->s.pos.trBase, &ent->r.currentOrigin, sizeof(ent->s.pos.trBase));
    }
}

// ea: 0x00476720
void G_RunItem(Entity* ent, int msec)
{
    unsigned int mVal = ent->s.mGroundEntity.mHandle.mVal;
    if (mVal == 0 && (ent->spawnflags & 1) == 0 && ent->s.pos.trType != TR_GRAVITY)
    {
        ent->s.pos.trType = TR_GRAVITY;
        ent->s.pos.trTime = level.time;
    }
    trType_t trType = ent->s.pos.trType;
    if (trType != TR_STATIONARY && trType != TR_GRAVITY_PAUSED && Entity_has_zone_collision(ent))
    {
        math::Position3 dir;
        BG_EvaluateTrajectory(&ent->s.pos, level.time, dir);
        if (IS_NAN(dir.v.m128_f32[0]) || IS_NAN(dir.v.m128_f32[1]) || IS_NAN(dir.v.m128_f32[2]))
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_items.cpp";
            AeAssert::gCurrentLine = 1822;
            AeAssert::gCurrentExpr = "!IS_NAN((origin)[0]) && !IS_NAN((origin)[1]) && !IS_NAN((origin)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        float delta[3];
        delta[0] = dir.v.m128_f32[0] - ent->r.currentOrigin.v.m128_f32[0];
        delta[1] = dir.v.m128_f32[1] - ent->r.currentOrigin.v.m128_f32[1];
        delta[2] = dir.v.m128_f32[2] - ent->r.currentOrigin.v.m128_f32[2];
        if (VectorNormalize(delta) >= 0.0049999999f)
        {
            int clipmask = ent->clipmask;
            if (clipmask == 0)
                clipmask = 1169;
            collision_context_t context1(ent->r.mOwner, clipmask);
            trace_t trace;
            if ((ent->s.eFlags & 0x10) != 0)
            {
                collision_context_t v7(ent->mHandle, 42008593);
                g_TraceCapsule(&trace, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, dir, v7);
            }
            else
            {
                g_Trace(&trace, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, dir, context1);
            }
            if (IS_NAN(ent->r.currentOrigin.v.m128_f32[0])
                || IS_NAN(ent->r.currentOrigin.v.m128_f32[1])
                || IS_NAN(ent->r.currentOrigin.v.m128_f32[2]))
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_items.cpp";
                AeAssert::gCurrentLine = 1848;
                AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                    __debugbreak();
            }
            if (trace.allsolid != 0)
            {
                trace.fraction = 0.0f;
                // keep old origin (do not move into solid)
            }
            else
            {
                ent->r.currentOrigin = trace.endpos;
            }
            g_LinkEntity(ent);
            G_RunThink(ent, msec);
            if (EntityHandleDb::sInst.mElements[ent->mHandle.mHandle.mVal & 0xFFF].mObject != nullptr
                && trace.fraction != 1.0f)
            {
                collision_context_t ctx(0x80000000);
                if (SV_PointContents(ent->r.currentOrigin, ctx) != 0)
                {
                    if (ent->s.eType != 7
                        || EntityHandleDb::sInst.mElements[ent->r.mOwner.mHandle.mVal & 0xFFF].mObject == nullptr)
                        G_FreeEntity(ent, msec);
                }
                else
                {
                    G_BounceItem(ent, &trace);
                }
            }
        }
        else
        {
            G_RunThink(ent, msec);
        }
    }
    else
    {
        G_RunThink(ent, msec);
    }
}

// ea: 0x00461D50
void FinishSpawningItem(Entity* ent, int msec)
{
    if ((ent->spawnflags & 1) != 0)
    {
        if (IS_NAN(ent->r.currentOrigin.v.m128_f32[0])
            || IS_NAN(ent->r.currentOrigin.v.m128_f32[1])
            || IS_NAN(ent->r.currentOrigin.v.m128_f32[2]))
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_items.cpp";
            AeAssert::gCurrentLine = 1409;
            AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        G_SetOrigin(ent, &ent->r.currentOrigin);
        g_LinkEntity(ent);
        return;
    }

    bool isWeapon = ent->item->giType == IT_WEAPON;
    math::Position3 mins;
    math::Position3 maxs;
    mins.v.m128_f32[0] = -1.0f;
    mins.v.m128_f32[1] = -1.0f;
    if (isWeapon)
    {
        mins.v.m128_f32[2] = -1.0f;
        maxs.v.m128_f32[0] = 1.0f;
        maxs.v.m128_f32[1] = 1.0f;
        maxs.v.m128_f32[2] = 1.0f;
    }
    else
    {
        mins.v.m128_f32[2] = 0.0f;
        maxs.v.m128_f32[0] = 1.0f;
        maxs.v.m128_f32[1] = 1.0f;
        maxs.v.m128_f32[2] = 2.0f;
    }
    ent->r.svFlags |= 0x200u;
    ent->s.eFlags |= 0x10;

    trace_t trace;
    trace.allsolid = 0;
    collision_context_t context(ent->mHandle, 1041);
    math::Position3 end;
    end.v.m128_f32[0] = ent->r.currentOrigin.v.m128_f32[0];
    end.v.m128_f32[1] = ent->r.currentOrigin.v.m128_f32[1];
    end.v.m128_f32[2] = ent->r.currentOrigin.v.m128_f32[2] - 4096.0f;
    SV_Trace(&trace, &ent->r.currentOrigin, &mins, &maxs, &end, &context, 1, 0, nullptr, 0, 0.0f);

    bool placed = trace.allsolid == 0;
    if (!placed)
    {
        math::Position3 start;
        start.v.m128_f32[0] = ent->r.currentOrigin.v.m128_f32[0];
        start.v.m128_f32[1] = ent->r.currentOrigin.v.m128_f32[1];
        start.v.m128_f32[2] = ent->r.currentOrigin.v.m128_f32[2] - 15.0f;
        SV_Trace(&trace, &start, &mins, &maxs, &end, &context, 1, 0, nullptr, 0, 0.0f);
        placed = trace.allsolid == 0;
    }
    if (placed)
    {
        ent->s.mGroundEntity = trace.mEntity;
        G_SetOrigin(ent, &trace.endpos);
        if (trace.fraction < 1.0f)
        {
            float fwd[3];
            float cross1[3];
            float cross2[3];
            float normal[3];
            normal[0] = trace.normal.v.m128_f32[0];
            normal[1] = trace.normal.v.m128_f32[1];
            normal[2] = trace.normal.v.m128_f32[2];
            AnglesToForward(ent->r.currentAngles.v.m128_f32, fwd);
            CrossProduct(normal, fwd, cross1);
            CrossProduct(cross1, normal, cross2);
            float ang[3];
            AxisToAngles((const float(*)[3])cross2, ang);
            if (bg_itemlist[ent->s.brushmodel].giType == IT_WEAPON)
                ang[0] += 90.0f;
            G_SetAngle(ent, ang);
        }
        g_LinkEntity(ent);
    }
    else
    {
        const char* classname = ent->mClassName.mBlock != nullptr
                                    ? (const char*)(ent->mClassName.mBlock + 1)
                                    : &defaultFileName[0];
        G_Printf("FinishSpawningItem: %s startsolid at %s\n", classname, vtos(&ent->r.currentOrigin));
        G_FreeEntity(ent, msec);
    }
}

// ea: 0x0044B1A0
int Add_Ammo(Entity* ent, int weapon, int count, int fillClip)
{
    int v4 = BG_AmmoForWeapon(weapon);
    int v6 = BG_ClipForWeapon(weapon);
    Client* client = ent->client;
    int iClipIndex = v6;
    int iOldClip = client->ps.ammoclip[v6];
    int v14 = client->ps.ammo[v4];
    int noPack = 0;
    client->ps.ammo[v4] = count + v14;
    if (BG_WeaponIsClipOnly(weapon) != 0)
    {
        BG_GivePlayerWeapon(&ent->client->ps, weapon);
        noPack = 1;
    }
    if (fillClip != 0 || noPack != 0)
        Fill_Clip(&ent->client->ps, weapon);
    if (ent->actor != nullptr || noPack == 0)
    {
        if (ent->client->ps.ammo[v4] > BG_GetAmmoTypeMax(v4))
            ent->client->ps.ammo[v4] = BG_GetAmmoTypeMax(v4);
    }
    else
    {
        ent->client->ps.ammo[v4] = 0;
    }
    if (ent->client->ps.ammoclip[iClipIndex] > BG_GetAmmoClipSize(iClipIndex))
        ent->client->ps.ammoclip[iClipIndex] = BG_GetAmmoClipSize(iClipIndex);
    if (BG_GetInfoForWeapon(weapon)->iSharedAmmoCapIndex >= 0)
    {
        int counta = BG_GetMaxPickupableAmmo(&ent->client->ps, weapon);
        if (counta < 0)
        {
            Client* v9 = ent->client;
            if (BG_WeaponIsClipOnly(weapon) == 0)
            {
                v9->ps.ammo[v4] += counta;
                Client* v13 = ent->client;
                if (v13->ps.ammo[v4] < 0)
                    v13->ps.ammo[v4] = 0;
                return ent->client->ps.ammo[v4] + ent->client->ps.ammoclip[iClipIndex]
                       - iOldClip - v14;
            }
            v9->ps.ammoclip[iClipIndex] += counta;
            Client* v11 = ent->client;
            if (v11->ps.ammoclip[iClipIndex] <= 0)
            {
                v11->ps.ammoclip[iClipIndex] = 0;
                BG_TakePlayerWeapon(&ent->client->ps, weapon);
                return 0;
            }
            return ent->client->ps.ammo[v4] + ent->client->ps.ammoclip[iClipIndex]
                   - iOldClip - v14;
        }
    }
    return ent->client->ps.ammo[v4] + ent->client->ps.ammoclip[iClipIndex]
           - iOldClip - v14;
}
