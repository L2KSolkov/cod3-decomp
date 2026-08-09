// ============================================================================
// g_trigger.cpp - trigger entities (g.o: g_trigger.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

// content-flag constants used by trigger code
// CONTENT axis/allies/neutral/body masks (verified via disasm)
#define CONTENT_AXIS       0x00040000
#define CONTENT_ALLIES     0x00080000
#define CONTENT_NEUTRAL    0x00100000
#define CONTENT_BODY       0x00400000
#define CONTENT_VEHICLE    0x01000000
#define CONTENT_PLAYERCLIP 0x40000000

// ea: 0x00448BE0
int GetEntityTouchTriggerType(Entity* pEnt)
{
    if (pEnt->scr_vehicle != nullptr)
        return 8;
    sentient_s* sentient = pEnt->sentient;
    if (sentient == nullptr)
        return CONTENT_BODY;
    if (pEnt->client != nullptr)
        return CONTENT_PLAYERCLIP;
    switch (sentient->eTeam)
    {
    case TEAM_AXIS:
        return CONTENT_AXIS;
    case TEAM_ALLIES:
        return CONTENT_ALLIES;
    case TEAM_NEUTRAL:
        return CONTENT_NEUTRAL;
    default:
        break;
    }
    return 0;
}

// ea: 0x0044CA80
void use_trigger_use(Entity* ent, Entity* other)
{
    if (level.time > ent->wait)
    {
        ent->wait = ent->delay + level.time;
        if (other->client == nullptr)
        {
            if ((ent->spawnflags & 1) != 0)
                ent->spawnflags &= ~1;
            else
                ent->spawnflags |= 1;
        }
    }
}

