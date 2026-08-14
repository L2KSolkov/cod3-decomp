// ============================================================================
// pathnode.cpp - mp_actors.o Path_* free functions (pathnode.cpp)
// ============================================================================

#include "game/logic/g_local.h"
#include "game/actor_types.h"

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <intrin.h>

extern level_locals_t level;           // ?level@@3Ulevel_locals_t@@A @ 0xEC9650
extern const float colorRed[4];        // 0xD0155C
extern const float colorYellow[4];     // 0xD0159C
extern const float colorGreen[4];      // 0xD0156C
extern const math::Position3 actorMins;  // 0xF99510
extern int currCl;                     // ?currCl@@3HA
extern char* va(const char* fmt, ...); // core.o
extern void Scr_Error(const char* error);  // core.o
extern void Z_FreeInternal(void* ptr);     // core.o
extern void G_DebugLine(const float* start, const float* end,
                        const float* color, int depthTest,
                        int duration);     // g.o
extern void G_DebugBox(const float* mins, const float* maxs,
                       const float* color, int depthTest, int duration,
                       int fade);          // g.o
extern void g_AddDebugString(const float* xyz, const float* color,
                             float scale, const char* pszText);  // g.o
extern void G_DPrintf(const char* fmt, ...);  // g.o
extern int irand(int min, int max);           // core.o
extern void FastSinCos(float radians, float* psin, float* pcos);  // core.o

// nodeColorTable / nodeStringTable - debug draw tables (mp_actors.o data)
extern const float nodeColorTable[0x13][4];   // 0xD1F0B8
extern const char* nodeStringTable[0x13];     // ?nodeStringTable@@3PAPBDA @ 0xE37A20

// ============================================================================
// pathnode.cpp data (verified VAs)
// ============================================================================
PathNodes::PathNode* debugPath = nullptr;  // ?debugPath @@ 0xF992CC
int iValidBits = 0;                        // 0xF992D0? (Path_DrawDebugFindPath)

// fields[] - script field descriptors (0xE37A70, 12 entries)
struct pathnode_field_t {
    const char* name;   // +0x00
    void (*getter)(PathNodes::PathNode*, int);  // +0x04
};
pathnode_field_t fields[12];  // ?fields @@ 0xE37A70

// ============================================================================
// Path_* free functions
// ============================================================================

// ea: 0x0077E1C0
void Scr_SetPathnodeField(int entnum, int offset)
{
    if ((unsigned int)offset >= 0xC)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 319;
        AeAssert::gCurrentExpr =
            "(unsigned) offset < (sizeof(fields) / sizeof(fields[0])) - 1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if ((unsigned int)entnum >= 0x800)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 320;
        AeAssert::gCurrentExpr = "(unsigned) entnum < 2048";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    const char* v2 = va("pathnode property '%s' is read-only\n",
                        fields[offset].name);
    Scr_Error(v2);
}

// ea: 0x0077E280
void Scr_GetPathnodeField(int entnum, int offset)
{
    unsigned int v2 = offset;
    if ((unsigned int)offset >= 0xC)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 335;
        AeAssert::gCurrentExpr =
            "(unsigned) offset < (sizeof(fields) / sizeof(fields[0])) - 1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    PathNodes::NodeHandle h;
    h.mValue = (uint16_t)entnum;
    PathNodes::PathNode* Node = PathNodeMgr::sInst->GetNode(h);
    if (Node != nullptr)
    {
        void (*getter)(PathNodes::PathNode*, int) = fields[v2].getter;
        if (getter != nullptr)
            getter(Node, v2);
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 342;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                "GetPathnodeField: Node is undefined.  Node may not be "
                "loaded."))
            __debugbreak();
    }
}

// ea: 0x0077E350
bool GScr_AddFieldsForPathnode()
{
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
    AeAssert::gCurrentLine = 425;
    AeAssert::gCurrentExpr = "0";
    bool result = AeAssert::IsIgnored();
    if (!result)
    {
        result = AeAssert::Assert("ma dead code");
        if (result)
            __debugbreak();
    }
    return result;
}

// ea: 0x0077E3A0
PathNodes::PathNode* Scr_GetPathnode(unsigned int entnum)
{
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
    AeAssert::gCurrentLine = 491;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("ma dead code"))
        __debugbreak();
    return nullptr;
}

// ea: 0x0077E400
int Path_CompareNodesIncreasing(const void* pe1, const void* pe2)
{
    if (((const float*)pe2)[1] <= ((const float*)pe1)[1])
        return 1;
    return -1;
}

// ea: 0x0077E420
int Path_CompareNodesDecreasing(const void* pe1, const void* pe2)
{
    if (((const float*)pe1)[1] <= ((const float*)pe2)[1])
        return 1;
    return -1;
}

// ea: 0x0077E440
void Path_Init()
{
    debugPath = nullptr;
}

// ea: 0x0077E450
void Path_Shutdown()
{
    if (debugPath != nullptr)
    {
        Z_FreeInternal(debugPath);
        debugPath = nullptr;
    }
}

// ea: 0x0077E470
int Path_IsDynamicBlockingEntity(Entity* ent)
{
    return ent->flags & 0x1000;
}

