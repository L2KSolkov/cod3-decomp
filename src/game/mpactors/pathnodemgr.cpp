// ============================================================================
// pathnodemgr.cpp - mp_actors.o PathNodeMgr methods (pathnodemgr.cpp)
// ============================================================================

#include "game/logic/g_local.h"
#include "game/actor_types.h"

#include <stdlib.h>
#include <string.h>

extern level_locals_t level;           // ?level@@3Ulevel_locals_t@@A @ 0xEC9650
extern char* va(const char* fmt, ...); // core.o
extern void Scr_Error(const char* error);  // core.o

// Circle query scratch state (mp_actors.o data @ 0xF992A4; anonymous)
struct {
    float origin[3];             // +0x00
    float maxDist;               // +0x0C
    float maxDistSq;             // +0x10
    int   typeFlags;             // +0x14
    float maxHeightSq;           // +0x18
    PathNodes::PathSort* nodes;  // +0x1C
    int   maxNodes;              // +0x20
    int   nodeCount;             // +0x24
} gCircle;                       // ?gCircle@@3U__unnamed@@A @ 0xF992A4
int g_doDontLinkCheck = 0;       // ?g_doDontLinkCheck@@3HA @ 0xE37A1C

void Path_SetupAnimFunc(PathNodes::PathNode* node,
                        PathNodes::ENodeType* type);  // pathnode.cpp

// ============================================================================
// PathNodeMgr
// ============================================================================

// ea: 0x0077F340
PathNodeMgr::PathNodeMgr()
{
    mLevelTOC = nullptr;
    mLevelTOC2 = nullptr;
    level.pathsInited = false;
    level.pathsConnected = false;
    level.pathsInvalid = true;
}

// ea: 0x0077F370
int PathNodeMgr::FindZoneIndex(TPakId pakId)
{
    (void)pakId;
    return 0;
}

// ea: 0x0077F380
int PathNodeMgr::FindZoneIndex(const char* zone)
{
    (void)zone;
    return 0;
}

// ea: 0x0077F390
void PathNodeMgr::UnloadBank(TPakId pakId)
{
    if (CurPakId() == pakId)
        mLevelTOC = nullptr;
}

// ea: 0x0077F3B0
void PathNodeMgr::CleanUpManager()
{
    mLevelTOC = nullptr;
    level.pathsInited = false;
    level.pathsInvalid = true;
    level.pathsConnected = false;
}

// ea: 0x0077F920
void PathNodeMgr::InitScriptVariables()
{
    if (mLevelTOC != nullptr)
        InitScriptVariables(0);
}

// ea: 0x0077F930
void PathNodeMgr::InitLinkCounts(int zoneIndex)
{
    (void)zoneIndex;
    PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
    if (mLevelTOC != nullptr)
    {
        for (int v3 = 0; v3 < mLevelTOC->mNodeCount; ++v3)
        {
            PathNodes::PathNode* v5 = &mLevelTOC->mNodes[v3];
            v5->mDynamic.mLinkCount =
                v5->mConstant.mTotalLinkCount;
            for (int j = 0; j < v5->mConstant.mTotalLinkCount; ++j)
            {
                v5->mConstant.mLinks[j].mBadPlaceCount[0] = 0;
                v5->mConstant.mLinks[j].mBadPlaceCount[1] = 0;
                v5->mConstant.mLinks[j].mBadPlaceCount[2] = 0;
                v5->mConstant.mLinks[j].mBadPlaceCount[3] = 0;
            }
        }
    }
}

// ea: 0x0077F9D0
void PathNodeMgr::InitLinkInfoArray()
{
    PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
    if (mLevelTOC != nullptr)
    {
        if (mLevelTOC->mLinkPool == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)9;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
            AeAssert::gCurrentLine = 1299;
            AeAssert::gCurrentExpr = "mLevelTOC->mLinkPool";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid link pool pointer"))
                __debugbreak();
        }
        for (int i = 0; i < 3072; ++i)
        {
            PathNodes::PathLinkInfo* p = &mLevelTOC->mLinkPool[i];
            p->from.mValue = 0;
            p->to.mValue = 0;
            p->prev = (uint16_t)((i == 0) ? 3071 : i - 1);
            p->next = (uint16_t)((i == 3071) ? 0 : i + 1);
        }
    }
}