// ea: 0x00451450
void InitSentientTrigger(Entity* self)
{
    if (self == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_trigger.cpp";
        AeAssert::gCurrentLine = 83;
        AeAssert::gCurrentExpr = "self";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int spawnflags = self->spawnflags;
    self->r.contents = 0;
    if ((spawnflags & 8) == 0)
        self->r.contents = CONTENT_PLAYERCLIP;
    if ((spawnflags & 1) != 0)
        self->r.contents |= CONTENT_AXIS;
    if ((spawnflags & 2) != 0)
        self->r.contents |= CONTENT_ALLIES;
    if ((spawnflags & 4) != 0)
        self->r.contents |= CONTENT_NEUTRAL;
    if ((spawnflags & 0x10) != 0)
        self->r.contents |= 8u;
}

// ea: 0x004514F0
void multi_wait(Entity* ent)
{
    ent->nextthink = 0;
}

// ea: 0x00451510
void multi_trigger(Entity* ent, Entity* activator)
{
    ent->activator = activator;
    if (ent->think != THINK__Think_SpawnNewAutoDoorTrigger && ent->nextthink == 0)
    {
        if (ent->wait <= 0.0f)
        {
            ent->touch = 0;
            ent->nextthink = level.time + 1;
            ent->think = THINK__G_FreeEntity;
        }
        else
        {
            ent->think = THINK__misc_spawner_think;
            ent->nextthink = level.time - (((((float)rand() * 0.000061035156f) - 1.0f) * ent->random + ent->wait) * -1000.0f);
        }
    }
}

// ea: 0x004515C0
void Use_Multi(Entity* ent, Entity* other, Entity* activator)
{
    multi_trigger(ent, activator);
}

// ea: 0x004515E0
void hurt_use(Entity* self)
{
    float delay = self->delay;
    self->touch = self->touch != 0 ? 0 : 3;
    if (delay != 0.0f)
    {
        self->nextthink = level.time + 50;
        self->think = THINK__GotoPos3;
        self->wait = delay * 1000.0f + level.time;
    }
}

// ea: 0x00451650
int Respond_trigger_damage(Entity* pEnt, int iMOD)
{
    int spawnflags = pEnt->spawnflags;
    if ((spawnflags & 1) != 0 && iMOD == 1 || (spawnflags & 2) != 0 && iMOD == 2)
        return 0;
    if ((spawnflags & 4) != 0)
    {
        switch (iMOD)
        {
        case 3:
        case 4:
        case 9:
        case 10:
            return 0;
        default:
            break;
        }
    }
    if ((spawnflags & 0x80u) != 0 && iMOD >= 7 && iMOD <= 8
        || (spawnflags & 8) != 0 && iMOD >= 13 && (iMOD <= 17 || iMOD == 27))
    {
        return 0;
    }
    if ((spawnflags & 0x10) != 0)
    {
        switch (iMOD)
        {
        case 4:
        case 6:
        case 8:
        case 10:
        case 14:
        case 16:
            return 0;
        default:
            return ((spawnflags & 0x20) == 0 || iMOD != 11)
                && ((spawnflags & 0x40) == 0 || iMOD != 29)
                && ((spawnflags & 0x100) == 0 || iMOD != 0 && (iMOD <= 18 || iMOD > 26));
        }
    }
    return ((spawnflags & 0x20) == 0 || iMOD != 11)
        && ((spawnflags & 0x40) == 0 || iMOD != 29)
        && ((spawnflags & 0x100) == 0 || iMOD != 0 && (iMOD <= 18 || iMOD > 26));
}

// ea: 0x00451730
void SP_trigger_lookat(Entity* self)
{
    SV_SetBrushModel(self);
    self->s.eFlags |= 2;
    self->r.contents = 0x20000000;
    self->r.svFlags = 1;
    g_LinkEntity(self);
}

// ea: 0x00451770
void SP_trigger_mount_no_brush(Entity* pSelf, int crouch)
{
    pSelf->s.eFlags |= 2u;
    pSelf->touch = 10;
    pSelf->r.contents = 1073741832;
    pSelf->r.svFlags = 1;
    if (crouch != 0)
    {
        pSelf->spawnflags |= 1u;
        pSelf->r.contents = 1077936136;
    }
    else
    {
        pSelf->spawnflags &= ~1u;
        pSelf->r.contents = 1090519048;
    }
    g_LinkEntity(pSelf);
}

// ea: 0x004641A0
void InitTrigger(Entity* self)
{
    if (self->r.currentAngles.v.m128_f32[0] != 0.0f
        || self->r.currentAngles.v.m128_f32[1] != 0.0f
        || self->r.currentAngles.v.m128_f32[2] != 0.0f)
    {
        G_SetMovedir(&self->r.currentAngles, &self->movedir);
    }
    SV_SetBrushModel(self);
    self->s.eFlags |= 2;
    self->flags |= 0x8000;
    self->r.contents = 1073741832;
    self->r.svFlags = 1;
}

// ea: 0x00464240
void SP_trigger_multiple(Entity* ent)
{
    static unsigned int sInit = 0;
    static unsigned int wait_hash;
    static unsigned int random_hash;
    if ((sInit & 1) == 0)
    {
        sInit |= 1u;
        wait_hash = HashString::CalcHash("wait");
    }
    if ((sInit & 2) == 0)
    {
        sInit |= 2u;
        random_hash = HashString::CalcHash("random");
    }
    G_SpawnFloat(wait_hash, 0.5f, &ent->wait);
    G_SpawnFloat(random_hash, 0.0f, &ent->random);
    if (ent->random >= ent->wait)
    {
        float wait = ent->wait;
        if (wait >= 0.0f)
        {
            ent->random = wait - 100.0f;
            G_Printf("trigger_multiple has random >= wait\n");
        }
    }
    ent->touch = 9;
    ent->use = 9;
    InitTrigger(ent);
    InitSentientTrigger(ent);
    g_LinkEntity(ent);
}

// ea: 0x00464350
void SP_trigger_friendlychain(Entity* ent)
{
    ent->touch = 5;
    if (ent->mTarget.mBlock == nullptr || (char*)ent->mTarget.mBlock + 4 == nullptr || *(char*)((char*)ent->mTarget.mBlock + 4) == 0)
        G_Error("trigger_friendlychain must target a friendly chain node");
    InitTrigger(ent);
    InitSentientTrigger(ent);
    g_LinkEntity(ent);
}

// ea: 0x004643A0
void hurt_think(Entity* ent, int msec)
{
    ent->nextthink = level.time + 1;
    if (level.time > ent->wait)
        G_FreeEntity(ent, msec);
}

// ea: 0x004643E0
void SP_trigger_hurt(Entity* self)
{
    static unsigned int sInit = 0;
    static unsigned int life_hash;
    Entity* v1 = self;
    InitTrigger(self);
    v1->noise_index = G_SoundAliasIndex("world_hurt_me");
    if (v1->damage == 0)
        v1->damage = 50;
    unsigned char spawnflags = v1->spawnflags;
    v1->r.contents = 1073741832;
    v1->use = 1;
    if ((spawnflags & 1) == 0)
        v1->touch = 3;
    if ((sInit & 1) == 0)
    {
        sInit |= 1u;
        life_hash = HashString::CalcHash("life");
    }
    G_SpawnFloat(life_hash, 0.0f, (float*)&self);
    v1->delay = *(float*)&self;
}

// ea: 0x004644B0
void SP_trigger_once(Entity* ent)
{
    ent->wait = -1.0f;
    ent->touch = 9;
    ent->use = 9;
    InitTrigger(ent);
    InitSentientTrigger(ent);
    g_LinkEntity(ent);
}

// ea: 0x004644F0
void SP_trigger_damage(Entity* pSelf)
{
    static unsigned int sInit = 0;
    static unsigned int wait_hash;
    static unsigned int random_hash;
    static unsigned int accumulate_hash;
    static unsigned int threshold_hash;
    if ((sInit & 1) == 0)
    {
        sInit |= 1u;
        wait_hash = HashString::CalcHash("wait");
    }
    if ((sInit & 2) == 0)
    {
        sInit |= 2u;
        random_hash = HashString::CalcHash("random");
    }
    G_SpawnFloat(wait_hash, 0.5f, &pSelf->wait);
    G_SpawnFloat(random_hash, 0.0f, &pSelf->random);
    if (pSelf->random >= pSelf->wait)
    {
        float wait = pSelf->wait;
        if (wait >= 0.0f)
        {
            pSelf->random = wait - 100.0f;
            G_Printf("trigger_damage has random >= wait\n");
        }
    }
    if ((sInit & 4) == 0)
    {
        sInit |= 4u;
        accumulate_hash = HashString::CalcHash("accumulate");
    }
    if ((sInit & 8) == 0)
    {
        sInit |= 8u;
        threshold_hash = HashString::CalcHash("threshold");
    }
    G_SpawnInt(accumulate_hash, 0, &pSelf->count);
    G_SpawnInt(threshold_hash, 0, &pSelf->key);
    pSelf->health = 32000;
    pSelf->takedamage = 1;
    pSelf->use = 10;
    pSelf->pain = 5;
    pSelf->die = 5;
    InitTrigger(pSelf);
    g_LinkEntity(pSelf);
}

// ea: 0x004646B0
void SP_trigger_mount(Entity* pSelf)
{
    pSelf->touch = 10;
    InitTrigger(pSelf);
    int contents = pSelf->r.contents;
    if ((pSelf->spawnflags & 1) != 0)
        pSelf->r.contents = CONTENT_BODY | contents;
    else
        pSelf->r.contents = CONTENT_VEHICLE | contents;
    g_LinkEntity(pSelf);
}

// ea: 0x00470840
void G_Trigger(Entity* self, Entity* other)
{
    int v2 = 0;
    trigger_info_t* triggerList = level.triggerList;
    if (level.triggerListSize > 0)
    {
        while (triggerList->mEntity.mHandle.mVal != self->mHandle.mHandle.mVal
               || triggerList->mOtherEntity.mHandle.mVal != other->mHandle.mHandle.mVal)
        {
            ++v2;
            ++triggerList;
            if (v2 >= level.triggerListSize)
                goto add_new;
        }
        return;
    }
add_new:
    if (self == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_trigger.cpp";
        AeAssert::gCurrentLine = 28;
        AeAssert::gCurrentExpr = "self";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (other == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_trigger.cpp";
        AeAssert::gCurrentLine = 29;
        AeAssert::gCurrentExpr = "other";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (Scr_IsSystemActive(1u) != 0)
    {
        if (level.triggerListSize == 256)
        {
            Scr_NotifyFromEnt(self, hash_const.trigger, other);
        }
        else
        {
            trigger_info_t* v4 = &level.triggerList[level.triggerListSize++];
            v4->mEntity.mHandle.mVal = self->mHandle.mHandle.mVal;
            v4->mOtherEntity.mHandle.mVal = other->mHandle.mHandle.mVal;
            v4->useCount = self->s.useCount;
            v4->otherUseCount = other->s.useCount;
        }
    }
}

// ea: 0x00470970
void Touch_Multi(Entity* self, Entity* other)
{
    G_Trigger(self, other);
    multi_trigger(self, other);
}

// ea: 0x00470990
void Touch_FriendlyChain(Entity* self, Entity* other)
{
    if (other->sentient == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_trigger.cpp";
        AeAssert::gCurrentLine = 174;
        AeAssert::gCurrentExpr = "other->sentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    PathNodeMgr::sInst->AttachSentientToChainNode(other->sentient, &self->mTarget);
    G_Trigger(self, other);
}

// ea: 0x00470A10
void Activate_trigger_damage(Entity* pEnt, Entity* pOther, int iDamage, int iMOD)
{
    if (pEnt->nextthink == 0 || pEnt->think == THINK__Think_SpawnNewAutoDoorTrigger)
    {
        int key = pEnt->key;
        if ((key <= 0 || iDamage >= key) && Respond_trigger_damage(pEnt, iMOD) != 0)
        {
            int v5 = pEnt->health - iDamage;
            pEnt->health = v5;
            int count = pEnt->count;
            if (count == 0 || 32000 - v5 >= count)
            {
                pEnt->activator = pOther;
                if (iMOD != -1)
                    G_Trigger(pEnt, pOther);
                if (pEnt->think != THINK__Think_SpawnNewAutoDoorTrigger)
                {
                    if (pEnt->wait > 0.0f)
                    {
                        pEnt->think = THINK__misc_spawner_think;
                        float v8 = random();
                        pEnt->nextthink = level.time - ((v8 + v8 - 1.0f) * pEnt->random + pEnt->wait) * -1000.0f;
                        pEnt->health = 32000;
                        return;
                    }
                    pEnt->touch = 0;
                    pEnt->nextthink = level.time + 1;
                    pEnt->think = THINK__G_FreeEntity;
                }
                pEnt->health = 32000;
            }
        }
    }
}

// ea: 0x00470B30
void Use_trigger_damage(Entity* pEnt, Entity* pOther)
{
    Activate_trigger_damage(pEnt, pOther, pEnt->count + 1, -1);
}

// ea: 0x00470B50
void Pain_trigger_damage(Entity* pSelf, Entity* pAttacker, int iDamage, const float* vPoint, int iMod)
{
    Activate_trigger_damage(pSelf, pAttacker, iDamage, iMod);
    if (pSelf->count == 0)
        pSelf->health = 32000;
}

// ea: 0x00470B90
void Die_trigger_damage(Entity* pSelf, Entity* pInflictor, Entity* pAttacker, int iDamage, int iMod)
{
    Activate_trigger_damage(pSelf, pAttacker, iDamage, iMod);
    if (pSelf->count == 0)
        pSelf->health = 32000;
}

// ea: 0x00470F20
void Touch_trigger_mount(Entity* self, Entity* other)
{
    G_Trigger(self, other);
}