// ea: 0x0077E490
void Path_DrawDebugNoLinks(const PathNodes::PathNode* pNode)
{
    float v1 = pNode->mConstant.mOrigin[0];
    float v2 = pNode->mConstant.mOrigin[1];
    float v3 = (pNode->mConstant.mOrigin[2] + actorMins.v.m128_f32[2])
               + 1.0f;
    float vStart[3];
    float vEnd[3];
    vStart[0] = v1 + 6.9282031f;
    vStart[1] = v2 + 4.0f;
    vStart[2] = v3;
    vEnd[0] = v1 - 6.9282031f;
    vEnd[1] = v2 - 4.0f;
    vEnd[2] = v3;
    G_DebugLine(vStart, vEnd, colorRed, 0, 0);
    vStart[0] = v1 - 4.0f;
    vStart[1] = v2 + 6.9282031f;
    vStart[2] = v3;
    vEnd[0] = v1 + 4.0f;
    vEnd[1] = v2 - 6.9282031f;
    vEnd[2] = v3;
    G_DebugLine(vStart, vEnd, colorRed, 0, 0);
}

// ea: 0x0077E5B0
void Path_DrawDebugFindPath(actor_s* pSelf, const float* const vGoalPos)
{
    (void)pSelf;
    (void)vGoalPos;
    iValidBits = 0;
}

// ea: 0x0077E5C0
Entity* Path_DrawDebug()
{
    return EntityManager::sInst->GetPlayer(currCl);
}

// ea: 0x0077E5E0
void Path_ClaimNode(PathNodes::PathNode* pNode, sentient_s* pClaimer)
{
    if (pNode == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1525;
        AeAssert::gCurrentExpr = "pNode";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pClaimer == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1526;
        AeAssert::gCurrentExpr = "pClaimer";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    team_t eTeam = pClaimer->eTeam;
    if (eTeam != TEAM_AXIS && eTeam != TEAM_ALLIES && eTeam != TEAM_NEUTRAL)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1527;
        AeAssert::gCurrentExpr =
            "pClaimer->eTeam == TEAM_AXIS || pClaimer->eTeam == TEAM_ALLIES "
            "|| pClaimer->eTeam == TEAM_NEUTRAL";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("%i", pClaimer->eTeam))
            __debugbreak();
    }
    if (pNode->mDynamic.mOwner != pClaimer
        && pNode->mDynamic.mFreeTime >= level.time)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1528;
        AeAssert::gCurrentExpr =
            "pNode->mDynamic.mOwner == pClaimer || "
            "pNode->mDynamic.mFreeTime < level.time";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("%i", pNode->mDynamic.mFreeTime))
            __debugbreak();
    }
    sentient_s* mOwner = pNode->mDynamic.mOwner;
    if (mOwner != nullptr && mOwner != pClaimer
        && level.time <= *(&pNode->mDynamic.mFreeTime + pClaimer->eTeam)
        && pNode->mHandle.mValue != pClaimer->mDesiredChainPos.mValue)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1534;
        AeAssert::gCurrentExpr =
            "pNode->mDynamic.mOwner == 0 || pNode->mDynamic.mOwner == "
            "pClaimer || level.time > pNode->mDynamic.mValidTime"
            "[pClaimer->eTeam - TEAM_AXIS] || "
            "pClaimer->mDesiredChainPos == pNode->mHandle";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    pNode->mDynamic.mOwner = pClaimer;
    pNode->mDynamic.mFreeTime = 0x7FFFFFFF;
    uint16_t mValue = pNode->mConstant.mOverlapNode[0].mValue;
    if (mValue != 0 && mValue != 0xFFFF)
    {
        PathNodes::PathNode* v6 =
            PathNodeMgr::sInst->GetNode(pNode->mConstant.mOverlapNode[0]);
        if (v6->mDynamic.mOwner != pClaimer
            && v6->mDynamic.mFreeTime == 0x7FFFFFFF
            && v6->mDynamic.mOverlapCount == 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
            AeAssert::gCurrentLine = 1555;
            AeAssert::gCurrentExpr =
                "((pOtherNode->mDynamic.mOwner == pClaimer) || "
                "(pOtherNode->mDynamic.mFreeTime != 2147483647) || "
                "(pOtherNode->mDynamic.mOverlapCount))";
            if (!AeAssert::IsIgnored())
            {
                sentient_s* v7 = v6->mDynamic.mOwner;
                int v8 = v7 != nullptr ? (int)(v7 - level.sentients) : -1;
                const char* v9 = va(
                    "node = %i, owner = %i, free time = %i",
                    v6->mHandle.mValue, v8, v6->mDynamic.mFreeTime);
                if (AeAssert::Assert(v9))
                    __debugbreak();
            }
        }
        ++v6->mDynamic.mOverlapCount;
        v6->mDynamic.mOwner = nullptr;
        v6->mDynamic.mFreeTime = 0x7FFFFFFF;
        if (v6->mDynamic.mOverlapCount > 2)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
            AeAssert::gCurrentLine = 1560;
            AeAssert::gCurrentExpr =
                "pOtherNode->mDynamic.mOverlapCount <= "
                "(sizeof(pOtherNode->mConstant.mOverlapNode) / "
                "sizeof(pOtherNode->mConstant.mOverlapNode[0]))";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("%i", v6->mDynamic.mOverlapCount))
                __debugbreak();
        }
        uint16_t v10 = pNode->mConstant.mOverlapNode[1].mValue;
        if (v10 != 0 && v10 != 0xFFFF)
        {
            PathNodes::PathNode* v12 =
                PathNodeMgr::sInst->GetNode(
                    pNode->mConstant.mOverlapNode[1]);
            if (v12->mDynamic.mOwner != pClaimer
                && v12->mDynamic.mFreeTime == 0x7FFFFFFF
                && v12->mDynamic.mOverlapCount == 0)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
                AeAssert::gCurrentLine = 1571;
                AeAssert::gCurrentExpr =
                    "((pOtherNode->mDynamic.mOwner == pClaimer) || "
                    "(pOtherNode->mDynamic.mFreeTime != 2147483647) || "
                    "(pOtherNode->mDynamic.mOverlapCount))";
                if (!AeAssert::IsIgnored())
                {
                    sentient_s* v14 = v12->mDynamic.mOwner;
                    int v15 =
                        v14 != nullptr ? (int)(v14 - level.sentients) : -1;
                    const char* v16 = va(
                        "node = %i, owner = %i, free time = %i",
                        v12->mHandle.mValue, v15,
                        v12->mDynamic.mFreeTime);
                    if (AeAssert::Assert(v16))
                        __debugbreak();
                }
            }
            ++v12->mDynamic.mOverlapCount;
            v12->mDynamic.mOwner = nullptr;
            v12->mDynamic.mFreeTime = 0x7FFFFFFF;
            if (v12->mDynamic.mOverlapCount > 2)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
                AeAssert::gCurrentLine = 1576;
                AeAssert::gCurrentExpr =
                    "pOtherNode->mDynamic.mOverlapCount <= "
                    "(sizeof(pOtherNode->mConstant.mOverlapNode) / "
                    "sizeof(pOtherNode->mConstant.mOverlapNode[0]))";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("%i", v12->mDynamic.mOverlapCount))
                    __debugbreak();
            }
            uint16_t v17 = pNode->mConstant.mOverlapNode[0].mValue;
            if (v17 != 0 && v17 != 0xFFFF
                && v12->mDynamic.mOverlapCount == 0)
                v12->mDynamic.mFreeTime = 0;
        }
    }
}

