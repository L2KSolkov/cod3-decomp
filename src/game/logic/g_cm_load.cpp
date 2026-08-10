// ============================================================================
// g_cm_load.cpp - game.o CM_ BSP leaf helpers (cm_load.cpp)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <string.h>

extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* desc);  // tl_xboxr

// phys_memory_heap - linear frame allocator (16 bytes; verified vs IDA)
struct phys_memory_heap {
    char* m_buffer_start;  // +0x00
    char* m_buffer_end;    // +0x04
    char* m_buffer_cur;    // +0x08
    char* m_user_start;    // +0x0C
};

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
    struct {
        int        mSize;   // +0x10
        BspPlane*  mList;   // +0x14
    } mPlanes;              // +0x10 (InplaceVector<BspPlane>)
    uint8_t _pad18[0x38 - 0x18];
    struct {
        int      mSize;   // +0x38
        void*    mList;   // +0x3C
    } mAreas;             // +0x38 (InplaceVector<BspArea>)
    struct {
        int      mSize;   // +0x40
        int*     mList;   // +0x44
    } mAreaPortals;       // +0x40 (InplaceVector<int>)
    struct {
        int        mSize;   // +0x48
        void*      mList;   // +0x4C
    } mVisibility;          // +0x48
    int      mVised;        // +0x50
    int      mClusterBytes; // +0x54
    uint8_t _pad58[0x5C - 0x58];
    int      floodvalid;    // +0x5C
    uint8_t _pad60[0x64 - 0x60];
    float    mins[2];       // +0x64
    uint8_t _pad6C[0x70 - 0x6C];
    float    maxs[2];       // +0x70
    int      checkcount;    // +0x78
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

// ============================================================================
// Plane / alloc / temp box helpers
// ============================================================================
extern phys_memory_heap g_cmgr_allocater;  // ?g_cmgr_allocater@@3Vphys_memory_heap@@A
extern DCGSet* gBoxDCGSet;                 // ?gBoxDCGSet@@3PAVDCGSet@@A

// ea: 0x0061A830
BspPlane* CM_GetPlaneNum(int pi)
{
    if (pi < 0 || pi >= g_bspTree->mPlanes.mSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 1572;
        AeAssert::gCurrentExpr = "pi >= 0 && pi < g_bspTree->mPlanes.size()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return &((BspPlane*)g_bspTree->mPlanes.mList)[pi];
}

// ea: 0x0061A8A0
math::Position3* alloc_verts()
{
    math::Position3* result =
        (math::Position3*)(((intptr_t)g_cmgr_allocater.m_buffer_cur + 15)
                           & 0xFFFFFFF0);
    math::Position3* v1;
    if (&result[32] > (math::Position3*)g_cmgr_allocater.m_buffer_end)
    {
        v1 = nullptr;
    }
    else
    {
        g_cmgr_allocater.m_buffer_cur = (char*)&result[32];
        v1 = result;
        if (result != nullptr)
            return result;
    }
    bool v2 = !_tlAssert("c:/cod/code/tl/physics/include\\phys_mem.h", 89,
                         "addr", "phys_memory_heap overflow.");
    result = v1;
    if (!v2)
        __debugbreak();
    return result;
}

// ea: 0x006188A0
int TempBoxModelContents()
{
    if (gBoxDCGSet == nullptr || gBoxDCGSet->objects_m_count != 1)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_load.cpp";
        AeAssert::gCurrentLine = 352;
        AeAssert::gCurrentExpr = "gBoxDCGSet && gBoxDCGSet->size() == 1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Bad gBoxDCGSet pointer."))
            __debugbreak();
    }
    cdl_object_t* objects = (cdl_object_t*)gBoxDCGSet->objects_m_elements;
    if (gBoxDCGSet->objects_m_count != 0)
        return objects->cflags;
    AeAssert::gCurrentAuthor = AeAssert::CD;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
    AeAssert::gCurrentLine = 77;
    AeAssert::gCurrentExpr = "index < size()";
    if (!AeAssert::IsIgnored() && AeAssert::Assert(""))
        __debugbreak();
    if (gBoxDCGSet->objects_m_count != 0)
        return objects->cflags;
    if (!_tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                   "index >= 0 && index < size()", "invalid index"))
        return objects->cflags;
    int result = objects->cflags;
    __debugbreak();
    return result;
}

// ============================================================================
// rtree_visitor_t - ea: 0x61AAF0..0x61AB20
// ============================================================================
enum visit_result_t {
    CONTINUE_VISITING = 0,
};

struct rtree_visitor_t {
    int objects_slot[256];   // +0x410
    int objects_count;       // +0x414
    int boxes_slot[128];     // +0x420
    int boxes_count;         // +0x624
    int brushes_slot[128];   // +0x830
    int brushes_count;       // +0x834
    int patches_slot[128];   // +0x840
    int patches_count;       // +0xA44
    CGBank* bank;            // +0xA50

    visit_result_t visit(int index);  // ?visit@rtree_visitor_t@@UAE?AW4visit_result_t@@H@Z
    void filter_objects(int mask);    // ?filter_objects@rtree_visitor_t@@QAEXH@Z
};

// ea: 0x0061AAF0
visit_result_t rtree_visitor_t::visit(int index)
{
    if (this->objects_count != 256)
        this->objects_slot[this->objects_count++] = index;
    return CONTINUE_VISITING;
}

