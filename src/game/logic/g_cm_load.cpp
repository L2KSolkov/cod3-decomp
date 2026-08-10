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
    uint8_t _pad20[0x38 - 0x18];
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
        BspPlane*  mList;   // +0x4C
    } mPlanes;              // +0x48 (InplaceVector<BspPlane>)
    uint8_t _pad50[0x5C - 0x50];
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
    math::Position3 offsets[8];    // +0x60
    float  trace_fraction;         // +0x120
    uint8_t _pad124[4];
    int    trace_contents;         // +0x128
    uint8_t _pad12C[0x13C - 0x12C];
    uint8_t trace_allsolid;        // +0x13C
    uint8_t trace_startsolid;      // +0x13D
    uint8_t _pad13E[0x150 - 0x13E];
    math::Position3 sphere_offset; // +0x150
    uint8_t _pad160[0x170 - 0x160];
    int    sphere_use;             // +0x170
    float  sphere_radius;          // +0x174
};

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