// ea: 0x0077EB10
void Path_RelinquishNodePermanently(PathNodes::PathNode* pNode,
                                    sentient_s* pClaimer)
{
    if (pNode != nullptr && pClaimer != nullptr
        && pNode->mDynamic.mOwner == pClaimer)
    {
        team_t eTeam = pClaimer->eTeam;
        if (eTeam != TEAM_AXIS && eTeam != TEAM_ALLIES
            && eTeam != TEAM_NEUTRAL)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
            AeAssert::gCurrentLine = 1689;
            AeAssert::gCurrentExpr =
                "pClaimer->eTeam == TEAM_AXIS || pClaimer->eTeam == TEAM_ALLIES "
                "|| pClaimer->eTeam == TEAM_NEUTRAL";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("%i", pClaimer->eTeam))
                __debugbreak();
        }
        pNode->mDynamic.mOwner = nullptr;
        pNode->mDynamic.mFreeTime = 0;
        uint16_t mValue = pNode->mConstant.mOverlapNode[0].mValue;
        if (mValue != 0 && mValue != 0xFFFF)
        {
            PathNodes::PathNode* Node =
                PathNodeMgr::sInst->GetNode(
                    pNode->mConstant.mOverlapNode[0]);
            if (Node->mDynamic.mOwner != nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
                AeAssert::gCurrentLine = 1701;
                AeAssert::gCurrentExpr = "pOtherNode->mDynamic.mOwner == 0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            if (Node->mDynamic.mOverlapCount-- == 1)
                Node->mDynamic.mFreeTime = 0;
            uint16_t v6 = pNode->mConstant.mOverlapNode[1].mValue;
            if (v6 != 0 && v6 != 0xFFFF)
            {
                PathNodes::PathNode* v7 =
                    PathNodeMgr::sInst->GetNode(
                        pNode->mConstant.mOverlapNode[1]);
                if (v7->mDynamic.mOwner != nullptr)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\pathnode.cpp";
                    AeAssert::gCurrentLine = 1709;
                    AeAssert::gCurrentExpr =
                        "pOtherNode->mDynamic.mOwner == 0";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                if (v7->mDynamic.mOverlapCount-- == 1)
                    v7->mDynamic.mFreeTime = 0;
            }
        }
    }
}

