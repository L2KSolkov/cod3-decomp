// ============================================================================
// g_cm_load.cpp - game.o CM_ BSP leaf helpers (cm_load.cpp)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <string.h>

// ============================================================================
// BSP types (local views; sizes verified against disasm)
// ============================================================================
struct BspPlane {
    __m128 mPlane;   // +0x00 (normal xyz + dist w)
    void ToCPlane(cplane_s& cp)
    {
        cp.normal[0] = mPlane.m128_f32[0];
        cp.normal[1] = mPlane.m128_f32[1];
        cp.normal[2] = mPlane.m128_f32[2];
        cp.dist = mPlane.m128_f32[3];
        cp.type = 3;
        cp.signbits = 0;
        if (cp.normal[0] < 0.0f)
            cp.signbits = 1;
        if (cp.normal[1] < 0.0f)
            cp.signbits |= 2;
        if (cp.normal[2] < 0.0f)
            cp.signbits |= 4;
    }
};

struct BspNode {
    unsigned short contents;   // +0x00 (0xFFFF for nodes)
    uint8_t        _pad2[2];   // +0x02
    union {
        struct {
            BspPlane* plane;      // +0x04 (nodes)
            BspNode*  children[2];// +0x08
        } node;
        struct {
            int cluster;          // +0x04 (leaves)
            int area;             // +0x08
        } leaf;
    } u;
};
static_assert(sizeof(BspNode) == 0x10, "BspNode size mismatch");

struct InplaceVectorBspNode {
    int       mSize;   // +0x00
    BspNode*  mList;   // +0x04
};

struct BspTree {
    uint8_t _pad[8];
    InplaceVectorBspNode mNodes;  // +0x08
    uint8_t _pad20[0x38 - 0x18];
    struct {
        int      mSize;   // +0x38
        void*    mList;   // +0x3C
    } mAreas;             // +0x38 (InplaceVector<BspArea>)
    struct {
        int      mSize;   // +0x40
        int*     mList;   // +0x44
    } mAreaPortals;       // +0x40 (InplaceVector<int>)
    uint8_t _pad48[0x5C - 0x48];
    int      floodvalid;  // +0x5C
};

extern BspTree* g_bspTree;  // ?g_bspTree@@3PAVBspTree@@A (game.o 0xF743DC)

struct BspArea {
    int floodnum;    // +0x00
    int floodvalid;  // +0x04
};

// ============================================================================
// leafList_s - leaf enumeration result
// ============================================================================
struct leafList_s {
    int              count;      // +0x00
    int              maxcount;   // +0x04
    int              overflowed; // +0x08
    int*             list;       // +0x0C
    math::Position3  bounds[2];  // +0x10
    int              lastLeaf;   // +0x30
};

inline BspNode& BspNodeAt(unsigned int index)
{
    if (index >= (unsigned int)g_bspTree->mNodes.mSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
        AeAssert::gCurrentLine = 81;
        AeAssert::gCurrentExpr = "index < mSize";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
            __debugbreak();
    }
    return g_bspTree->mNodes.mList[index];
}

