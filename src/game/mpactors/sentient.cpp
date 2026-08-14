// ============================================================================
// sentient.cpp - mp_actors.o Sentient_* family (sentient.cpp)
// ============================================================================

#include "game/logic/g_local.h"
#include "game/actor_types.h"

#include <string.h>
#include <math.h>
#include <intrin.h>

extern level_locals_t level;           // ?level@@3Ulevel_locals_t@@A @ 0xEC9650
extern math::Position3 playerMaxs;     // 0xEC9640
extern math::Position3 playerMins;     // 0xEC9620
extern const math::Position3 actorMaxs;  // 0xF99330
extern const math::Position3 actorMins;  // 0xF99510
extern const char* const pszTeamName[5];  // 0xE37BF8
extern int g_iSentientFreeSequence;   // ?g_iSentientFreeSequence@@3HA @ 0xF992D0
extern void* SV_SaveWrite(const void* buffer, int len);  // sv.o
extern const float AngleSubtract(float a1, float a2);     // core.o
extern const float AngleNormalize360Accurate(float angle);  // core.o
extern void YawVectors(float yaw, float* const forward,
                       float* const right);  // core.o
extern void G_AddLean(Entity* ent, float* point);  // g.o
extern void G_DPrintf(const char* fmt, ...);       // g.o
extern void G_FreeEntityRefs(Entity* ed);          // g.o
extern void Scr_FreeSentientFields(sentient_s* pSentient);  // scr.o
extern int sLatency;                               // @ 0xE37C98

// ============================================================================
// sentient glob state (anonymous struct @ 0xF992D4, IDA-verified)
// ============================================================================
struct {
    float lastTime[2];         // +0x00 (lastTime + lastSample storage)
    int   lastSample;          // +0x04
    float playerTrail[16][3];  // +0x08 (192 bytes)
    int   sampleTime[16];      // +0x20 (64 bytes)
} glob;  // ?glob@@3U__unnamed@@A @ 0xF992D4

const char* const pszTeamName[5] = {
    "invalid", "dead", "neutral", "allies", "axis",
};  // ?pszTeamName@@3QBQBDB @ 0xE37BF8

// ============================================================================
// Sentient lifecycle
// ============================================================================

// ea: 0x0077CBF0
sentient_s* Sentient_Alloc()
{
    if (level.sentients == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 45;
        AeAssert::gCurrentExpr = "level.sentients != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int v0 = -1;
    int v1 = 0x80000000;
    int v2 = 0;
    int* p_iSpawnTime = &level.sentients->iSpawnTime;
    while (1)
    {
        int v4 = *p_iSpawnTime;
        if (*p_iSpawnTime == -1)
            break;
        if (v4 < 0 && v4 > v1)
        {
            v0 = v2;
            v1 = *p_iSpawnTime;
        }
        ++v2;
        p_iSpawnTime += 92;
        if (v2 >= 48)
            goto done;
    }
    v0 = v2;
done:
    if (v0 >= 0)
    {
        int v6 = v0;
        memset(&level.sentients[v6], 0, sizeof(level.sentients[v6]));
        level.sentients[v6].iSpawnTime = level.time;
        return &level.sentients[v6];
    }
    G_DPrintf("Sentient allocation failed\n");
    return nullptr;
}

// ea: 0x0077CCC0
void Sentient_Clean(sentient_s* sentient)
{
    sentient->pEnt = nullptr;
    sentient->iSpawnTime = level.time;
}

// ea: 0x0077CCE0
void Sentient_InheritGoal(sentient_s* pSelf, sentient_s* pBequeather,
                          team_t eBequeatherTeam)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 180;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pGoalEnt == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 181;
        AeAssert::gCurrentExpr = "pSelf->pGoalEnt";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pGoalEnt->sentient == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 182;
        AeAssert::gCurrentExpr = "pSelf->pGoalEnt->sentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->eTeam == eBequeatherTeam)
        pSelf->pGoalEnt = pBequeather->pGoalEnt;
    else
        pSelf->pGoalEnt = nullptr;
}