// ea: 0x0077EC80
void Path_RelinquishNodeTemporarily(PathNodes::PathNode* pNode,
                                    sentient_s* pClaimer)
{
    if (pNode == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1730;
        AeAssert::gCurrentExpr = "pNode";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pNode->mDynamic.mOwner != pClaimer)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1731;
        AeAssert::gCurrentExpr = "pNode->mDynamic.mOwner == pClaimer";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pClaimer == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1732;
        AeAssert::gCurrentExpr = "pClaimer";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    team_t eTeam = pClaimer->eTeam;
    if (eTeam != TEAM_AXIS && eTeam != TEAM_ALLIES && eTeam != TEAM_NEUTRAL)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1733;
        AeAssert::gCurrentExpr =
            "pClaimer->eTeam == TEAM_AXIS || pClaimer->eTeam == TEAM_ALLIES "
            "|| pClaimer->eTeam == TEAM_NEUTRAL";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("%i", pClaimer->eTeam))
            __debugbreak();
    }
    pNode->mDynamic.mFreeTime = level.time + 2000;
    uint16_t mValue = pNode->mConstant.mOverlapNode[0].mValue;
    if (mValue != 0 && mValue != 0xFFFF)
    {
        PathNodes::PathNode* Node =
            PathNodeMgr::sInst->GetNode(pNode->mConstant.mOverlapNode[0]);
        if (Node->mDynamic.mOwner != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
            AeAssert::gCurrentLine = 1744;
            AeAssert::gCurrentExpr = "pOtherNode->mDynamic.mOwner == 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        if (Node->mDynamic.mOverlapCount-- == 1)
        {
            Node->mDynamic.mOwner = pNode->mDynamic.mOwner;
            Node->mDynamic.mFreeTime = pNode->mDynamic.mFreeTime;
        }
        uint16_t v6 = pNode->mConstant.mOverlapNode[1].mValue;
        if (v6 != 0 && v6 != 0xFFFF)
        {
            PathNodes::PathNode* v7 =
                PathNodeMgr::sInst->GetNode(
                    pNode->mConstant.mOverlapNode[1]);
            if (v7->mDynamic.mOwner != nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\pathnode.cpp";
                AeAssert::gCurrentLine = 1755;
                AeAssert::gCurrentExpr =
                    "pOtherNode->mDynamic.mOwner == 0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            if (v7->mDynamic.mOverlapCount-- == 1)
            {
                v7->mDynamic.mOwner = pNode->mDynamic.mOwner;
                v7->mDynamic.mFreeTime = pNode->mDynamic.mFreeTime;
            }
        }
    }
}

// ea: 0x0077EEC0
int Path_IsNodeValid(const PathNodes::PathNode* pNode, team_t eTeam)
{
    if (pNode == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1769;
        AeAssert::gCurrentExpr = "pNode";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (eTeam != TEAM_AXIS && eTeam != TEAM_ALLIES && eTeam != TEAM_NEUTRAL)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1770;
        AeAssert::gCurrentExpr =
            "eTeam == TEAM_AXIS || eTeam == TEAM_ALLIES || eTeam == "
            "TEAM_NEUTRAL";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", eTeam))
            __debugbreak();
    }
    return level.time > *(&pNode->mDynamic.mFreeTime + eTeam);
}

// ea: 0x0077EF80
void Path_MarkNodeUnsafe(PathNodes::PathNode* pNode, team_t eTeam)
{
    if (pNode == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1877;
        AeAssert::gCurrentExpr = "pNode";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (eTeam != TEAM_AXIS && eTeam != TEAM_ALLIES && eTeam != TEAM_NEUTRAL)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1878;
        AeAssert::gCurrentExpr =
            "eTeam == TEAM_AXIS || eTeam == TEAM_ALLIES || eTeam == "
            "TEAM_NEUTRAL";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", eTeam))
            __debugbreak();
    }
    // IDA renders this as mValidTime[eTeam + 2]; the adjacent mSafeTime
    // array aliases that slot, so the compiled offset is mSafeTime[eTeam-1].
    pNode->mDynamic.mSafeTime[eTeam - TEAM_AXIS] =
        (level.time + 5000) + irand(-2000, 2000);
}

// ea: 0x0077F060
int Path_IsNodeUnsafe(const PathNodes::PathNode* pNode, team_t eTeam)
{
    if (pNode == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1895;
        AeAssert::gCurrentExpr = "pNode";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (eTeam != TEAM_AXIS && eTeam != TEAM_ALLIES && eTeam != TEAM_NEUTRAL)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1896;
        AeAssert::gCurrentExpr =
            "eTeam == TEAM_AXIS || eTeam == TEAM_ALLIES || eTeam == "
            "TEAM_NEUTRAL";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", eTeam))
            __debugbreak();
    }
    return pNode->mDynamic.mSafeTime[eTeam - TEAM_AXIS] > level.time;
}

// ea: 0x0077F120
ai_stance_e Path_AllowedStancesForNode(PathNodes::PathNode* pNode)
{
    if (pNode == nullptr)
        return STANCE_BAD;
    uint16_t mSpawnFlags = pNode->mConstant.mSpawnFlags;
    int v3 = STANCE_ANY;
    if ((mSpawnFlags & 4) != 0)
        v3 = STANCE_PRONE | STANCE_CROUCH;
    if ((mSpawnFlags & 8) != 0)
        v3 &= ~STANCE_CROUCH;
    if ((mSpawnFlags & 0x10) != 0)
        v3 &= ~STANCE_PRONE;
    if (v3 == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1922;
        AeAssert::gCurrentExpr = "eAllowedStances";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("%i", pNode->mConstant.mSpawnFlags))
            __debugbreak();
    }
    return (ai_stance_e)v3;
}

