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
void Think_GeneralLink(Entity* ent, int /*unused*/)
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
void G_SetOrigin(Entity* ent, const math::Position3& origin)
{
    if (IS_NAN(origin.v.m128_f32[0]) || IS_NAN(origin.v.m128_f32[1]) || IS_NAN(origin.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2224;
        AeAssert::gCurrentExpr = "!IS_NAN((origin)[0]) && !IS_NAN((origin)[1]) && !IS_NAN((origin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    memcpy(ent->s.pos.trBase, &origin, sizeof(ent->s.pos.trBase));
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
    ent->r.currentOrigin.v.m128_f32[0] = origin.v.m128_f32[0];
    ent->r.currentOrigin.v.m128_f32[1] = origin.v.m128_f32[1];
    ent->r.currentOrigin.v.m128_f32[2] = origin.v.m128_f32[2];
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
void G_SetAngle(Entity* ent, const math::Position3& angle)
{
    memcpy(ent->s.apos.trBase, &angle, sizeof(ent->s.apos.trBase));
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
    if (IS_NAN(angle.v.m128_f32[0]) || IS_NAN(angle.v.m128_f32[1]) || IS_NAN(angle.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 2260;
        AeAssert::gCurrentExpr = "!IS_NAN((angle)[0]) && !IS_NAN((angle)[1]) && !IS_NAN((angle)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    ent->r.currentAngles.v.m128_f32[0] = angle.v.m128_f32[0];
    ent->r.currentAngles.v.m128_f32[1] = angle.v.m128_f32[1];
    ent->r.currentAngles.v.m128_f32[2] = angle.v.m128_f32[2];
}

// ea: 0x00460410
void G_SetMovedir(math::Position3& angles, math::Position3& movedir)
{
    if (angles.v.m128_f32[0] == VEC_UP[0]
        && angles.v.m128_f32[1] == VEC_UP[1]
        && angles.v.m128_f32[2] == VEC_UP[2])
    {
        memcpy(&movedir, MOVEDIR_UP, 12);
    }
    else if (angles.v.m128_f32[0] == VEC_DOWN[0]
             && angles.v.m128_f32[1] == VEC_DOWN[1]
             && angles.v.m128_f32[2] == VEC_DOWN[2])
    {
        memcpy(&movedir, MOVEDIR_DOWN, 12);
    }
    else
    {
        float tmp[3];
        tmp[0] = movedir.v.m128_f32[0];
        tmp[1] = movedir.v.m128_f32[1];
        tmp[2] = movedir.v.m128_f32[2];
        AnglesToForward(angles.v.m128_f32, tmp);
        movedir.v.m128_f32[0] = tmp[0];
        movedir.v.m128_f32[1] = tmp[1];
        movedir.v.m128_f32[2] = tmp[2];
    }
    angles.v.m128_f32[1] = 0.0f;
    angles.v.m128_f32[0] = 0.0f;
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

// ea: 0x00474C90
void G_DoTouchTriggers(Entity* ent, const math::Position3& origin,
                       TouchEntityData* tData, const collision_context_t& context)
{
    int v24 = GetEntityTouchTriggerType(ent);
    if (v24 == 0)
        return;
    int num;
    math::Position3 mins;
    math::Position3 maxs;
    DbLinkedHandle<EntityHandleDb, Entity>* touch_ptr;
    DbLinkedHandle<EntityHandleDb, Entity> entityList[64];
    if (tData != nullptr)
    {
        num = tData->num;
        mins.v.m128_f32[0] = tData->mins.v.m128_f32[0];
        mins.v.m128_f32[1] = tData->mins.v.m128_f32[1];
        mins.v.m128_f32[2] = tData->mins.v.m128_f32[2];
        mins.v.m128_f32[3] = tData->mins.v.m128_f32[3];
        maxs.v = tData->maxs.v;
        touch_ptr = tData->touch;
    }
    else
    {
        __m128 box = _mm_set_ps(0.0f, 52.0f, 40.0f, 40.0f);
        maxs.v = _mm_add_ps(origin.v, box);
        mins.v = _mm_sub_ps(origin.v, box);
        num = CM_AreaEntities(mins, maxs, entityList, 64, v24);
        mins.v = _mm_add_ps(origin.v, ent->r.mins.v);
        maxs.v = _mm_add_ps(origin.v, ent->r.maxs.v);
        touch_ptr = entityList;
    }
    for (int v13 = 0; v13 < num; ++v13)
    {
        unsigned int v14 = touch_ptr[v13].mHandle.mVal;
        unsigned int v15 = v14 & 0xFFF;
        if (v15 >= 0x540)
            continue;
        if (v14 >> 12 != EntityHandleDb::sInst.mElements[v15].mKey)
            continue;
        Entity* mObject = EntityHandleDb::sInst.mElements[v15].mObject;
        if (mObject == nullptr)
            continue;
        if ((v24 & mObject->r.contents) == 0)
            continue;
        if (mObject->touch == 0 && ent->touch == 0)
            continue;
        if (context.filter(mObject))
            continue;
        if (mObject->s.eType == 2)
        {
            Client* client = ent->client;
            if (client != nullptr && BG_PlayerTouchesItem(&client->ps, &mObject->s, level.time))
                goto touch;
        }
        else
        {
            bool v19;
            if (mObject->s.eType == 3)
            {
                Client* v18 = ent->client;
                if (v18 != nullptr && !BG_PlayerTouchesMine(&v18->ps, &mObject->s, level.time))
                    continue;
                if (ent->scr_vehicle == nullptr)
                    goto touch;
                v19 = !VEH_VehicleTouchesMine(ent, &mObject->s);
            }
            else
            {
                v19 = g_EntityContactCapsule(&mins, &maxs, mObject) == 0;
            }
            if (!v19)
                goto touch;
        }
        continue;
    touch:
        if (Scr_IsSystemActive(1) != 0)
        {
            Scr_NotifyFromEnt(mObject, hash_const.touch, ent);
            Scr_NotifyFromEnt(ent, hash_const.touch, mObject);
        }
        uint8_t touch = mObject->touch;
        if (touch != 0)
        {
            if (touch >= 0xDu)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_active.cpp";
                AeAssert::gCurrentLine = 410;
                AeAssert::gCurrentExpr = "hit->touch > 0 && hit->touch < TOUCH_MAX";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            touchtable[mObject->touch](mObject, ent, 1);
        }
        if (ent->actor != nullptr)
        {
            uint8_t v21 = ent->touch;
            if (v21 != 0)
            {
                if (v21 >= 0xDu)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_active.cpp";
                    AeAssert::gCurrentLine = 416;
                    AeAssert::gCurrentExpr = "ent->touch > 0 && ent->touch < TOUCH_MAX";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                touchtable[ent->touch](ent, mObject, 1);
            }
        }
    }
}

// ea: 0x004748A0
void G_TouchVehicles(Entity* ent, const math::Position3& origin,
                     TouchEntityData* tData, const collision_context_t& context)
{
    if (ent->tagInfo != nullptr)
        return;
    int num;
    math::Position3 mins;
    math::Position3 maxs;
    DbLinkedHandle<EntityHandleDb, Entity>* touch_ptr;
    ae_sized_array<DbLinkedHandle<EntityHandleDb, Entity>, 256> touch;
    if (tData != nullptr)
    {
        num = tData->num;
        mins.v.m128_f32[0] = tData->mins.v.m128_f32[0];
        mins.v.m128_f32[1] = tData->mins.v.m128_f32[1];
        mins.v.m128_f32[2] = tData->mins.v.m128_f32[2];
        maxs.v.m128_f32[0] = tData->maxs.v.m128_f32[0];
        maxs.v.m128_f32[1] = tData->maxs.v.m128_f32[1];
        maxs.v.m128_f32[2] = tData->maxs.v.m128_f32[2];
        for (int i = 0; i < tData->num; ++i)
            touch[i] = tData->touch[i];
        touch_ptr = touch.m_elements;
    }
    else
    {
        mins.v.m128_f32[0] = origin.v.m128_f32[0] - 40.0f;
        mins.v.m128_f32[1] = origin.v.m128_f32[1] - 40.0f;
        mins.v.m128_f32[2] = origin.v.m128_f32[2] - 52.0f;
        maxs.v.m128_f32[0] = origin.v.m128_f32[0] + 40.0f;
        maxs.v.m128_f32[1] = origin.v.m128_f32[1] + 40.0f;
        maxs.v.m128_f32[2] = origin.v.m128_f32[2] + 52.0f;
        num = CM_AreaEntities(mins, maxs, touch.m_elements, 256, 0x800000);
        mins.v.m128_f32[0] = ent->r.mins.v.m128_f32[0] + origin.v.m128_f32[0];
        mins.v.m128_f32[1] = ent->r.mins.v.m128_f32[1] + origin.v.m128_f32[1];
        mins.v.m128_f32[2] = ent->r.mins.v.m128_f32[2] + origin.v.m128_f32[2];
        maxs.v.m128_f32[0] = ent->r.maxs.v.m128_f32[0] + origin.v.m128_f32[0];
        maxs.v.m128_f32[1] = ent->r.maxs.v.m128_f32[1] + origin.v.m128_f32[1];
        maxs.v.m128_f32[2] = ent->r.maxs.v.m128_f32[2] + origin.v.m128_f32[2];
        touch_ptr = touch.m_elements;
    }
    for (int v14 = 0; v14 < num; ++v14)
    {
        unsigned int v15 = touch_ptr[v14].mHandle.mVal & 0xFFF;
        if (v15 >= 0x540)
            continue;
        if (touch_ptr[v14].mHandle.mVal >> 12
            != EntityHandleDb::sInst.mElements[v15].mKey)
            continue;
        Entity* mObject = EntityHandleDb::sInst.mElements[v15].mObject;
        if (mObject == nullptr)
            continue;
        if ((0x800000 & mObject->r.contents) == 0)
            continue;
        if (mObject->s.eType != 14)
            continue;
        if (mObject->r.bmodel == nullptr)
            continue;
        if (mObject->touch == 0 && ent->touch == 0)
            continue;
        if (context.filter(mObject))
            continue;
        if (g_EntityContactCapsule(&mins, &maxs, mObject) == 0)
            continue;
        if (Scr_IsSystemActive(1) != 0)
        {
            Scr_NotifyFromEnt(mObject, hash_const.touch, ent);
            Scr_NotifyFromEnt(ent, hash_const.touch, mObject);
        }
        uint8_t v17 = mObject->touch;
        if (v17 != 0)
        {
            if (v17 >= 0xDu)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_active.cpp";
                AeAssert::gCurrentLine = 257;
                AeAssert::gCurrentExpr = "hit->touch > 0 && hit->touch < TOUCH_MAX";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            touchtable[mObject->touch](mObject, ent, 1);
        }
        if (ent->actor != nullptr)
        {
            uint8_t v18 = ent->touch;
            if (v18 != 0)
            {
                if (v18 >= 0xDu)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_active.cpp";
                    AeAssert::gCurrentLine = 263;
                    AeAssert::gCurrentExpr = "( ent->touch > 0 ) && ( ent->touch < TOUCH_MAX )";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                touchtable[ent->touch](ent, mObject, 1);
            }
        }
    }
}