// ea: 0x0077FB60
bool PathNodeMgr::IsConnectedTo(const PathNodes::PathNode* pNode1,
                                const PathNodes::PathNode* pNode2) const
{
    if (pNode1 == nullptr)
        return false;
    if (pNode2 == nullptr)
        return false;
    int mLinkCount = pNode1->mDynamic.mLinkCount;
    if (mLinkCount <= 0)
        return false;
    int v4 = 0;
    for (PathNodes::PathLink* i = pNode1->mConstant.mLinks;
         pNode2->mHandle.mValue != i->mNodeHandle.mValue;
         ++i)
    {
        if (++v4 >= mLinkCount)
            return false;
    }
    return true;
}

// ea: 0x0077FBE0
PathNodes::PathNodeTree* PathNodeMgr::CreateTree_r(
    PathNodes::PathNode** treeNodes, PathNodes::PathNodeTree** tree)
{
    PathNodes::PathNodeTree* v3 = (*tree)++;
    if (v3->axis < 0)
    {
        v3->u.leaf.nodes = &treeNodes[(intptr_t)v3->u.leaf.nodes];
    }
    else
    {
        v3->u.children.left = CreateTree_r(treeNodes, tree);
        v3->u.children.right = CreateTree_r(treeNodes, tree);
    }
    return v3;
}

// ea: 0x0077FC30
const int PathNodeMgr::NodeVisCacheEntry(
    const PathNodes::NodeHandle& a, const PathNodes::NodeHandle& b) const
{
    return (b.mValue - 1) + mLevelTOC->mNodeCount * (a.mValue - 1);
}

// ea: 0x0077FC60
const int PathNodeMgr::ExpandedNodeVisCacheEntry(
    const PathNodes::NodeHandle& a, const PathNodes::NodeHandle& b) const
{
    return (b.mValue - 1) + mLevelTOC->mNodeCount * ((a.mValue - 1) - 1);
}

// ea: 0x0077FC90
bool PathNodeMgr::IsBadPlaceLink(
    const PathNodes::NodeHandle& iNodeNumFrom,
    const PathNodes::NodeHandle& iNodeNumTo, team_t eTeam) const
{
    if (iNodeNumFrom.mValue == 0 || iNodeNumFrom.mValue == 0xFFFF)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1748;
        AeAssert::gCurrentExpr = "iNodeNumFrom.IsAssigned()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (iNodeNumTo.mValue == 0 || iNodeNumTo.mValue == 0xFFFF)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1749;
        AeAssert::gCurrentExpr = "iNodeNumTo.IsAssigned()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (eTeam >= TEAM_DEAD)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1750;
        AeAssert::gCurrentExpr =
            "eTeam >= 0 && eTeam < (sizeof(((PathNodes::PathLink*)0)->"
            "mBadPlaceCount) / sizeof(((PathNodes::PathLink*)0)->"
            "mBadPlaceCount[0]))";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", eTeam))
            __debugbreak();
    }
    const PathNodes::PathNode* Node = GetNode(iNodeNumFrom);
    int mTotalLinkCount = Node->mConstant.mTotalLinkCount;
    int v7 = 0;
    if (mTotalLinkCount <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1759;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored())
        {
            const char* v10 = va(
                "Path_IsBadPlaceLink called from %i to %i, but there is no "
                "such link",
                iNodeNumFrom.mValue, iNodeNumTo.mValue);
            if (AeAssert::Warning(v10))
                __debugbreak();
        }
        return false;
    }
    PathNodes::PathLink* mLinks = Node->mConstant.mLinks;
    PathNodes::PathLink* v9 = mLinks;
    while (iNodeNumTo.mValue != v9->mNodeHandle.mValue)
    {
        ++v7;
        ++v9;
        if (v7 >= mTotalLinkCount)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
            AeAssert::gCurrentLine = 1759;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored())
            {
                const char* v11 = va(
                    "Path_IsBadPlaceLink called from %i to %i, but there is "
                    "no such link",
                    iNodeNumFrom.mValue, iNodeNumTo.mValue);
                if (AeAssert::Warning(v11))
                    __debugbreak();
            }
            return false;
        }
    }
    return mLinks[v7].mBadPlaceCount[eTeam] != 0;
}

