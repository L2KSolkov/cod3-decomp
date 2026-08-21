// ============================================================================
// pathnodemgr.cpp - mp_actors.o PathNodeMgr methods (pathnodemgr.cpp)
// ============================================================================

#include "game/logic/g_local.h"
#include "game/actor_types.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <new>

extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file,
                                 int line);

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
int sPenaltyNotReserve = 3;      // ?sPenaltyNotReserve@@3HA @ 0xE37CC0
float sDistMax = 9216.0f;        // ?sDistMax@@3MA @ 0xE37CD4

void Path_SetupAnimFunc(PathNodes::PathNode* node,
                        PathNodes::ENodeType* type);  // pathnode.cpp

extern float flrand(float min, float max);  // core.o
extern const float VectorDistanceSquared(const float* const p1,
                                         const float* const p2);  // core.o
extern const float VectorDistanceSquared2D(const float* const p1,
                                           const float* const p2);  // core.o
extern void YawVectors(float yaw, float* const forward,
                       float* const right);  // core.o
extern int g_SightTraceToEntity(const math::Position3& start,
                                const math::Position3& mins,
                                const math::Position3& maxs,
                                const math::Position3& end,
                                DbLinkedHandle<EntityHandleDb, Entity> entity,
                                const collision_context_t& context); // g.o
extern void G_Printf(const char* fmt, ...);  // g.o
extern char* vtos(const float* v);           // g.o
extern const math::Position3 actorMaxs;      // 0xF99330
extern const math::Position3 actorMins;      // 0xF99510
extern const char* nodeStringTable[PathNodes::NODE_NUMTYPES]; // 0xE37A20
math::Position3 gDisconnectMins(-15.0f, -15.0f, 18.0f); // 0xF99310
math::Position3 gDisconnectMaxs(15.0f, 15.0f, 72.0f);   // 0xF99500
vmCvar_t g_ignorePathErrors = {};            // ?g_ignorePathErrors@@3UvmCvar_t@@A @ 0xEAC5E8

// ea: 0x0077F1A0 (static helper, pathnodemgr.cpp)
static void Path_UpdateBadPlaceCountForLink(PathNodes::PathLink* pLink,
                                            int teamflags, int delta)
{
    int v3 = 0;
    while (((1 << v3) & teamflags) == 0)
    {
        if (++v3 >= 4)
            return;
    }
    while (1)
    {
        int v4 = delta + pLink->mBadPlaceCount[v3];
        if (v4 < 0)
        {
            Scr_Error("Bad place underflow -- negative count");
            return;
        }
        if (v4 <= 255)
        {
            pLink->mBadPlaceCount[v3] = (uint8_t)v4;
            if (++v3 >= 4)
                return;
            continue;
        }
        Scr_Error("Bad place overflow -- count exceeds 255");
        return;
    }
}

// ============================================================================
// PathNodeMgr
// ============================================================================

// ea: 0x0077BD20
void Actor_SetSubState(actor_s* pSelf, ai_substate_e eSubState)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\actor.h";
        AeAssert::gCurrentLine = 1233;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    pSelf->eSubState = eSubState;
    pSelf->iSubStateTime = level.time + 1;
    pSelf->changeYawTime = level.time + 501;
}

// ea: 0x007820F0
void PathNodeMgr::SetCoverNodeStatus(const Broc::string& name, int inValid)
{
    PathNodes::TOC1* levelTOC = mLevelTOC;
    if (levelTOC == nullptr)
        return;

    for (int i = 0; i < levelTOC->mNodeCount; ++i)
    {
        PathNodes::PathNode* node = &levelTOC->mNodes[i];
        if (node->mConstant.mTargetName.mBlock == nullptr
            || !Broc::operator==(name, node->mConstant.mTargetName))
            continue;

        node->mDynamic.mSafeTime[0] = 0;
        node->mDynamic.mFlags = (char)inValid;
        node->mDynamic.mValidTime[2] = 0;
        sentient_s* owner = node->mDynamic.mOwner;
        node->mDynamic.mValidTime[1] = 0;
        node->mDynamic.mValidTime[0] = 0;
        node->mDynamic.mSafeTime[2] = 0;
        node->mDynamic.mSafeTime[1] = 0;

        if (owner == nullptr || owner->pEnt == nullptr)
            continue;

        actor_s* actor = owner->pEnt->actor;
        if (actor == nullptr)
            continue;

        Actor_SetSubState(actor, (ai_substate_e)0x66);
        if (owner->mClaimedNode)
        {
            PathNodes::PathNode* claimedNode =
                PathNodeMgr::sInst->GetNode(owner->mClaimedNode);
            Path_RelinquishNodePermanently(claimedNode, owner);
            owner->mClaimedNode.mValue = 0;
        }
        PathNodeMgr::sInst->DissociateSentient(owner);
    }
}

// ea: 0x0077F340
PathNodeMgr::PathNodeMgr()
    : AssetBankSet()
{
    mLevelTOC = nullptr;
    mLevelTOC2 = nullptr;
    level.pathsInited = false;
    level.pathsConnected = false;
    level.pathsInvalid = true;
}

// ea: 0x00518620
char PathNodeMgr::IsZoneValid(int index)
{
    (void)index;
    return 1;
}

// ea: 0x00518630
PathNodes::TOC1* PathNodeMgr::GetTOC()
{
    return mLevelTOC;
}

// ea: 0x004DD320
PathNodeMgr* PathNodeMgr::CreateInst()
{
    if (sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PathNodeMgr.h";
        AeAssert::gCurrentLine = 40;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }

    PathNodeMgr* result = static_cast<PathNodeMgr*>(
        mem_heap_malloc_ctx(0x0Cu, 4, "core",
                            "c:\\cod\\code\\game\\PathNodeMgr.h", 40));
    if (result != nullptr)
    {
        result = new (result) PathNodeMgr();
        sInst = result;
    }
    else
    {
        sInst = nullptr;
    }
    return result;
}

// ea: 0x004DD410
void PathNodeMgr::DeleteInst()
{
    if (sInst == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PathNodeMgr.h";
        AeAssert::gCurrentLine = 40;
        AeAssert::gCurrentExpr = "sInst!=0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton not created!"))
            __debugbreak();
    }

    if (sInst != nullptr)
        delete sInst;
    sInst = nullptr;
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

