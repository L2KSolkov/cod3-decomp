// ============================================================================
// g_actor_prone.cpp - actor prone state (g.o: g_actor_prone.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

// ea: 0x00449060
actor_prone_info_t* G_GetActorProneInfo(actor_s* pActor)
{
    if (pActor->mActorIndex >= 0x20)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_actor_prone.cpp";
        AeAssert::gCurrentLine = 24;
        AeAssert::gCurrentExpr = "(iIndex >= 0) && (iIndex < 32)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return &pActor->ProneInfo;
}

// ea: 0x004490D0
void G_InitActorProneInfo(actor_s* pActor)
{
    pActor->ProneInfo.bCorpseOrientation = 0;
    pActor->ProneInfo.iProneTime = 0;
    pActor->ProneInfo.iProneTrans = 0;
    pActor->ProneInfo.fTorsoHeight = 0.0f;
    pActor->ProneInfo.fTorsoPitch = 0.0f;
    pActor->ProneInfo.fWaistPitch = 0.0f;
}

// ea: 0x00449100
void G_ActorExitProne(actor_s* pActor, int iTransTime)
{
    actor_prone_info_t* p_ProneInfo = &pActor->ProneInfo;
    if (BG_ActorIsProne(&pActor->ProneInfo, level.time) != 0)
    {
        int iProneTrans = pActor->ProneInfo.iProneTrans;
        if (iProneTrans == 0 || iTransTime == iProneTrans)
            pActor->ProneInfo.iProneTime = level.time;
        else
            pActor->ProneInfo.iProneTime = level.time
                - (int)(iTransTime * BG_GetActorProneFraction(p_ProneInfo, level.time));
        pActor->ProneInfo.iProneTrans = -iTransTime;
    }
    if (BG_ActorGoalIsProne(p_ProneInfo) != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_actor_prone.cpp";
        AeAssert::gCurrentLine = 167;
        AeAssert::gCurrentExpr = "!BG_ActorGoalIsProne(&pActor->ProneInfo)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
}

// ea: 0x00455600
void G_ActorEnterProne(actor_s* pActor, int iTransTime)
{
    actor_prone_info_t* p_ProneInfo = &pActor->ProneInfo;
    if (BG_ActorIsProne(&pActor->ProneInfo, level.time) != 0)
    {
        int iProneTrans = pActor->ProneInfo.iProneTrans;
        if (iProneTrans != 0 && iTransTime != iProneTrans)
        {
            float ActorProneFraction = BG_GetActorProneFraction(p_ProneInfo, level.time);
            if (ActorProneFraction < 1.0f)
                pActor->ProneInfo.iProneTime = level.time - (int)(iTransTime * ActorProneFraction);
            pActor->ProneInfo.iProneTrans = iTransTime;
        }
    }
    else
    {
        p_ProneInfo->bCorpseOrientation = 0;
        pActor->ProneInfo.iProneTime = level.time;
        pActor->ProneInfo.iProneTrans = iTransTime;
        Entity* pEnt = pActor->pEnt;
        math::Dir3 v7;
        v7.v = _mm_setzero_ps();
        typedef void (__cdecl* ProneTrace)(trace_t*, const math::Position3*,
                                           const math::Position3*,
                                           const math::Position3*,
                                           const math::Position3*,
                                           const collision_context_t&);
        typedef int (__cdecl* ProneContents)(const math::Position3*,
                                             const collision_context_t&);
        pActor->bProneOK = BG_CheckProneValid(
            pEnt->mHandle,
            &pEnt->r.currentOrigin,
            actorMaxs.v.m128_f32[0],
            24.0f,
            pEnt->r.currentAngles.v.m128_f32[1],
            &pActor->ProneInfo.fTorsoHeight,
            &pActor->ProneInfo.fTorsoPitch,
            &pActor->ProneInfo.fWaistPitch,
            0,
            1,
            &v7,
            (ProneTrace)g_TraceCapsule,
            (ProneTrace)g_Trace,
            (ProneContents)SV_PointContents,
            PCT_ACTOR,
            45.0f);
    }
    if (BG_ActorGoalIsProne(p_ProneInfo) == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_actor_prone.cpp";
        AeAssert::gCurrentLine = 139;
        AeAssert::gCurrentExpr = "BG_ActorGoalIsProne(&pActor->ProneInfo)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
}

// ea: 0x00466A90
actor_prone_info_t* G_GetActorProneInfoFromEntHandle(DbLinkedHandle<EntityHandleDb, Entity> entity)
{
    unsigned int v1 = entity.mHandle.mVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v1 < 0x540 && entity.mHandle.mVal >> 12 == EntityHandleDb::sInst.mElements[v1].mKey)
        mObject = EntityHandleDb::sInst.mElements[v1].mObject;
    if (mObject->actor != nullptr)
        return G_GetActorProneInfo(mObject->actor);
    if (mObject->s.eType != 13)
        return nullptr;
    int ActorCorpseIndex = G_GetActorCorpseIndex(mObject);
    if (g_scr_data.actorCorpseInfo[ActorCorpseIndex].mEntity.mHandle.mVal != mObject->mHandle.mHandle.mVal)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_actor_prone.cpp";
        AeAssert::gCurrentLine = 46;
        AeAssert::gCurrentExpr = "g_scr_data.actorCorpseInfo[iCorpseIndex].mEntity == pEnt->GetHandle()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return &g_scr_data.actorCorpseInfo[ActorCorpseIndex].proneInfo;
}

// ea: 0x00466B50
actor_prone_info_t* G_GetClientActorProneInfoFromEntHandle(DbLinkedHandle<EntityHandleDb, Entity> entity)
{
    unsigned int v1 = entity.mHandle.mVal & 0xFFF;
    Entity* mObject;
    if (v1 >= 0x540
        || entity.mHandle.mVal >> 12 != EntityHandleDb::sInst.mElements[v1].mKey
        || (mObject = EntityHandleDb::sInst.mElements[v1].mObject) == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_actor_prone.cpp";
        AeAssert::gCurrentLine = 64;
        AeAssert::gCurrentExpr = "e";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        return nullptr;
    }
        actor_s* actor = mObject->actor;
    if (actor == nullptr)
        return nullptr;
    return &actor->ProneInfo;
}

// ============================================================================
// BG prone queries - ea: 0x603FF0..0x604070 (bg_pmove.cpp)
// ============================================================================

// ea: 0x00603FF0
int BG_ActorIsProne(actor_prone_info_t* pInfo, int iCurrentTime)
{
    int iProneTime = pInfo->iProneTime;
    if (iProneTime == 0 || pInfo->bCorpseOrientation != 0)
        return 0;
    int iProneTrans = pInfo->iProneTrans;
    if (iProneTrans != 0)
    {
        if (iProneTrans >= 0)
        {
            if (iProneTime + iProneTrans < iCurrentTime)
                pInfo->iProneTrans = 0;
        }
        else if (iProneTime - iProneTrans < iCurrentTime)
        {
            pInfo->iProneTime = 0;
            return 0;
        }
    }
    return 1;
}

// ea: 0x00604040
int BG_ActorGoalIsProne(actor_prone_info_t* pInfo)
{
    return pInfo->iProneTime != 0
        && pInfo->bCorpseOrientation == 0
        && pInfo->iProneTrans >= 0;
}

// ea: 0x00604070
float BG_GetActorProneFraction(actor_prone_info_t* pInfo, int iCurrentTime)
{
    int iProneTime = pInfo->iProneTime;
    if (iProneTime == 0)
        return 0.0f;
    int iProneTrans = pInfo->iProneTrans;
    if (iProneTrans == 0)
        return 1.0f;
    if (iProneTrans >= 0)
    {
        if (iProneTrans + iProneTime >= iCurrentTime)
            return (float)(iCurrentTime - iProneTime) / (float)pInfo->iProneTrans;
        pInfo->iProneTrans = 0;
        return 1.0f;
    }
    if (iProneTime - iProneTrans < iCurrentTime)
    {
        pInfo->iProneTime = 0;
        return 0.0f;
    }
    return 1.0f - (float)(iCurrentTime - iProneTime) / (float)(-iProneTrans);
}