// ea: 0x0077CDE0
void Sentient_DissociateSentient(sentient_s* pSelf, sentient_s* pOther,
                                 team_t eOtherTeam)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 199;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf == pOther)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 200;
        AeAssert::gCurrentExpr = "pSelf != pOther";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pOther == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 201;
        AeAssert::gCurrentExpr = "pOther";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pOther->eTeam != TEAM_DEAD)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 202;
        AeAssert::gCurrentExpr = "pOther->eTeam == TEAM_DEAD";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Entity* pGoalEnt = pSelf->pGoalEnt;
    if (pGoalEnt != nullptr && pGoalEnt->sentient == pOther)
        Sentient_InheritGoal(pSelf, pOther, eOtherTeam);
    if (pSelf->pEnemy == pOther)
        pSelf->pEnemy = nullptr;
}

// ea: 0x0077CF30
void Sentient_DissociateEntity(sentient_s* pSelf, Entity* pOther)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 229;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pOther == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 230;
        AeAssert::gCurrentExpr = "pOther";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pOther->sentient != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 231;
        AeAssert::gCurrentExpr = "pOther->sentient == 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pGoalEnt == pOther)
        pSelf->pGoalEnt = nullptr;
}

// ea: 0x0077D020
void __fastcall Sentient_GetOrigin(const sentient_s* pSelf,
                                   float* const vOriginOut)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 246;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 247;
        AeAssert::gCurrentExpr = "pSelf->pEnt";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt->actor == nullptr && pSelf->pEnt->client == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 248;
        AeAssert::gCurrentExpr = "pSelf->pEnt->actor || pSelf->pEnt->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (vOriginOut == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 249;
        AeAssert::gCurrentExpr = "vOriginOut";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    vOriginOut[0] = pSelf->pEnt->r.currentOrigin.v.m128_f32[0];
    vOriginOut[1] = pSelf->pEnt->r.currentOrigin.v.m128_f32[1];
    vOriginOut[2] = pSelf->pEnt->r.currentOrigin.v.m128_f32[2];
}

// ea: 0x0077D170
void __fastcall Sentient_GetForwardDir(sentient_s* pSelf,
                                       float* const vDirOut)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 263;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 264;
        AeAssert::gCurrentExpr = "pSelf->pEnt";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt->actor == nullptr && pSelf->pEnt->client == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 265;
        AeAssert::gCurrentExpr = "pSelf->pEnt->actor || pSelf->pEnt->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (vDirOut == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 266;
        AeAssert::gCurrentExpr = "vDirOut";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    YawVectors(pSelf->pEnt->r.currentAngles.v.m128_f32[1], vDirOut, 0);
}

// ea: 0x0077D2B0
void __fastcall Sentient_GetVelocity(sentient_s* pSelf,
                                     float* const vVelOut)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 280;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 281;
        AeAssert::gCurrentExpr = "pSelf->pEnt";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt->actor == nullptr && pSelf->pEnt->client == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 282;
        AeAssert::gCurrentExpr = "pSelf->pEnt->actor || pSelf->pEnt->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (vVelOut == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 283;
        AeAssert::gCurrentExpr = "vVelOut";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    actor_s* actor = pSelf->pEnt->actor;
    if (actor != nullptr)
    {
        vVelOut[0] = actor->Physics.vVelocity.v.m128_f32[0];
        vVelOut[1] = pSelf->pEnt->actor->Physics.vVelocity.v.m128_f32[1];
        vVelOut[2] = pSelf->pEnt->actor->Physics.vVelocity.v.m128_f32[2];
    }
    else
    {
        vVelOut[0] = pSelf->pEnt->client->ps.velocity.v.m128_f32[0];
        vVelOut[1] = pSelf->pEnt->client->ps.velocity.v.m128_f32[1];
        vVelOut[2] = pSelf->pEnt->client->ps.velocity.v.m128_f32[2];
    }
}

// ea: 0x0077D440
void __fastcall Sentient_GetCentroid(sentient_s* pSelf,
                                     float* const vCentroidOut)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 300;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 301;
        AeAssert::gCurrentExpr = "pSelf->pEnt";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt->actor == nullptr && pSelf->pEnt->client == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 302;
        AeAssert::gCurrentExpr = "pSelf->pEnt->actor || pSelf->pEnt->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (vCentroidOut == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 303;
        AeAssert::gCurrentExpr = "vCentroidOut";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float v4 = pSelf->pEnt->r.currentOrigin.v.m128_f32[2];
    vCentroidOut[0] = pSelf->pEnt->r.currentOrigin.v.m128_f32[0];
    vCentroidOut[1] = pSelf->pEnt->r.currentOrigin.v.m128_f32[1];
    vCentroidOut[2] = v4;
    float v5, v6;
    if (pSelf->pEnt->actor != nullptr)
    {
        v5 = actorMaxs.v.m128_f32[2];
        v6 = actorMins.v.m128_f32[2];
    }
    else
    {
        v5 = playerMaxs.v.m128_f32[2];
        v6 = playerMins.v.m128_f32[2];
    }
    vCentroidOut[2] = ((v5 - v6) * 0.5f + v4) + v6;
}