// ea: 0x00618590
int CM_LeafCluster(int leafnum)
{
    if (leafnum >= (unsigned int)g_bspTree->mNodes.mSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_load.cpp";
        AeAssert::gCurrentLine = 313;
        AeAssert::gCurrentExpr = "(unsigned)leafnum < g_bspTree->mNodes.size()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return BspNodeAt(leafnum).u.leaf.cluster;
}

// ea: 0x00618600
int CM_LeafArea(int leafnum)
{
    if (leafnum >= (unsigned int)g_bspTree->mNodes.mSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_load.cpp";
        AeAssert::gCurrentLine = 325;
        AeAssert::gCurrentExpr = "(unsigned)leafnum < g_bspTree->mNodes.size()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return BspNodeAt(leafnum).u.leaf.area;
}

// ea: 0x00618BA0
int CM_PointLeafnum_r(const math::Position3* p, unsigned int nodeIndex)
{
    BspNode* v2 = &g_bspTree->mNodes.mList[0];
    BspNode* v3 = &BspNodeAt(nodeIndex);
    while (v3->contents == 0xFFFF)
    {
        cplane_s plane;
        v3->u.node.plane->ToCPlane(plane);
        float v4;
        if (plane.type >= 3u)
        {
            v4 = ((p->v.m128_f32[2] * plane.normal[2])
                  + (p->v.m128_f32[1] * plane.normal[1]))
                + (p->v.m128_f32[0] * plane.normal[0]);
        }
        else
        {
            v4 = p->v.m128_f32[plane.type];
        }
        if ((v4 - plane.dist) >= 0.0f)
            v3 = v3->u.node.children[0];
        else
            v3 = v3->u.node.children[1];
    }
    return (int)(v3 - v2);
}

// ea: 0x00618C50
int CM_PointLeafnum(const math::Position3* p)
{
    return CM_PointLeafnum_r(p, 0);
}

// ea: 0x00618C70
void CM_StoreLeafs(leafList_s* ll, int nodeIndex)
{
    if (BspNodeAt(nodeIndex).u.leaf.cluster != -1)
        ll->lastLeaf = nodeIndex;
    if (ll->count < ll->maxcount)
        ll->list[ll->count++] = nodeIndex;
    else
        ll->overflowed = 1;
}

// ea: 0x00618CB0
void CM_BoxLeafnums_r(leafList_s* ll, unsigned int nodeIndex)
{
    BspNode* v26 = &g_bspTree->mNodes.mList[0];
LABEL_2:
    while (1)
    {
        unsigned int v5 = nodeIndex;
        if (nodeIndex >= (unsigned int)g_bspTree->mNodes.mSize)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
            AeAssert::gCurrentLine = 81;
            AeAssert::gCurrentExpr = "index < mSize";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
                __debugbreak();
            if (nodeIndex >= (unsigned int)g_bspTree->mNodes.mSize)
                v5 = 0;
        }
        BspNode* v6 = &g_bspTree->mNodes.mList[v5];
        if (v6->contents != 0xFFFF)
            break;
        const float* v7 = v6->u.node.plane->mPlane.m128_f32;  // normal[0..2] + dist at [3]
        float v9 = 0.0f;
        float v11 = v7[2] <= 0.0f ? ll->bounds[1].v.m128_f32[2]
                                  : ll->bounds[0].v.m128_f32[2];
        float v13 = v7[1] <= 0.0f ? ll->bounds[1].v.m128_f32[1]
                                  : ll->bounds[0].v.m128_f32[1];
        float v14 = v7[0] <= 0.0f ? ll->bounds[1].v.m128_f32[0]
                                  : ll->bounds[0].v.m128_f32[0];
        float v22[3] = { v14, v13, v11 };
        float v15 = 0.0f <= v7[2] ? ll->bounds[1].v.m128_f32[2]
                                  : ll->bounds[0].v.m128_f32[2];
        float v16 = 0.0f <= v7[1] ? ll->bounds[1].v.m128_f32[1]
                                  : ll->bounds[0].v.m128_f32[1];
        float v17 = 0.0f <= v7[0] ? ll->bounds[1].v.m128_f32[0]
                                  : ll->bounds[0].v.m128_f32[0];
        float v21[3] = { v17, v16, v15 };
        float v24 = v21[0] * v7[0] + v21[1] * v7[1] + v21[2] * v7[2];
        float v23 = v22[0] * v7[0] + v22[1] * v7[1] + v22[2] * v7[2];
        float v20 = v6->u.node.plane->mPlane.m128_f32[3];
        int v10 = 0;
        if (v24 >= v20)
            v10 = 1;
        if (v20 > v23)
            v10 |= 2;
        if (v10 == 3)
        {
            CM_BoxLeafnums_r(ll,
                             (unsigned int)(v6->u.node.children[0] - v26) >> 4);
            nodeIndex = (unsigned int)(v6->u.node.children[1] - v26) >> 4;
            goto LABEL_2;
        }
        nodeIndex = (unsigned int)(v6->u.node.children[v10 - 1] - v26) >> 4;
    }
    CM_StoreLeafs(ll, (int)nodeIndex);
}

// ============================================================================
// Area flood helpers - ea: 0x619650..0x6199C0
// ============================================================================
inline BspArea& BspAreaAt(unsigned int index)
{
    if (index >= (unsigned int)g_bspTree->mAreas.mSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
        AeAssert::gCurrentLine = 81;
        AeAssert::gCurrentExpr = "index < mSize";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
            __debugbreak();
    }
    return ((BspArea*)g_bspTree->mAreas.mList)[index];
}

inline int& AreaPortalAt(unsigned int index)
{
    if (index >= (unsigned int)g_bspTree->mAreaPortals.mSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
        AeAssert::gCurrentLine = 81;
        AeAssert::gCurrentExpr = "index < mSize";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
            __debugbreak();
    }
    return g_bspTree->mAreaPortals.mList[index];
}

// ea: 0x00619650
void CM_FloodArea_r(unsigned int areaNum, int floodnum)
{
    BspArea* v3 = &BspAreaAt(areaNum);
    if (v3->floodvalid == g_bspTree->floodvalid)
    {
        if (v3->floodnum == floodnum)
            return;
        Com_Error(ERR_DROP, "FloodArea_r: reflooded");
    }
    v3->floodnum = floodnum;
    v3->floodvalid = g_bspTree->floodvalid;
    unsigned int v5 = 0;
    if (g_bspTree->mAreas.mSize != 0)
    {
        unsigned int v6 = areaNum * (unsigned int)g_bspTree->mAreas.mSize;
        do
        {
            if (AreaPortalAt(v6) > 0)
                CM_FloodArea_r(v5, floodnum);
            ++v5;
            ++v6;
        } while (v5 < (unsigned int)g_bspTree->mAreas.mSize);
    }
}

// ea: 0x00619750
unsigned int CM_FloodAreaConnections()
{
    ++g_bspTree->floodvalid;
    int floodnum = 0;
    unsigned int result = (unsigned int)g_bspTree->mAreas.mSize;
    for (unsigned int v2 = 0; v2 < result; ++v2)
    {
        if (BspAreaAt(v2).floodvalid != g_bspTree->floodvalid)
            CM_FloodArea_r(v2, ++floodnum);
        result = (unsigned int)g_bspTree->mAreas.mSize;
    }
    return result;
}

// ea: 0x00619810
void CM_AdjustAreaPortalState(int area1, int area2, int open)
{
    if ((area1 & 0x80000000) == 0 && (area2 & 0x80000000) == 0)
    {
        unsigned int mSize = (unsigned int)g_bspTree->mAreas.mSize;
        if (area1 >= mSize || area2 >= mSize)
            Com_Error(ERR_DROP, "CM_ChangeAreaPortalState: bad area number");
        if (open != 0)
        {
            ++AreaPortalAt(area2 + area1 * (unsigned int)g_bspTree->mAreas.mSize);
            ++AreaPortalAt(area1 + area2 * (unsigned int)g_bspTree->mAreas.mSize);
            CM_FloodAreaConnections();
        }
        else
        {
            if (AreaPortalAt(area1 + area2 * (unsigned int)g_bspTree->mAreas.mSize) != 0)
            {
                --AreaPortalAt(area2 + area1 * (unsigned int)g_bspTree->mAreas.mSize);
                --AreaPortalAt(area1 + area2 * (unsigned int)g_bspTree->mAreas.mSize);
                if (AreaPortalAt(area1 + area2 * (unsigned int)g_bspTree->mAreas.mSize) < 0)
                    Com_Error(ERR_DROP,
                              "CM_AdjustAreaPortalState: negative reference count");
            }
            CM_FloodAreaConnections();
        }
    }
}

// ea: 0x00619910
int CM_AreasConnected(int area1, int area2)
{
    if ((area1 & 0x80000000) != 0 || (area2 & 0x80000000) != 0)
        return false;
    unsigned int mSize = (unsigned int)g_bspTree->mAreas.mSize;
    if (area1 >= mSize || area2 >= mSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_test.cpp";
        AeAssert::gCurrentLine = 442;
        AeAssert::gCurrentExpr =
            "area1 < g_bspTree->mAreas.size() && area2 < g_bspTree->mAreas.size()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return BspAreaAt(area2).floodnum == BspAreaAt(area1).floodnum;
}