// ea: 0x0061AB20
void rtree_visitor_t::filter_objects(int mask)
{
    unsigned int nobjects = (unsigned int)this->objects_count;
    unsigned int oi = 0;
    if (nobjects != 0)
    {
        int v3 = 0;
        while (1)
        {
            if ((v3 < 0 || v3 >= this->objects_count)
                && _tlAssert(
                    "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                    108, "i >= 0 && i < m_alloc_count", ""))
                __debugbreak();
            CGBank* bank = this->bank;
            unsigned int index = (unsigned int)this->objects_slot[v3];
            if (index >= (unsigned int)bank->objects.m_count)
            {
                AeAssert::gCurrentAuthor = AeAssert::CD;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                AeAssert::gCurrentLine = 233;
                AeAssert::gCurrentExpr = "index < size()";
                if (!AeAssert::IsIgnored() && AeAssert::Assert(""))
                    __debugbreak();
                if (index >= (unsigned int)bank->objects.m_count
                    && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                 89, "index >= 0 && index < size()",
                                 "invalid index"))
                    __debugbreak();
            }
            cdl_object_t* obj = &((cdl_object_t*)bank->objects.m_elements)[index];
            if ((mask & obj->cflags) != 0)
            {
                unsigned int nboxes = (unsigned int)bank->nboxes;
                int type = index < nboxes
                    ? 0
                    : 2 - (index < nboxes + (unsigned int)bank->nbrushes);
                if (type == 1)
                {
                    if (this->brushes_count == 128)
                        goto LABEL_28;
                    this->brushes_slot[this->brushes_count++] = (int)index;
                }
                else
                {
                    if (type != 0)
                    {
                        if (this->patches_count == 128)
                            goto LABEL_28;
                        this->patches_slot[this->patches_count++] = (int)index;
                    }
                    else
                    {
                        if (this->boxes_count == 128)
                            goto LABEL_28;
                        this->boxes_slot[this->boxes_count++] = (int)index;
                    }
                }
            }
        LABEL_28:
            ++oi;
            if (oi >= nobjects)
                return;
            v3 = (int)oi;
        }
    }
}

// ============================================================================
// can_place_decal - ea: 0x61A900 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x0061A900
bool can_place_decal(const math::Position3& p, const math::Dir3& n,
                     const math::Position3& bmin,
                     const math::Position3& bmax, const cdlPlane* sides,
                     unsigned int nsides, float decal_radius)
{
    float v18 = p.v.m128_f32[0] * n.v.m128_f32[0]
        + p.v.m128_f32[1] * n.v.m128_f32[1]
        + p.v.m128_f32[2] * n.v.m128_f32[2];
    math::Dir3 axis;
    axis.v.m128_f32[0] = -1.0f; axis.v.m128_f32[1] = 0.0f;
    axis.v.m128_f32[2] = 0.0f; axis.v.m128_f32[3] = 0.0f;
    if (!is_plane_ok(p, n, (unsigned int)v18, axis,
                     0.0f - bmin.v.m128_f32[0], decal_radius))
        return false;
    axis.v.m128_f32[0] = 0.0f; axis.v.m128_f32[1] = -1.0f;
    axis.v.m128_f32[2] = 0.0f;
    if (!is_plane_ok(p, n, (unsigned int)v18, axis,
                     0.0f - bmin.v.m128_f32[1], decal_radius))
        return false;
    axis.v.m128_f32[1] = 0.0f; axis.v.m128_f32[2] = -1.0f;
    if (!is_plane_ok(p, n, (unsigned int)v18, axis,
                     0.0f - bmin.v.m128_f32[2], decal_radius))
        return false;
    axis.v.m128_f32[2] = 0.0f; axis.v.m128_f32[0] = 1.0f;
    if (!is_plane_ok(p, n, (unsigned int)v18, axis,
                     bmax.v.m128_f32[0], decal_radius))
        return false;
    axis.v.m128_f32[0] = 0.0f; axis.v.m128_f32[1] = 1.0f;
    if (!is_plane_ok(p, n, (unsigned int)v18, axis,
                     bmax.v.m128_f32[1], decal_radius))
        return false;
    axis.v.m128_f32[1] = 0.0f; axis.v.m128_f32[2] = 1.0f;
    if (!is_plane_ok(p, n, (unsigned int)v18, axis,
                     bmax.v.m128_f32[2], decal_radius))
        return false;
    for (unsigned int i = 0; i < nsides; ++i)
    {
        float offs = *(const float*)&sides[i].packed[3];
        if (!is_plane_ok(p, n, (unsigned int)v18,
                         *(const math::Dir3*)&sides[i].packed[0],
                         offs, decal_radius))
            return false;
    }
    return true;
}

// ea: 0x0061CBD0
bool TestPointInBox(const math::Position3& p, const math::Position3& bmin,
                    const math::Position3& bmax)
{
    return (p.v.m128_f32[0] >= bmin.v.m128_f32[0]
            && p.v.m128_f32[1] >= bmin.v.m128_f32[1]
            && p.v.m128_f32[2] >= bmin.v.m128_f32[2]
            && p.v.m128_f32[0] <= bmax.v.m128_f32[0]
            && p.v.m128_f32[1] <= bmax.v.m128_f32[1]
            && p.v.m128_f32[2] <= bmax.v.m128_f32[2]);
}

// ea: 0x0061CC30
int TestPointInBrush(const math::Position3& p, const math::Position3& bmin,
                     const math::Position3& bmax, const cdlPlane* sides,
                     unsigned int nsides)
{
    if (!(p.v.m128_f32[0] >= bmin.v.m128_f32[0]
          && p.v.m128_f32[1] >= bmin.v.m128_f32[1]
          && p.v.m128_f32[2] >= bmin.v.m128_f32[2]
          && p.v.m128_f32[0] <= bmax.v.m128_f32[0]
          && p.v.m128_f32[1] <= bmax.v.m128_f32[1]
          && p.v.m128_f32[2] <= bmax.v.m128_f32[2]))
        return 1;
    for (unsigned int v5 = 0; v5 < nsides; ++v5)
    {
        float dot = p.v.m128_f32[0] * *(const float*)&sides[v5].packed[0]
            + p.v.m128_f32[1] * *(const float*)&sides[v5].packed[1]
            + p.v.m128_f32[2] * *(const float*)&sides[v5].packed[2];
        if (dot - *(const float*)&sides[v5].packed[3] >= 0.0f)
            return 0;
    }
    return 1;
}

// ============================================================================
// collide_sphere_poly - ea: 0x61B980 (CollisionMgr.cpp)
// ============================================================================
extern math::Position3 calc_closest(const math::Position3& p,
                                    const math::Position3& v0,
                                    const math::Position3& v1,
                                    const math::Position3& v2);  // cdl_common.o

// ea: 0x0061B980
bool collide_sphere_poly(const math::Position3& c, float r,
                         const math::Position3& v0,
                         const math::Position3& v1,
                         const math::Position3& v2,
                         const math::Vector4& plane)
{
    float d = plane.v.m128_f32[0] * c.v.m128_f32[0]
        + plane.v.m128_f32[1] * c.v.m128_f32[1]
        + plane.v.m128_f32[2] * c.v.m128_f32[2]
        + plane.v.m128_f32[3];
    if (fabsf(d) > r)
        return false;
    // Project the sphere center onto the plane.
    math::Position3 proj;
    proj.v.m128_f32[0] = c.v.m128_f32[0] - plane.v.m128_f32[0] * d;
    proj.v.m128_f32[1] = c.v.m128_f32[1] - plane.v.m128_f32[1] * d;
    proj.v.m128_f32[2] = c.v.m128_f32[2] - plane.v.m128_f32[2] * d;
    // Barycentric edge signs: if all three are >= 0 the projection is inside.
    float e0[3] = { v0.v.m128_f32[0] - v1.v.m128_f32[0],
                    v0.v.m128_f32[1] - v1.v.m128_f32[1],
                    v0.v.m128_f32[2] - v1.v.m128_f32[2] };
    float p1[3] = { proj.v.m128_f32[0] - v1.v.m128_f32[0],
                    proj.v.m128_f32[1] - v1.v.m128_f32[1],
                    proj.v.m128_f32[2] - v1.v.m128_f32[2] };
    float e1[3] = { v1.v.m128_f32[0] - v2.v.m128_f32[0],
                    v1.v.m128_f32[1] - v2.v.m128_f32[1],
                    v1.v.m128_f32[2] - v2.v.m128_f32[2] };
    float p2[3] = { proj.v.m128_f32[0] - v2.v.m128_f32[0],
                    proj.v.m128_f32[1] - v2.v.m128_f32[1],
                    proj.v.m128_f32[2] - v2.v.m128_f32[2] };
    float e2[3] = { v2.v.m128_f32[0] - v0.v.m128_f32[0],
                    v2.v.m128_f32[1] - v0.v.m128_f32[1],
                    v2.v.m128_f32[2] - v0.v.m128_f32[2] };
    float pe[3] = { proj.v.m128_f32[0] - v0.v.m128_f32[0],
                    proj.v.m128_f32[1] - v0.v.m128_f32[1],
                    proj.v.m128_f32[2] - v0.v.m128_f32[2] };
    float s0 = (e0[1] * p1[2] - e0[2] * p1[1]) * plane.v.m128_f32[0]
        + (e0[2] * p1[0] - e0[0] * p1[2]) * plane.v.m128_f32[1]
        + (e0[0] * p1[1] - e0[1] * p1[0]) * plane.v.m128_f32[2];
    float s1 = (e1[1] * p2[2] - e1[2] * p2[1]) * plane.v.m128_f32[0]
        + (e1[2] * p2[0] - e1[0] * p2[2]) * plane.v.m128_f32[1]
        + (e1[0] * p2[1] - e1[1] * p2[0]) * plane.v.m128_f32[2];
    float s2 = (e2[1] * pe[2] - e2[2] * pe[1]) * plane.v.m128_f32[0]
        + (e2[2] * pe[0] - e2[0] * pe[2]) * plane.v.m128_f32[1]
        + (e2[0] * pe[1] - e2[1] * pe[0]) * plane.v.m128_f32[2];
    if (s0 >= 0.0f && s1 >= 0.0f && s2 >= 0.0f)
        return true;
    math::Position3 closest = calc_closest(proj, v0, v1, v2);
    float dx = closest.v.m128_f32[0] - c.v.m128_f32[0];
    float dy = closest.v.m128_f32[1] - c.v.m128_f32[1];
    float dz = closest.v.m128_f32[2] - c.v.m128_f32[2];
    return (r * r) > (dx * dx + dy * dy + dz * dz);
}

// ============================================================================
// traceWork_t + TestBoxInBrush - ea: 0x61CD30 (CollisionMgr.cpp)
// ============================================================================
struct traceWork_t {
    math::Position3 bounds[2];     // +0x00
    math::Position3 end;           // +0x20
    math::Position3 start;         // +0x30
    math::Position3 size[2];       // +0x40 (mins/maxs for TempBoxModel)
    math::Position3 offsets[8];    // +0x60
    math::Dir3 delta;              // +0xE0
    float  deltaLenSqrd;           // +0xF0
    uint8_t _padF4[0x120 - 0xF4];
    float  trace_fraction;         // +0x120
    uint8_t _pad124[4];
    int    trace_contents;         // +0x128
    float  trace_normal[3];        // +0x130
    uint8_t trace_allsolid;        // +0x13C
    uint8_t trace_startsolid;      // +0x13D
    uint8_t _pad13E[0x150 - 0x13E];
    math::Position3 sphere_offset; // +0x150
    math::Dir3 sphere_radiusOffset;// +0x160
    int    sphere_use;             // +0x170
    float  sphere_radius;          // +0x174
    float  sphere_halfheight;      // +0x178
};

// ============================================================================
// PartialClipMap + WorldSector (pcm)
// ============================================================================
struct WorldSector {
    WorldSector* parent;      // +0x00
    WorldSector* child0;      // +0x04
    WorldSector* child1;      // +0x08
    int      axis;                // +0x0C
    float    dist;                // +0x10
    int      contentsEntities;    // +0x14
    int      contentsStaticModels;// +0x18
    void*    entities;            // +0x1C
    void*    staticModels;        // +0x20
};
static_assert(sizeof(WorldSector) == 0x24, "WorldSector size mismatch");

struct PartialClipMap {
    char         name[128];        // +0x00
    int         clusterBytes;      // +0x80
    uint8_t*    visibility;        // +0x84
    int         vised;             // +0x88
    WorldSector worldSectorHead;   // +0x8C
    WorldSector* freeHead;         // +0xB0
    WorldSector dummyNode;         // +0xB4
    WorldSector worldSectors[1024];// +0xD8
};
extern PartialClipMap pcm;        // ?pcm@@3UPartialClipMap@@A (game.o)

// ea: 0x006199C0
char InitEntitiesBSP()
{
    pcm.freeHead = pcm.worldSectors;
    for (unsigned int i = 0; i < 1023; ++i)
    {
    pcm.worldSectors[i].parent = &pcm.worldSectors[i + 1];
        pcm.worldSectors[i].axis = 0;
        pcm.worldSectors[i].dist = 0.0f;
        pcm.worldSectors[i].contentsEntities = 0;
        pcm.worldSectors[i].contentsStaticModels = 0;
        pcm.worldSectors[i].entities = nullptr;
        pcm.worldSectors[i].staticModels = nullptr;
    }
    pcm.worldSectors[1023].parent = nullptr;
    int v2 = (g_bspTree->maxs[1] - g_bspTree->mins[1])
        >= (g_bspTree->maxs[0] - g_bspTree->mins[0]);
    pcm.worldSectorHead.axis = v2;
    pcm.worldSectorHead.dist =
        (g_bspTree->maxs[v2] + g_bspTree->mins[v2]) * 0.5f;
    pcm.worldSectorHead.child0 = &pcm.dummyNode;
    pcm.worldSectorHead.child1 = &pcm.dummyNode;
    pcm.vised = g_bspTree->mVised;
    pcm.clusterBytes = g_bspTree->mClusterBytes;
    if (g_bspTree->mVisibility.mSize == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
        AeAssert::gCurrentLine = 81;
        AeAssert::gCurrentExpr = "index < mSize";
        if (AeAssert::IsIgnored())
        {
            pcm.visibility = (uint8_t*)g_bspTree->mVisibility.mList;
            return 0;
        }
        if (!AeAssert::Assert("Bounds check"))
        {
            pcm.visibility = (uint8_t*)g_bspTree->mVisibility.mList;
            return 0;
        }
        __debugbreak();
    }
    pcm.visibility = (uint8_t*)g_bspTree->mVisibility.mList;
    return 0;
}

// ============================================================================
// CM_LoadMap / CM_LoadLump - ea: 0x618390..0x618410
// ============================================================================
extern cvar_t* Cvar_Get(const char* var_name, const char* var_value,
                        int flags);  // core.o
extern void Com_DPrintf(const char* fmt, ...);  // core.o
extern int FS_FOpenFileRead(const char* filename, int* file, int uniqueFILE);  // core.o
extern int FS_Read(void* buffer, int len, int f);  // core.o
extern int FS_Seek(int f, long offset, int origin);  // core.o ?FS_Seek@@YAHHJH@Z
extern void FS_FCloseFile(int f);  // core.o
extern void* mem_heap_malloc_ctx(int alignment, unsigned int size,
                                 const char* ctx, const char* file,
                                 int line);  // core.o
extern char* com_lumpBuf;  // ?com_lumpBuf@@3PADA (game.o)
extern cvar_t* cm_noCurves;        // ?cm_noCurves@@3PAUcvar_t@@A
extern cvar_t* cm_playerCurveClip; // ?cm_playerCurveClip@@3PAUcvar_t@@A

struct dheader_t {
    int version;
    struct {
        int fileofs;
        int filelen;
    } lumps[78];
};

// ea: 0x00618390
void CM_LoadMap(const char* name, int clientload, int* checksum)
{
    if (name == nullptr || *name == 0)
        Com_Error(ERR_DROP, "EXE_ERR_COULDNT_LOAD");
    cm_noCurves = Cvar_Get("cm_noCurves", "0", 512);
    cm_playerCurveClip = Cvar_Get("cm_playerCurveClip", "1", 513);
    Com_DPrintf("CM_LoadMap( %s, %i )\n", name, clientload);
    if (clientload == 0)
        Q_strncpyz(pcm.name, name, 128);
}

// ea: 0x00618410
int CM_LoadLump(int lumpnum, char** pBuf)
{
    if (pcm.name[0] == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_load.cpp";
        AeAssert::gCurrentLine = 244;
        AeAssert::gCurrentExpr = "pcm.name && pcm.name[0]";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int h = 0;
    FS_FOpenFileRead(pcm.name, &h, 0);
    if (h == 0)
        Com_Error(ERR_DROP, va("EXE_ERR_COULDNT_LOAD", &pcm));
    dheader_t header;
    FS_Read(&header, 312, h);
    if (header.version != 63)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_load.cpp";
        AeAssert::gCurrentLine = 255;
        AeAssert::gCurrentExpr = "header.version == 63";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int filelen = header.lumps[lumpnum].filelen;
    if (filelen != 0)
    {
        FS_Seek(h, header.lumps[lumpnum].fileofs - 312, 0);
        com_lumpBuf = (char*)mem_heap_malloc_ctx(
            16, (unsigned int)filelen, "hunk",
            "c:\\cod\\code\\game\\cm_load.cpp", 267);
        FS_Read(com_lumpBuf, filelen, h);
        FS_FCloseFile(h);
        *pBuf = com_lumpBuf;
        return filelen;
    }
    FS_FCloseFile(h);
    return 0;
}

// ============================================================================
// BinFileManager + DecodeBin - ea: 0x608ED0..0x608F70, 0x6180D0
// ============================================================================
extern unsigned int AeHash(const char* str);  // core.o

struct BinFileEntry {
    unsigned int mHash;    // +0x00
    TPakId       mPakId;   // +0x04
    unsigned char* mData;  // +0x08
};

class BinFileManager {
public:
    int mTotalFiles;         // +0x00
    BinFileEntry mArray[256];  // +0x04
    static BinFileManager* sInst;  // ?sInst@BinFileManager@@2PAV1@A

    BinFileManager();        // ??0BinFileManager@@AAE@XZ
    ~BinFileManager();       // ??1BinFileManager@@AAE@XZ
    void Clear();            // ?Clear@BinFileManager@@QAEXXZ
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pakId);  // ?DecodeBank@BinFileManager@@QAEXPBDPAEHW4TPakId@@@Z
    unsigned char* Find(const char* name);  // ?Find@BinFileManager@@QAEPAEPBD@Z
};
BinFileManager* BinFileManager::sInst = nullptr;

// ea: 0x00608ED0
BinFileManager::BinFileManager()
{
    this->mTotalFiles = 0;
}

// ea: 0x00608EE0
BinFileManager::~BinFileManager()
{
}

// ea: 0x00608EF0
void BinFileManager::Clear()
{
    this->mTotalFiles = 0;
}

// ea: 0x00608F00
void BinFileManager::DecodeBank(const char* name, unsigned char* data,
                                int size, TPakId pakId)
{
    const char* v6 = name;
    if (*name != '.')
    {
        do
            v6++;
        while (*v6 != '.');
    }
    // Strip the extension (name is modified in place through the original
    // pointer; the decompile writes through v6 which aliases name).
    char* end = const_cast<char*>(v6);
    *end = 0;
    this->mArray[this->mTotalFiles].mHash = AeHash(name);
    this->mArray[this->mTotalFiles].mPakId = pakId;
    this->mArray[this->mTotalFiles++].mData = data;
}

// ea: 0x00608F70
unsigned char* BinFileManager::Find(const char* name)
{
    unsigned int nameHash = AeHash(name);
    int v3 = 0;
    if (this->mTotalFiles == 0)
        return nullptr;
    while (this->mArray[v3].mHash != nameHash
           || !PakManager::sInst->IsLoaded(this->mArray[v3].mPakId))
    {
        if (++v3 >= this->mTotalFiles)
            return nullptr;
    }
    return this->mArray[v3].mData;
}

// ea: 0x006180D0
void DecodeBin(const char* name, unsigned char* data, int size, TPakId pakId)
{
    BinFileManager::sInst->DecodeBank(name, data, size, pakId);
}

// ============================================================================
// GetLeaves / CM_BoxLeafnums - ea: 0x619050..0x6194A0
// ============================================================================
// ea: 0x00619050
void GetLeaves(leafList_s* ll, unsigned int nodeIndex, float* mindist)
{
    BspNode* v30 = &g_bspTree->mNodes.mList[0];
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
        const float* v7 = v6->u.node.plane->mPlane.m128_f32;
        // Point-plane distance box (ll->bounds[0..1] around origin).
        float dmax = 0.0f, dmin = 0.0f;
        float v27 = v7[0] * (v7[0] < 0.0f ? ll->bounds[1].v.m128_f32[0]
                                           : ll->bounds[0].v.m128_f32[0])
            + v7[1] * (v7[1] < 0.0f ? ll->bounds[1].v.m128_f32[1]
                                     : ll->bounds[0].v.m128_f32[1])
            + v7[2] * (v7[2] < 0.0f ? ll->bounds[1].v.m128_f32[2]
                                     : ll->bounds[0].v.m128_f32[2]);
        float v26 = v7[0] * (v7[0] > 0.0f ? ll->bounds[1].v.m128_f32[0]
                                           : ll->bounds[0].v.m128_f32[0])
            + v7[1] * (v7[1] > 0.0f ? ll->bounds[1].v.m128_f32[1]
                                     : ll->bounds[0].v.m128_f32[1])
            + v7[2] * (v7[2] > 0.0f ? ll->bounds[1].v.m128_f32[2]
                                     : ll->bounds[0].v.m128_f32[2]);
        float v20 = v7[3];
        int v21 = 0;
        float v22 = 0.0f;
        if (v20 <= v27)
            v21 = 1;
        else
            v22 = v20 - v27;
        if (v26 <= v20)
            v21 |= 2;
        else
            v22 = v26 - v20;
        if (v22 > 0.0f && *mindist > v22)
            *mindist = v22;
        if (v21 != 0)
        {
            if (v21 != 3)
                goto LABEL_40;
            GetLeaves(ll, (unsigned int)(v6->u.node.children[0] - v30) >> 4,
                      mindist);
            nodeIndex = (unsigned int)(v6->u.node.children[1] - v30) >> 4;
        }
        else
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_test.cpp";
            AeAssert::gCurrentLine = 153;
            AeAssert::gCurrentExpr = "s != 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("illegal value for s, 0"))
                __debugbreak();
        LABEL_40:
            nodeIndex = (unsigned int)(v6->u.node.children[v21 - 1] - v30) >> 4;
        }
    }
    CM_StoreLeafs(ll, (int)nodeIndex);
}