// ea: 0x0077FFE0
int PathNodeMgr::FindChainIndex(const PathNodes::NodeHandle& handle)
{
    PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
    int mChainNodeCount = mLevelTOC->mChainNodeCount;
    if (mChainNodeCount <= 0)
        return -1;
    int result = 0;
    for (PathNodes::NodeHandle* i = mLevelTOC->mChainNodes;
         handle.mValue != i->mValue;
         ++i)
    {
        if (++result >= mChainNodeCount)
            return -1;
    }
    return result;
}

// ea: 0x00780020
PathNodes::PathNode* PathNodeMgr::ChooseDesperationNewChainNode(
    int iDepthMin, int iDepthMax, int chainIndex,
    PathNodes::PathNode* pRefPos, sentient_s* pClaimer)
{
    (void)iDepthMin;
    (void)iDepthMax;
    (void)chainIndex;
    (void)pRefPos;
    (void)pClaimer;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
    AeAssert::gCurrentLine = 2533;
    AeAssert::gCurrentExpr =
        "0 && \"Commented by michel because we shouldnt use it anymore\"";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
        __debugbreak();
    return nullptr;
}

// ea: 0x00780070
PathNodes::PathNode* PathNodeMgr::FirstNode(int iTypeFlags)
{
    PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
    if (mLevelTOC == nullptr)
        return nullptr;
    int mNodeCount = mLevelTOC->mNodeCount;
    if (mNodeCount == 0)
        return nullptr;
    int v6 = 0;
    if (mNodeCount <= 0)
        return nullptr;
    PathNodes::PathNode* i = mLevelTOC->mNodes;
    while (((1 << (int)i->mConstant.mType) & iTypeFlags) == 0)
    {
        if (++v6 >= mNodeCount)
            return nullptr;
        ++i;
    }
    int v8 = v6;
    if (&this->mLevelTOC->mNodes[v8]
        != GetNode(this->mLevelTOC->mNodes[v8].mHandle))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2698;
        AeAssert::gCurrentExpr =
            "&mLevelTOC->mNodes[j] == GetNode(mLevelTOC->mNodes[j].mHandle)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return &this->mLevelTOC->mNodes[v8];
}

// ea: 0x00780140
PathNodes::PathNode* PathNodeMgr::NextNode(PathNodes::PathNode* pPrevNode,
                                           int iTypeFlags)
{
    if (pPrevNode == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2729;
        AeAssert::gCurrentExpr = "pPrevNode";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pPrevNode->mHandle.mValue == 0
        || pPrevNode->mHandle.mValue == 0xFFFF)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2730;
        AeAssert::gCurrentExpr = "pPrevNode->mHandle.IsAssigned()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pPrevNode
        != PathNodeMgr::sInst->GetNode(pPrevNode->mHandle))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2731;
        AeAssert::gCurrentExpr =
            "pPrevNode == PathNodeMgr::Inst()->GetNode(pPrevNode->mHandle)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int v4 = (pPrevNode->mHandle.mValue - 1);
    if (this->mLevelTOC == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2743;
        AeAssert::gCurrentExpr = "mLevelTOC";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
    int mNodeCount = mLevelTOC->mNodeCount;
    int v7 = v4 + 1;
    if (v7 >= mNodeCount)
        return nullptr;
    PathNodes::PathNode* pPrevNodea = mLevelTOC->mNodes;
    while (((1 << (int)pPrevNodea[v7].mConstant.mType) & iTypeFlags) == 0)
    {
        if (++v7 >= mNodeCount)
            return nullptr;
    }
    return &pPrevNodea[v7];
}