// ea: 0x0077D5D0
void __fastcall Sentient_GetEyePosition(sentient_s* pSelf,
                                        float* const vEyePosOut)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 329;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 330;
        AeAssert::gCurrentExpr = "pSelf->pEnt";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt->actor == nullptr && pSelf->pEnt->client == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 331;
        AeAssert::gCurrentExpr = "pSelf->pEnt->actor || pSelf->pEnt->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (vEyePosOut == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 332;
        AeAssert::gCurrentExpr = "vEyePosOut";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int IsVisible = pSelf->pEnt->IsVisible();
    if (pSelf->pEnt->actor == nullptr || IsVisible == 0)
    {
        float v5 = pSelf->pEnt->r.currentOrigin.v.m128_f32[2];
        vEyePosOut[0] = pSelf->pEnt->r.currentOrigin.v.m128_f32[0];
        vEyePosOut[1] = pSelf->pEnt->r.currentOrigin.v.m128_f32[1];
        vEyePosOut[2] = v5;
        Client* client = pSelf->pEnt->client;
        if (client != nullptr)
        {
            vEyePosOut[2] = client->ps.viewHeightCurrent + v5;
            G_AddLean(pSelf->pEnt, vEyePosOut);
        }
        else
        {
            vEyePosOut[2] = v5 + 64.0f;
        }
    }
}

// ea: 0x0077D770
void __fastcall Sentient_GetEyePosition(sentient_s* pSelf,
                                        math::Position3& vEyePosOut)
{
    float tmp[3];
    Sentient_GetEyePosition(pSelf, tmp);
    memcpy(&vEyePosOut, tmp, 12);
}

// ea: 0x0077D7B0
float __fastcall Sentient_GetHeadHeight(sentient_s* pSelf)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 372;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 373;
        AeAssert::gCurrentExpr = "pSelf->pEnt";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt->actor == nullptr && pSelf->pEnt->sentient == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 374;
        AeAssert::gCurrentExpr = "pSelf->pEnt->actor || pSelf->pEnt->sentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Entity* pEnt = pSelf->pEnt;
    if (pEnt->actor != nullptr)
        return actorMaxs.v.m128_f32[2] - 64.0f;
    return playerMaxs.v.m128_f32[2]
           - pEnt->client->ps.standViewHeight;
}

// ea: 0x0077D8D0
void __fastcall Sentient_InvalidateNearestNode(sentient_s* pSelf)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 642;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    pSelf->bNearestNodeValid = 0;
}

// ============================================================================
// Goal management
// ============================================================================

// ea: 0x0077D930
void __fastcall Sentient_SetGoalRadius(sentient_s* pSelf, float fRadius)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 967;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float v4 = fRadius;
    if (fRadius < 0.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 968;
        AeAssert::gCurrentExpr = "fRadius >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        v4 = fRadius;
    }
    if (v4 < 4.0f)
        v4 = 4.0f;
    pSelf->fGoalRadius = v4;
    pSelf->fGoalRadiusSqrd = v4 * v4;
}

// ea: 0x0077DA00
void __fastcall Sentient_SetGoalScriptCallback(sentient_s* pSelf,
                                               HashString callbackName)
{
    if (pSelf != nullptr)
        goto set;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
    AeAssert::gCurrentLine = 986;
    AeAssert::gCurrentExpr = "pSelf";
    if (AeAssert::IsIgnored())
    {
        pSelf->bUsedScriptCallback = 0;
        pSelf->hGoalScriptCallback = callbackName;
    }
    else
    {
        if (AeAssert::Assert("old cod assert"))
        {
            __debugbreak();
        set:
            pSelf->bUsedScriptCallback = false;
            pSelf->hGoalScriptCallback = callbackName;
            return;
        }
        pSelf->bUsedScriptCallback = 0;
        pSelf->hGoalScriptCallback = callbackName;
    }
}

// ea: 0x0077DA80
void __fastcall Sentient_SetGoalAngle(sentient_s* pSelf, float fAngle)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 1002;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float a2 = AngleNormalize360Accurate(fAngle);
    pSelf->fGoalAngle = a2;
}