// ea: 0x006194A0
int CM_BoxLeafnums(math::Vector4& cached_pos, int& cached_leaf,
                   const math::Position3* pos, const math::Position3* mins,
                   const math::Position3* maxs, int* list, int listsize,
                   int* lastLeaf)
{
    ++g_bspTree->checkcount;
    leafList_s ll;
    ll.bounds[0].v = mins->v;
    ll.bounds[1].v = maxs->v;
    ll.count = 0;
    ll.maxcount = listsize;
    ll.list = list;
    ll.overflowed = 0;
    ll.lastLeaf = 0;
    float v15 = cached_pos.v.m128_f32[3] - 5.0f;
    if (v15 <= 0.0f)
        goto LABEL_4;
    ll.bounds[0].v = cached_pos.v;
    float dx = cached_pos.v.m128_f32[0] - pos->v.m128_f32[0];
    float dy = cached_pos.v.m128_f32[1] - pos->v.m128_f32[1];
    float dz = cached_pos.v.m128_f32[2] - pos->v.m128_f32[2];
    float dist2 = dx * dx + dy * dy + dz * dz;
    if ((v15 * v15) <= dist2)
    {
    LABEL_4:
        float mindist = 3.4028235e38f;
        GetLeaves(&ll, 0, &mindist);
        int result = ll.count;
        float v21 = 0.0f;
        if (ll.count == 1)
            v21 = mindist;
        cached_pos.v.m128_f32[3] = v21;
        if (v21 > 0.0f)
        {
            cached_leaf = *list;
            cached_pos.v.m128_f32[0] = pos->v.m128_f32[0];
            cached_pos.v.m128_f32[1] = pos->v.m128_f32[1];
            cached_pos.v.m128_f32[2] = pos->v.m128_f32[2];
            cached_pos.v.m128_f32[3] = v21;
        }
        *lastLeaf = ll.lastLeaf;
        return result;
    }
    CM_StoreLeafs(&ll, cached_leaf);
    *lastLeaf = ll.lastLeaf;
    return ll.count;
}