// ea: 0x0077F1F0
void Path_GetType(PathNodes::PathNode* pNode, int offset)
{
    (void)pNode;
    (void)offset;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
    AeAssert::gCurrentLine = 2136;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("DEAD CODE"))
        __debugbreak();
}

// ea: 0x0077F240
void Path_SetSentientFlagsFromNode(sentient_s* pSent,
                                   const PathNodes::PathNode* pNode)
{
    if (pNode != nullptr)
    {
        if ((pNode->mConstant.mSpawnFlags & 0x10) != 0)
            (uint8_t&)pSent->sFlags |= 1u;
        else
            (uint8_t&)pSent->sFlags &= ~1u;
        if ((pNode->mConstant.mSpawnFlags & 8) != 0)
            pSent->sFlags |= 2u;
        else
            pSent->sFlags &= ~2u;
        if ((pNode->mConstant.mSpawnFlags & 4) != 0)
            pSent->sFlags |= 4u;
        else
            pSent->sFlags &= ~4u;
    }
}

// ea: 0x0077F280
ai_stance_e Path_GetAllowedStancesFromSentientFlags(sentient_s* pSent)
{
    int16_t sFlags = pSent->sFlags;
    ai_stance_e result = STANCE_ANY;
    if ((sFlags & 4) != 0)
        result = (ai_stance_e)(STANCE_PRONE | STANCE_CROUCH);
    if ((sFlags & 2) != 0)
        result = (ai_stance_e)((int)result & ~STANCE_CROUCH);
    if ((sFlags & 1) != 0)
        return (ai_stance_e)((int)result & 0xFFFFFFFB);
    return result;
}

// ea: 0x0077F2B0
void Path_ResetSentientFlags(sentient_s* pSent)
{
    pSent->sFlags = 0;
}

// ea: 0x0077F2C0
PathNodes::TOC1::TOC1()
{
    mVersion = 0;
    mNodeCount = 0;
    mChainNodeCount = 0;
    mLinkCount = 0;
    mVariableCount = 0;
    mNodes = nullptr;
    mLinks = nullptr;
    mVariables = nullptr;
    mTree = nullptr;
    mStrings = nullptr;
    mTreeNodes = nullptr;
}

// ea: 0x0077F2F0
PathNodes::TOC2::TOC2()
{
    mVersion = 0;
    mVisData = nullptr;
}

// ea: 0x0077F300 / 0x0077F320 / 0x0077F330
PathNodes::PathNode* PathNodes::NodeHandle::operator*()
{
    return PathNodeMgr::sInst->GetNode(*this);
}
PathNodes::PathNode* PathNodes::NodeHandle::operator->()
{
    return PathNodeMgr::sInst->GetNode(*this);
}
const PathNodes::PathNode* PathNodes::NodeHandle::operator->() const
{
    return PathNodeMgr::sInst->GetNode(*this);
}

// ea: 0x00781980
void Path_RevokeClaim(PathNodes::PathNode* pNode, sentient_s* pNewClaimer)
{
    sentient_s* mOwner = pNode->mDynamic.mOwner;
    if (mOwner != nullptr && mOwner != pNewClaimer
        && pNode->mDynamic.mFreeTime == 0x7FFFFFFF)
    {
        Sentient_NodeClaimRevoked(mOwner, pNode->mHandle);
        if (pNode->mDynamic.mFreeTime == 0x7FFFFFFF)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
            AeAssert::gCurrentLine = 1599;
            AeAssert::gCurrentExpr =
                "pNode->mDynamic.mFreeTime != 2147483647";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
    }
}