// ea: 0x00780C30
void PathNodeMgr::ValidateNode(PathNodes::PathNode* pNode) const
{
    int v3 = pNode->mConstant.mTotalLinkCount - 1;
    if (v3 >= pNode->mDynamic.mLinkCount)
    {
        do
        {
            PathNodes::PathLink* pLink = &pNode->mConstant.mLinks[v3];
            if (pLink->mDisconnectCount == 0)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\pathnodemgr.cpp";
                AeAssert::gCurrentLine = 3287;
                AeAssert::gCurrentExpr = "pLink->mDisconnectCount > 0";
                if (!AeAssert::IsIgnored())
                {
                    const char* v6 = va(
                        "%d, %d, %d, %d, %d", 0,
                        (pNode->mHandle.mValue - 1), v3, 0,
                        (pLink->mNodeHandle.mValue - 1));
                    if (AeAssert::Assert(v6))
                        __debugbreak();
                }
            }
            --v3;
        } while (v3 >= pNode->mDynamic.mLinkCount);
    }
    if (v3 != pNode->mDynamic.mLinkCount - 1)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 3290;
        AeAssert::gCurrentExpr = "j == pNode->mDynamic.mLinkCount-1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (v3 >= 0)
    {
        do
        {
            PathNodes::PathLink* pLink = &pNode->mConstant.mLinks[v3];
            if (pLink->mDisconnectCount != 0)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\pathnodemgr.cpp";
                AeAssert::gCurrentLine = 3294;
                AeAssert::gCurrentExpr = "!pLink->mDisconnectCount";
                if (!AeAssert::IsIgnored())
                {
                    const char* v9 = va(
                        "%d, %d, %d, %d, %d", 0,
                        (pNode->mHandle.mValue - 1), v3, 0,
                        (pLink->mNodeHandle.mValue - 1));
                    if (AeAssert::Assert(v9))
                        __debugbreak();
                }
            }
            --v3;
        } while (v3 >= 0);
    }
}

// ea: 0x00780DC0
void PathNodeMgr::ValidateAllNodes() const
{
    PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
    if (mLevelTOC != nullptr)
    {
        int mNodeCount = mLevelTOC->mNodeCount;
        if (mNodeCount > 0)
        {
            for (int v4 = 0; v4 < mNodeCount; ++v4)
                ValidateNode(&this->mLevelTOC->mNodes[v4]);
        }
    }
}

// ea: 0x00780E00
void PathNodeMgr::CheckLinkLeaks() const
{
    PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
    if (mLevelTOC != nullptr)
    {
        PathNodes::PathLinkInfo* mLinkPool = mLevelTOC->mLinkPool;
        if (mLinkPool != nullptr)
        {
            int next = mLinkPool->next;
            int v5 = 0;
            if (mLinkPool->next != 0)
            {
                do
                {
                    next = mLinkPool[next].next;
                    ++v5;
                } while (next != 0);
                if (v5 != 3071)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\pathnodemgr.cpp";
                    AeAssert::gCurrentLine = 3341;
                    AeAssert::gCurrentExpr = "count == 3072-1";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
            }
            else
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
                AeAssert::gCurrentLine = 3341;
                AeAssert::gCurrentExpr = "count == 3072-1";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            PathNodes::PathLinkInfo* v6 = this->mLevelTOC->mLinkPool;
            int prev = v6->prev;
            int v8 = 0;
            if (v6->prev != 0)
            {
                do
                {
                    prev = v6[prev].prev;
                    ++v8;
                } while (prev != 0);
                if (v8 != 3071)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\pathnodemgr.cpp";
                    AeAssert::gCurrentLine = 3346;
                    AeAssert::gCurrentExpr = "count == 3072-1";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
            }
            else
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
                AeAssert::gCurrentLine = 3346;
                AeAssert::gCurrentExpr = "count == 3072-1";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
        }
    }
}

// ea: 0x00780EE0
unsigned char* PathNodeMgr::LooseFileSupport(unsigned char* data,
                                             TPakId pakId,
                                             const char* fileSuffix,
                                             int* sizeInOut)
{
    (void)pakId;
    (void)fileSuffix;
    (void)sizeInOut;
    return data;
}