// ea: 0x0077DAE0
void __fastcall Sentient_ClearGoalAngle(sentient_s* pSelf)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 1015;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    pSelf->fGoalAngle = -1.0f;
}

// ea: 0x0077DB40
void __fastcall Sentient_SetGoalAngleTolerance(sentient_s* pSelf,
                                               float fTolerance)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 1029;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float v4 = 180.0f;
    float fTolerancea = fabsf(fTolerance);
    if (fTolerancea <= 180.0f)
        v4 = fTolerancea;
    pSelf->fGoalAngleTolerance = v4;
}

// ea: 0x0077DBC0
int __fastcall Sentient_HasGoalAngle(sentient_s* pSelf)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 1042;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return pSelf->fGoalAngle != -1.0f;
}

// ea: 0x0077DC30
int __fastcall Sentient_AngleSatisfiesGoal(sentient_s* pSelf, float fAngle)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 1059;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int result = true;
    if (pSelf->fGoalAngle != -1.0f)
    {
        float fAngleDelta = AngleSubtract(pSelf->fGoalAngle, fAngle);
        if (fAngleDelta < -pSelf->fGoalAngleTolerance
            || pSelf->fGoalAngleTolerance < fAngleDelta)
            return false;
    }
    return result;
}

// ============================================================================
// Sentient enumeration
// ============================================================================

// ea: 0x0077DCE0
sentient_s* __fastcall Sentient_FirstSentient(int iTeamFlags)
{
    if (iTeamFlags > 31)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 1220;
        AeAssert::gCurrentExpr = "iTeamFlags <= (1 << TEAM_NUM_TEAMS) - 1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int v2 = 0;
    for (team_t* i = &level.sentients->eTeam;
         *(i + 38) < 0 || ((1 << *i) & iTeamFlags) == 0;
         i += 92)
    {
        if (++v2 >= 48)
            return nullptr;
    }
    return &level.sentients[v2];
}

// ea: 0x0077DD80
sentient_s* __fastcall Sentient_NextSentient(sentient_s* pPrevSentient,
                                             int iTeamFlags)
{
    if (iTeamFlags > 31)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 1247;
        AeAssert::gCurrentExpr = "iTeamFlags <= (1 << TEAM_NUM_TEAMS) - 1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pPrevSentient == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 1249;
        AeAssert::gCurrentExpr = "pPrevSentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    sentient_s* sentients = level.sentients;
    if (pPrevSentient < level.sentients || pPrevSentient >= &level.sentients[48])
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 1250;
        AeAssert::gCurrentExpr =
            "pPrevSentient >= level.sentients && "
            "pPrevSentient < level.sentients + (16 + 32)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        sentients = level.sentients;
    }
    if (pPrevSentient != &sentients[pPrevSentient - sentients])
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 1251;
        AeAssert::gCurrentExpr =
            "pPrevSentient == level.sentients + "
            "(pPrevSentient - level.sentients)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        sentients = level.sentients;
    }
    int v4 = pPrevSentient - sentients + 1;
    if (v4 >= 48)
        return nullptr;
    for (team_t* i = &sentients[v4].eTeam;
         *(i + 38) < 0 || ((1 << *i) & iTeamFlags) == 0;
         i += 92)
    {
        if (++v4 >= 48)
            return nullptr;
    }
    return &sentients[v4];
}

// ea: 0x0077DF40
const char* __fastcall Sentient_NameForTeam(team_t eTeam)
{
    if (eTeam > 4)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 1277;
        AeAssert::gCurrentExpr = "eTeam >= 0 && eTeam < TEAM_NUM_TEAMS";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return pszTeamName[eTeam];
}

// ea: 0x0077DFA0
void __fastcall Sentient_SetTeam(sentient_s* pSelf, team_t eTeam)
{
    if (eTeam <= TEAM_FREE || eTeam >= TEAM_NUM_TEAMS)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 1303;
        AeAssert::gCurrentExpr = "eTeam > TEAM_BAD && eTeam < TEAM_NUM_TEAMS";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->eTeam != eTeam)
        pSelf->eTeam = eTeam;
}

// ============================================================================
// Trail / save state
// ============================================================================