extern "C" int __fpclass(float);

// ea: 0x0061CD30
void TestBoxInBrush(traceWork_t* tw, const math::Position3& bmin,
                    const math::Position3& bmax, const cdlPlane* sides,
                    unsigned int nsides, unsigned int cflags)
{
    int v7 = 0;
    if ((__fpclass(tw->start.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(tw->start.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(tw->start.v.m128_f32[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 2101;
        AeAssert::gCurrentExpr =
            "!IS_NAN((tw->start)[0]) && !IS_NAN((tw->start)[1]) && "
            "!IS_NAN((tw->start)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((__fpclass(tw->end.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(tw->end.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(tw->end.v.m128_f32[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 2102;
        AeAssert::gCurrentExpr =
            "!IS_NAN((tw->end)[0]) && !IS_NAN((tw->end)[1]) && "
            "!IS_NAN((tw->end)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    // Box overlap check between the brush AABB and tw->bounds.
    if (!(tw->bounds[0].v.m128_f32[0] <= bmax.v.m128_f32[0]
          && tw->bounds[1].v.m128_f32[0] >= bmin.v.m128_f32[0]
          && tw->bounds[0].v.m128_f32[1] <= bmax.v.m128_f32[1]
          && tw->bounds[1].v.m128_f32[1] >= bmin.v.m128_f32[1]
          && tw->bounds[0].v.m128_f32[2] <= bmax.v.m128_f32[2]
          && tw->bounds[1].v.m128_f32[2] >= bmin.v.m128_f32[2]))
        return;
    if (tw->sphere_use != 0)
    {
        unsigned int v25 = 0;
        if (nsides != 0)
        {
            const cdlPlane* v33 = sides;
            do
            {
                float v27 = *(const float*)&v33->packed[3];
                if ((__fpclass(v27) & 0x297) != 0)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\CollisionMgr.cpp";
                    AeAssert::gCurrentLine = 2115;
                    AeAssert::gCurrentExpr = "!IS_NAN(offset)";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Invalid number!"))
                        __debugbreak();
                }
                if ((__fpclass(tw->sphere_radius) & 0x297) != 0)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\CollisionMgr.cpp";
                    AeAssert::gCurrentLine = 2116;
                    AeAssert::gCurrentExpr = "!IS_NAN(tw->sphere.radius)";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Invalid number!"))
                        __debugbreak();
                }
                float v8 = v27 + tw->sphere_radius;
                if ((__fpclass(v8) & 0x297) != 0)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\CollisionMgr.cpp";
                    AeAssert::gCurrentLine = 2120;
                    AeAssert::gCurrentExpr = "!IS_NAN(dist)";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Invalid number!"))
                        __debugbreak();
                }
                if ((__fpclass(*(const float*)&v33->packed[0]) & 0x297) != 0
                    || (__fpclass(*(const float*)&v33->packed[1]) & 0x297)
                        != 0
                    || (__fpclass(*(const float*)&v33->packed[2]) & 0x297)
                        != 0)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\CollisionMgr.cpp";
                    AeAssert::gCurrentLine = 2121;
                    AeAssert::gCurrentExpr =
                        "!IS_NAN((normal)[0]) && !IS_NAN((normal)[1]) && "
                        "!IS_NAN((normal)[2])";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Invalid vector"))
                        __debugbreak();
                }
                if ((__fpclass(tw->sphere_offset.v.m128_f32[0]) & 0x297)
                        != 0
                    || (__fpclass(tw->sphere_offset.v.m128_f32[1]) & 0x297)
                        != 0
                    || (__fpclass(tw->sphere_offset.v.m128_f32[2]) & 0x297)
                        != 0)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\CollisionMgr.cpp";
                    AeAssert::gCurrentLine = 2122;
                    AeAssert::gCurrentExpr =
                        "!IS_NAN((tw->sphere.offset)[0]) && "
                        "!IS_NAN((tw->sphere.offset)[1]) && "
                        "!IS_NAN((tw->sphere.offset)[2])";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Invalid vector"))
                        __debugbreak();
                }
                math::Position3 v9 = tw->sphere_offset;
                float v24 = *(const float*)&v33->packed[0]
                        * v9.v.m128_f32[0]
                    + *(const float*)&v33->packed[1] * v9.v.m128_f32[1]
                    + *(const float*)&v33->packed[2] * v9.v.m128_f32[2];
                math::Position3 v11 = tw->start;
                math::Position3 v12;
                if (v24 <= 0.0f)
                {
                    v12.v.m128_f32[0] = v11.v.m128_f32[0] + v9.v.m128_f32[0];
                    v12.v.m128_f32[1] = v11.v.m128_f32[1] + v9.v.m128_f32[1];
                    v12.v.m128_f32[2] = v11.v.m128_f32[2] + v9.v.m128_f32[2];
                }
                else
                {
                    v12.v.m128_f32[0] = v11.v.m128_f32[0] - v9.v.m128_f32[0];
                    v12.v.m128_f32[1] = v11.v.m128_f32[1] - v9.v.m128_f32[1];
                    v12.v.m128_f32[2] = v11.v.m128_f32[2] - v9.v.m128_f32[2];
                }
                if ((__fpclass(v12.v.m128_f32[0]) & 0x297) != 0
                    || (__fpclass(v12.v.m128_f32[1]) & 0x297) != 0
                    || (__fpclass(v12.v.m128_f32[2]) & 0x297) != 0)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\CollisionMgr.cpp";
                    AeAssert::gCurrentLine = 2131;
                    AeAssert::gCurrentExpr =
                        "!IS_NAN((startp)[0]) && !IS_NAN((startp)[1]) && "
                        "!IS_NAN((startp)[2])";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Invalid vector"))
                        __debugbreak();
                }
                float d1 = v12.v.m128_f32[0]
                        * *(const float*)&v33->packed[0]
                    + v12.v.m128_f32[1] * *(const float*)&v33->packed[1]
                    + v12.v.m128_f32[2] * *(const float*)&v33->packed[2]
                    - v8;
                if ((__fpclass(d1) & 0x297) != 0)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\CollisionMgr.cpp";
                    AeAssert::gCurrentLine = 2133;
                    AeAssert::gCurrentExpr = "!IS_NAN(d1)";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Invalid number!"))
                        __debugbreak();
                }
                if (d1 > 0.0f)
                    return;
                ++v25;
                ++v33;
            } while (v25 < nsides);
        }
        goto LABEL_94;
    }
    int v28 = 0;
    if (nsides == 0)
        goto LABEL_94;
    const cdlPlane* v15 = sides;
    while (1)
    {
        float v26 = *(const float*)&v15->packed[3];
        if (*(const float*)&v15->packed[0] < 0.0f)
            v7 = 1;
        if (*(const float*)&v15->packed[1] < 0.0f)
            v7 |= 2;
        if (*(const float*)&v15->packed[2] < 0.0f)
            v7 |= 4;
        if ((__fpclass(v26) & 0x297) != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
            AeAssert::gCurrentLine = 2154;
            AeAssert::gCurrentExpr = "!IS_NAN(offset)";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid number!"))
                __debugbreak();
        }
        math::Position3* startp = &tw->offsets[v7];
        if ((__fpclass(startp->v.m128_f32[0]) & 0x297) != 0
            || (__fpclass(startp->v.m128_f32[1]) & 0x297) != 0
            || (__fpclass(startp->v.m128_f32[2]) & 0x297) != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
            AeAssert::gCurrentLine = 2155;
            AeAssert::gCurrentExpr =
                "!IS_NAN((tw->offsets[signbits])[0]) && "
                "!IS_NAN((tw->offsets[signbits])[1]) && "
                "!IS_NAN((tw->offsets[signbits])[2])";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        if ((__fpclass(*(const float*)&v15->packed[0]) & 0x297) != 0
            || (__fpclass(*(const float*)&v15->packed[1]) & 0x297) != 0
            || (__fpclass(*(const float*)&v15->packed[2]) & 0x297) != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
            AeAssert::gCurrentLine = 2156;
            AeAssert::gCurrentExpr =
                "!IS_NAN((normal)[0]) && !IS_NAN((normal)[1]) && "
                "!IS_NAN((normal)[2])";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        float dist = v26
            - (startp->v.m128_f32[0] * *(const float*)&v15->packed[0]
               + startp->v.m128_f32[1] * *(const float*)&v15->packed[1]
               + startp->v.m128_f32[2] * *(const float*)&v15->packed[2]);
        if ((__fpclass(dist) & 0x297) != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
            AeAssert::gCurrentLine = 2160;
            AeAssert::gCurrentExpr = "!IS_NAN(dist)";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid number!"))
                __debugbreak();
        }
        float d1 = tw->start.v.m128_f32[0] * *(const float*)&v15->packed[0]
            + tw->start.v.m128_f32[1] * *(const float*)&v15->packed[1]
            + tw->start.v.m128_f32[2] * *(const float*)&v15->packed[2]
            - dist;
        if ((__fpclass(d1) & 0x297) != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
            AeAssert::gCurrentLine = 2163;
            AeAssert::gCurrentExpr = "!IS_NAN(d1)";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid number!"))
                __debugbreak();
        }
        if (d1 > 0.0f)
            return;
        ++v15;
        if (++v28 >= (int)nsides)
        {
            goto LABEL_94;
        }
        v7 = 0;
    }
LABEL_94:
    tw->trace_allsolid = 1;
    tw->trace_startsolid = 1;
    tw->trace_fraction = 0.0f;
    tw->trace_contents = (int)cflags;
}

// ============================================================================
// TraceSphereThroughSphere - ea: 0x61C1A0
// ============================================================================
// ea: 0x0061C1A0
int TraceSphereThroughSphere(traceWork_t* tw,
                             const math::Position3& vStart,
                             const math::Position3& vEnd,
                             const math::Position3& vStationary,
                             float radius)
{
    float v6[3] = { vStart.v.m128_f32[0] - vStationary.v.m128_f32[0],
                    vStart.v.m128_f32[1] - vStationary.v.m128_f32[1],
                    vStart.v.m128_f32[2] - vStationary.v.m128_f32[2] };
    float v7 = (tw->sphere_radius + radius) * (tw->sphere_radius + radius);
    float dist2 = v6[0] * v6[0] + v6[1] * v6[1] + v6[2] * v6[2];
    float v9 = dist2 - v7;
    if (v9 > 0.0f)
    {
        float b = tw->delta.v.m128_f32[0] * v6[0]
            + tw->delta.v.m128_f32[1] * v6[1]
            + tw->delta.v.m128_f32[2] * v6[2];
        if (b >= 0.0f)
            return 1;
        float deltaLenSqrd = tw->deltaLenSqrd;
        float disc = (b * b) - (deltaLenSqrd * v9);
        if (disc < 0.0f)
            return 1;
        float nlen = VectorNormalize2((const math::Dir3*)v6,
                                      (math::Dir3*)v6);
        float sqrtdisc = sqrtf(disc);
        float t = nlen * 0.125f / b + (-b - sqrtdisc) / deltaLenSqrd;
        if (tw->trace_fraction <= t)
            return 1;
        float v17 = t;
        if (t <= 0.0f)
            v17 = 0.0f;
        tw->trace_normal[0] = v6[0];
        tw->trace_normal[1] = v6[1];
        tw->trace_normal[2] = v6[2];
        tw->trace_fraction = v17;
        tw->trace_normal[0] /*w*/ = 0.0f;
        tw->trace_contents = TempBoxModelContents();
        return 0;
    }
    tw->trace_fraction = 0.0f;
    tw->trace_startsolid = 1;
    float len = sqrtf(dist2);
    if (len != 0.0f)
    {
        tw->trace_normal[0] = v6[0] / len;
        tw->trace_normal[1] = v6[1] / len;
        tw->trace_normal[2] = v6[2] / len;
    }
    float v10[3] = { vEnd.v.m128_f32[0] - vStationary.v.m128_f32[0],
                     vEnd.v.m128_f32[1] - vStationary.v.m128_f32[1],
                     vEnd.v.m128_f32[2] - vStationary.v.m128_f32[2] };
    float endDist2 = v10[0] * v10[0] + v10[1] * v10[1] + v10[2] * v10[2];
    if (v7 >= endDist2)
        tw->trace_allsolid = 1;
    return 0;
}

// ============================================================================
// TraceCylinderThroughCylinder - ea: 0x61C3D0
// ============================================================================
// ea: 0x0061C3D0
int TraceCylinderThroughCylinder(traceWork_t* tw,
                                 const math::Position3& vStationary,
                                 float fStationaryHalfHeight, float radius)
{
    float v5 = tw->sphere_radius + radius;
    float vNormal[3] = { tw->start.v.m128_f32[0] - vStationary.v.m128_f32[0],
                         tw->start.v.m128_f32[1] - vStationary.v.m128_f32[1],
                         tw->start.v.m128_f32[2] - vStationary.v.m128_f32[2] };
    float v6 = (vNormal[0] * vNormal[0] + vNormal[1] * vNormal[1])
        - (v5 * v5);
    if (v6 <= 0.0f)
    {
        float v24 = (tw->sphere_halfheight - tw->sphere_radius)
            + fStationaryHalfHeight;
        if (vNormal[2] <= v24)
        {
            float v20 = 0.0f - v24;
            if (v20 <= vNormal[2])
            {
                vNormal[2] = 0.0f;
                tw->trace_fraction = 0.0f;
                float len = sqrtf(vNormal[0] * vNormal[0]
                                  + vNormal[1] * vNormal[1]);
                tw->trace_startsolid = 1;
                if (len != 0.0f)
                {
                    tw->trace_normal[0] = vNormal[0] / len;
                    tw->trace_normal[1] = vNormal[1] / len;
                }
                tw->trace_normal[2] = 0.0f;
                tw->trace_normal[0] /*w*/ = 0.0f;
                tw->trace_contents = TempBoxModelContents();
                float v15 = tw->end.v.m128_f32[2] - vStationary.v.m128_f32[2];
                if (v24 >= v15 && v15 >= v20)
                    tw->trace_allsolid = 1;
                return 0;
            }
        }
        return 1;
    }
    float b = tw->delta.v.m128_f32[0] * vNormal[0]
        + tw->delta.v.m128_f32[1] * vNormal[1];
    if (b >= 0.0f)
        return 1;
    float deltaXY = tw->delta.v.m128_f32[1] * tw->delta.v.m128_f32[1]
        + tw->delta.v.m128_f32[0] * tw->delta.v.m128_f32[0];
    float disc = (b * b) - (deltaXY * v6);
    if (disc < 0.0f)
        return 1;
    vNormal[2] = 0.0f;
    float nlen = VectorNormalize2((const math::Dir3*)vNormal,
                                  (math::Dir3*)vNormal);
    float t = nlen * 0.125f / b;
    float t2 = (-b - sqrtf(disc)) / deltaXY + t;
    if (tw->trace_fraction <= t2)
        return 1;
    float v19 = (tw->sphere_halfheight - tw->sphere_radius)
        + fStationaryHalfHeight;
    float v20 = (((t2 - t) * tw->delta.v.m128_f32[2])
                 + tw->start.v.m128_f32[2]) - vStationary.v.m128_f32[2];
    if (v20 > v19)
        return 1;
    if ((0.0f - v19) > v20)
        return 1;
    float v21 = 0.0f;
    if (t2 > 0.0f)
        v21 = t2;
    tw->trace_normal[0] = vNormal[0];
    tw->trace_normal[1] = vNormal[1];
    tw->trace_normal[2] = vNormal[2];
    tw->trace_fraction = v21;
    tw->trace_normal[0] /*w*/ = 0.0f;
    tw->trace_contents = TempBoxModelContents();
    return 0;
}

// ============================================================================
// TraceCapsuleThroughCapsule - ea: 0x61C690
// ============================================================================
// ea: 0x0061C690
void TraceCapsuleThroughCapsule(traceWork_t* tw)
{
    // Build the stationary capsule from the temp box's min/max.
    math::Position3 v2 = gBoxDCGSet->min;
    math::Position3 v3 = gBoxDCGSet->max;
    // Box overlap test with tw->bounds inflated by 1.
    if (!(tw->bounds[0].v.m128_f32[0] - 1.0f <= v3.v.m128_f32[0]
          && tw->bounds[1].v.m128_f32[0] + 1.0f >= v2.v.m128_f32[0]
          && tw->bounds[0].v.m128_f32[1] - 1.0f <= v3.v.m128_f32[1]
          && tw->bounds[1].v.m128_f32[1] + 1.0f >= v2.v.m128_f32[1]
          && tw->bounds[0].v.m128_f32[2] - 1.0f <= v3.v.m128_f32[2]
          && tw->bounds[1].v.m128_f32[2] + 1.0f >= v2.v.m128_f32[2]))
        return;
    math::Dir3 v4;
    v4.v = tw->sphere_offset.v;
    math::Position3 top[3];  // top[0] unused, top[1] = center (xyz), top[2] = 0
    float topCenter[3] = { (v2.v.m128_f32[0] + v3.v.m128_f32[0]) * 0.5f,
                           (v2.v.m128_f32[1] + v3.v.m128_f32[1]) * 0.5f,
                           (v2.v.m128_f32[2] + v3.v.m128_f32[2]) * 0.5f };
    float half[3] = { (v3.v.m128_f32[0] - v2.v.m128_f32[0]) * 0.5f,
                      (v3.v.m128_f32[1] - v2.v.m128_f32[1]) * 0.5f,
                      (v3.v.m128_f32[2] - v2.v.m128_f32[2]) * 0.5f };
    float v10 = half[0] < half[2] ? half[0] : half[2];
    float v11 = half[2] - v10;
    float v18 = v11;
    math::Position3 bottom;
    bottom.v.m128_f32[0] = topCenter[0];
    bottom.v.m128_f32[1] = topCenter[1];
    bottom.v.m128_f32[2] = topCenter[2] - v11;
    float offs = topCenter[2] + v11;
    math::Position3 startPlus = tw->start;
    startPlus.v.m128_f32[0] += v4.v.m128_f32[0];
    startPlus.v.m128_f32[1] += v4.v.m128_f32[1];
    startPlus.v.m128_f32[2] += v4.v.m128_f32[2];
    math::Position3 startMinus = tw->start;
    startMinus.v.m128_f32[0] -= v4.v.m128_f32[0];
    startMinus.v.m128_f32[1] -= v4.v.m128_f32[1];
    startMinus.v.m128_f32[2] -= v4.v.m128_f32[2];
    math::Position3 endPlus = tw->end;
    endPlus.v.m128_f32[0] += v4.v.m128_f32[0];
    endPlus.v.m128_f32[1] += v4.v.m128_f32[1];
    endPlus.v.m128_f32[2] += v4.v.m128_f32[2];
    math::Position3 endMinus = tw->end;
    endMinus.v.m128_f32[0] -= v4.v.m128_f32[0];
    endMinus.v.m128_f32[1] -= v4.v.m128_f32[1];
    endMinus.v.m128_f32[2] -= v4.v.m128_f32[2];
    math::Position3 stationaryCenter;
    stationaryCenter.v.m128_f32[0] = topCenter[0];
    stationaryCenter.v.m128_f32[1] = topCenter[1];
    stationaryCenter.v.m128_f32[2] = topCenter[2];
    if (tw->start.v.m128_f32[2] + v4.v.m128_f32[2] > offs)
    {
        if (TraceSphereThroughSphere(tw, startMinus, endMinus,
                                     stationaryCenter, v10) == 0)
            return;
        if (!(tw->delta.v.m128_f32[2] < 0.0f))
            return;
    }
    else if (topCenter[2] - v11 > tw->start.v.m128_f32[2] - v4.v.m128_f32[2])
    {
        if (TraceSphereThroughSphere(tw, startPlus, endPlus, bottom, v10) == 0)
            return;
        if (!(tw->delta.v.m128_f32[2] > 0.0f))
            return;
    }
    if (TraceCylinderThroughCylinder(tw, *(const math::Position3*)topCenter,
                                     v18, v10) == 0)
        return;
    if (tw->end.v.m128_f32[2] + v4.v.m128_f32[2] > offs)
    {
        if (offs < tw->start.v.m128_f32[2] - v4.v.m128_f32[2])
            return;
        TraceSphereThroughSphere(tw, startMinus, endMinus,
                                 stationaryCenter, v10);
    }
    else if (bottom.v.m128_f32[2] > tw->end.v.m128_f32[2] - v4.v.m128_f32[2]
             && tw->start.v.m128_f32[2] + v4.v.m128_f32[2] >= bottom.v.m128_f32[2])
    {
        TraceSphereThroughSphere(tw, startPlus, endPlus, bottom, v10);
    }
}

// ============================================================================
// TraceBoundingBoxThroughCapsule - ea: 0x61C8D0
// ============================================================================
bool collide_brush_segment(traceWork_t* tw,
                           const math::Position3& bmin,
                           const math::Position3& bmax,
                           const cdlPlane* sides, unsigned int nsides);
void collide_brush_velocity_sphere(traceWork_t* tw,
                                   const math::Position3& bmin,
                                   const math::Position3& bmax,
                                   const cdlPlane* sides,
                                   unsigned int nsides);

// ea: 0x0061C8D0
void TraceBoundingBoxThroughCapsule(traceWork_t* tw)
{
    math::Position3 v2 = gBoxDCGSet->max;
    math::Position3 v3;
    v3.v.m128_f32[0] = (gBoxDCGSet->min.v.m128_f32[0] + v2.v.m128_f32[0])
        * 0.5f;
    v3.v.m128_f32[1] = (gBoxDCGSet->min.v.m128_f32[1] + v2.v.m128_f32[1])
        * 0.5f;
    v3.v.m128_f32[2] = (gBoxDCGSet->min.v.m128_f32[2] + v2.v.m128_f32[2])
        * 0.5f;
    math::Position3 size4[2];
    size4[0].v.m128_f32[0] = v2.v.m128_f32[0] - v3.v.m128_f32[0];
    size4[0].v.m128_f32[1] = v2.v.m128_f32[1] - v3.v.m128_f32[1];
    size4[0].v.m128_f32[2] = v2.v.m128_f32[2] - v3.v.m128_f32[2];
    tw->start.v = _mm_sub_ps(tw->start.v, v3.v);
    tw->end.v = _mm_sub_ps(tw->end.v, v3.v);
    float v5 = size4[0].v.m128_f32[2];
    float v6 = size4[0].v.m128_f32[0];
    tw->sphere_use = 1;
    if (v6 > v5)
        v6 = v5;
    tw->sphere_radius = v6;
    tw->sphere_halfheight = v5;
    tw->sphere_offset.v.m128_f32[0] = 0.0f;
    tw->sphere_offset.v.m128_f32[1] = 0.0f;
    tw->sphere_offset.v.m128_f32[2] = v5 - tw->sphere_radius;
    tw->sphere_radiusOffset.v.m128_f32[0] = tw->sphere_radius;
    tw->sphere_radiusOffset.v.m128_f32[1] = tw->sphere_radius;
    tw->sphere_radiusOffset.v.m128_f32[2] = tw->sphere_halfheight;
    int v7 = TempBoxModelContents();
    TempBoxModel(&tw->size[0], &tw->size[1], v7, 0);
    if (gBoxDCGSet->objects_m_count == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::CD;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
        AeAssert::gCurrentLine = 77;
        AeAssert::gCurrentExpr = "index < size()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(""))
            __debugbreak();
        if (gBoxDCGSet->objects_m_count == 0
            && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                         "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
    }
    cdl_object_t* m_elements =
        (cdl_object_t*)gBoxDCGSet->objects_m_elements;
    if (tw->sphere_use != 0)
    {
        math::Position3 center, rad;
        center.v.m128_f32[0] = m_elements->center[0];
        center.v.m128_f32[1] = m_elements->center[1];
        center.v.m128_f32[2] = m_elements->center[2];
        rad.v.m128_f32[0] = m_elements->box_radius[0];
        rad.v.m128_f32[1] = m_elements->box_radius[1];
        rad.v.m128_f32[2] = m_elements->box_radius[2];
        size4[0].v = _mm_add_ps(center.v, rad.v);
        size4[1].v = _mm_sub_ps(center.v, rad.v);
        collide_brush_velocity_sphere(tw, size4[1], size4[0], nullptr, 0);
    }
    else
    {
        math::Position3 center, rad;
        center.v.m128_f32[0] = m_elements->center[0];
        center.v.m128_f32[1] = m_elements->center[1];
        center.v.m128_f32[2] = m_elements->center[2];
        rad.v.m128_f32[0] = m_elements->box_radius[0];
        rad.v.m128_f32[1] = m_elements->box_radius[1];
        rad.v.m128_f32[2] = m_elements->box_radius[2];
        size4[0].v = _mm_add_ps(center.v, rad.v);
        size4[1].v = _mm_sub_ps(center.v, rad.v);
        collide_brush_segment(tw, size4[1], size4[0], nullptr, 0);
    }
}

// ============================================================================
// TempBoxModel - ea: 0x618670
// ============================================================================
// ea: 0x00618670
DCGSet* TempBoxModel(const math::Position3* mins,
                     const math::Position3* maxs, int contents, int capsule)
{
    if (gBoxDCGSet == nullptr || gBoxDCGSet->objects_m_count != 1)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_load.cpp";
        AeAssert::gCurrentLine = 334;
        AeAssert::gCurrentExpr = "gBoxDCGSet && gBoxDCGSet->size() == 1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Bad gBoxDCGSet pointer."))
            __debugbreak();
    }
    gBoxDCGSet->min.v = mins->v;
    gBoxDCGSet->max.v = maxs->v;
    if (gBoxDCGSet->objects_m_count == 0
        && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                     "index >= 0 && index < size()", "invalid index"))
        __debugbreak();
    cdl_object_t* m_elements =
        (cdl_object_t*)gBoxDCGSet->objects_m_elements;
    m_elements->center[0] = (mins->v.m128_f32[0] + maxs->v.m128_f32[0])
        * 0.5f;
    m_elements->center[1] = (mins->v.m128_f32[1] + maxs->v.m128_f32[1])
        * 0.5f;
    m_elements->center[2] = (mins->v.m128_f32[2] + maxs->v.m128_f32[2])
        * 0.5f;
    m_elements->box_radius[0] = maxs->v.m128_f32[0] - m_elements->center[0];
    m_elements->box_radius[1] = maxs->v.m128_f32[1] - m_elements->center[1];
    m_elements->box_radius[2] = maxs->v.m128_f32[2] - m_elements->center[2];
    float r2 = m_elements->box_radius[0] * m_elements->box_radius[0]
        + m_elements->box_radius[1] * m_elements->box_radius[1]
        + m_elements->box_radius[2] * m_elements->box_radius[2];
    m_elements->cflags = contents;
    m_elements->sphere_radius = sqrtf(r2);
    gBoxDCGSet->id = 4095 - (capsule != 0);
    return gBoxDCGSet;
}

