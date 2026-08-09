// ============================================================================
// g_items.cpp - item spawning/pickup/bounce (g.o: g_items.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

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