// ea: 0x0077E030
void __fastcall Sentient_GetTrailPos(sentient_s* pSelf, float* pos, int* time)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 1512;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt->actor == nullptr && pSelf->pEnt->client == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 1513;
        AeAssert::gCurrentExpr = "pSelf->pEnt->actor || pSelf->pEnt->client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->pEnt->client != nullptr)
    {
        int v5 = glob.lastSample - 1;
        if (glob.lastSample - 1 < 0)
            v5 = 1;
        pos[0] = glob.playerTrail[v5][0];
        pos[1] = glob.playerTrail[v5][1];
        pos[2] = glob.playerTrail[v5][2];
        *time = glob.sampleTime[v5];
    }
    else
    {
        float eye[3];
        Sentient_GetEyePosition(pSelf, eye);
        pos[0] = eye[0];
        pos[1] = eye[1];
        pos[2] = eye[2];
        *time = level.time;
    }
}

// ea: 0x0077E010
void Sentient_WriteGlob()
{
    SV_SaveWrite(&glob, 40);
}

// ea: 0x0077E000 (empty in MP release)
void Sentient_UpdatePlayerTrail(PlayerState* ps)
{
    (void)ps;
}

// ea: 0x0077E020 (empty in MP release)
void Sentient_ReadGlob()
{
}

// ea: 0x00780FD0
void Sentient_Dissociate(sentient_s* pSentient)
{
    unsigned short mValue = pSentient->mClaimedNode.mValue;
    if (mValue != 0 && mValue != 0xFFFF
        && PathNodeMgr::sInst->GetNode(
               *(const PathNodes::NodeHandle*)&pSentient->mClaimedNode)
               != nullptr)
    {
        PathNodes::PathNode* Node =
            PathNodeMgr::sInst->GetNode(
                *(const PathNodes::NodeHandle*)&pSentient->mClaimedNode);
        Path_RelinquishNodePermanently(Node, pSentient);
        pSentient->mClaimedNode.mValue = 0;
    }
    PathNodeMgr::sInst->DissociateSentient(pSentient);
    team_t eTeam = pSentient->eTeam;
    pSentient->eTeam = TEAM_DEAD;
    for (sentient_s* i = Sentient_FirstSentient(-1);
         i != nullptr;
         i = Sentient_NextSentient(i, -1))
    {
        if (i != pSentient)
            Sentient_DissociateSentient(i, pSentient, eTeam);
    }
}

// ea: 0x00783AC0
void Sentient_Free(sentient_s* sentient)
{
    if (sentient == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 95;
        AeAssert::gCurrentExpr = "sentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (level.sentients == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 96;
        AeAssert::gCurrentExpr = "level.sentients";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (sentient < level.sentients || sentient >= &level.sentients[48])
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 97;
        AeAssert::gCurrentExpr =
            "sentient >= level.sentients && "
            "sentient < level.sentients + (16 + 32)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (&level.sentients[sentient - level.sentients] != sentient)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 98;
        AeAssert::gCurrentExpr =
            "&level.sentients[sentient - level.sentients] == sentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (sentient->pEnt == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 99;
        AeAssert::gCurrentExpr = "sentient->pEnt";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (sentient->pEnt->actor != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 100;
        AeAssert::gCurrentExpr = "sentient->pEnt->actor == 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    G_FreeEntityRefs(sentient->pEnt);
    Sentient_Dissociate(sentient);
    sentient->pEnt->sentient = nullptr;
    Scr_FreeSentientFields(sentient);
    memset(sentient, 0xF0, sizeof(sentient_s));
    sentient->iSpawnTime = g_iSentientFreeSequence - 1;
    g_iSentientFreeSequence = g_iSentientFreeSequence - 1;
}

// ea: 0x00781070
void __fastcall Sentient_UpdateActualChainPos(sentient_s* pSelf)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sentient.cpp";
        AeAssert::gCurrentLine = 397;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (level.pathsInvalid)
    {
        pSelf->mActualChainPos.mValue = 0;
    }
    else if (level.time - pSelf->iActualChainPosTime > sLatency)
    {
        pSelf->iActualChainPosTime = level.time;
        float vOrigin[3];
        Sentient_GetOrigin(pSelf, vOrigin);
        PathNodeMgr* v3 = PathNodeMgr::sInst;
        PathNodes::PathNode* Node =
            PathNodeMgr::sInst->GetNode(pSelf->mActualChainPos);
        PathNodes::PathNode* ChainPos =
            v3->FindChainPos(vOrigin, Node);
        if (ChainPos != nullptr)
            pSelf->mActualChainPos.mValue = ChainPos->mHandle.mValue;
    }
}