// ea: 0x007802E0
void PathNodeMgr::DissociateSentient(sentient_s* pSentient)
{
    if (pSentient == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2789;
        AeAssert::gCurrentExpr = "pSentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    PathNodes::TOC1* toc = mLevelTOC;
    if (toc != nullptr && toc->mNodeCount > 0)
    {
        int index = 0;
        for (int remaining = toc->mNodeCount; remaining != 0; --remaining)
        {
            PathNodes::PathNode* nodes = mLevelTOC->mNodes;
            sentient_s* owner = nodes[index].mDynamic.mOwner;
            PathNodes::PathNodeDynamic* dynamic = &nodes[index].mDynamic;
            if (owner == pSentient)
                dynamic->mOwner = nullptr;
            ++index;
        }
    }
}

// ea: 0x00783820
void PathNodeMgr::AttachSentientToChainNode(
    sentient_s* pSentient, const Broc::string& targetname)
{
    if (pSentient == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2616;
        AeAssert::gCurrentExpr = "pSentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }

    PathNodes::TOC1* levelTOC = mLevelTOC;
    if (levelTOC == nullptr || levelTOC->mNodeCount == 0)
        return;

    PathNodes::NodeHandle* actualChainPos = &pSentient->mActualChainPos;
    PathNodes::PathNode* node = nullptr;
    if (actualChainPos->mValue != 0 && actualChainPos->mValue != 0xFFFF)
        node = GetNode(*actualChainPos);
    if (node != nullptr
        && Broc::operator==(node->mConstant.mTargetName, targetname))
        return;

    for (int i = 0; i < levelTOC->mChainNodeCount; ++i)
    {
        PathNodes::NodeHandle chainHandle = levelTOC->mChainNodes[i];
        if (levelTOC->mNodeCount <= (chainHandle.mValue - 1))
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\pathnodemgr.cpp";
            AeAssert::gCurrentLine = 2646;
            AeAssert::gCurrentExpr =
                "zone->mNodeCount > mLevelTOC->mChainNodes[i].GetZoneIndex()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }

        PathNodes::PathNode* chainNode =
            &levelTOC->mNodes[chainHandle.mValue - 1];
        if (chainNode == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\pathnodemgr.cpp";
            AeAssert::gCurrentLine = 2649;
            AeAssert::gCurrentExpr = "pNode";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        if (Broc::operator==(chainNode->mConstant.mTargetName, targetname))
        {
            pSentient->mActualChainPos.mValue = chainNode->mHandle.mValue;
            return;
        }
    }

    const char* text = targetname.mBlock != nullptr
        ? (const char*)(targetname.mBlock + 1)
        : defaultFileName;
    G_Error("Friendly chain node '%s' does not exist\n", text);
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

// ea: 0x0077F620
void PathNodeMgr::InitScriptVariables(int zoneIndex)
{
    PathNodes::TOC1* zone = mLevelTOC;
    if (zone == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1109;
        AeAssert::gCurrentExpr = "zone";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }

    for (int i = 0; i < zone->mVariableCount; ++i)
    {
        if (zoneIndex != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\pathnodemgr.cpp";
            AeAssert::gCurrentLine = 1114;
            AeAssert::gCurrentExpr =
                "zone->mVariables[i].mNode.GetZone() == zoneIndex";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }

        PathNodes::NodeVariableSaver* variable = &zone->mVariables[i];
        const unsigned int nodeHandle =
            zone->mNodes[variable->mNode.mValue - 1].mHandle.mValue;
        const char* key = variable->mKey;
        const char* valueText = variable->mValue;
        const unsigned int keyHash = HashString::CalcHash(key);
        const int valueInt = atoi(valueText);
        const float valueFloat = (float)atof(valueText);

        extern cvar_t* cl_noprint;
        if (cl_noprint != nullptr && cl_noprint->integer == 0)
            tlPrintf("node(%d) %s = %s\n", nodeHandle, key, valueText);

        if (gpBrocAPI != nullptr)
        {
            typedef void (*SetPNodeFieldFloat)(unsigned int, unsigned int,
                                               float);
            typedef void (*SetPNodeFieldInt)(unsigned int, unsigned int,
                                             int);
            typedef void (*SetPNodeFieldString)(unsigned int, unsigned int,
                                                Broc::string);
            SetPNodeFieldFloat setFloat =
                *reinterpret_cast<SetPNodeFieldFloat*>(
                    reinterpret_cast<unsigned char*>(gpBrocAPI) + 0xC0C);
            SetPNodeFieldInt setInt =
                *reinterpret_cast<SetPNodeFieldInt*>(
                    reinterpret_cast<unsigned char*>(gpBrocAPI) + 0xC04);
            SetPNodeFieldString setString =
                *reinterpret_cast<SetPNodeFieldString*>(
                    reinterpret_cast<unsigned char*>(gpBrocAPI) + 0xBFC);

            if (_stricmp(key, "script_delay") == 0)
                setFloat(nodeHandle, keyHash, valueFloat);

            if (_stricmp(key, "_color") != 0)
            {
                if (_stricmp(key, "script_mg42") == 0
                    || _stricmp(key, "script_fb_id") == 0)
                {
                    setInt(nodeHandle, keyHash, valueInt);
                }
                else if (_stricmp(key, "script_delay") == 0)
                {
                    setFloat(nodeHandle, keyHash, valueFloat);
                }
                else if (_stricmp(key, "script_waittill") == 0
                         || _stricmp(key, "script_chain") == 0
                         || _stricmp(key, "script_door") == 0
                         || _stricmp(key, "script_ambush_type") == 0)
                {
                    Broc::string value(valueText);
                    setString(nodeHandle, keyHash, value);
                }
                else if (_stricmp(key, "script_ambush_trigger_distance")
                         == 0)
                {
                    setInt(nodeHandle, keyHash, valueInt);
                }
                else if (cl_noprint != nullptr && cl_noprint->integer == 0)
                {
                    tlPrintf("pathnode field KEY=%s, VAL=%s will be "
                             "inaccessable",
                             key, valueText);
                }
            }
        }
    }
}

// ea: 0x0077FE30
PathNodes::PathNode* PathNodeMgr::FindChainPos(
    const float* vOrigin, PathNodes::PathNode* pPrevChainPos)
{
    int v3 = 0;
    if (vOrigin == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1769;
        AeAssert::gCurrentExpr = "vOrigin";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (mLevelTOC == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1770;
        AeAssert::gCurrentExpr = "mLevelTOC";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    PathNodes::PathNode* pBestNode = nullptr;
    int16_t wPrevChainId = pPrevChainPos != nullptr
        ? pPrevChainPos->mConstant.mChainId : 0;
    float fMinDistSqrd = 3.4028235e38f;
    int count = mLevelTOC->mChainNodeCount;
    if (count <= 0)
        return nullptr;
    do
    {
        PathNodes::PathNode* pNode = &mLevelTOC->mNodes[
            mLevelTOC->mChainNodes[v3].mValue - 1];
        if (pNode == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
            AeAssert::gCurrentLine = 1799;
            AeAssert::gCurrentExpr = "pNode";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        if (pNode->mConstant.mChainId == wPrevChainId)
        {
            float dz = pNode->mConstant.mOrigin[2] - vOrigin[2];
            float dy = pNode->mConstant.mOrigin[1] - vOrigin[1];
            float dx = pNode->mConstant.mOrigin[0] - vOrigin[0];
            float distSq = dz * dz + dy * dy + dx * dx;
            if (fMinDistSqrd > distSq)
            {
                fMinDistSqrd = distSq;
                pBestNode = pNode;
            }
        }
        ++v3;
    } while (v3 < count);
    return pBestNode;
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

// ea: 0x00782B10
bool PathNodeMgr::NodeNumsVisible(
    const PathNodes::NodeHandle& iNode1,
    const PathNodes::NodeHandle& iNode2) const
{
    if (iNode1.mValue == 0 || iNode1.mValue == 0xFFFF)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1333;
        AeAssert::gCurrentExpr = "iNode1.IsAssigned()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (iNode2.mValue == 0 || iNode2.mValue == 0xFFFF)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1334;
        AeAssert::gCurrentExpr = "iNode2.IsAssigned()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (iNode2.mValue == iNode1.mValue)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1335;
        AeAssert::gCurrentExpr = "iNode1 != iNode2";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    unsigned char* mVisData = mLevelTOC2->mVisData;
    unsigned int v6 = iNode1.mValue - 1;
    unsigned int v7 = iNode2.mValue - 1;
    int v8;
    if (v6 >= v7)
        v8 = (int)(v6 + mLevelTOC->mNodeCount * v7);
    else
        v8 = (int)(v7 + mLevelTOC->mNodeCount * v6);
    return mVisData != nullptr &&
           ((1 << (v8 & 7)) & mVisData[v8 >> 3]) != 0;
}

// ea: 0x00782C70
bool PathNodeMgr::ExpandedNodeNumsVisible(
    const PathNodes::NodeHandle& iNode1,
    const PathNodes::NodeHandle& iNode2) const
{
    if (iNode1.mValue == 0 || iNode1.mValue == 0xFFFF)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1369;
        AeAssert::gCurrentExpr = "iNode1.IsAssigned()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (iNode2.mValue == 0 || iNode2.mValue == 0xFFFF)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1370;
        AeAssert::gCurrentExpr = "iNode2.IsAssigned()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (iNode2.mValue == iNode1.mValue)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1371;
        AeAssert::gCurrentExpr = "iNode1 != iNode2";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    unsigned char* mVisData = mLevelTOC2->mVisData;
    unsigned int v5 = iNode1.mValue - 1;
    unsigned int v6 = iNode2.mValue - 1;
    int v7;
    if (v5 <= v6)
        v7 = (int)(v5 + mLevelTOC->mNodeCount * (v6 - 1));
    else
        v7 = (int)(v6 + mLevelTOC->mNodeCount * (v5 - 1));
    return mVisData != nullptr &&
           ((1 << (v7 & 7)) & mVisData[v7 >> 3]) != 0;
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

// ea: 0x0077FBA0
void PathNodeMgr::CheckpointResetNodes()
{
    PathNodes::TOC1* levelTOC = mLevelTOC;
    if (levelTOC == nullptr)
        return;

    int nodeCount = levelTOC->mNodeCount;
    for (int i = 0; i < nodeCount; ++i)
    {
        PathNodes::PathNode* node = &levelTOC->mNodes[i];
        node->mDynamic.mFreeTime = 0;
        node->mTransient.mSearchFrame = -2;
    }
}

// ea: 0x00780880
void PathNodeMgr::NodeList()
{
    int counters[PathNodes::NODE_NUMTYPES] = {};
    int total = 0;
    PathNodes::TOC1* levelTOC = mLevelTOC;
    if (levelTOC != nullptr)
    {
        for (int i = 0; i < levelTOC->mNodeCount; ++i)
            ++counters[levelTOC->mNodes[i].mConstant.mType];

        for (int i = 0; i < PathNodes::NODE_NUMTYPES; ++i)
        {
            G_Printf("%s nodes %d\n", nodeStringTable[i], counters[i]);
            total += counters[i];
        }
        G_Printf("total nodes %d of %d\n", total, 2048);
    }
}

// ea: 0x00783A00
void PathNodeMgr::InitPaths()
{
    if (mLevelTOC != nullptr)
    {
        InitScriptVariables(0);
        if (mLevelTOC != nullptr)
            InitScriptFunctions(0);
    }
    level.pathsInited = true;
    level.pathsConnected = true;
    FindOverlappingNodes();

    PathNodes::TOC1* levelTOC = mLevelTOC;
    if (levelTOC != nullptr)
    {
        for (int i = 0; i < levelTOC->mNodeCount; ++i)
            ValidateNode(&levelTOC->mNodes[i]);
    }
}

// ea: 0x007832C0
void PathNodeMgr::ConnectPathsForEntity(Entity* ent)
{
    if ((ent->flags & 0x1000) == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1551;
        AeAssert::gCurrentExpr = "Path_IsDynamicBlockingEntity(ent)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }

    uint16_t disconnectedLinks = ent->disconnectedLinks;
    ent->flags |= 0x4000u;
    if (disconnectedLinks != 0)
    {
        ent->disconnectedLinks = 0;
        PathNodes::PathLinkInfo* linkPool = mLevelTOC->mLinkPool;
        linkPool[linkPool[disconnectedLinks].prev].next = 0;
        linkPool[linkPool->prev].next = disconnectedLinks;
        uint16_t prev = linkPool->prev;
        linkPool->prev = linkPool[disconnectedLinks].prev;
        linkPool[disconnectedLinks].prev = prev;

        do
        {
            uint16_t index = disconnectedLinks;
            PathNodes::PathNode* node = GetNode(linkPool[index].from);
            ConnectPath(node, linkPool[index].to);
            disconnectedLinks = linkPool[index].next;
        } while (disconnectedLinks != 0);
    }
}

// ea: 0x007833C0
void PathNodeMgr::DisconnectPathsForEntity(Entity* ent)
{
    PathNodes::TOC1* levelTOC = mLevelTOC;
    if (levelTOC == nullptr)
        return;

    if ((ent->flags & 0x1000) == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1623;
        AeAssert::gCurrentExpr = "Path_IsDynamicBlockingEntity(ent)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }

    ConnectPathsForEntity(ent);
    math::Position3 origin = (ent->r.absmin + ent->r.absmax) * 0.5f;
    ent->flags &= ~0x4000u;
    ent->iDisconnectTime = level.time;

    PathNodes::PathSort nodes[128];
    int nodeCount = NodesInCylinder(origin.v.m128_f32, 306.0f, 256.0f,
                                    nodes, 128, -1);
    collision_context_t context(0x02820011);

    for (int i = 0; i < nodeCount; ++i)
    {
        PathNodes::PathNode* node = nodes[i].pNode;
        int linkCount = node->mDynamic.mLinkCount;
        for (int linkIndex = linkCount - 1; linkIndex >= 0; --linkIndex)
        {
            PathNodes::PathLink* link =
                &node->mConstant.mLinks[linkIndex];
            PathNodes::PathNode* toNode = GetNode(link->mNodeHandle);
            if (toNode == nullptr)
                continue;

            math::Position3 start(node->mConstant.mOrigin[0],
                                  node->mConstant.mOrigin[1],
                                  node->mConstant.mOrigin[2]);
            math::Position3 end(toNode->mConstant.mOrigin[0],
                                toNode->mConstant.mOrigin[1],
                                toNode->mConstant.mOrigin[2]);
            if (g_SightTraceToEntity(start, gDisconnectMins,
                                     gDisconnectMaxs, end,
                                     ent->mHandle, context) != 0)
                DisconnectPath(ent, node, link);
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

// ea: 0x00785420
PathNodes::PathNode* PathNodeMgr::RunToFirstReserveNode(
    PathNodes::PathNode* parentNode, sentient_s* pClaimer)
{
    if (this->mLevelTOC == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1935;
        AeAssert::gCurrentExpr = "mLevelTOC";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pClaimer == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1936;
        AeAssert::gCurrentExpr = "pClaimer";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (parentNode == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1937;
        AeAssert::gCurrentExpr = "parentNode";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (parentNode == nullptr)
        return nullptr;
    int16_t wChainId = parentNode->mConstant.mChainId;
    int mChainNodeCount = this->mLevelTOC->mChainNodeCount;
    PathNodes::PathNode* pFirstResNode = nullptr;
    for (int v7 = 0; v7 < mChainNodeCount; ++v7)
    {
        PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
        int v9 = (mLevelTOC->mChainNodes[v7].mValue - 1);
        if (v9 >= mLevelTOC->mNodeCount)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
            AeAssert::gCurrentLine = 1957;
            AeAssert::gCurrentExpr =
                "handle.GetZoneIndex() < mLevelTOC->mNodeCount";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        PathNodes::PathNode* v10 = &this->mLevelTOC->mNodes[v9];
        if (v10 == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
            AeAssert::gCurrentLine = 1963;
            AeAssert::gCurrentExpr = "node";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        if (v10->mConstant.mChainId == wChainId
            && (pFirstResNode == nullptr
                || v10->mConstant.mChainDepth
                       <= pFirstResNode->mConstant.mChainDepth)
            && pClaimer != nullptr)
        {
            Broc::string::Block* mBlock =
                v10->mConstant.mReserveName.mBlock;
            if (mBlock != nullptr)
            {
                const char* name = (const char*)(mBlock + 1);
                if (name != nullptr && name[0] != 0
                    && Broc::operator==(pClaimer->pEnt->targetname,
                                        v10->mConstant.mReserveName))
                    pFirstResNode = v10;
            }
        }
    }
    return pFirstResNode;
}

// ea: 0x00785650
PathNodes::PathNode* PathNodeMgr::ChooseAnyChainNodeIfDeadEnd(
    int iDepthMin, int iDepthMax, PathNodes::PathNode* pChainPos,
    sentient_s* pClaimer)
{
    if (iDepthMin > iDepthMax)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2364;
        AeAssert::gCurrentExpr = "iDepthMin <= iDepthMax";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pChainPos == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2365;
        AeAssert::gCurrentExpr = "pChainPos";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pChainPos->mConstant.mChainDepth >= iDepthMax)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2366;
        AeAssert::gCurrentExpr =
            "pChainPos->mConstant.mChainDepth < iDepthMax";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pClaimer == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2367;
        AeAssert::gCurrentExpr = "pClaimer";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    uint16_t mValue = pChainPos->mHandle.mValue;
    int16_t mChainId = pChainPos->mConstant.mChainId;
    PathNodes::PathNode* v9 = pChainPos + 1;
    int16_t wChainId = mChainId;
    int iFoundCount = 0;
    PathNodes::PathNode* best = nullptr;
    if (v9->mConstant.mChainId == mChainId)
    {
        do
        {
            int mChainDepth = v9->mConstant.mChainDepth;
            if (mChainDepth > iDepthMax)
                break;
            if (mValue == v9->mConstant.mChainParent.mValue)
                return nullptr;
            if (mChainDepth >= iDepthMin
                && Path_CanClaimChainNode(v9, pClaimer) != 0)
                UpdateBestChainNode(iDepthMin, iDepthMax, v9, &best,
                                    &iFoundCount, pClaimer);
            ++v9;
        } while (v9->mConstant.mChainId == wChainId);
    }
    return best;
}

// ea: 0x00785810
PathNodes::PathNode* PathNodeMgr::ChoosePreviousChainNode(
    int iDepthMin, int iDepthMax, PathNodes::PathNode* pChainPos,
    sentient_s* pClaimer)
{
    PathNodes::PathNode* pBestNode = nullptr;
    if (iDepthMin > iDepthMax)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2401;
        AeAssert::gCurrentExpr = "iDepthMin <= iDepthMax";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pChainPos == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2402;
        AeAssert::gCurrentExpr = "pChainPos";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        return pBestNode;
    }
    if (pClaimer == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2405;
        AeAssert::gCurrentExpr = "pClaimer";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (IsReserveForMe(pChainPos, pClaimer))
        pBestNode = pChainPos;
    PathNodes::PathNode* Node = pChainPos;
    while (Node->mConstant.mChainDepth > iDepthMax)
    {
        Node = PathNodeMgr::sInst->GetNode(Node->mConstant.mChainParent);
        if (Node == nullptr)
            return pBestNode;
    }
    while (Node->mConstant.mChainDepth >= iDepthMin)
    {
        if (Path_CanClaimChainNode(Node, pClaimer) != 0)
        {
            if (pClaimer != nullptr)
            {
                Broc::string::Block* mBlock =
                    Node->mConstant.mReserveName.mBlock;
                if (mBlock != nullptr)
                {
                    const char* name = (const char*)(mBlock + 1);
                    if (name != nullptr && name[0] != 0
                        && Broc::operator==(pClaimer->pEnt->targetname,
                                            Node->mConstant.mReserveName))
                        return Node;
                }
            }
            if (pBestNode == nullptr)
                pBestNode = Node;
        }
        Node = GetNode(Node->mConstant.mChainParent);
        if (Node == nullptr)
            return pBestNode;
    }
    return pBestNode;
}

// ea: 0x00784F10
PathNodes::PathNode* PathNodeMgr::ChooseSubsequentChainNode_r(
    int iDepthMin, int iDepthMax, int parentIndex, sentient_s* pClaimer)
{
    if (this->mLevelTOC == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1823;
        AeAssert::gCurrentExpr = "mLevelTOC";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (iDepthMin > iDepthMax)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1824;
        AeAssert::gCurrentExpr = "iDepthMin <= iDepthMax";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (parentIndex < 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1825;
        AeAssert::gCurrentExpr = "parentIndex >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (parentIndex >= this->mLevelTOC->mChainNodeCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1826;
        AeAssert::gCurrentExpr =
            "parentIndex < mLevelTOC->mChainNodeCount";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pClaimer == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1827;
        AeAssert::gCurrentExpr = "pClaimer";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    PathNodes::TOC1* zone = this->mLevelTOC;
    if (zone == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1837;
        AeAssert::gCurrentExpr = "zone";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        return nullptr;
    }
    if (zone->mNodeCount
        <= (zone->mChainNodes[parentIndex].mValue - 1))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1844;
        AeAssert::gCurrentExpr =
            "zone->mNodeCount > mLevelTOC->mChainNodes[parentIndex]."
            "GetZoneIndex()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    PathNodes::PathNode* parentNode =
        &zone->mNodes[zone->mChainNodes[parentIndex].mValue - 1];
    if (parentNode == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1846;
        AeAssert::gCurrentExpr = "parentNode";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (parentNode->mConstant.mChainDepth >= iDepthMax)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 1848;
        AeAssert::gCurrentExpr =
            "parentNode->mConstant.mChainDepth < iDepthMax";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int16_t wChainId = parentNode->mConstant.mChainId;
    PathNodes::NodeHandle wChainParent;
    wChainParent.mValue = mLevelTOC->mChainNodes[parentIndex].mValue;
    int iDepthStop = iDepthMax;
    if (iDepthMax >= parentNode->mConstant.mChainDepth + 1)
        iDepthStop = parentNode->mConstant.mChainDepth + 1;
    int iFoundCount = 0;
    PathNodes::PathNode* best = nullptr;
    PathNodes::PathNode* Node = PathNodeMgr::sInst->GetNode(wChainParent);
    if (IsReserveForMe(Node, pClaimer)
        && PathNodeMgr::sInst->GetNode(wChainParent)
               ->mConstant.mChainDepth
               <= iDepthMax)
    {
        PathNodes::PathNode* v13 =
            PathNodeMgr::sInst->GetNode(wChainParent);
        UpdateBestChainNode(iDepthMin, iDepthMax, v13, &best, &iFoundCount,
                            pClaimer);
    }
    int count = mLevelTOC->mChainNodeCount;
    for (int i = parentIndex + 1; i < count; ++i)
    {
        PathNodes::TOC1* v15 = this->mLevelTOC;
        int v16 = (v15->mChainNodes[i].mValue - 1);
        if (v16 >= v15->mNodeCount)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
            AeAssert::gCurrentLine = 1888;
            AeAssert::gCurrentExpr =
                "handle.GetZoneIndex() < mLevelTOC->mNodeCount";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        PathNodes::PathNode* v17 = &this->mLevelTOC->mNodes[v16];
        if (v17 == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
            AeAssert::gCurrentLine = 1894;
            AeAssert::gCurrentExpr = "node";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        if (v17->mConstant.mChainId != wChainId)
            break;
        int mChainDepth = v17->mConstant.mChainDepth;
        if (mChainDepth > iDepthStop)
            break;
        if (wChainParent.mValue == v17->mConstant.mChainParent.mValue)
        {
            if (mChainDepth < iDepthMax)
            {
                PathNodes::PathNode* v19 =
                    ChooseSubsequentChainNode_r(iDepthMin, iDepthMax, i,
                                                pClaimer);
                if (v19 != nullptr)
                    UpdateBestChainNode(iDepthMin, iDepthMax, v19, &best,
                                        &iFoundCount, pClaimer);
                if (IsReserveForMe(v17, pClaimer))
                    UpdateBestChainNode(iDepthMin, iDepthMax, v17, &best,
                                        &iFoundCount, pClaimer);
            }
            int v20 = v17->mConstant.mChainDepth;
            if ((v20 == iDepthMax
                 || (best == nullptr
                     && (v20 >= iDepthMin
                         || IsReserveForMe(v17, pClaimer))))
                && Path_CanClaimChainNode(v17, pClaimer) != 0)
            {
                UpdateBestChainNode(iDepthMin, iDepthMax, v17, &best,
                                    &iFoundCount, pClaimer);
            }
        }
    }
    return best;
}

// ea: 0x007859C0
PathNodes::PathNode* PathNodeMgr::ChooseDesperationChainNode(
    int iDepthMin, int iDepthMax, int chainIndex,
    PathNodes::PathNode* pRefPos, sentient_s* pClaimer)
{
    if (pRefPos == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2453;
        AeAssert::gCurrentExpr = "pRefPos";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pClaimer == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2454;
        AeAssert::gCurrentExpr = "pClaimer";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (iDepthMin > iDepthMax)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2455;
        AeAssert::gCurrentExpr = "iDepthMin <= iDepthMax";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    uint16_t mValue = pRefPos->mConstant.mChainParent.mValue;
    if (mValue != 0 && mValue != 0xFFFF)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2456;
        AeAssert::gCurrentExpr =
            "!pRefPos->mConstant.mChainParent.IsAssigned()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
    PathNodes::PathNode* pBestMatch = nullptr;
    int iMinError = 0x7FFFFFFF;
    int v8 = chainIndex;
    if (chainIndex < mLevelTOC->mChainNodeCount)
    {
        while (1)
        {
            if (mLevelTOC != nullptr)
            {
                if (mLevelTOC->mNodeCount
                    <= (mLevelTOC->mChainNodes[v8].mValue - 1))
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\pathnodemgr.cpp";
                    AeAssert::gCurrentLine = 2478;
                    AeAssert::gCurrentExpr =
                        "zone->mNodeCount > mLevelTOC->mChainNodes[i]."
                        "GetZoneIndex()";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                const PathNodes::PathNode* v9 =
                    &mLevelTOC->mNodes[
                        (this->mLevelTOC->mChainNodes[v8].mValue - 1)];
                if (v9->mConstant.mChainId
                    != pRefPos->mConstant.mChainId)
                    return pBestMatch;
                int mChainDepth = v9->mConstant.mChainDepth;
                int v11;
                if (mChainDepth < iDepthMin)
                {
                    v11 = iDepthMin - mChainDepth;
                }
                else if (mChainDepth > iDepthMax)
                {
                    v11 = mChainDepth - iDepthMax;
                }
                else
                {
                    if (Path_CanClaimChainNode(v9, pClaimer) != 0)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\pathnodemgr.cpp";
                        AeAssert::gCurrentLine = 2500;
                        AeAssert::gCurrentExpr =
                            "!Path_CanClaimChainNode(pNode, pClaimer)";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("old cod assert"))
                            __debugbreak();
                    }
                    goto LABEL_43;
                }
                bool isReserve =
                    pClaimer != nullptr
                    && v9->mConstant.mReserveName.mBlock != nullptr
                    && ((const char*)(v9->mConstant.mReserveName.mBlock
                                      + 1))[0] != 0
                    && Broc::operator==(pClaimer->pEnt->targetname,
                                        v9->mConstant.mReserveName);
                if (!isReserve)
                    v11 += sPenaltyNotReserve;
                if (v11 <= iMinError
                    && Path_CanClaimChainNode(v9, pClaimer) != 0)
                {
                    iMinError = v11;
                    pBestMatch = (PathNodes::PathNode*)v9;
                    if (v9->mConstant.mChainDepth
                        > iDepthMax + sPenaltyNotReserve)
                        return pBestMatch;
                }
            }
        LABEL_43:
            mLevelTOC = this->mLevelTOC;
            if (++v8 >= mLevelTOC->mChainNodeCount)
                return pBestMatch;
        }
    }
    return nullptr;
}

// ea: 0x007866B0
PathNodes::PathNode* PathNodeMgr::ChooseChainPosFromCurrent(
    int iDepthMin, int iDepthMax, PathNodes::PathNode* pRefPos,
    PathNodes::PathNode* pCurChainPos, bool bPrevChainPosOkay,
    sentient_s* pClaimer, int chainFallback)
{
    if (pCurChainPos != nullptr
        && pCurChainPos->mConstant.mChainId
               == pRefPos->mConstant.mChainId)
    {
        if (bPrevChainPosOkay
            && pCurChainPos->mConstant.mChainDepth == iDepthMax
            && IsReserveForMe(pCurChainPos, pClaimer))
            return pCurChainPos;
        if (pCurChainPos->mConstant.mChainDepth < iDepthMax)
        {
            int ChainIndex = FindChainIndex(pCurChainPos->mHandle);
            if (ChainIndex < 0
                || ChainIndex >= this->mLevelTOC->mChainNodeCount)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile =
                    "c:\\cod\\code\\game\\pathnodemgr.cpp";
                AeAssert::gCurrentLine = 2079;
                AeAssert::gCurrentExpr =
                    "previousChainNodeIndex >= 0 && "
                    "previousChainNodeIndex < "
                    "mLevelTOC->mChainNodeCount";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            PathNodes::PathNode* v8 =
                ChooseSubsequentChainNode_r(iDepthMin, iDepthMax,
                                            ChainIndex, pClaimer);
            if (v8 == nullptr)
            {
                v8 = ChooseAnyChainNodeIfDeadEnd(
                    iDepthMin, iDepthMax, pCurChainPos, pClaimer);
                if (v8 == nullptr)
                    goto LABEL_18;
            }
            return v8;
        }
    }
LABEL_18:
    if (bPrevChainPosOkay)
    {
        int mChainDepth = pCurChainPos->mConstant.mChainDepth;
        if (mChainDepth >= iDepthMin && mChainDepth <= iDepthMax)
            return pCurChainPos;
    }
    if (chainFallback == 0)
        return pCurChainPos;
    if (pCurChainPos->mConstant.mChainDepth >= iDepthMin)
        return ChoosePreviousChainNode(iDepthMin, iDepthMax, pCurChainPos,
                                       pClaimer);
    return nullptr;
}

// ea: 0x007867F0
PathNodes::PathNode* PathNodeMgr::ChooseChainPos(
    PathNodes::PathNode* pRefPos, int iFollowMin, int iFollowMax,
    PathNodes::PathNode* pPrevChainPos, sentient_s* pClaimer,
    int chainFallback)
{
    PathNodes::PathNode* Node = pRefPos;
    if (pRefPos == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2129;
        AeAssert::gCurrentExpr = "pRefPos";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (iFollowMin > iFollowMax)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2130;
        AeAssert::gCurrentExpr = "iFollowMin <= iFollowMax";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    PathNodes::PathNode* v8 = pPrevChainPos;
    if (pPrevChainPos != nullptr && pPrevChainPos->mHandle.mValue == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 2131;
        AeAssert::gCurrentExpr =
            "pPrevChainPos == 0 || pPrevChainPos->mHandle.GetValue()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int16_t bPrevChainPosOkay = pRefPos->mConstant.mChainId;
    int mChainDepth = Node->mConstant.mChainDepth;
    int iDepthMin = mChainDepth + iFollowMin;
    int iDepthMax = iFollowMax + mChainDepth;
    bool bPrevChainPosOkaya = false;
    if (pPrevChainPos != nullptr
        && Path_CanClaimChainNode(pPrevChainPos, pClaimer) != 0
        && pPrevChainPos->mConstant.mChainId == bPrevChainPosOkay)
        bPrevChainPosOkaya = true;
    const PathNodes::PathNode* v12 = ChooseChainPosFromCurrent(
        iDepthMin, iDepthMax, Node, pPrevChainPos, bPrevChainPosOkaya,
        pClaimer, chainFallback);
    if (v12 == nullptr)
    {
        int ChainIndex = FindChainIndex(Node->mHandle);
        if (ChainIndex < 0 || ChainIndex >= this->mLevelTOC->mChainNodeCount)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
            AeAssert::gCurrentLine = 2149;
            AeAssert::gCurrentExpr =
                "refChainNodeIndex >= 0 && refChainNodeIndex < "
                "mLevelTOC->mChainNodeCount";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        v12 = ChooseChainNodeInRange(iDepthMin, iDepthMax, Node,
                                     ChainIndex, pClaimer);
        if (v12 == nullptr)
        {
            while (1)
            {
                uint16_t mValue = Node->mConstant.mChainParent.mValue;
                if (mValue == 0 || mValue == 0xFFFF)
                    break;
                Node = PathNodeMgr::sInst->GetNode(
                    Node->mConstant.mChainParent);
                if (Node == nullptr)
                    goto LABEL_46;
            }
            if (Node != nullptr)
            {
                PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
                if (Node->mHandle.mValue
                    != mLevelTOC->mChainNodes[ChainIndex].mValue)
                {
                    int v16 = FindChainIndex(Node->mHandle);
                    ChainIndex = v16;
                    if (v16 < 0 || v16 >= mLevelTOC->mChainNodeCount)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\pathnodemgr.cpp";
                        AeAssert::gCurrentLine = 2171;
                        AeAssert::gCurrentExpr =
                            "refChainNodeIndex >= 0 && "
                            "refChainNodeIndex < "
                            "mLevelTOC->mChainNodeCount";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("old cod assert"))
                            __debugbreak();
                    }
                }
                int v17 = iDepthMax;
                if (Node->mConstant.mChainDepth >= iDepthMax)
                    goto LABEL_39;
                v12 = ChooseSubsequentChainNode_r(iDepthMin, iDepthMax,
                                                  ChainIndex, pClaimer);
                if (v12 == nullptr)
                {
                    v17 = iDepthMax;
                LABEL_39:
                    if (bPrevChainPosOkaya)
                        return pPrevChainPos;
                    if (Node->mConstant.mChainParent.mValue != 0
                        && Node->mConstant.mChainParent.mValue != 0xFFFF)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\pathnodemgr.cpp";
                        AeAssert::gCurrentLine = 2198;
                        AeAssert::gCurrentExpr =
                            "!pRefPos->mConstant.mChainParent.IsAssigned()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("old cod assert"))
                            __debugbreak();
                    }
                    v12 = ChooseDesperationChainNode(
                        iDepthMin, v17, ChainIndex, Node, pClaimer);
                }
            }
        }
    }
LABEL_46:
    v8 = pPrevChainPos;
    if (v12 != v8 && bPrevChainPosOkaya)
    {
        int v19 = v8->mConstant.mChainDepth;
        if (v19 >= iDepthMin && v19 <= iDepthMax)
        {
            bool IsReserveForMe_ = IsReserveForMe(v8, pClaimer);
            if (IsReserveForMe_ == IsReserveForMe(
                                       (PathNodes::PathNode*)v12, pClaimer))
            {
                float v21 = VectorDistanceSquared(
                    v8->mConstant.mOrigin,
                    pClaimer->pEnt->r.currentOrigin.v.m128_f32);
                if (sDistMax >= v21)
                {
                    float r = flrand(0.0f, 1.0f);
                    if (pClaimer->fKeepOldDesiredChainOdds > r)
                        return v8;
                }
            }
            else if (IsReserveForMe_)
            {
                return v8;
            }
        }
    }
    return (PathNodes::PathNode*)v12;
}

// ea: 0x007824E0
int PathNodeMgr::GetNode(const Broc::string& name, const Broc::string& key,
                         int* array)
{
    PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
    if (mLevelTOC == nullptr)
        return 0;
    int mNodeCount = mLevelTOC->mNodeCount;
    if (mNodeCount == 0)
        return 0;
    int v5 = 0;
    int count = 0;
    if (array != nullptr)
    {
        for (int v9 = 0; v9 < mLevelTOC->mNodeCount; ++v9)
            array[v5++] = mLevelTOC->mNodes[v9].mHandle.mValue;
        return v5;
    }
    const char* keyName =
        key.mBlock != nullptr ? (const char*)(key.mBlock + 1)
                              : defaultFileName;
    int v12;
    if (Q_stricmp("targetname", keyName) == 0)
        v12 = 48;
    else if (Q_stricmp("target", keyName) == 0)
        v12 = 56;
    else if (Q_stricmp("on_goal", keyName) == 0)
        v12 = 60;
    else if (Q_stricmp("reservename", keyName) == 0)
        v12 = 64;
    else if (Q_stricmp("animscript", keyName) == 0)
        v12 = 68;
    else if (Q_stricmp("script_noteworthy", keyName) == 0)
        v12 = 52;
    else if (Q_stricmp("origin", keyName) == 0)
        v12 = 76;
    else if (Q_stricmp("angles", keyName) == 0)
        v12 = 88;
    else if (Q_stricmp("radius", keyName) == 0)
        v12 = 92;
    else if (Q_stricmp("spawnflags", keyName) == 0)
        v12 = 44;
    else if (Q_stricmp("type", keyName) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 851;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored())
        {
            if (AeAssert::Warning("Unsupported GetNode field %s", keyName))
                __debugbreak();
        }
        return -1;
    }
    else
        v12 = 40;
    PathNodes::TOC1* v23 = this->mLevelTOC;
    PathNodes::PathNode* mNodes = v23->mNodes;
    while (mNodes < &v23->mNodes[v23->mNodeCount])
    {
        char* pField = (char*)mNodes + v12;
        if (*(int*)pField != 0
            && Broc::operator==(name, *(Broc::string*)pField))
        {
            if (array == nullptr)
                return mNodes->mHandle.mValue;
            array[count++] = mNodes->mHandle.mValue;
        }
        ++mNodes;
    }
    if (array != nullptr)
        return count;
    return -1;
}

// ea: 0x00782810
int PathNodeMgr::GetVehicleNodeIndex(const Broc::string& name,
                                     const Broc::string& key, int* array,
                                     int forceAllNodes)
{
    int v5 = 0;
    if (this->mLevelTOC == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 924;
        AeAssert::gCurrentExpr = "mLevelTOC";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (array != nullptr)
    {
        for (int j = 0; j < s_numNodes; ++j)
            array[v5++] = j;
        return v5;
    }
    if (forceAllNodes == 0 && name.mBlock != nullptr
        && key.mBlock != nullptr)
    {
        const char* keyName = (const char*)(key.mBlock + 1);
        int v10;
        if (Q_stricmp("targetname", keyName) == 0)
            v10 = 0;
        else if (Q_stricmp("target", keyName) == 0)
            v10 = 4;
        else if (Q_stricmp("origin", keyName) == 0)
            v10 = 20;
        else if (Q_stricmp("angles", keyName) == 0)
            v10 = 44;
        else if (Q_stricmp("speed", keyName) == 0)
            v10 = 8;
        else if (Q_stricmp("lookahead", keyName) == 0)
            v10 = 12;
        else if (Q_stricmp("script_noteworthy", keyName) != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
            AeAssert::gCurrentLine = 950;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored())
            {
                if (AeAssert::Warning(
                        "Unsupported GetVehicleNode field %s", keyName))
                    __debugbreak();
            }
            return -1;
        }
        else
            v10 = 16;
        for (int i = 0; i < s_numNodes; ++i)
        {
            vehicle_node_t* v18 = s_nodes[i];
            Broc::string* pStr =
                (Broc::string*)((char*)&v18->mName + v10);
            if (pStr->mBlock != 0
                && ((const char*)(pStr->mBlock + 1))[0] != 0
                && Broc::operator==(*pStr, name))
            {
                if (array == nullptr)
                    return i;
                array[v5++] = i;
            }
        }
        if (array != nullptr)
            return v5;
        return -1;
    }
    // key null or forceAllNodes: field lookup with defaultFileName key
    {
        const char* keyName = defaultFileName;
        int v10;
        if (Q_stricmp("targetname", keyName) == 0)
            v10 = 0;
        else if (Q_stricmp("target", keyName) == 0)
            v10 = 4;
        else if (Q_stricmp("origin", keyName) == 0)
            v10 = 20;
        else if (Q_stricmp("angles", keyName) == 0)
            v10 = 44;
        else if (Q_stricmp("speed", keyName) == 0)
            v10 = 8;
        else if (Q_stricmp("lookahead", keyName) == 0)
            v10 = 12;
        else if (Q_stricmp("script_noteworthy", keyName) != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
            AeAssert::gCurrentLine = 950;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored())
            {
                if (AeAssert::Warning(
                        "Unsupported GetVehicleNode field %s", keyName))
                    __debugbreak();
            }
            return -1;
        }
        else
            v10 = 16;
        for (int i = 0; i < s_numNodes; ++i)
        {
            vehicle_node_t* v18 = s_nodes[i];
            Broc::string* pStr =
                (Broc::string*)((char*)&v18->mName + v10);
            if (pStr->mBlock != 0
                && ((const char*)(pStr->mBlock + 1))[0] != 0
                && Broc::operator==(*pStr, name))
            {
                if (array == nullptr)
                    return i;
                array[v5++] = i;
            }
        }
        if (array != nullptr)
            return v5;
        return -1;
    }
}

// ea: 0x00780370
void PathNodeMgr::UpdateArcBadPlaceCount(BadPlaceArc* arc, int teamflags,
                                         int delta)
{
    float angle0 = arc->angle0;
    float side0[3];
    float side1[3];
    YawVectors(angle0, nullptr, side0);
    YawVectors(arc->angle1, nullptr, side1);
    side0[1] = -side0[1];
    side0[0] = -side0[0];
    side0[2] = (arc->origin[0] * -side0[0])
               + (side0[1] * arc->origin[1]);
    side1[2] = (arc->origin[0] * side1[0])
               + (side1[1] * arc->origin[1]);
    float angle = arc->angle1 - arc->angle0;
    if (angle < 0.0f)
        angle = angle + 360.0f;
    bool bArcLessThan180 = angle < 180.0f;
    float forward[3];
    YawVectors((angle * 0.5f) + arc->angle0, forward, nullptr);
    PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
    float fRadiusSqrd = arc->radius * arc->radius;
    float fHeightSqrd = arc->halfheight * arc->halfheight;
    float fMaxRadiusSqrd =
        (arc->radius + 256.0f) * (arc->radius + 256.0f);
    float fMaxHeightSqrd =
        (arc->halfheight + 128.0f) * (arc->halfheight + 128.0f);
    float centroid[3];
    float v13 = sin(angle * 0.0087266462f) / angle * 76.394371f
                * arc->radius;
    centroid[0] = forward[0] * v13 + arc->origin[0];
    centroid[1] = forward[1] * v13 + arc->origin[1];
    centroid[2] = forward[2] * v13 + arc->origin[2];
    int mNodeCount = mLevelTOC->mNodeCount;
    if (mNodeCount > 0)
    {
        for (int i = 0; i < mNodeCount; ++i)
        {
            PathNodes::PathNode* v18 = &mLevelTOC->mNodes[i];
            float vPosDelta[3];
            vPosDelta[0] = v18->mConstant.mOrigin[0] - arc->origin[0];
            vPosDelta[1] = v18->mConstant.mOrigin[1] - arc->origin[1];
            vPosDelta[2] = v18->mConstant.mOrigin[2] - arc->origin[2];
            float fPosDeltaSqrd =
                (vPosDelta[1] * vPosDelta[1])
                + (vPosDelta[0] * vPosDelta[0]);
            if (fPosDeltaSqrd < fMaxRadiusSqrd
                && (vPosDelta[2] * vPosDelta[2]) < fMaxHeightSqrd)
            {
                float fCentroidDeltaSqrd = VectorDistanceSquared2D(
                    v18->mConstant.mOrigin, centroid);
                int j = 0;
                while (j < v18->mConstant.mTotalLinkCount)
                {
                    PathNodes::PathLink* pLink =
                        &v18->mConstant.mLinks[j];
                    PathNodes::PathNode* Node = GetNode(pLink->mNodeHandle);
                    float v22 = Node->mConstant.mOrigin[1]
                                - arc->origin[1];
                    float vOtherDelta_8 =
                        Node->mConstant.mOrigin[2] - arc->origin[2];
                    float fOtherDeltaSqrd =
                        (v22 * v22)
                        + ((Node->mConstant.mOrigin[0] - arc->origin[0])
                           * (Node->mConstant.mOrigin[0]
                              - arc->origin[0]));
                    float v23 = VectorDistanceSquared2D(
                        Node->mConstant.mOrigin, centroid);
                    if (v23 <= fCentroidDeltaSqrd)
                    {
                        bool bInside = true;
                        if (vPosDelta[2] < arc->halfheight)
                        {
                            float v24 = -arc->halfheight;
                            if (v24 >= vPosDelta[2] && v24 >= vOtherDelta_8)
                                bInside = false;
                        }
                        else if (vOtherDelta_8 >= arc->halfheight)
                        {
                            bInside = false;
                        }
                        if (bInside
                            && fPosDeltaSqrd > fRadiusSqrd
                            && fOtherDeltaSqrd > fRadiusSqrd)
                        {
                            float v25 =
                                Node->mConstant.mOrigin[0]
                                - v18->mConstant.mOrigin[0];
                            float v26 =
                                Node->mConstant.mOrigin[1]
                                - v18->mConstant.mOrigin[1];
                            float v27 =
                                Node->mConstant.mOrigin[2]
                                - v18->mConstant.mOrigin[2];
                            float v28 =
                                -((v26 * vPosDelta[1])
                                  + (v25 * vPosDelta[0]));
                            if (v28 > 0.0f)
                            {
                                float v29 =
                                    (v26 * v26) + (v25 * v25);
                                float fDiscriminant =
                                    (v28 * v28)
                                    - ((fPosDeltaSqrd - fRadiusSqrd)
                                       * v29);
                                if (fDiscriminant > 0.0f)
                                {
                                    float v30 = 1.0f / v29;
                                    float fSqrtDisc =
                                        sqrtf(fDiscriminant);
                                    float v31 =
                                        (v28 - fSqrtDisc) * v30;
                                    if (v31 < 1.0f)
                                    {
                                        float fHeight0 =
                                            (v31 * v27) + vPosDelta[2];
                                        if ((fHeight0 * fHeight0)
                                            > fHeightSqrd)
                                        {
                                            float v32 =
                                                (fSqrtDisc + v28) * v30;
                                            if (v32 >= 1.0f)
                                                bInside = false;
                                            else
                                            {
                                                float v33 =
                                                    (v32 * v27)
                                                    + vPosDelta[2];
                                                if ((v33 * v33)
                                                        > fHeightSqrd
                                                    && (v33 * fHeight0)
                                                           >= 0.0f)
                                                    bInside = false;
                                            }
                                        }
                                    }
                                    else
                                        bInside = false;
                                }
                                else
                                    bInside = false;
                            }
                            else
                                bInside = false;
                        }
                        if (bInside
                            && (arc->angle0 != 0.0f
                                || arc->angle1 != 360.0f))
                        {
                            float v34 =
                                ((side0[1]
                                  * v18->mConstant.mOrigin[1])
                                 + (side0[0]
                                    * v18->mConstant.mOrigin[0]))
                                - side0[2];
                            float v35 =
                                ((Node->mConstant.mOrigin[1] * side0[1])
                                 + (Node->mConstant.mOrigin[0] * side0[0]))
                                - side0[2];
                            if (bArcLessThan180)
                            {
                                if (v34 < 0.0f && v35 < 0.0f)
                                    bInside = false;
                            }
                            else if (v34 < 0.0f && v35 < 0.0f)
                            {
                                if (((side1[1]
                                      * v18->mConstant.mOrigin[1])
                                     + (side1[0]
                                        * v18->mConstant.mOrigin[0]))
                                        - side1[2]
                                        < 0.0f
                                    && ((Node->mConstant.mOrigin[1]
                                         * side1[1])
                                        + (Node->mConstant.mOrigin[0]
                                           * side1[0]))
                                           - side1[2]
                                           < 0.0f)
                                    bInside = false;
                            }
                        }
                        if (bInside)
                            Path_UpdateBadPlaceCountForLink(pLink,
                                                            teamflags,
                                                            delta);
                    }
                    ++j;
                }
            }
        }
    }
}

// ea: 0x00780940
void PathNodeMgr::FindOverlappingNodes()
{
    PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
    if (mLevelTOC == nullptr)
        return;
    if (mLevelTOC->mNodeCount > 0)
    {
        for (int i = 0; i < mLevelTOC->mNodeCount; ++i)
        {
            this->mLevelTOC->mNodes[i].mConstant.mOverlapNode[0].mValue = 0;
            this->mLevelTOC->mNodes[i].mConstant.mOverlapNode[1].mValue = 0;
        }
    }
    PathNodes::TOC1* v4 = this->mLevelTOC;
    int v5 = v4->mNodeCount;
    float v7 = (actorMaxs.v.m128_f32[1] - actorMins.v.m128_f32[1]) + 1.0f;
    float v8 = (actorMaxs.v.m128_f32[2] - actorMins.v.m128_f32[2]) + 1.0f;
    float actorSize = (actorMaxs.v.m128_f32[0] - actorMins.v.m128_f32[0])
                      + 1.0f;
    int iErrorCount = 0;
    for (int i = 0; i < v5; ++i)
    {
        PathNodes::PathNode* cur = &v4->mNodes[i];
        for (int j = 0; j < i; ++j)
        {
            PathNodes::PathNode* v15 = &v4->mNodes[j];
            float v16 = cur->mConstant.mOrigin[0]
                        - v15->mConstant.mOrigin[0];
            float v13 = cur->mConstant.mOrigin[1]
                        - v15->mConstant.mOrigin[1];
            float v14 = cur->mConstant.mOrigin[2]
                        - v15->mConstant.mOrigin[2];
            if (-actorSize <= v16 && v16 <= actorSize
                && -v7 <= v13 && v13 <= v7
                && -v8 <= v14 && v14 <= v8)
            {
                if (((v13 * v13) + (v16 * v16)) < 1.0f
                    && ((cur->mConstant.mSpawnFlags
                         | v15->mConstant.mSpawnFlags)
                        & 1) == 0)
                {
                    G_Printf(
                        "ERROR:  Duplicate linking nodes at %f %f %f.  "
                        "Removed dupe node or set DONTLINK.\n",
                        cur->mConstant.mOrigin[0],
                        cur->mConstant.mOrigin[1],
                        cur->mConstant.mOrigin[2]);
                    ++iErrorCount;
                }
                uint16_t v17 = cur->mConstant.mOverlapNode[0].mValue;
                if (v17 != 0 && v17 != 0xFFFF)
                {
                    uint16_t v18 =
                        cur->mConstant.mOverlapNode[1].mValue;
                    if (v18 != 0 && v18 != 0xFFFF)
                    {
                        G_Printf(
                            "WARNING: node at %s overlaps more than 2 other "
                            "nodes\n",
                            vtos(cur->mConstant.mOrigin));
                        ++iErrorCount;
                    }
                    else
                    {
                        cur->mConstant.mOverlapNode[1].mValue =
                            v15->mHandle.mValue;
                    }
                }
                else
                {
                    cur->mConstant.mOverlapNode[0].mValue =
                        v15->mHandle.mValue;
                }
                uint16_t mValue =
                    v15->mConstant.mOverlapNode[0].mValue;
                if (mValue != 0 && mValue != 0xFFFF)
                {
                    uint16_t v21 =
                        v15->mConstant.mOverlapNode[1].mValue;
                    if (v21 != 0 && v21 != 0xFFFF)
                    {
                        G_Printf(
                            "WARNING: node at %s overlaps more than 2 other "
                            "nodes\n",
                            vtos(v15->mConstant.mOrigin));
                        ++iErrorCount;
                    }
                    else
                    {
                        v15->mConstant.mOverlapNode[1].mValue =
                            cur->mHandle.mValue;
                    }
                }
                else
                {
                    v15->mConstant.mOverlapNode[0].mValue =
                        cur->mHandle.mValue;
                }
            }
        }
    }
    if (iErrorCount != 0)
    {
        if (g_ignorePathErrors.integer != 0)
        {
            level.pathsInvalid = true;
        }
        else
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
            AeAssert::gCurrentLine = 3257;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning(
                    "%d duplicate/overlap node errors.  Check log for list "
                    "to fix.\n",
                    iErrorCount))
                __debugbreak();
        }
    }
}

// ea: 0x00784AB0
bool PathNodeMgr::SetupLevelPaths()
{
    InitLinkInfoArray();
    level.pathsInited = false;
    level.pathsInvalid = true;
    level.pathsConnected = false;
    PathNodes::TOC1* mLevelTOC = this->mLevelTOC;
    if (mLevelTOC == nullptr)
        return false;
    PathNodes::TOC2* mLevelTOC2 = this->mLevelTOC2;
    if (mLevelTOC2 == nullptr)
        return false;
    if (mLevelTOC->mVersion != 15)
    {
        G_Printf(
            "^1WARNING: Path data is an old version #%i. Should be #%i. It "
            "needs to be rebuilt.\n",
            mLevelTOC->mVersion, 15);
        return false;
    }
    if (mLevelTOC2->mVersion != 15)
    {
        G_Printf(
            "^1WARNING: Path data is an old version #%i. Should be #%i. It "
            "needs to be rebuilt.\n",
            15, 15);
        return false;
    }
    if (mLevelTOC->mNodeCount >= 2048)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)9;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 470;
        AeAssert::gCurrentExpr = "mLevelTOC->mNodeCount < 2048";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("To many path nodes. More then %i", 2048))
            __debugbreak();
    }
    for (int i = 0; i < this->mLevelTOC->mNodeCount; ++i)
    {
        PathNodes::TOC1* v6 = this->mLevelTOC;
        PathNodes::PathNode* mNodes = v6->mNodes;
        PathNodes::PathLink* mLinks = mNodes[i].mConstant.mLinks;
        if ((intptr_t)mLinks + mNodes[i].mConstant.mTotalLinkCount - 1
            >= v6->mLinkCount)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\pathnodemgr.cpp";
            AeAssert::gCurrentLine = 480;
            AeAssert::gCurrentExpr =
                "index + (mLevelTOC->mNodes[i].mConstant.mTotalLinkCount - "
                "1) < mLevelTOC->mLinkCount";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        this->mLevelTOC->mNodes[i].mConstant.mLinks =
            &this->mLevelTOC->mLinks[(intptr_t)mLinks];
    }
    if (this->mLevelTOC->mTreeNodes == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\pathnodemgr.cpp";
        AeAssert::gCurrentLine = 485;
        AeAssert::gCurrentExpr = "mLevelTOC->mTreeNodes";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    for (int j = 0; j < this->mLevelTOC->mNodeCount; ++j)
    {
        PathNodes::TOC1* v10 = this->mLevelTOC;
        PathNodes::PathNode** mTreeNodes = v10->mTreeNodes;
        mTreeNodes[j] = &v10->mNodes[(intptr_t)mTreeNodes[j]];
    }
    PathNodes::TOC1* v14 = this->mLevelTOC;
    if (v14->mNodeCount != 0)
    {
        this->mLevelTOC->mTree =
            CreateTree_r(v14->mTreeNodes, &this->mLevelTOC->mTree);
    }
    else
    {
        this->mLevelTOC->mTree = nullptr;
        this->mLevelTOC->mTreeNodes = nullptr;
    }
    PathNodes::TOC1* v15 = this->mLevelTOC;
    for (int ia = 0; ia < v15->mNodeCount; ++ia)
    {
        PathNodes::PathNode* node = &v15->mNodes[ia];
        if (node->mConstant.mTargetName.mBlock != nullptr)
            node->mConstant.mTargetName = Broc::string(
                (const char*)((char*)node->mConstant.mTargetName.mBlock - 1
                              + (intptr_t)v15->mStrings));
        if (node->mConstant.mScriptNoteWorthy.mBlock != nullptr)
            node->mConstant.mScriptNoteWorthy = Broc::string(
                (const char*)((char*)node->mConstant.mScriptNoteWorthy.mBlock
                                  - 1
                              + (intptr_t)this->mLevelTOC->mStrings));
        if (node->mConstant.mTarget.mBlock != nullptr)
            node->mConstant.mTarget = Broc::string(
                (const char*)((char*)node->mConstant.mTarget.mBlock - 1
                              + (intptr_t)this->mLevelTOC->mStrings));
        if (node->mConstant.mAnimScript.mBlock != nullptr)
            node->mConstant.mAnimScript = Broc::string(
                (const char*)((char*)node->mConstant.mAnimScript.mBlock - 1
                              + (intptr_t)this->mLevelTOC->mStrings));
        if (node->mConstant.mOnGoalCallback.mBlock != nullptr)
            node->mConstant.mOnGoalCallback = Broc::string(
                (const char*)((char*)node->mConstant.mOnGoalCallback.mBlock
                                  - 1
                              + (intptr_t)this->mLevelTOC->mStrings));
        if (node->mConstant.mReserveName.mBlock != nullptr)
            node->mConstant.mReserveName = Broc::string(
                (const char*)((char*)node->mConstant.mReserveName.mBlock - 1
                              + (intptr_t)this->mLevelTOC->mStrings));
    }
    if (level.pathsInited && this->mLevelTOC != nullptr)
    {
        InitScriptVariables(0);
        if (this->mLevelTOC != nullptr)
            InitScriptFunctions(0);
    }
    InitLinkCounts(0);
    level.pathsInvalid = false;
    return true;
}