// ea: 0x00782A80
void PathNodeMgr::InitScriptFunctions(int zoneIndex)
{
    (void)zoneIndex;
    PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
    if (mLevelTOC == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1226;
        AeAssert::gCurrentExpr = "zone";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    PathNodes::PathNode* mNodes = mLevelTOC->mNodes;
    int v3 = 0;
    if (mLevelTOC->mNodeCount > 0)
    {
        do
        {
            if (mNodes->mConstant.mAnimScript.mBlock != nullptr)
                Path_SetupAnimFunc(mNodes, nullptr);
            ++v3;
            ++mNodes;
        } while (v3 < mLevelTOC->mNodeCount);
    }
}

// ea: 0x00782B00
void PathNodeMgr::InitScriptFunctions()
{
    if (mLevelTOC != nullptr)
        InitScriptFunctions(0);
}

// ea: 0x00782DE0
void PathNodeMgr::DisconnectPath(PathNodes::PathNode* pNode,
                                 PathNodes::PathLink* pLink)
{
    ValidateNode(pNode);
    uint8_t v3 = pLink->mDisconnectCount + 1;
    pLink->mDisconnectCount = v3;
    if (v3 == 0)
    {
        Scr_Error("too many disconnects on a single path link (overflow on "
                  "disconnect count)");
        return;
    }
    if (v3 <= 1)
    {
        if (--pNode->mDynamic.mLinkCount < 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
            AeAssert::gCurrentLine = 1447;
            AeAssert::gCurrentExpr = "pNode->mDynamic.mLinkCount >= 0";
            if (!AeAssert::IsIgnored())
            {
                const char* v5 = va(
                    "pNode: %d, %d, %d", 0,
                    (pNode->mHandle.mValue - 1),
                    pNode->mDynamic.mLinkCount);
                if (AeAssert::Assert(v5))
                    __debugbreak();
            }
        }
        if (&pNode->mConstant.mLinks[pNode->mDynamic.mLinkCount] < pLink)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
            AeAssert::gCurrentLine = 1448;
            AeAssert::gCurrentExpr =
                "&pNode->mConstant.mLinks[pNode->mDynamic.mLinkCount] >= "
                "pLink";
            if (!AeAssert::IsIgnored())
            {
                const char* v6 = va(
                    "pNode: %d, %d, %d (%d) %d (%d)", 0,
                    (pNode->mHandle.mValue - 1),
                    pNode->mDynamic.mLinkCount,
                    pNode->mConstant.mLinks[pNode->mDynamic.mLinkCount]
                        .mNodeHandle.mValue,
                    (int)(pLink - pNode->mConstant.mLinks),
                    pLink->mNodeHandle.mValue);
                if (AeAssert::Assert(v6))
                    __debugbreak();
            }
        }
        uint16_t v7 = pLink->mNodeHandle.mValue;
        uint32_t v8 = *(const uint32_t*)pLink->mBadPlaceCount;
        float mDist = pLink->mDist;
        *pLink = pNode->mConstant.mLinks[pNode->mDynamic.mLinkCount];
        PathNodes::PathLink* v9 =
            &pNode->mConstant.mLinks[pNode->mDynamic.mLinkCount];
        v9->mNodeHandle.mValue = v7;
        *(uint32_t*)v9->mBadPlaceCount = v8;
        v9->mDist = mDist;
    }
    else if (&pNode->mConstant.mLinks[pNode->mDynamic.mLinkCount] > pLink)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1440;
        AeAssert::gCurrentExpr =
            "&pNode->mConstant.mLinks[pNode->mDynamic.mLinkCount] <= pLink";
        if (!AeAssert::IsIgnored())
        {
            const char* v4 = va(
                "pNode: %d, %d, %d (%d) %d (%d)", 0,
                (pNode->mHandle.mValue - 1),
                pNode->mDynamic.mLinkCount,
                pNode->mConstant.mLinks[pNode->mDynamic.mLinkCount]
                    .mNodeHandle.mValue,
                (int)(pLink - pNode->mConstant.mLinks),
                pLink->mNodeHandle.mValue);
            if (AeAssert::Assert(v4))
            {
                __debugbreak();
                ValidateNode(pNode);
                return;
            }
        }
    }
    ValidateNode(pNode);
}