// ea: 0x00781A00
void Path_ForceClaimNode(PathNodes::PathNode* pNode, sentient_s* pClaimer)
{
    if (pNode == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1616;
        AeAssert::gCurrentExpr = "pNode";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pClaimer == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1617;
        AeAssert::gCurrentExpr = "pClaimer";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    team_t eTeam = pClaimer->eTeam;
    if (eTeam != TEAM_AXIS && eTeam != TEAM_ALLIES && eTeam != TEAM_NEUTRAL)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1618;
        AeAssert::gCurrentExpr =
            "pClaimer->eTeam == TEAM_AXIS || pClaimer->eTeam == TEAM_ALLIES "
            "|| pClaimer->eTeam == TEAM_NEUTRAL";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("%i", pClaimer->eTeam))
            __debugbreak();
    }
    if (pNode->mDynamic.mOwner == pClaimer)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1619;
        AeAssert::gCurrentExpr = "pNode->mDynamic.mOwner != pClaimer";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Path_RevokeClaim(pNode, pClaimer);
    if (PathNodeMgr::sInst->GetNode(pNode->mConstant.mOverlapNode[0])
        != nullptr)
    {
        PathNodes::PathNode* Node =
            PathNodeMgr::sInst->GetNode(pNode->mConstant.mOverlapNode[0]);
        Path_RevokeClaim(Node, pClaimer);
        if (PathNodeMgr::sInst->GetNode(pNode->mConstant.mOverlapNode[1])
            != nullptr)
        {
            PathNodes::PathNode* v5 =
                PathNodeMgr::sInst->GetNode(
                    pNode->mConstant.mOverlapNode[1]);
            Path_RevokeClaim(v5, pClaimer);
        }
    }
    if (pNode->mDynamic.mFreeTime == 0x7FFFFFFF)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1632;
        AeAssert::gCurrentExpr = "pNode->mDynamic.mFreeTime != 2147483647";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    pNode->mDynamic.mOwner = pClaimer;
    pNode->mDynamic.mFreeTime = 0x7FFFFFFF;
    if (pNode->mConstant.mOverlapNode[0].mValue != 0
        && pNode->mConstant.mOverlapNode[0].mValue != 0xFFFF)
    {
        PathNodes::PathNode* v6 =
            PathNodeMgr::sInst->GetNode(pNode->mConstant.mOverlapNode[0]);
        ++v6->mDynamic.mOverlapCount;
        v6->mDynamic.mOwner = nullptr;
        v6->mDynamic.mFreeTime = 0x7FFFFFFF;
        uint16_t mValue = pNode->mConstant.mOverlapNode[1].mValue;
        if (mValue != 0 && mValue != 0xFFFF)
        {
            PathNodes::PathNode* v8 =
                PathNodeMgr::sInst->GetNode(
                    pNode->mConstant.mOverlapNode[1]);
            char v9 = v8->mDynamic.mOverlapCount + 1;
            v8->mDynamic.mOwner = nullptr;
            v8->mDynamic.mFreeTime = 0x7FFFFFFF;
            v8->mDynamic.mOverlapCount = v9;
        }
    }
}

// ea: 0x00781CE0
void Path_MarkNodeInvalid(PathNodes::PathNode* pNode, team_t eTeam)
{
    if (pNode == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1788;
        AeAssert::gCurrentExpr = "pNode";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (eTeam != TEAM_AXIS && eTeam != TEAM_ALLIES && eTeam != TEAM_NEUTRAL)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1789;
        AeAssert::gCurrentExpr =
            "eTeam == TEAM_AXIS || eTeam == TEAM_ALLIES || eTeam == "
            "TEAM_NEUTRAL";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", eTeam))
            __debugbreak();
    }
    sentient_s* mOwner = pNode->mDynamic.mOwner;
    if (mOwner != nullptr && mOwner->eTeam == eTeam
        && pNode->mDynamic.mFreeTime == 0x7FFFFFFF)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1800;
        AeAssert::gCurrentExpr =
            "pNode->mDynamic.mOwner == 0 || pNode->mDynamic.mOwner->eTeam != "
            "eTeam || pNode->mDynamic.mFreeTime != 2147483647";
        if (!AeAssert::IsIgnored())
        {
            const char* v3 = va(
                "pOwner = %08x = sentient %i, entnum %i; eTeam = %i; "
                "pNode->mDynamic.mFreeTime = %i\n",
                pNode->mDynamic.mOwner,
                pNode->mDynamic.mOwner - level.sentients,
                pNode->mDynamic.mOwner->pEnt->mHandle.mHandle.mVal, eTeam,
                pNode->mDynamic.mFreeTime);
            if (AeAssert::Assert(v3))
                __debugbreak();
        }
    }
    sentient_s* v4 = pNode->mDynamic.mOwner;
    if (v4 != nullptr && v4->eTeam == eTeam)
        pNode->mDynamic.mOwner = nullptr;
    *(&pNode->mDynamic.mFreeTime + eTeam) = level.time + 2000;
    if (pNode->mDynamic.mOverlapCount == 0)
    {
        pNode->mDynamic.mFreeTime = 0;
        uint16_t mValue = pNode->mConstant.mOverlapNode[0].mValue;
        if (mValue != 0 && mValue != 0xFFFF)
        {
            PathNodes::PathNode* Node =
                PathNodeMgr::sInst->GetNode(
                    pNode->mConstant.mOverlapNode[0]);
            if (Node->mDynamic.mOverlapCount == 0
                && Node->mDynamic.mFreeTime != 0x7FFFFFFF)
            {
                sentient_s* v7 = Node->mDynamic.mOwner;
                if (v7 != nullptr && v7->eTeam == eTeam)
                    Node->mDynamic.mOwner = nullptr;
                Node->mDynamic.mFreeTime = 0;
            }
            uint16_t v8 = pNode->mConstant.mOverlapNode[1].mValue;
            if (v8 != 0 && v8 != 0xFFFF)
            {
                PathNodes::PathNode* v9 =
                    PathNodeMgr::sInst->GetNode(
                        pNode->mConstant.mOverlapNode[1]);
                if (v9->mDynamic.mOverlapCount == 0
                    && v9->mDynamic.mFreeTime != 0x7FFFFFFF)
                {
                    sentient_s* v10 = v9->mDynamic.mOwner;
                    if (v10 != nullptr && v10->eTeam == eTeam)
                        v9->mDynamic.mOwner = nullptr;
                    v9->mDynamic.mFreeTime = 0;
                }
            }
        }
    }
    sentient_s* v11 = pNode->mDynamic.mOwner;
    if (v11 != nullptr && v11->eTeam == eTeam)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1833;
        AeAssert::gCurrentExpr =
            "pNode->mDynamic.mOwner == 0 || pNode->mDynamic.mOwner->eTeam != "
            "eTeam";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
}