// ============================================================================
// collide_brush_segment - ea: 0x61B080 (CollisionMgr.cpp slab sweep)
// ============================================================================
// ea: 0x0061B080
bool collide_brush_segment(traceWork_t* tw,
                           const math::Position3& bmin,
                           const math::Position3& bmax,
                           const cdlPlane* sides, unsigned int nsides)
{
    float v49[16];  // [0]=end, [4]=start, [8]=bound, [12]=lead normal
    v49[4] = tw->start.v.m128_f32[0];
    v49[5] = tw->start.v.m128_f32[1];
    v49[6] = tw->start.v.m128_f32[2];
    v49[7] = tw->start.v.m128_f32[3];
    v49[0] = tw->end.v.m128_f32[0];
    v49[1] = tw->end.v.m128_f32[1];
    v49[2] = tw->end.v.m128_f32[2];
    v49[3] = tw->end.v.m128_f32[3];
    v49[8] = bmin.v.m128_f32[0];
    v49[9] = bmin.v.m128_f32[1];
    v49[10] = bmin.v.m128_f32[2];
    v49[11] = bmin.v.m128_f32[3];
    float fraction = tw->trace_fraction;
    float v9 = -1.0f;
    float v13 = 0.0f;
    float delta = 0.0f;
    float v56 = fraction;
    int d1 = 0x10000;  // HIBYTE=1, BYTE2=0
    v49[12] = 0.0f;
    int v14 = 0;
    while (1)
    {
        float* v15 = &tw->size[1].v.m128_f32[0];
        for (int i = 0; i < 12; i += 4)
        {
            float v17 = ((v49[i + 4] - v49[i + 8]) * v9) - v15[0];
            float v18 = ((v49[i] - v49[i + 8]) * v9) - v15[0];
            if (v17 <= 0.0f)
            {
                if (v18 > 0.0f)
                {
                    d1 &= 0xFFFF00FF;
                    if (v17 > ((v17 - v18) * fraction))
                    {
                        float v22 = v17 / (v17 - v18);
                        v56 = v22;
                        if (v13 >= v22)
                            return false;
                        fraction = v22;
                    }
                }
            }
            else
            {
                float v19 = v17 - v18;
                if (v18 > 0.0f)
                {
                    if (v19 <= 0.0f || v18 >= 0.125f)
                        return false;
                    d1 &= 0xFFFF00FF;
                }
                float v20 = v17 - 0.125f;
                if (v20 <= (v19 * v13))
                {
                    if ((d1 >> 24) != 0)
                        goto LABEL_17;
                }
                else
                {
                    float v21 = v20 / v19;
                    delta = v21;
                    if (v21 >= fraction)
                        return false;
                    v13 = v21;
                }
                v49[12] = 0.0f;
                d1 |= 0x10000;
                v49[i + 12] = v9;
            }
        LABEL_17:
            v15 += 4;
        }
        if (v14 == 0)
        {
            v9 = 1.0f;
            v49[8] = bmax.v.m128_f32[0];
            v49[9] = bmax.v.m128_f32[1];
            v49[10] = bmax.v.m128_f32[2];
            v49[11] = bmax.v.m128_f32[3];
            v14 = 1;
            continue;
        }
        break;
    }
    unsigned int v24 = 0;
    float v34 = 0.0f, v35 = 0.0f;
    if (nsides != 0)
    {
        while (1)
        {
            const cdlPlane* s = sides;
            float v53 = *(const float*)&s->packed[3];
            int v27 = *(const float*)&s->packed[0] < 0.0f;
            if (*(const float*)&s->packed[1] < 0.0f)
                v27 |= 2;
            if (*(const float*)&s->packed[2] < 0.0f)
                v27 |= 4;
            v49[8] = tw->offsets[v27].v.m128_f32[0];
            v49[9] = tw->offsets[v27].v.m128_f32[1];
            v49[10] = tw->offsets[v27].v.m128_f32[2];
            float v52 = v49[8] * *(const float*)&s->packed[0]
                + v49[9] * *(const float*)&s->packed[1]
                + v49[10] * *(const float*)&s->packed[2];
            float v51 = v49[4] * *(const float*)&s->packed[0]
                + v49[5] * *(const float*)&s->packed[1]
                + v49[6] * *(const float*)&s->packed[2];
            float v50 = v49[0] * *(const float*)&s->packed[0]
                + v49[1] * *(const float*)&s->packed[1]
                + v49[2] * *(const float*)&s->packed[2];
            float v32 = v51 - (v53 - v52);
            float v33 = v50 - (v53 - v52);
            if (v32 <= 0.0f)
            {
                if (v33 > 0.0f)
                {
                    float v39 = v32 - v33;
                    d1 &= 0xFFFF00FF;
                    float v54 = v32 - v33;
                    if ((v32 - v33) >= 0.0f)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\CollisionMgr.cpp";
                        AeAssert::gCurrentLine = 721;
                        AeAssert::gCurrentExpr = "delta < 0.0f";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("old cod assert"))
                            __debugbreak();
                        v13 = delta;
                        fraction = v56;
                        v32 = v51 - (v53 - v52);
                        v39 = v54;
                    }
                    if (v32 > (v39 * fraction))
                    {
                        float v40 = v32 / v39;
                        v56 = v40;
                        if (v13 >= v40)
                            return false;
                        fraction = v40;
                    }
                }
                goto LABEL_45;
            }
            v34 = v32 - v33;
            if (v33 > 0.0f)
            {
                if (v34 <= 0.0f || v33 >= 0.125f)
                    return false;
                d1 &= 0xFFFF00FF;
            }
            v35 = v32 - 0.125f;
            if (v35 > (v34 * v13))
                break;
            if ((d1 >> 24) == 0)
                goto LABEL_35;
        LABEL_45:
            ++v24;
            ++sides;
            if (v24 >= nsides)
                goto LABEL_46;
            continue;
        }
        {
            float v36 = v35 / v34;
            delta = v36;
            if (v36 >= fraction)
                return false;
            v13 = v36;
        }
    LABEL_35:
        v49[12] = *(const float*)&sides->packed[0];
        v49[13] = *(const float*)&sides->packed[1];
        v49[14] = *(const float*)&sides->packed[2];
        v49[15] = *(const float*)&sides->packed[3];
        d1 |= 0x10000;
        goto LABEL_45;
    }
