// ============================================================================
// g_active.cpp - entity link/origin helpers + free list (g.o)
// Source: g_active.cpp / g_utils.cpp families
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <string.h>

extern "C" int __fpclass(float);

static bool IS_NAN(float x) {
    return (__fpclass(x) & 0x297) != 0;
}

// .rdata shared constants (verified against XBE bytes)
static const float VEC_UP[3] = { 0.0f, 0.0f, 1.0f };
static const float VEC_DOWN[3] = { 0.0f, 0.0f, -1.0f };
static const float MOVEDIR_UP[3] = { 0.0f, 0.0f, 1.0f };
static const float MOVEDIR_DOWN[3] = { 0.0f, 0.0f, -1.0f };

// ea: 0x00448E50
void G_SetClientContents(Entity* pEnt)
{
    if (pEnt->client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_active.cpp";
        AeAssert::gCurrentLine = 1186;
        AeAssert::gCurrentExpr = "pEnt->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Client* client = pEnt->client;
    if (client->noclip != 0 || client->ufo != 0)
    {
        pEnt->r.contents = 0;
    }
    else
    {
        int playerState = client->pers.playerState;
        if (playerState == 4 || playerState == 5)
            pEnt->r.contents = 0x4000000;
        else
            pEnt->r.contents = 0x2000000;
    }
}

// ea: 0x0044A000
void G_SetPlayerSize()
{
    playerMaxs.v.m128_f32[1] = g_bounds_width.value * 0.5f;
    playerMaxs.v.m128_f32[0] = g_bounds_width.value * 0.5f;
    playerMins.v.m128_f32[1] = g_bounds_width.value * -0.5f;
    playerMins.v.m128_f32[0] = g_bounds_width.value * -0.5f;
    playerMaxs.v.m128_f32[2] = g_bounds_height_standing.value;
}

// ea: 0x00453BE0
int G_EntIsLinkedTo(Entity* ent, Entity* parent)
{
    tagInfo_t* tagInfo = ent->tagInfo;
    return tagInfo != nullptr && tagInfo->parent == parent;
}

// ea: 0x00453C10
void Think_GeneralLink(Entity* ent)
{
    ent->nextthink = level.time + 1;
}

// ea: 0x00454440
void G_SetOrigin(Entity* ent, const float* origin)
{
    if (IS_NAN(origin[0]) || IS_NAN(origin[1]) || IS_NAN(origin[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2211;
        AeAssert::gCurrentExpr = "!IS_NAN((origin)[0]) && !IS_NAN((origin)[1]) && !IS_NAN((origin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    ent->s.pos.trBase[0] = origin[0];
    ent->s.pos.trBase[1] = origin[1];
    ent->s.pos.trBase[2] = origin[2];
    ent->s.pos.trType = TR_STATIONARY;
    ent->s.pos.trTime = 0;
    ent->s.pos.trDuration = 0;
    ent->s.pos.trDelta[1] = 0.0f;
    ent->s.pos.trDelta[0] = 0.0f;
    if (IS_NAN(ent->r.currentOrigin.v.m128_f32[0])
        || IS_NAN(ent->r.currentOrigin.v.m128_f32[1])
        || IS_NAN(ent->r.currentOrigin.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2218;
        AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    ent->r.currentOrigin.v.m128_f32[0] = origin[0];
    ent->r.currentOrigin.v.m128_f32[1] = origin[1];
    ent->r.currentOrigin.v.m128_f32[2] = origin[2];
}

// ea: 0x004545B0
void G_SetOrigin(Entity* ent, const math::Position3* origin)
{
    if (IS_NAN(origin->v.m128_f32[0]) || IS_NAN(origin->v.m128_f32[1]) || IS_NAN(origin->v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2224;
        AeAssert::gCurrentExpr = "!IS_NAN((origin)[0]) && !IS_NAN((origin)[1]) && !IS_NAN((origin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    memcpy(ent->s.pos.trBase, origin, sizeof(ent->s.pos.trBase));
    ent->s.pos.trType = TR_STATIONARY;
    ent->s.pos.trTime = 0;
    ent->s.pos.trDuration = 0;
    ent->s.pos.trDelta[1] = 0.0f;
    ent->s.pos.trDelta[0] = 0.0f;
    if (IS_NAN(ent->r.currentOrigin.v.m128_f32[0])
        || IS_NAN(ent->r.currentOrigin.v.m128_f32[1])
        || IS_NAN(ent->r.currentOrigin.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2230;
        AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    ent->r.currentOrigin.v.m128_f32[0] = origin->v.m128_f32[0];
    ent->r.currentOrigin.v.m128_f32[1] = origin->v.m128_f32[1];
    ent->r.currentOrigin.v.m128_f32[2] = origin->v.m128_f32[2];
}

// ea: 0x00454720
void G_SetAngle(Entity* ent, const float* angle)
{
    ent->s.apos.trBase[0] = angle[0];
    ent->s.apos.trBase[1] = angle[1];
    ent->s.apos.trBase[2] = angle[2];
    ent->s.apos.trType = TR_STATIONARY;
    ent->s.apos.trTime = 0;
    ent->s.apos.trDuration = 0;
    ent->s.apos.trDelta[1] = 0.0f;
    ent->s.apos.trDelta[0] = 0.0f;
    if (IS_NAN(ent->r.currentAngles.v.m128_f32[0])
        || IS_NAN(ent->r.currentAngles.v.m128_f32[1])
        || IS_NAN(ent->r.currentAngles.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2247;
        AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentAngles)[0]) && !IS_NAN((ent->r.currentAngles)[1]) && !IS_NAN((ent->r.currentAngles)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(angle[0]) || IS_NAN(angle[1]) || IS_NAN(angle[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2248;
        AeAssert::gCurrentExpr = "!IS_NAN((angle)[0]) && !IS_NAN((angle)[1]) && !IS_NAN((angle)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    ent->r.currentAngles.v.m128_f32[0] = angle[0];
    ent->r.currentAngles.v.m128_f32[1] = angle[1];
    ent->r.currentAngles.v.m128_f32[2] = angle[2];
}

// ea: 0x00454890
void G_SetAngle(Entity* ent, const math::Position3* angle)
{
    memcpy(ent->s.apos.trBase, angle, sizeof(ent->s.apos.trBase));
    ent->s.apos.trType = TR_STATIONARY;
    ent->s.apos.trTime = 0;
    ent->s.apos.trDuration = 0;
    ent->s.apos.trDelta[1] = 0.0f;
    ent->s.apos.trDelta[0] = 0.0f;
    if (IS_NAN(ent->r.currentAngles.v.m128_f32[0])
        || IS_NAN(ent->r.currentAngles.v.m128_f32[1])
        || IS_NAN(ent->r.currentAngles.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2259;
        AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentAngles)[0]) && !IS_NAN((ent->r.currentAngles)[1]) && !IS_NAN((ent->r.currentAngles)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(angle->v.m128_f32[0]) || IS_NAN(angle->v.m128_f32[1]) || IS_NAN(angle->v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2260;
        AeAssert::gCurrentExpr = "!IS_NAN((angle)[0]) && !IS_NAN((angle)[1]) && !IS_NAN((angle)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    ent->r.currentAngles.v.m128_f32[0] = angle->v.m128_f32[0];
    ent->r.currentAngles.v.m128_f32[1] = angle->v.m128_f32[1];
    ent->r.currentAngles.v.m128_f32[2] = angle->v.m128_f32[2];
}

// ea: 0x00460410
void G_SetMovedir(math::Position3* angles, math::Position3* movedir)
{
    if (angles->v.m128_f32[0] == VEC_UP[0]
        && angles->v.m128_f32[1] == VEC_UP[1]
        && angles->v.m128_f32[2] == VEC_UP[2])
    {
        memcpy(movedir, MOVEDIR_UP, 12);
    }
    else if (angles->v.m128_f32[0] == VEC_DOWN[0]
             && angles->v.m128_f32[1] == VEC_DOWN[1]
             && angles->v.m128_f32[2] == VEC_DOWN[2])
    {
        memcpy(movedir, MOVEDIR_DOWN, 12);
    }
    else
    {
        float tmp[3];
        tmp[0] = movedir->v.m128_f32[0];
        tmp[1] = movedir->v.m128_f32[1];
        tmp[2] = movedir->v.m128_f32[2];
        AnglesToForward(angles->v.m128_f32, tmp);
        movedir->v.m128_f32[0] = tmp[0];
        movedir->v.m128_f32[1] = tmp[1];
        movedir->v.m128_f32[2] = tmp[2];
    }
    angles->v.m128_f32[1] = 0.0f;
    angles->v.m128_f32[0] = 0.0f;
}

// ea: 0x00460540
void G_EntUnlinkFree(Entity* ent)
{
    if (ent->scripted != nullptr)
    {
        mem_heap_free(ent->scripted);
        ent->scripted = nullptr;
    }
    G_EntUnlink(ent);
}

// ea: 0x00460570
void G_FreeEntity(Entity* e, int msec)
{
    if (dont_delete)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2006;
        AeAssert::gCurrentExpr = "!dont_delete";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Can't delete now"))
            __debugbreak();
    }
    if (gCareAboutCheckpoint)
    {
        InplaceVector<unsigned char>* mPersistantStorage = SceneManager::sInst->mPersistantStorage;
        if (mPersistantStorage != nullptr)
        {
            int16_t mPersistentIndex = e->mPersistentIndex;
            if (mPersistentIndex > -1)
                (*mPersistantStorage)[mPersistentIndex] = 1;
        }
    }
    gCareAboutCheckpoint = true;
    if (e->mPakId == PAK_ID_INVALID)
        CurPakId();
    e->~Entity();
    memset(e, 0xCF, sizeof(Entity));
    e->mHandle.mHandle.mVal = 0;
    ++gEntFreeList.mFree;
    --gEntFreeList.mUsed;
    *(int*)&e->s.eType = (int)(intptr_t)gEntFreeList.mpFree;
    gEntFreeList.mpFree = e;
}

// ea: 0x00460690
Entity* G_TempEntity(const float* origin, int event)
{
    Entity* v2 = G_Spawn(PAK_ID_INVALID);
    Entity* v4 = v2;
    v4->s.eType = event + 18;
    if ((int)v4->s.eType != event + 18)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2063;
        AeAssert::gCurrentExpr = "e->s.eType == ET_EVENTS + event";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    v4->mClassName = str_const.tempEntity;
    v4->mClassNameHash.mHash = HashString(v4->mClassName).mHash;
    v4->r.eventType = 1;
    v4->r.eventTime = level.time;
    float snapped[3];
    snapped[0] = origin[0];
    snapped[1] = origin[1];
    snapped[2] = origin[2];
    G_SetOrigin(v4, snapped);
    g_LinkEntity(v4);
    UpdateEntityHash(v4);
    return v4;
}

// ea: 0x00457F30
void G_FreeEntities()
{
    PathNodeMgr::sInst->ValidateAllNodes();
    EntityManager::sInst->DeleteAllEntities();
    SceneManager::sInst->ResetAllStaticModels();
    Entity::FreeAllDObjs(true);
    G_CleanupAnimTrees();
}

// ea: 0x00458200
void G_SetEntityOceanHeight(Entity* pEnt)
{
    float pos[3];
    pos[0] = pEnt->r.currentOrigin.v.m128_f32[0];
    pos[1] = pEnt->r.currentOrigin.v.m128_f32[1];
    float heightOffset = 0.0f;
    trRefEntity* mRenderEntity = pEnt->mRenderEntity;
    if (mRenderEntity != nullptr)
        heightOffset = mRenderEntity->mWaterHeightOffset;
    pos[2] = cdOceanGlobals::GetHeight(0, pos[0], pos[1]) + heightOffset;
    G_SetOrigin(pEnt, pos);
}

// ea: 0x00467DD0
void G_RunThink(Entity* ent, int msec)
{
    unsigned int v2 = ent->mHandle.mHandle.mVal & 0xFFF;
    if (v2 < 0x540
        && ent->mHandle.mHandle.mVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && EntityHandleDb::sInst.mElements[v2].mObject != nullptr
        && (ent->flags & 0x4000000) == 0)
    {
        int nextthink = ent->nextthink;
        if (nextthink > 0 && nextthink <= level.time)
        {
            int think = ent->think;
            ent->nextthink = 0;
            if (think == THINK__NULL)
                G_Error("NULL ent->think");
            if (think <= THINK__NULL || think >= THINK_MAX)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_main.cpp";
                AeAssert::gCurrentLine = 2285;
                AeAssert::gCurrentExpr = "ent->think > 0 && ent->think < THINK_MAX";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            thinktable[ent->think](ent, msec);
        }
    }
}