// ea: 0x00783020
void PathNodeMgr::ConnectPath(PathNodes::PathNode* pNode,
                              PathNodes::PathLink* pLink)
{
    if (pLink->mDisconnectCount == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1463;
        AeAssert::gCurrentExpr = "pLink->mDisconnectCount";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (&pNode->mConstant.mLinks[pNode->mDynamic.mLinkCount] > pLink)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1464;
        AeAssert::gCurrentExpr =
            "&pNode->mConstant.mLinks[pNode->mDynamic.mLinkCount] <= pLink";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    ValidateNode(pNode);
    if (pLink->mDisconnectCount-- == 1)
    {
        uint16_t v5 = pLink->mNodeHandle.mValue;
        uint32_t v6 = *(const uint32_t*)pLink->mBadPlaceCount;
        float mDist = pLink->mDist;
        *pLink = pNode->mConstant.mLinks[pNode->mDynamic.mLinkCount];
        PathNodes::PathLink* v7 =
            &pNode->mConstant.mLinks[pNode->mDynamic.mLinkCount];
        v7->mNodeHandle.mValue = v5;
        *(uint32_t*)v7->mBadPlaceCount = v6;
        v7->mDist = mDist;
        ++pNode->mDynamic.mLinkCount;
        ValidateNode(pNode);
    }
    else
    {
        ValidateNode(pNode);
    }
}

// ea: 0x00783150
void PathNodeMgr::ConnectPath(PathNodes::PathNode* pNode,
                              const PathNodes::NodeHandle& toNodeNum)
{
    int mLinkCount = pNode->mDynamic.mLinkCount;
    int mTotalLinkCount = pNode->mConstant.mTotalLinkCount;
    if (mLinkCount >= mTotalLinkCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1502;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("Path_ConnectPath: should be unreachable"))
            __debugbreak();
    }
    else
    {
        PathNodes::PathLink* v5 = &pNode->mConstant.mLinks[mLinkCount];
        while (toNodeNum.mValue != v5->mNodeHandle.mValue)
        {
            ++mLinkCount;
            ++v5;
            if (mLinkCount >= mTotalLinkCount)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\pathnodemgr.cpp";
                AeAssert::gCurrentLine = 1502;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored()
                    && AeAssert::Warning(
                        "Path_ConnectPath: should be unreachable"))
                    __debugbreak();
                return;
            }
        }
        ConnectPath(pNode, v5);
    }
}

// ea: 0x007831E0
void PathNodeMgr::DisconnectPath(Entity* ent, PathNodes::PathNode* pNode,
                                 PathNodes::PathLink* pLink)
{
    uint16_t next = mLevelTOC->mLinkPool->next;
    if (next == 0)
        Com_Error(ERR_DROP,
                  "Max number of disconnected paths exceeded");
    PathNodes::PathLinkInfo* mLinkPool = mLevelTOC->mLinkPool;
    PathNodes::PathLinkInfo* v7 = &mLinkPool[next];
    mLinkPool->next = v7->next;
    mLevelTOC->mLinkPool[v7->next].prev = 0;
    v7->from.mValue = pNode->mHandle.mValue;
    v7->to.mValue = pLink->mNodeHandle.mValue;
    uint16_t disconnectedLinks = ent->disconnectedLinks;
    if (disconnectedLinks != 0)
    {
        v7->prev = disconnectedLinks;
        uint16_t v9 = mLevelTOC->mLinkPool[ent->disconnectedLinks].next;
        v7->next = v9;
        mLevelTOC->mLinkPool[v9].prev = next;
        mLevelTOC->mLinkPool[v7->prev].next = next;
    }
    else
    {
        ent->disconnectedLinks = next;
        v7->next = next;
        v7->prev = next;
    }
    DisconnectPath(pNode, pLink);
}

// ea: 0x007835F0
bool PathNodeMgr::IsReserveForMe(const PathNodes::PathNode* pNode,
                                 const sentient_s* pClaimer) const
{
    if (pNode == nullptr || pClaimer == nullptr)
        return false;
    Broc::string::Block* mBlock = pNode->mConstant.mReserveName.mBlock;
    if (mBlock != nullptr)
    {
        const char* name = (const char*)(mBlock + 1);
        if (name != nullptr && name[0] != 0
            && Broc::operator==(pClaimer->pEnt->targetname,
                                pNode->mConstant.mReserveName))
            return true;
    }
    return false;
}