LABEL_46:
    if ((d1 >> 24) != 0)
    {
        float v50 = v49[12] * v49[12] + v49[13] * v49[13]
            + v49[14] * v49[14];
        if (fabsf(sqrtf(v50) - 1.0f) >= 0.0099999998f)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
            AeAssert::gCurrentLine = 744;
            AeAssert::gCurrentExpr = "fabsf(Abs(leadNormal) - 1.0f) < .01f";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(""))
                __debugbreak();
            v13 = delta;
        }
        tw->trace_fraction = v13;
        tw->trace_normal[0] = v49[12];
        tw->trace_normal[1] = v49[13];
        tw->trace_normal[2] = v49[14];
        return true;
    }
    tw->trace_startsolid = 1;
    if ((d1 >> 8 & 0xFF) != 0)
    {
        tw->trace_allsolid = 1;
        tw->trace_fraction = 0.0f;
    }
    return true;
}

// ============================================================================
// collide_brush_velocity_sphere - ea: 0x61B550
// ============================================================================
// ea: 0x0061B550
void collide_brush_velocity_sphere(traceWork_t* tw,
                                   const math::Position3& bmin,
                                   const math::Position3& bmax,
                                   const cdlPlane* sides,
                                   unsigned int nsides)
{
    float v46[16];  // [0]=offset, [4]=end, [8]=start, [12]=bound
    v46[12] = bmin.v.m128_f32[0];
    v46[13] = bmin.v.m128_f32[1];
    v46[14] = bmin.v.m128_f32[2];
    v46[15] = bmin.v.m128_f32[3];
    v46[8] = tw->start.v.m128_f32[0];
    v46[9] = tw->start.v.m128_f32[1];
    v46[10] = tw->start.v.m128_f32[2];
    v46[11] = tw->start.v.m128_f32[3];
    float fraction = tw->trace_fraction;
    float v10 = -1.0f;
    v46[4] = tw->end.v.m128_f32[0];
    v46[5] = tw->end.v.m128_f32[1];
    v46[6] = tw->end.v.m128_f32[2];
    v46[7] = tw->end.v.m128_f32[3];
    v46[0] = tw->sphere_offset.v.m128_f32[0];
    v46[1] = tw->sphere_offset.v.m128_f32[1];
    v46[2] = tw->sphere_offset.v.m128_f32[2];
    v46[3] = tw->sphere_offset.v.m128_f32[3];
    float v14 = 0.0f;
    float delta = 0.0f;
    float v54 = fraction;
    int d1 = 0x10000;
    float bounds[8] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    bounds[1] = 0.0f;
    int v15 = 0;
    while (1)
    {
        float* p_radiusOffset = &tw->sphere_radiusOffset.v.m128_f32[0];
        for (int i = 0; i < 12; i += 4)
        {
            float v18 = ((v46[i + 8] - v46[i + 12]) * v10) - p_radiusOffset[0];
            float v19 = ((v46[i + 4] - v46[i + 12]) * v10) - p_radiusOffset[0];
            if (v18 <= 0.0f)
            {
                if (v19 > 0.0f)
                {
                    d1 &= 0xFFFF00FF;
                    if (v18 > ((v18 - v19) * fraction))
                    {
                        float v23 = v18 / (v18 - v19);
                        v54 = v23;
                        if (v14 >= v23)
                            return;
                        fraction = v23;
                    }
                }
            }
            else
            {
                float v20 = v18 - v19;
                if (v19 > 0.0f)
                {
                    if (v20 <= 0.0f || v19 >= 0.125f)
                        return;
                    d1 &= 0xFFFF00FF;
                }
                float v21 = v18 - 0.125f;
                if (v21 <= (v20 * v14))
                {
                    if ((d1 >> 24) != 0)
                        goto LABEL_17;
                }
                else
                {
                    float v22 = v21 / v20;
                    delta = v22;
                    if (v22 >= fraction)
                        return;
                    v14 = v22;
                }
                bounds[1] = 0.0f;
                d1 |= 0x10000;
                bounds[i + 1] = v10;
            }
        LABEL_17:
            p_radiusOffset += 4;
        }
        if (v15 == 0)
        {
            v10 = 1.0f;
            v46[12] = bmax.v.m128_f32[0];
            v46[13] = bmax.v.m128_f32[1];
            v46[14] = bmax.v.m128_f32[2];
            v46[15] = bmax.v.m128_f32[3];
            v15 = 1;
            continue;
        }
        break;
    }
    float v25 = bounds[4];
    unsigned int v51 = 0;
    if (nsides != 0)
    {
        while (1)
        {
            const cdlPlane* s = sides;
            float v50 = *(const float*)&s->packed[3];
            float v28 = v50 + tw->sphere_radius;
            float v49 = v46[0] * *(const float*)&s->packed[0]
                + v46[1] * *(const float*)&s->packed[1]
                + v46[2] * *(const float*)&s->packed[2];
            float v30[3], v31[3];
            if (v49 <= 0.0f)
            {
                for (int k = 0; k < 3; ++k)
                {
                    v30[k] = v46[8 + k] + v46[k];
                    v31[k] = v46[4 + k] + v46[k];
                }
            }
            else
            {
                for (int k = 0; k < 3; ++k)
                {
                    v30[k] = v46[8 + k] - v46[k];
                    v31[k] = v46[4 + k] - v46[k];
                }
            }
            float v48 = v30[0] * *(const float*)&s->packed[0]
                + v30[1] * *(const float*)&s->packed[1]
                + v30[2] * *(const float*)&s->packed[2];
            float v34 = v48 - v28;
            float v35 = (v31[0] * *(const float*)&s->packed[0]
                         + v31[1] * *(const float*)&s->packed[1]
                         + v31[2] * *(const float*)&s->packed[2]) - v28;
            if (v34 <= 0.0f)
            {
                if (v35 > 0.0f)
                {
                    float v40 = v34 - v35;
                    d1 &= 0xFFFF00FF;
                    float v52 = v34 - v35;
                    if ((v34 - v35) >= 0.0f)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\CollisionMgr.cpp";
                        AeAssert::gCurrentLine = 982;
                        AeAssert::gCurrentExpr = "delta < 0.0f";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("old cod assert"))
                            __debugbreak();
                        v14 = delta;
                        v34 = v48 - v28;
                        v40 = v52;
                    }
                    if (v34 > (v40 * v54))
                    {
                        v54 = v34 / v40;
                        if (v14 >= (v34 / v40))
                            return;
                    }
                }
            }
            else
            {
                float v36 = v34 - v35;
                if (v35 > 0.0f)
                {
                    if (v36 <= 0.0f || v35 >= 0.125f)
                        return;
                    d1 &= 0xFFFF00FF;
                }
                float v37 = v34 - 0.125f;
                if (v37 <= (v36 * v14))
                {
                    if ((d1 >> 24) == 0)
                        goto LABEL_33;
                }
                else
                {
                    float v38 = v37 / v36;
                    delta = v38;
                    if (v38 >= v54)
                        return;
                    v14 = v38;
                LABEL_33:
                    bounds[1] = *(const float*)&s->packed[0];
                    bounds[2] = *(const float*)&s->packed[1];
                    bounds[3] = *(const float*)&s->packed[2];
                    v25 = *(const float*)&s->packed[3];
                    d1 |= 0x10000;
                }
            }
            ++sides;
            if (++v51 >= nsides)
                break;
        }
    }
    if ((d1 >> 24) != 0)
    {
        tw->trace_fraction = v14;
        tw->trace_normal[0] = bounds[1];
        tw->trace_normal[1] = bounds[2];
        tw->trace_normal[2] = bounds[3];
        return;
    }
    tw->trace_startsolid = 1;
    if ((d1 >> 8 & 0xFF) != 0)
    {
        tw->trace_allsolid = 1;
        tw->trace_fraction = 0.0f;
    }
}