// ea: 0x00781F20
void Path_MarkNodeValid(PathNodes::PathNode* pNode, team_t eTeam)
{
    if (pNode == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1848;
        AeAssert::gCurrentExpr = "pNode";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (eTeam != TEAM_AXIS && eTeam != TEAM_ALLIES && eTeam != TEAM_NEUTRAL)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1849;
        AeAssert::gCurrentExpr =
            "eTeam == TEAM_AXIS || eTeam == TEAM_ALLIES || eTeam == "
            "TEAM_NEUTRAL";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", eTeam))
            __debugbreak();
    }
    sentient_s* mOwner = pNode->mDynamic.mOwner;
    if (mOwner == nullptr || mOwner->eTeam == eTeam
        || pNode->mDynamic.mFreeTime != 0x7FFFFFFF)
    {
        *(&pNode->mDynamic.mFreeTime + eTeam) = 0;
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnode.cpp";
        AeAssert::gCurrentLine = 1860;
        AeAssert::gCurrentExpr =
            "pNode->mDynamic.mOwner == 0 || pNode->mDynamic.mOwner->eTeam == "
            "eTeam || pNode->mDynamic.mFreeTime != 2147483647";
        if (!AeAssert::IsIgnored())
        {
            const char* v3 = va(
                "pOwner = %08x = sentient %i, entnum %i; eTeam = %i; "
                "pNode->mDynamic.mFreeTime = %i\n",
                pNode->mDynamic.mOwner,
                pNode->mDynamic.mOwner - level.sentients,
                pNode->mDynamic.mOwner->pEnt->mHandle.mHandle.mVal, eTeam,
                pNode->mDynamic.mFreeTime);
            if (AeAssert::Assert(v3))
                __debugbreak();
        }
        *(&pNode->mDynamic.mFreeTime + eTeam) = 0;
    }
}

// ea: 0x00782070
void Path_SetSentientFlagsFromPath(sentient_s* pSent, path_t* pPath)
{
    PathNodes::PathNode* Node;
    uint16_t mValue = pSent->mClaimedNode.mValue;
    if (mValue != 0 && mValue != 0xFFFF
        && PathNodeMgr::sInst->GetNode(pSent->mClaimedNode) != nullptr)
    {
        Node = PathNodeMgr::sInst->GetNode(pSent->mClaimedNode);
    }
    else
    {
        Node = PathNodeMgr::sInst->GetNode(pPath->pts[0].mNodeHandle);
    }
    if (Node != nullptr)
    {
        if ((Node->mConstant.mSpawnFlags & 0x10) != 0)
            (uint8_t&)pSent->sFlags |= 1u;
        else
            (uint8_t&)pSent->sFlags &= ~1u;
        if ((Node->mConstant.mSpawnFlags & 8) != 0)
            (uint8_t&)pSent->sFlags |= 2u;
        else
            (uint8_t&)pSent->sFlags &= ~2u;
        if ((Node->mConstant.mSpawnFlags & 4) != 0)
            pSent->sFlags |= 4u;
        else
            (uint8_t&)pSent->sFlags &= ~4u;
    }
}