// ea: 0x00783640
bool PathNodeMgr::IsReserveForOther(const PathNodes::PathNode* pNode,
                                    const sentient_s* pClaimer) const
{
    if (pNode == nullptr || pClaimer == nullptr)
        return false;
    Broc::string::Block* mBlock = pNode->mConstant.mReserveName.mBlock;
    if (mBlock != nullptr)
    {
        const char* name = (const char*)(mBlock + 1);
        if (name != nullptr && name[0] != 0
            && !Broc::operator==(pClaimer->pEnt->targetname,
                                 pNode->mConstant.mReserveName))
            return true;
    }
    return false;
}

// ea: 0x00783690
PathNodes::PathNode* PathNodeMgr::GetPreviousChainNodeReserveForMe(
    PathNodes::PathNode* pNode, sentient_s* pClaimer) const
{
    if (pNode == nullptr)
        return nullptr;
    PathNodes::PathNode* Node = pNode;
    while (1)
    {
        if (pClaimer != nullptr)
        {
            Broc::string::Block* mBlock = Node->mConstant.mReserveName.mBlock;
            if (mBlock != nullptr)
            {
                const char* name = (const char*)(mBlock + 1);
                if (name != nullptr && name[0] != 0
                    && Broc::operator==(pClaimer->pEnt->targetname,
                                        Node->mConstant.mReserveName))
                    break;
            }
        }
        Node = PathNodeMgr::sInst->GetNode(Node->mConstant.mChainParent);
        if (Node == nullptr)
            return nullptr;
    }
    return Node;
}