// ============================================================================
// collide_velocity_sphere_poly - ea: 0x61BBB0
// ============================================================================
// ea: 0x0061BBB0
bool collide_velocity_sphere_poly(const math::Position3& c0,
                                  math::Position3& c1,
                                  const math::Dir3& ndir, float r,
                                  const math::Position3& v0,
                                  const math::Position3& v1,
                                  const math::Position3& v2,
                                  const math::Vector4& plane,
                                  bool& insolid)
{
    insolid = false;
    float c0c[3] = { c0.v.m128_f32[0], c0.v.m128_f32[1],
                     c0.v.m128_f32[2] };
    float c1c[3] = { c1.v.m128_f32[0], c1.v.m128_f32[1],
                     c1.v.m128_f32[2] };
    float v12[3] = { c1c[0] - c0c[0], c1c[1] - c0c[1],
                     c1c[2] - c0c[2] };
    float v52 = v12[0] * v12[0] + v12[1] * v12[1] + v12[2] * v12[2];
    if (v52 < 0.0000099999997f)
    {
        if (collide_sphere_poly(c0, r, v0, v1, v2, plane))
        {
            insolid = true;
            return true;
        }
        return false;
    }
    float pn[4] = { plane.v.m128_f32[0], plane.v.m128_f32[1],
                    plane.v.m128_f32[2], plane.v.m128_f32[3] };
    float d0 = pn[3]
        + (c0c[0] * pn[0] + c0c[1] * pn[1] + c0c[2] * pn[2]);
    float d1 = pn[3]
        + (c1c[0] * pn[0] + c1c[1] * pn[1] + c1c[2] * pn[2]);
    if (r >= d0 && d0 >= (pn[3] + d1))
    {
        if (collide_sphere_poly(c0, r, v0, v1, v2, plane))
        {
            insolid = true;
            return true;
        }
        d0 = c0c[0] * pn[0] + c0c[1] * pn[1] + c0c[2] * pn[2] + pn[3];
    }
    float ndot = ndir.v.m128_f32[0] * pn[0]
        + ndir.v.m128_f32[1] * pn[1] + ndir.v.m128_f32[2] * pn[2];
    if (ndot >= 0.0f || (0.0f - r) >= d0)
        return false;
    float startp[3] = { c0c[0], c0c[1], c0c[2] };
    math::Position3 hitp;
    if (d0 <= 0.0f)
    {
        if (d0 <= (0.0f - r))
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
            AeAssert::gCurrentLine = 1174;
            AeAssert::gCurrentExpr = "d0 > -r";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(""))
                __debugbreak();
            d0 = c0c[0] * pn[0] + c0c[1] * pn[1] + c0c[2] * pn[2] + pn[3];
        }
        hitp.v.m128_f32[0] = startp[0] + pn[0] * d0;
        hitp.v.m128_f32[1] = startp[1] + pn[1] * d0;
        hitp.v.m128_f32[2] = startp[2] + pn[2] * d0;
    }
    else if (d0 <= r)
    {
        hitp.v.m128_f32[0] = startp[0] - pn[0] * d0;
        hitp.v.m128_f32[1] = startp[1] - pn[1] * d0;
        hitp.v.m128_f32[2] = startp[2] - pn[2] * d0;
        // Edge containment test for the hit point.
        float e0[3] = { v0.v.m128_f32[0] - v1.v.m128_f32[0],
                        v0.v.m128_f32[1] - v1.v.m128_f32[1],
                        v0.v.m128_f32[2] - v1.v.m128_f32[2] };
        float p1[3] = { hitp.v.m128_f32[0] - v1.v.m128_f32[0],
                        hitp.v.m128_f32[1] - v1.v.m128_f32[1],
                        hitp.v.m128_f32[2] - v1.v.m128_f32[2] };
        float e1[3] = { v1.v.m128_f32[0] - v2.v.m128_f32[0],
                        v1.v.m128_f32[1] - v2.v.m128_f32[1],
                        v1.v.m128_f32[2] - v2.v.m128_f32[2] };
        float p2[3] = { hitp.v.m128_f32[0] - v2.v.m128_f32[0],
                        hitp.v.m128_f32[1] - v2.v.m128_f32[1],
                        hitp.v.m128_f32[2] - v2.v.m128_f32[2] };
        float e2[3] = { v2.v.m128_f32[0] - v0.v.m128_f32[0],
                        v2.v.m128_f32[1] - v0.v.m128_f32[1],
                        v2.v.m128_f32[2] - v0.v.m128_f32[2] };
        float pe[3] = { hitp.v.m128_f32[0] - v0.v.m128_f32[0],
                        hitp.v.m128_f32[1] - v0.v.m128_f32[1],
                        hitp.v.m128_f32[2] - v0.v.m128_f32[2] };
        float s0 = (e0[1] * p1[2] - e0[2] * p1[1]) * pn[0]
            + (e0[2] * p1[0] - e0[0] * p1[2]) * pn[1]
            + (e0[0] * p1[1] - e0[1] * p1[0]) * pn[2];
        float s1 = (e1[1] * p2[2] - e1[2] * p2[1]) * pn[0]
            + (e1[2] * p2[0] - e1[0] * p2[2]) * pn[1]
            + (e1[0] * p2[1] - e1[1] * p2[0]) * pn[2];
        float s2 = (e2[1] * pe[2] - e2[2] * pe[1]) * pn[0]
            + (e2[2] * pe[0] - e2[0] * pe[2]) * pn[1]
            + (e2[0] * pe[1] - e2[1] * pe[0]) * pn[2];
        if (s0 >= 0.0f && s1 >= 0.0f && s2 >= 0.0f)
        {
            c1.v = _mm_add_ps(
                hitp.v,
                _mm_mul_ps(plane.v, _mm_set_ss(r + 0.000099999997f)));
            return true;
        }
    }
    else
    {
        // Clamp the moving start toward the plane by r along the normal, then
        // slide along ndir until the plane distance is 0.
        float v26[3] = { startp[0] - pn[0] * r, startp[1] - pn[1] * r,
                         startp[2] - pn[2] * r };
        float v27 = v26[0] * pn[0] + v26[1] * pn[1] + v26[2] * pn[2];
        float t = (pn[3] + v27) * (-1.0f / ndot);
        hitp.v.m128_f32[0] = v26[0] + ndir.v.m128_f32[0] * t;
        hitp.v.m128_f32[1] = v26[1] + ndir.v.m128_f32[1] * t;
        hitp.v.m128_f32[2] = v26[2] + ndir.v.m128_f32[2] * t;
    }
    // Closest point on the triangle to hitp (projected onto the plane).
    math::Position3 closest = calc_closest(hitp, v0, v1, v2);
    float d[3] = { closest.v.m128_f32[0] - startp[0],
                   closest.v.m128_f32[1] - startp[1],
                   closest.v.m128_f32[2] - startp[2] };
    float v52b = d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
    float w[3] = { closest.v.m128_f32[0] - c1c[0],
                   closest.v.m128_f32[1] - c1c[1],
                   closest.v.m128_f32[2] - c1c[2] };
    float w2 = w[0] * w[0] + w[1] * w[1] + w[2] * w[2];
    float b = d[0] * w[0] + d[1] * w[1] + d[2] * w[2];
    float disc = ((r * r) - w2) * v52b + (b * b);
    if (disc <= 0.0f)
        return false;
    float sqrtdisc = sqrtf(disc);
    float v47 = -b - sqrtdisc;
    if ((sqrtdisc - b) < 0.0f || v52b < v47
        || ((v52b * v52b) + 0.000099999997f) <= v47)
        return false;
    float v48 = v47 / v52b;
    if ((0.0f - v48) >= 0.0f)
        c1.v = c0.v;
    else
        c1.v = _mm_add_ps(
            *(__m128*)startp,
            _mm_mul_ps(*(__m128*)d, _mm_set_ss(0.0f - v48)));
    return true;
}