// ea: 0x00784280
void Path_DrawDebugNode(const PathNodes::PathNode* pNode)
{
    float mins[3];
    float maxs[3];
    float start[3];
    float end[3];
    float org[3];
    float fSin;
    float fCos;
    char buf[512];

    PathNodes::ENodeType mType = pNode->mConstant.mType;
    mins[0] = pNode->mConstant.mOrigin[0] - 16.0f;
    mins[1] = pNode->mConstant.mOrigin[1] - 16.0f;
    mins[2] = pNode->mConstant.mOrigin[2];
    maxs[0] = pNode->mConstant.mOrigin[0] + 16.0f;
    maxs[1] = pNode->mConstant.mOrigin[1] + 16.0f;
    maxs[2] = pNode->mConstant.mOrigin[2] + 16.0f;
    G_DebugBox(mins, maxs, nodeColorTable[mType], 1, 0, 0);
    if ((pNode->mConstant.mSpawnFlags & 0x8000) != 0)
    {
        FastSinCos(pNode->mConstant.mAngle * 0.017453292f, &fSin, &fCos);
        start[0] = (maxs[0] + mins[0]) * 0.5f;
        start[1] = (maxs[1] + mins[1]) * 0.5f;
        start[2] = (maxs[2] + mins[2]) * 0.5f;
        end[0] = (fCos * 16.0f) + start[0];
        end[1] = (fSin * 16.0f) + start[1];
        end[2] = start[2];
        G_DebugLine(start, end, nodeColorTable[mType], 1, 0);
    }
    org[0] = pNode->mConstant.mOrigin[0];
    org[1] = pNode->mConstant.mOrigin[1];
    org[2] = pNode->mConstant.mOrigin[2] + 8.0f;
    float scale = 1.0f;
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player != nullptr)
    {
        const float* v = Player->client->ps.origin.v.m128_f32;
        float v6 = v[1] - org[1];
        float v7 = (v[2] + Player->client->ps.viewHeightCurrent) - org[2];
        float v8 = v[0] - org[0];
        scale = sqrtf(v7 * v7 + v6 * v6 + v8 * v8) * 0.0022222223f;
    }
    float origScale = scale;
    sprintf(buf, "%s [%d] [%d]", nodeStringTable[mType],
            (int)pNode->mHandle.mValue,
            (int)(uint16_t)(pNode->mHandle.mValue - 1));
    g_AddDebugString(org, colorYellow, scale, buf);
    sprintf(buf, "%.1f %.1f %.1f", pNode->mConstant.mOrigin[0],
            pNode->mConstant.mOrigin[1], pNode->mConstant.mOrigin[2]);
    scale = scale * 20.0f;
    org[2] = scale + org[2];
    g_AddDebugString(org, colorYellow, origScale, buf);
    if (pNode->mConstant.mTargetName.mBlock != nullptr)
    {
        sprintf(buf, "%s", (const char*)(pNode->mConstant.mTargetName.mBlock
                                         + 1));
        org[2] = scale + org[2];
        g_AddDebugString(org, colorYellow, origScale, buf);
    }
    sentient_s* mOwner = pNode->mDynamic.mOwner;
    if (mOwner != nullptr && mOwner->pEnt != nullptr)
    {
        if (mOwner->pEnt->actor != nullptr
            && mOwner->pEnt->actor->mProperName.mBlock != nullptr)
            sprintf(buf, "%s",
                    (const char*)(mOwner->pEnt->actor->mProperName.mBlock
                                  + 1));
        else
            sprintf(buf, "%x", (unsigned int)mOwner->pEnt);
        org[2] = scale + org[2];
        g_AddDebugString(org, colorRed, origScale, buf);
    }
    uint16_t mSpawnFlags = pNode->mConstant.mSpawnFlags;
    buf[0] = 0;
    if ((mSpawnFlags & 0x10) != 0)
        strcat(buf, "DONT_PRONE, ");
    if ((mSpawnFlags & 8) != 0)
        strcat(buf, "DONT_CROUCH, ");
    if ((mSpawnFlags & 4) != 0)
        strcat(buf, "DONT_STAND, ");
    if ((mSpawnFlags & 0x40) != 0)
        strcat(buf, "DONT_STOP, ");
    if (strlen(buf) != 0)
    {
        org[2] = scale + org[2];
        g_AddDebugString(org, colorYellow, origScale, buf);
    }
    char mFlags = pNode->mDynamic.mFlags;
    buf[0] = 0;
    if (mFlags != 0)
        strcat(buf, "INVALID COVER, ");
    if (strlen(buf) != 0)
    {
        org[2] = scale + org[2];
        g_AddDebugString(org, colorRed, origScale, buf);
    }
}

// ea: 0x00784760
void Path_DrawFriendlyChain()
{
    EntityManager::sInst->GetPlayer(currCl);
}

// ea: 0x00784780
void Path_DrawVisData()
{
    Entity* pPlayer = EntityManager::sInst->GetPlayer(currCl);
    if (pPlayer == nullptr)
        return;
    PathNodes::PathNode* v1 =
        Sentient_NearestNode(pPlayer->sentient, nullptr, nullptr, 0, 1,
                             192.0f, 0);
    if (v1 == nullptr)
        return;
    Path_DrawDebugNode(v1);
    int NumZones = StreamZoneManager::sInst->GetNumZones();
    if (NumZones > 0)
    {
        for (int i = NumZones; i != 0; --i)
        {
            PathNodes::TOC1* mLevelTOC = PathNodeMgr::sInst->mLevelTOC;
            if (mLevelTOC->mNodeCount > 0)
            {
                int v3 = 0;
                int mNodeCount = mLevelTOC->mNodeCount;
                while (1)
                {
                    PathNodes::PathNode* v4 =
                        &mLevelTOC->mNodes[v3];
                    if (v4 != v1)
                    {
                        if (PathNodeMgr::sInst->ExpandedNodeNumsVisible(
                                v1->mHandle, v4->mHandle)
                            && PathNodeMgr::sInst->NodeNumsVisible(
                                v1->mHandle, v4->mHandle))
                        {
                            Path_DrawDebugNode(v4);
                            G_DebugLine(v1->mConstant.mOrigin,
                                        v4->mConstant.mOrigin, colorGreen,
                                        0, 0);
                        }
                        else if (PathNodeMgr::sInst->ExpandedNodeNumsVisible(
                                     v1->mHandle, v4->mHandle))
                        {
                            Path_DrawDebugNode(v4);
                            G_DebugLine(v1->mConstant.mOrigin,
                                        v4->mConstant.mOrigin, colorRed,
                                        0, 0);
                        }
                        else if (PathNodeMgr::sInst->NodeNumsVisible(
                                     v1->mHandle, v4->mHandle))
                        {
                            Path_DrawDebugNode(v4);
                            G_DebugLine(v1->mConstant.mOrigin,
                                        v4->mConstant.mOrigin, colorYellow,
                                        0, 0);
                        }
                    }
                    ++v3;
                    if (--mNodeCount == 0)
                        break;
                }
            }
        }
    }
}