// ea: 0x00783700
void PathNodeMgr::UpdateBestChainNode(
    int iDepthMin, int iDepthMax, PathNodes::PathNode* pNode,
    PathNodes::PathNode** ppBestNode, int* piFoundCount,
    sentient_s* pClaimer) const
{
    (void)iDepthMin;
    (void)iDepthMax;
    if (pNode == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2274;
        AeAssert::gCurrentExpr = "pNode";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (*ppBestNode == nullptr)
    {
        *ppBestNode = pNode;
        if (piFoundCount != nullptr)
            *piFoundCount = 1;
        return;
    }
    bool bestPrevReserve_3 = IsReserveForMe(*ppBestNode, pClaimer);
    if (bestPrevReserve_3 != IsReserveForMe(pNode, pClaimer))
    {
        if (!bestPrevReserve_3)
            *ppBestNode = pNode;
        if (piFoundCount != nullptr)
            *piFoundCount = 1;
        return;
    }
    PathNodes::PathNode* bestPrevReserve =
        GetPreviousChainNodeReserveForMe(*ppBestNode, pClaimer);
    PathNodes::PathNode* PreviousChainNodeReserveForMe =
        GetPreviousChainNodeReserveForMe(pNode, pClaimer);
    int mChainDepth =
        (bestPrevReserve != nullptr)
            ? bestPrevReserve->mConstant.mChainDepth
            : 0;
    int v12 =
        (PreviousChainNodeReserveForMe != nullptr)
            ? PreviousChainNodeReserveForMe->mConstant.mChainDepth
            : 0;
    if (mChainDepth != v12)
    {
        if (mChainDepth <= v12)
            *ppBestNode = pNode;
        if (piFoundCount != nullptr)
            *piFoundCount = 1;
        return;
    }
    PathNodes::PathNode* v13 = *ppBestNode;
    int v14 = pNode->mConstant.mChainDepth;
    int v15 = (*ppBestNode)->mConstant.mChainDepth;
    if (v15 != v14)
    {
        if (v15 <= v14)
            v13 = pNode;
        *ppBestNode = v13;
        if (piFoundCount != nullptr)
            *piFoundCount = 1;
        return;
    }
    if (*piFoundCount * rand() <= 0x7FFF)
        *ppBestNode = pNode;
    ++*piFoundCount;
}

// ea: 0x00784E50
bool PathNodeMgr::NodesVisible(PathNodes::PathNode* p1,
                               PathNodes::PathNode* p2) const
{
    if (p1 == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1319;
        AeAssert::gCurrentExpr = "p1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (p2 == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1320;
        AeAssert::gCurrentExpr = "p2";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return p1 == p2 || NodeNumsVisible(p1->mHandle, p2->mHandle);
}

// ea: 0x00785E90 - no-op; 0x00786660
PathNodes::PathNode* PathNodeMgr::ChooseChainNodeInRange(
    int iDepthMin, int iDepthMax, PathNodes::PathNode* pChainPos,
    int refChainNodeIndex, sentient_s* pClaimer)
{
    PathNodes::PathNode* v7 = pChainPos;
    PathNodes::PathNode* result = nullptr;
    if (iDepthMax > pChainPos->mConstant.mChainDepth)
    {
        result = ChooseSubsequentChainNode_r(
            iDepthMin, iDepthMax, refChainNodeIndex, pClaimer);
        if (result != nullptr)
            return result;
        v7 = pChainPos;
    }
    if (iDepthMin <= v7->mConstant.mChainDepth)
        return ChoosePreviousChainNode(iDepthMin, iDepthMax, v7, pClaimer);
    return result;
}

// ea: 0x0077F560
int PathNodeMgr::NodesInCylinder(const float* const origin, float maxDist,
                                 float maxHeight,
                                 PathNodes::PathSort* nodes, int maxNodes,
                                 int typeFlags)
{
    PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
    if (mLevelTOC == nullptr || mLevelTOC->mNodeCount == 0)
        return 0;
    gCircle.origin[0] = origin[0];
    gCircle.origin[1] = origin[1];
    gCircle.origin[2] = origin[2];
    gCircle.maxDist = maxDist;
    gCircle.maxDistSq = maxDist * maxDist;
    gCircle.typeFlags = typeFlags;
    gCircle.maxHeightSq = maxHeight * maxHeight;
    gCircle.nodes = nodes;
    gCircle.maxNodes = maxNodes;
    gCircle.nodeCount = 0;
    PathNodes::TOC1* v8 = this->mLevelTOC;
    if (v8 != nullptr)
    {
        PathNodes::PathNodeTree* mTree = v8->mTree;
        if (mTree != nullptr)
            NodesInCylinder_r(mTree);
    }
    return gCircle.nodeCount;
}

// ea: 0x0077F400
void PathNodeMgr::NodesInCylinder_r(PathNodes::PathNodeTree* tree)
{
    PathNodes::PathNodeTree* v2 = tree;
    int axis = tree->axis;
    if (axis < 0)
        goto LABEL_leaf;
    while (1)
    {
        float v6 = gCircle.origin[axis] - v2->dist;
        if (gCircle.maxDist >= v6)
        {
            NodesInCylinder_r(v2->u.children.left);
        }
        if (v6 < -gCircle.maxDist)
            break;
        v2 = v2->u.children.right;
        axis = v2->axis;
        if (axis < 0)
            goto LABEL_leaf;
    }
    return;
LABEL_leaf:
    {
        PathNodes::PathNode** nodes = v2->u.leaf.nodes;
        int v8 = 0;
        if (v2->u.leaf.count > 0)
        {
            int typeFlags = gCircle.typeFlags;
            while (1)
            {
                PathNodes::PathNode* v10 = nodes[v8];
                float dx =
                    v10->mConstant.mOrigin[0] - gCircle.origin[0];
                float dy =
                    v10->mConstant.mOrigin[1] - gCircle.origin[1];
                float v11 = dy * dy + dx * dx;
                float dz =
                    v10->mConstant.mOrigin[2] - gCircle.origin[2];
                if (gCircle.maxDistSq >= v11
                    && ((1 << (int)v10->mConstant.mType) & typeFlags) != 0
                    && (g_doDontLinkCheck == 0
                        || (v10->mConstant.mSpawnFlags & 1) == 0
                        || (typeFlags & 1) != 0)
                    && (dz * dz) <= gCircle.maxHeightSq)
                {
                    if (gCircle.nodeCount == gCircle.maxNodes)
                        return;
                    gCircle.nodes[gCircle.nodeCount].pNode = v10;
                    gCircle.nodes[gCircle.nodeCount].fMetric = v11;
                    typeFlags = gCircle.typeFlags;
                    ++gCircle.nodeCount;
                }
                if (++v8 >= v2->u.leaf.count)
                    break;
                nodes = v2->u.leaf.nodes;
            }
        }
    }
}
