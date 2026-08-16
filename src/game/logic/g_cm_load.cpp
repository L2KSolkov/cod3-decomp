// ============================================================================
// g_cm_load.cpp - game.o CM_ BSP leaf helpers (cm_load.cpp)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"
#include "core/color.h"

#include <intrin.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <utility>
#include <vector>

extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* desc);  // tl_xboxr
extern void tlMemFree(void* ptr);  // tl_xboxr

// phys_memory_heap - linear frame allocator (16 bytes; verified vs IDA)
struct phys_memory_heap {
    char* m_buffer_start;  // +0x00
    char* m_buffer_end;    // +0x04
    char* m_buffer_cur;    // +0x08
    char* m_user_start;    // +0x0C

    void set_buffer(void* const start, int size, int alignment)
    {
        m_buffer_start = (char*)start;
        m_buffer_end = (char*)start + size;
        m_buffer_cur = (char*)start;
        m_user_start = (char*)start;
    }
};

// ============================================================================
// BSP types (local views; sizes verified against disasm)
// ============================================================================
class BspPlane {
public:
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
    struct BspCell {
        uint8_t _pad[0x3C];
        void*   mMeshFile;   // +0x3C
    };
    InplaceVector<BspCell> mCells;  // +0x18
    uint8_t _pad20[0x38 - 0x20];
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
    int      numClusters;   // +0x58
    int      floodvalid;    // +0x5C
    uint8_t _pad60[0x64 - 0x60];
    float    mins[2];       // +0x64
    uint8_t _pad6C[0x70 - 0x6C];
    float    maxs[2];       // +0x70
    int      checkcount;    // +0x78
};

struct BspTree* g_bspTree;  // ?g_bspTree (game.o 0xF743DC)

// helper for Entity::has_zone_collision (cross-TU)
bool BspTree_CellHasMeshFile(int cell_index)
{
    if (cell_index < 0)
        return false;
    return g_bspTree->mCells.mList[cell_index].mMeshFile != nullptr;
}

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
int CM_PointLeafnum_r(const math::Position3& p, int nodeIndex)
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
            v4 = ((p.v.m128_f32[2] * plane.normal[2])
                  + (p.v.m128_f32[1] * plane.normal[1]))
                + (p.v.m128_f32[0] * plane.normal[0]);
        }
        else
        {
            v4 = p.v.m128_f32[plane.type];
        }
        if ((v4 - plane.dist) >= 0.0f)
            v3 = v3->u.node.children[0];
        else
            v3 = v3->u.node.children[1];
    }
    return (int)(v3 - v2);
}

// ea: 0x00618C50
int CM_PointLeafnum(const math::Position3& p)
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
void CM_BoxLeafnums_r(leafList_s* ll, int nodeIndex)
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
void CM_FloodArea_r(int areaNum, int floodnum)
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
void CM_FloodAreaConnections()
{
    ++g_bspTree->floodvalid;
    int floodnum = 0;
    for (unsigned int v2 = 0; v2 < (unsigned int)g_bspTree->mAreas.mSize; ++v2)
    {
        if (BspAreaAt(v2).floodvalid != g_bspTree->floodvalid)
            CM_FloodArea_r(v2, ++floodnum);
    }
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
phys_memory_heap g_cmgr_allocater;  // ?g_cmgr_allocater@@3Vphys_memory_heap@@A (game.o @ 0x13401D0)
DCGSet* gBoxDCGSet = nullptr;              // ?gBoxDCGSet@@3PAVDCGSet@@A (g.o)
char cmgr_memory_buffer[0x400];            // ?cmgr_memory_buffer@@3PADA (game.o)
extern bool tlScratchpadLocked;            // ?tlScratchpadLocked@@3_NA
bool g_in_cmgr_mem_context;         // ?g_in_cmgr_mem_context@@3_NA

// ============================================================================
// cmgr_mem_ctx_t - ea: 0x65FE20 / 0x65FEF0 (CollisionMgr.cpp)
// ============================================================================
struct cmgr_mem_ctx_t {
    cmgr_mem_ctx_t();
    ~cmgr_mem_ctx_t();
};

// ea: 0x0065FE20
cmgr_mem_ctx_t::cmgr_mem_ctx_t()
{
    if (g_in_cmgr_mem_context)
    {
        AeAssert::gCurrentAuthor = AeAssert::JSV;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 171;
        AeAssert::gCurrentExpr = "!g_in_cmgr_mem_context";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Nested collision memory context!"))
            __debugbreak();
    }
    g_in_cmgr_mem_context = true;
    if (tlScratchpadLocked
        && _tlAssert("c:/cod/code/tl/base/include\\tl_system.h", 294,
                     "!tlScratchpadLocked",
                     "Scratchpad is already locked!"))
        __debugbreak();
    tlScratchpadLocked = true;
    g_cmgr_allocater.set_buffer(cmgr_memory_buffer, 0x400, 1);
}

// ea: 0x0065FEF0
cmgr_mem_ctx_t::~cmgr_mem_ctx_t()
{
    g_cmgr_allocater.m_buffer_start = nullptr;
    g_cmgr_allocater.m_buffer_end = nullptr;
    g_cmgr_allocater.m_buffer_cur = nullptr;
    g_cmgr_allocater.m_user_start = nullptr;
    if (!tlScratchpadLocked
        && _tlAssert("c:/cod/code/tl/base/include\\tl_system.h", 300,
                     "tlScratchpadLocked",
                     "Scratchpad is already unlocked!"))
        __debugbreak();
    g_in_cmgr_mem_context = false;
    tlScratchpadLocked = false;
}

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
struct rtree_visitor_t : public subdivision_visitor {
    // objects (m_buffer +0x10, m_alloc_count +0x414, m_slot_array +0x410)
    uint8_t _pad4[0xC];               // +0x04
    uint8_t objects_m_buffer[0x400];  // +0x10
    int*    objects_m_slot_array;     // +0x410
    int     objects_m_alloc_count;    // +0x414
    uint8_t _pad418[0x8];             // +0x418 (unknown; TODO phys_array_base tail)
    // boxes (m_buffer +0x420, m_alloc_count +0x624, m_slot_array +0x620)
    uint8_t boxes_m_buffer[0x200];    // +0x420
    int*    boxes_m_slot_array;       // +0x620
    int     boxes_m_alloc_count;      // +0x624
    uint8_t _pad628[0x8];             // +0x628 (unknown; TODO)
    // brushes (m_buffer +0x630, m_alloc_count +0x834, m_slot_array +0x830)
    uint8_t brushes_m_buffer[0x200];  // +0x630
    int*    brushes_m_slot_array;     // +0x830
    int     brushes_m_alloc_count;    // +0x834
    uint8_t _pad838[0x8];             // +0x838 (unknown; TODO)
    // patches (m_buffer +0x840, m_alloc_count +0xA44, m_slot_array +0xA40)
    uint8_t patches_m_buffer[0x200];  // +0x840
    int*    patches_m_slot_array;     // +0xA40
    int     patches_m_alloc_count;    // +0xA44
    uint8_t _padA48[0x8];             // +0xA48 (unknown; TODO)
    CGBank* bank;                     // +0xA50
    uint8_t _padA54[0xC];             // +0xA54 (tail padding to 0xA60)

    rtree_visitor_t(const CGBank* _bank);  // ??0rtree_visitor_t@@QAE@PBVCGBank@@@Z (game.o 0x622AE0)
    ~rtree_visitor_t();                  // ??1rtree_visitor_t@@QAE@XZ (game.o 0x661870) - vtable guard
    visit_result_t visit(int index) override;  // ?visit@rtree_visitor_t@@UAE?AW4visit_result_t@@H@Z
    void filter_objects(int mask);       // ?filter_objects@rtree_visitor_t@@QAEXH@Z
    void post_process(int bi, proximity_data_t& proximity_data);  // ?post_process@rtree_visitor_t@@QAEXHAAUproximity_data_t@@@Z (game.o 0x633290)

    static int* add_fast(int* slot, int& count, int capacity)
    {
        if (count >= capacity
            && _tlAssert("c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                         44, "m_alloc_count < m_slot_array_size",
                         "phys_array overflow"))
            __debugbreak();
        int* p = &slot[count];
        ++count;
        return p;
    }
};
static_assert(sizeof(rtree_visitor_t) == 0xA60, "rtree_visitor_t size mismatch");

// CGBank::get_type (cgbank.h inline 0x65FB40)
static int CGBank_get_type(const CGBank* bank, unsigned int index)
{
    if (index >= (unsigned int)bank->objects.m_count)
    {
        AeAssert::gCurrentAuthor = AeAssert::JSV;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
        AeAssert::gCurrentLine = 239;
        AeAssert::gCurrentExpr = "index < size()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    unsigned int nboxes = (unsigned int)bank->nboxes;
    if (index >= nboxes)
        return 2 - (index < nboxes + (unsigned int)bank->nbrushes);
    return 0;
}

// ea: 0x00622AE0
rtree_visitor_t::rtree_visitor_t(const CGBank* _bank)
{
    this->objects_m_alloc_count = 0;
    this->objects_m_slot_array = (int*)this->objects_m_buffer;
    this->boxes_m_alloc_count = 0;
    this->boxes_m_slot_array = (int*)this->boxes_m_buffer;
    this->brushes_m_alloc_count = 0;
    this->brushes_m_slot_array = (int*)this->brushes_m_buffer;
    this->patches_m_alloc_count = 0;
    this->patches_m_slot_array = (int*)this->patches_m_buffer;
    this->bank = (CGBank*)_bank;
    this->objects_m_alloc_count = 0;
    this->boxes_m_alloc_count = 0;
    this->brushes_m_alloc_count = 0;
    this->patches_m_alloc_count = 0;
}

// ea: 0x00661870
rtree_visitor_t::~rtree_visitor_t()
{
    // Binary body: mov [ecx], offset rtree_visitor_t::vftable; ret (vtable guard)
}

// ea: 0x0061AAF0
visit_result_t rtree_visitor_t::visit(int index)
{
    if (this->objects_m_alloc_count != 256)
        *add_fast(this->objects_m_slot_array, this->objects_m_alloc_count,
                  256) = index;
    return CONTINUE_VISITING;
}

// ea: 0x0061AB20
void rtree_visitor_t::filter_objects(int mask)
{
    unsigned int nobjects = (unsigned int)this->objects_m_alloc_count;
    unsigned int oi = 0;
    if (nobjects != 0)
    {
        int v3 = 0;
        while (1)
        {
            if ((v3 < 0 || v3 >= this->objects_m_alloc_count)
                && _tlAssert(
                    "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                    108, "i >= 0 && i < m_alloc_count", ""))
                __debugbreak();
            CGBank* bank = this->bank;
            unsigned int index =
                (unsigned int)this->objects_m_slot_array[v3];
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
                int type = CGBank_get_type(bank, index);
                if (type == 1)
                {
                    if (this->brushes_m_alloc_count == 128)
                        goto LABEL_28;
                    *add_fast(this->brushes_m_slot_array,
                              this->brushes_m_alloc_count, 128) =
                        (int)index;
                }
                else
                {
                    if (type != 0)
                    {
                        if (this->patches_m_alloc_count == 128)
                            goto LABEL_28;
                        *add_fast(this->patches_m_slot_array,
                                  this->patches_m_alloc_count, 128) =
                            (int)index;
                    }
                    else
                    {
                        if (this->boxes_m_alloc_count == 128)
                            goto LABEL_28;
                        *add_fast(this->boxes_m_slot_array,
                                  this->boxes_m_alloc_count, 128) =
                            (int)index;
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
// ea: 0x0060C510 (triangle variant: point-in-triangle + radius projection)
bool can_place_decal(const math::Position3& p, const math::Dir3* verts,
                     const unsigned char* inds, int tid, float decal_radius)
{
    const math::Dir3& v0 = verts[inds[tid]];
    const math::Dir3& v1 = verts[inds[tid + 1]];
    const math::Dir3& v2 = verts[inds[tid + 2]];
    float r2 = decal_radius * decal_radius;

    __m128 e0 = _mm_sub_ps(v1.v, v0.v);
    __m128 d0 = _mm_sub_ps(p.v, v0.v);
    __m128 m0 = _mm_mul_ps(d0, e0);
    float a0 = m0.m128_f32[0] + m0.m128_f32[1] + m0.m128_f32[2];
    __m128 e0s = _mm_mul_ps(e0, e0);
    float e0l = sqrtf(e0s.m128_f32[0] + e0s.m128_f32[1] + e0s.m128_f32[2]);
    __m128 d0s = _mm_mul_ps(d0, d0);
    float dist0 = d0s.m128_f32[0] + d0s.m128_f32[1] + d0s.m128_f32[2];
    bool result = r2 <= dist0 - (a0 / e0l) * (a0 / e0l);

    __m128 e1 = _mm_sub_ps(v2.v, v0.v);
    __m128 m1 = _mm_mul_ps(d0, e1);
    float a1 = m1.m128_f32[0] + m1.m128_f32[1] + m1.m128_f32[2];
    __m128 e1s = _mm_mul_ps(e1, e1);
    float e1l = sqrtf(e1s.m128_f32[0] + e1s.m128_f32[1] + e1s.m128_f32[2]);
    if (r2 > dist0 - (a1 / e1l) * (a1 / e1l))
        result = false;

    __m128 e2 = _mm_sub_ps(v2.v, v1.v);
    __m128 d2 = _mm_sub_ps(p.v, v1.v);
    __m128 m2 = _mm_mul_ps(d2, e2);
    float a2 = m2.m128_f32[0] + m2.m128_f32[1] + m2.m128_f32[2];
    __m128 e2s = _mm_mul_ps(e2, e2);
    float e2l = sqrtf(e2s.m128_f32[0] + e2s.m128_f32[1] + e2s.m128_f32[2]);
    __m128 d2s = _mm_mul_ps(d2, d2);
    float dist2 = d2s.m128_f32[0] + d2s.m128_f32[1] + d2s.m128_f32[2];
    if (r2 > dist2 - (a2 / e2l) * (a2 / e2l))
        return false;
    return result;
}

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
    if (!is_plane_ok(p, n, v18, axis,
                     0.0f - bmin.v.m128_f32[0], decal_radius))
        return false;
    axis.v.m128_f32[0] = 0.0f; axis.v.m128_f32[1] = -1.0f;
    axis.v.m128_f32[2] = 0.0f;
    if (!is_plane_ok(p, n, v18, axis,
                     0.0f - bmin.v.m128_f32[1], decal_radius))
        return false;
    axis.v.m128_f32[1] = 0.0f; axis.v.m128_f32[2] = -1.0f;
    if (!is_plane_ok(p, n, v18, axis,
                     0.0f - bmin.v.m128_f32[2], decal_radius))
        return false;
    axis.v.m128_f32[2] = 0.0f; axis.v.m128_f32[0] = 1.0f;
    if (!is_plane_ok(p, n, v18, axis,
                     bmax.v.m128_f32[0], decal_radius))
        return false;
    axis.v.m128_f32[0] = 0.0f; axis.v.m128_f32[1] = 1.0f;
    if (!is_plane_ok(p, n, v18, axis,
                     bmax.v.m128_f32[1], decal_radius))
        return false;
    axis.v.m128_f32[1] = 0.0f; axis.v.m128_f32[2] = 1.0f;
    if (!is_plane_ok(p, n, v18, axis,
                     bmax.v.m128_f32[2], decal_radius))
        return false;
    for (unsigned int i = 0; i < nsides; ++i)
    {
        float offs = *(const float*)&sides[i].packed[3];
        if (!is_plane_ok(p, n, v18,
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
bool TestPointInBrush(const math::Position3& p, const math::Position3& bmin,
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
extern void traverse_rtree(const math::Position3& p0,
                           const math::Position3& p1,
                           const rtree_root_t& root,
                           subdivision_visitor& visitor);  // physics.o 0x6F6620

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
// collide_sphere_triangle - ea: 0x65BD30 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x0065BD30
bool collide_sphere_triangle(const math::Position3& sphere_center,
                             float sphere_radius,
                             const math::Position3& v0_in,
                             const math::Position3& v1_in,
                             const math::Position3& v2_in,
                             const math::Dir3& normal_in,
                             math::Position3* hitp, math::Dir3* hitn)
{
    __m128 v = sphere_center.v;
    math::Dir3 v9;
    v9.v = _mm_sub_ps(v0_in.v, sphere_center.v);
    __m128 v10 = _mm_mul_ps(v9.v, normal_in.v);
    float v0_12 =
        v10.m128_f32[0]
        + (v10.m128_f32[1] + v10.m128_f32[2]);
    if (v0_12 > 0.0f || (0.0f - sphere_radius) > v0_12)
        return 0;

    math::Dir3 v11;
    math::Dir3 v12;
    v11.v = _mm_sub_ps(v1_in.v, v);
    v12.v = _mm_sub_ps(v2_in.v, v);
    __m128 v13 = _mm_mul_ps(v9.v, v9.v);
    float c0_ =
        v13.m128_f32[0]
        + (v13.m128_f32[1] + v13.m128_f32[2]);
    __m128 v14 = _mm_mul_ps(v9.v, v11.v);
    float v39 =
        v14.m128_f32[0]
        + (v14.m128_f32[1] + v14.m128_f32[2]);
    __m128 v15 = _mm_mul_ps(v9.v, v12.v);
    float c1_ =
        v15.m128_f32[0]
        + (v15.m128_f32[1] + v15.m128_f32[2]);
    float v16 = v39;
    float v17 = c0_;
    float v18 = c1_;
    math::Dir3* v19 = hitn;
    if (v39 >= c0_ && c1_ >= c0_)
    {
        *hitn = v9;
        goto LABEL_27;
    }
    __m128 v20 = _mm_mul_ps(v11.v, v11.v);
    float nhitn_sq =
        v20.m128_f32[0]
        + (v20.m128_f32[1] + v20.m128_f32[2]);
    __m128 v21 = _mm_mul_ps(v11.v, v12.v);
    float v44 =
        v21.m128_f32[0]
        + (v21.m128_f32[1] + v21.m128_f32[2]);
    v17 = nhitn_sq;
    if (v39 >= nhitn_sq && v44 >= nhitn_sq)
    {
        *hitn = v11;
        goto LABEL_27;
    }
    __m128 v22 = _mm_mul_ps(v12.v, v12.v);
    float c1_a =
        v22.m128_f32[0]
        + (v22.m128_f32[1] + v22.m128_f32[2]);
    v17 = c1_a;
    if (v18 >= c1_a && v44 >= c1_a)
    {
        *hitn = v12;
        goto LABEL_27;
    }
    float v24 = c0_;
    if (nhitn_sq >= v39 && c0_ >= v39)
    {
        float v0_8 = c0_ - v39;
        float v40 = nhitn_sq - v39;
        if ((v44 * (c0_ - v16)) + (v18 * (nhitn_sq - v16))
            >= ((nhitn_sq * c0_) - (v16 * v16)))
        {
            float v25 = 1.0f / (v0_8 + v40);
            float v26 = v25 * v0_8;
            v25 = v25 * v40;
            hitn->v = _mm_add_ps(
                _mm_mul_ps(v9.v, _mm_set1_ps(v25)),
                _mm_mul_ps(v11.v, _mm_set1_ps(v26)));
            goto LABEL_26;
        }
        v24 = c0_;
        v17 = c1_a;
        v18 = v44;
    }
    if (v17 >= v18 && v24 >= v18)
    {
        float v0_8a = v24 - v18;
        float v41 = v17 - v18;
        if ((v44 * (v24 - v18)) + (v16 * (v17 - v18))
            >= ((c1_a * c0_) - (v18 * v18)))
        {
            float v27 = 1.0f / (v0_8a + v41);
            float v28 = v27 * v0_8a;
            v27 = v27 * v41;
            hitn->v = _mm_add_ps(
                _mm_mul_ps(v9.v, _mm_set1_ps(v27)),
                _mm_mul_ps(v12.v, _mm_set1_ps(v28)));
            goto LABEL_26;
        }
        v17 = c1_a;
        v18 = v44;
    }
    if (v17 < v18 || nhitn_sq < v18)
    {
        hitp->v = _mm_add_ps(
            sphere_center.v,
            _mm_mul_ps(normal_in.v, _mm_set1_ps(v0_12)));
        *hitn = normal_in;
        return 1;
    }
    {
        float v29 = v17 - v18;
        float v30 = nhitn_sq - v44;
        if ((v18 * (nhitn_sq - v44)) + (v16 * v29)
            < ((v17 * nhitn_sq) - (v44 * v44)))
        {
            hitp->v = _mm_add_ps(
                sphere_center.v,
                _mm_mul_ps(normal_in.v, _mm_set1_ps(v0_12)));
            *hitn = normal_in;
            return 1;
        }
        float v31 = 1.0f / (v30 + v29);
        float v32 = v31 * v30;
        v31 = v31 * v29;
        hitn->v = _mm_add_ps(
            _mm_mul_ps(v11.v, _mm_set1_ps(v31)),
            _mm_mul_ps(v12.v, _mm_set1_ps(v32)));
    }
LABEL_26:
    {
        __m128 v33 = _mm_mul_ps(hitn->v, hitn->v);
        v17 = v33.m128_f32[0]
            + (v33.m128_f32[1] + v33.m128_f32[2]);
    }
LABEL_27:
    if (v17 > 0.000099999997f
        && (sphere_radius * sphere_radius) >= v17)
    {
        hitp->v = _mm_add_ps(sphere_center.v, hitn->v);
        float sq = -sqrtf(v17);
        hitn->v = _mm_div_ps(hitn->v, _mm_set1_ps(sq));
        return 1;
    }
    return 0;
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
    uint8_t _padF4[0xF8 - 0xF4];
    int    contents;               // +0xF8 (visitor filter mask)
    uint8_t isPoint;               // +0xFC
    uint8_t _padFD[0x110 - 0xFD];
    math::Position3 trace_endpos;  // +0x100
    float  trace_normal[4];        // +0x110
    float  trace_fraction;         // +0x120
    int    trace_surfaceFlags;     // +0x124
    int    trace_contents;         // +0x128
    uint8_t _pad12C[0x13C - 0x12C];
    uint8_t trace_allsolid;        // +0x13C
    uint8_t trace_startsolid;      // +0x13D
    uint8_t _pad13E[0x140 - 0x13E];
    float  trace_decal_radius;     // +0x140
    uint8_t trace_check_decal;     // +0x144
    uint8_t _pad145[0x150 - 0x145];
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
    int          axis;                  // +0x00
    int          contentsStaticModels;  // +0x04
    int          contentsEntities;      // +0x08
    float        dist;                  // +0x0C
    EntityShared* entities;             // +0x10
    StaticModel*  staticModels;         // +0x14
    WorldSector* parent;                // +0x18
    WorldSector* child[2];              // +0x1C
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
PartialClipMap pcm;               // ?pcm@@3UPartialClipMap@@A (game.o @ 0x1334E68)

// ea: 0x006199C0
void InitEntitiesBSP()
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
    pcm.worldSectorHead.child[0] = &pcm.dummyNode;
    pcm.worldSectorHead.child[1] = &pcm.dummyNode;
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
            return;
        }
        if (!AeAssert::Assert("Bounds check"))
        {
            pcm.visibility = (uint8_t*)g_bspTree->mVisibility.mList;
            return;
        }
        __debugbreak();
    }
    pcm.visibility = (uint8_t*)g_bspTree->mVisibility.mList;
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
extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file,
                                 int line);  // core.o
char* com_lumpBuf;  // ?com_lumpBuf@@3PADA (game.o)
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
    FS_Read((unsigned char*)&header, 312u, h);
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
            (unsigned int)filelen, 16, "hunk",
            "c:\\cod\\code\\game\\cm_load.cpp", 267);
        FS_Read((unsigned char*)com_lumpBuf, (unsigned int)filelen, h);
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
private:
    BinFileManager();        // ??0BinFileManager@@AAE@XZ
    ~BinFileManager();       // ??1BinFileManager@@AAE@XZ
public:
    int mTotalFiles;         // +0x00
    BinFileEntry mArray[256];  // +0x04
    static BinFileManager* sInst;  // ?sInst@BinFileManager@@2PAV1@A

    void Clear();            // ?Clear@BinFileManager@@QAEXXZ
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pakId);  // ?DecodeBank@BinFileManager@@QAEXPBDPAEHW4TPakId@@@Z
    unsigned char* Find(const char* name);  // ?Find@BinFileManager@@QAEPAEPBD@Z
};
BinFileManager* BinFileManager::sInst = nullptr;

// C-style bridge for cross-TU callers (g_cmd / g_scr_vehicle)
unsigned char* BinFileManager_Find(void* self, const char* name)
{
    return ((BinFileManager*)self)->Find(name);
}

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
void DecodeBin(const char* name, unsigned char* data, int size, TPakId pakId,
               PakFile* pakFile)
{
    BinFileManager::sInst->DecodeBank(name, data, size, pakId);
}

// ============================================================================
// GetLeaves / CM_BoxLeafnums - ea: 0x619050..0x6194A0
// ============================================================================
// ea: 0x00619050
void GetLeaves(leafList_s* ll, int nodeIndex, float& mindist)
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
        if (v22 > 0.0f && mindist > v22)
            mindist = v22;
        if (v21 != 0)
        {
            if (v21 != 3)
                goto LABEL_40;
            GetLeaves(ll, (int)((v6->u.node.children[0] - v30) >> 4),
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
                   const math::Position3& pos, const math::Position3& mins,
                   const math::Position3& maxs, int* list, int listsize,
                   int* lastLeaf)
{
    ++g_bspTree->checkcount;
    leafList_s ll;
    ll.bounds[0].v = mins.v;
    ll.bounds[1].v = maxs.v;
    ll.count = 0;
    ll.maxcount = listsize;
    ll.list = list;
    ll.overflowed = 0;
    ll.lastLeaf = 0;
    float v15 = cached_pos.v.m128_f32[3] - 5.0f;
    if (v15 <= 0.0f)
        goto LABEL_4;
    ll.bounds[0].v = cached_pos.v;
    float dx = cached_pos.v.m128_f32[0] - pos.v.m128_f32[0];
    float dy = cached_pos.v.m128_f32[1] - pos.v.m128_f32[1];
    float dz = cached_pos.v.m128_f32[2] - pos.v.m128_f32[2];
    float dist2 = dx * dx + dy * dy + dz * dz;
    if ((v15 * v15) <= dist2)
    {
    LABEL_4:
        float mindist = 3.4028235e38f;
        GetLeaves(&ll, 0, mindist);
        int result = ll.count;
        float v21 = 0.0f;
        if (ll.count == 1)
            v21 = mindist;
        cached_pos.v.m128_f32[3] = v21;
        if (v21 > 0.0f)
        {
            cached_leaf = *list;
            cached_pos.v.m128_f32[0] = pos.v.m128_f32[0];
            cached_pos.v.m128_f32[1] = pos.v.m128_f32[1];
            cached_pos.v.m128_f32[2] = pos.v.m128_f32[2];
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
        float nlen = VectorNormalize2(*(const math::Dir3*)v6,
                                      *(math::Dir3*)v6);
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
    float nlen = VectorNormalize2(*(const math::Dir3*)vNormal,
                                  *(math::Dir3*)vNormal);
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
// Sight-trace boolean primitives - ea: 0x60CD80..0x60D180
// ============================================================================
// These only test, returning true when a hit is possible; they do not write
// tw->trace state (unlike the Trace* variants above).
static float LengthSq3(__m128 v)
{
    __m128 s = _mm_mul_ps(v, v);
    return s.m128_f32[0] + s.m128_f32[1] + s.m128_f32[2];
}

// ea: 0x0060CD80
void TestCapsuleInCapsule(traceWork_t* tw)
{
    math::Dir3 v1;
    v1.v = tw->sphere_offset.v;
    math::Position3 v2 = tw->start;
    v2.v = _mm_add_ps(tw->start.v, v1.v);
    math::Position3 v3 = tw->start;
    v3.v = _mm_sub_ps(tw->start.v, v1.v);
    math::Position3 v4 = gBoxDCGSet->max;
    math::Position3 v5;
    v5.v = _mm_mul_ps(
        _mm_add_ps(gBoxDCGSet->min.v, v4.v),
        _mm_set1_ps(0.5f));
    math::Position3 p2;
    p2.v = _mm_sub_ps(v4.v, v5.v);
    math::Position3 top = v5;
    float v6 = p2.v.m128_f32[0] > p2.v.m128_f32[2]
        ? p2.v.m128_f32[2]
        : p2.v.m128_f32[0];
    float v7 = p2.v.m128_f32[2] - v6;
    float v10 = (tw->sphere_radius + v6) * (tw->sphere_radius + v6);
    math::Position3 p1;
    p1.v = top.v;
    p1.v.m128_f32[2] = top.v.m128_f32[2] + v7;
    if (v10 > LengthSq3(_mm_sub_ps(p1.v, v2.v)))
    {
        tw->trace_allsolid = 1;
        tw->trace_startsolid = 1;
        tw->trace_fraction = 0.0f;
    }
    if (v10 > LengthSq3(_mm_sub_ps(p1.v, v3.v)))
    {
        tw->trace_allsolid = 1;
        tw->trace_startsolid = 1;
        tw->trace_fraction = 0.0f;
    }
    p2.v = top.v;
    p2.v.m128_f32[2] = top.v.m128_f32[2] - v7;
    if (v10 > LengthSq3(_mm_sub_ps(p2.v, v2.v)))
    {
        tw->trace_allsolid = 1;
        tw->trace_startsolid = 1;
        tw->trace_fraction = 0.0f;
    }
    if (v10 > LengthSq3(_mm_sub_ps(p2.v, v3.v)))
    {
        tw->trace_allsolid = 1;
        tw->trace_startsolid = 1;
        tw->trace_fraction = 0.0f;
    }
    float v17 = tw->start.v.m128_f32[2] - top.v.m128_f32[2];
    float v18 = (tw->sphere_halfheight + v7) - tw->sphere_radius;
    if (v18 >= v17 && v17 >= (0.0f - v18))
    {
        p1.v.m128_f32[2] = 0.0f;
        v2.v.m128_f32[2] = 0.0f;
        if (v10 > LengthSq3(_mm_sub_ps(v2.v, p1.v)))
        {
            tw->trace_allsolid = 1;
            tw->trace_startsolid = 1;
            tw->trace_fraction = 0.0f;
        }
    }
}

// ea: 0x0060CFE0
int SightTraceCylinderThroughCylinder(traceWork_t* tw,
                                      const math::Position3& vStationary,
                                      float fStationaryHalfHeight,
                                      float radius)
{
    float vNormal[4] = {
        tw->start.v.m128_f32[0] - vStationary.v.m128_f32[0],
        tw->start.v.m128_f32[1] - vStationary.v.m128_f32[1],
        tw->start.v.m128_f32[2] - vStationary.v.m128_f32[2],
        0.0f,
    };
    float v5 =
        (vNormal[1] * vNormal[1] + vNormal[0] * vNormal[0])
        - ((tw->sphere_radius + radius) * (tw->sphere_radius + radius));
    if (v5 > 0.0f)
    {
        float fA = (tw->delta.v.m128_f32[1] * vNormal[1])
            + (tw->delta.v.m128_f32[0] * vNormal[0]);
        if (fA < 0.0f)
        {
            float deltaLenSqrd = tw->deltaLenSqrd;
            float disc = (fA * fA) - (deltaLenSqrd * v5);
            if (disc >= 0.0f)
            {
                vNormal[2] = 0.0f;
                float nlen =
                    VectorNormalize2(*(const math::Dir3*)vNormal,
                                     *(math::Dir3*)vNormal);
                float t = fA * 0.125f / nlen;
                float hit = (-fA - sqrtf(disc)) / deltaLenSqrd + t;
                if (tw->trace_fraction > hit)
                {
                    float v10 = (tw->sphere_halfheight - tw->sphere_radius)
                        + fStationaryHalfHeight;
                    float v11 =
                        (((hit - t) * tw->delta.v.m128_f32[2])
                         + tw->start.v.m128_f32[2])
                        - vStationary.v.m128_f32[2];
                    if (v11 <= v10 && (0.0f - v10) <= v11)
                        return 0;
                }
            }
        }
    }
    else
    {
        float v6 = (tw->sphere_halfheight - tw->sphere_radius)
            + fStationaryHalfHeight;
        if (vNormal[2] <= v6 && (0.0f - v6) <= vNormal[2])
            return 0;
    }
    return 1;
}

// ea: 0x0060D180
int SightTraceSphereThroughSphere(traceWork_t* tw,
                                  const math::Position3& vStart,
                                  const math::Position3& vEnd,
                                  const math::Position3& vStationary,
                                  float radius)
{
    float v6[4] = {
        vStart.v.m128_f32[0] - vStationary.v.m128_f32[0],
        vStart.v.m128_f32[1] - vStationary.v.m128_f32[1],
        vStart.v.m128_f32[2] - vStationary.v.m128_f32[2],
        0.0f,
    };
    float fA = v6[0] * v6[0] + v6[1] * v6[1] + v6[2] * v6[2];
    float v8 = fA - ((tw->sphere_radius + radius) * (tw->sphere_radius + radius));
    if (v8 <= 0.0f)
        return false;
    fA = tw->delta.v.m128_f32[0] * v6[0]
        + tw->delta.v.m128_f32[1] * v6[1]
        + tw->delta.v.m128_f32[2] * v6[2];
    if (fA >= 0.0f)
        return true;
    float deltaLenSqrd = tw->deltaLenSqrd;
    float disc = (fA * fA) - (deltaLenSqrd * v8);
    if (disc < 0.0f)
        return true;
    float nlen = VectorNormalize2(*(const math::Dir3*)v6,
                                  *(math::Dir3*)v6);
    float sqrtdisc = sqrtf(disc);
    return tw->trace_fraction
        <= nlen * 0.125f / fA + (-fA - sqrtdisc) / deltaLenSqrd;
}

// ============================================================================
// SightTraceCapsuleThroughCapsule / BoundingBox - ea: 0x61D760 / 0x61D9C0
// ============================================================================
extern DCGSet* TempBoxModel(const math::Position3& mins,
                            const math::Position3& maxs, int contents,
                            int capsule);  // game.o
bool collide_brush_segment(traceWork_t* tw, const math::Position3& bmin,
                           const math::Position3& bmax,
                           const cdlPlane* sides, unsigned int nsides);
    // ea: 0x61B080 (defined below)

// ea: 0x0061D760
int SightTraceCapsuleThroughCapsule(traceWork_t* tw)
{
    math::Position3 v2 = gBoxDCGSet->min;
    math::Position3 v3 = gBoxDCGSet->max;
    // AABB overlap test (inflated by 1 on all axes via bounds[0]-1/bounds[1]+1)
    if ((_mm_movemask_ps(_mm_cmplt_ps(
             _mm_max_ps(
                 _mm_sub_ps(
                     v2.v,
                     _mm_add_ps(tw->bounds[1].v, _mm_set1_ps(1.0f))),
                 _mm_sub_ps(
                     _mm_sub_ps(tw->bounds[0].v, _mm_set1_ps(1.0f)),
                     v3.v)),
             _mm_setzero_ps()))
         & 7) != 7)
        return 0;
    math::Position3 v4;
    v4.v = tw->sphere_offset.v;
    math::Position3 v5;
    v5.v = _mm_add_ps(tw->start.v, v4.v);
    math::Position3 v6;
    v6.v = _mm_sub_ps(tw->start.v, v4.v);
    math::Position3 v7 = tw->end;
    math::Position3 v8;
    v8.v = _mm_add_ps(v7.v, v4.v);
    math::Position3 v9;
    v9.v = _mm_sub_ps(v7.v, v4.v);
    math::Position3 center;
    center.v = _mm_mul_ps(
        _mm_add_ps(v2.v, v3.v), _mm_set1_ps(0.5f));
    math::Position3 half;
    half.v = _mm_sub_ps(v3.v, center.v);
    float v10 = half.v.m128_f32[0] > half.v.m128_f32[2]
        ? half.v.m128_f32[2]
        : half.v.m128_f32[0];
    float v11 = half.v.m128_f32[2] - v10;
    math::Position3 top = center;
    top.v.m128_f32[2] += v11;
    math::Position3 bottom = center;
    bottom.v.m128_f32[2] -= v11;
    if (tw->start.v.m128_f32[2] + v4.v.m128_f32[2] <= top.v.m128_f32[2])
    {
        if (bottom.v.m128_f32[2] > tw->start.v.m128_f32[2] - v4.v.m128_f32[2])
        {
            if (SightTraceSphereThroughSphere(tw, v6, v9, bottom, v10) == 0)
                return -1;
            if (tw->delta.v.m128_f32[2] <= 0.0f)
                return 0;
        }
    }
    else
    {
        if (SightTraceSphereThroughSphere(tw, v5, v8, top, v10) == 0)
            return -1;
        if (tw->delta.v.m128_f32[2] >= 0.0f)
            return 0;
    }
    if (SightTraceCylinderThroughCylinder(tw, center, v11, v10) != 0)
    {
        const math::Position3* p_bottom;
        const math::Position3* v14;
        const math::Position3* v15;
        if (tw->end.v.m128_f32[2] <= top.v.m128_f32[2])
        {
            if (bottom.v.m128_f32[2] <= tw->end.v.m128_f32[2]
                || tw->start.v.m128_f32[2] < bottom.v.m128_f32[2])
                return 0;
            p_bottom = &bottom;
            v14 = &v9;
            v15 = &v6;
        }
        else
        {
            if (top.v.m128_f32[2] < tw->start.v.m128_f32[2])
                return 0;
            p_bottom = &top;
            v14 = &v8;
            v15 = &v5;
        }
        if (SightTraceSphereThroughSphere(tw, *v15, *v14, *p_bottom,
                                          v10) != 0)
            return 0;
    }
    return -1;
}

// ea: 0x0061D9C0
int SightTraceBoundingBoxThroughCapsule(traceWork_t* tw)
{
    math::Position3 v2 = gBoxDCGSet->max;
    math::Position3 center;
    center.v = _mm_mul_ps(
        _mm_add_ps(gBoxDCGSet->min.v, v2.v), _mm_set1_ps(0.5f));
    math::Position3 half;
    half.v = _mm_sub_ps(v2.v, center.v);
    tw->start.v = _mm_sub_ps(tw->start.v, center.v);
    tw->end.v = _mm_sub_ps(tw->end.v, center.v);
    float v5 = half.v.m128_f32[2];
    float v6 = half.v.m128_f32[0] > v5 ? v5 : half.v.m128_f32[0];
    tw->sphere_use = 1;
    tw->sphere_radius = v6;
    tw->sphere_halfheight = v5;
    tw->sphere_offset.v.m128_f32[0] = 0.0f;
    tw->sphere_offset.v.m128_f32[1] = 0.0f;
    tw->sphere_offset.v.m128_f32[2] = v5 - tw->sphere_radius;
    tw->sphere_offset.v.m128_f32[3] = 0.0f;
    tw->sphere_radiusOffset.v.m128_f32[0] = tw->sphere_radius;
    tw->sphere_radiusOffset.v.m128_f32[1] = tw->sphere_radius;
    tw->sphere_radiusOffset.v.m128_f32[2] = tw->sphere_halfheight;
    int v7 = TempBoxModelContents();
    TempBoxModel(*(const math::Position3*)&tw->size[0],
                 *(const math::Position3*)&tw->size[1], v7, 0);
    cdl_object_t* m_elements =
        (cdl_object_t*)gBoxDCGSet->objects_m_elements;
    if (gBoxDCGSet->objects_m_count == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::JSV;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
        AeAssert::gCurrentLine = 77;
        AeAssert::gCurrentExpr = "index < size()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    math::Position3 bmin;
    math::Position3 bmax;
    bmax.v = _mm_add_ps(
        _mm_setr_ps(m_elements->center[0], m_elements->center[1],
                    m_elements->center[2], 0.0f),
        _mm_setr_ps(m_elements->box_radius[0], m_elements->box_radius[1],
                    m_elements->box_radius[2], 0.0f));
    bmin.v = _mm_sub_ps(
        _mm_setr_ps(m_elements->center[0], m_elements->center[1],
                    m_elements->center[2], 0.0f),
        _mm_setr_ps(m_elements->box_radius[0], m_elements->box_radius[1],
                    m_elements->box_radius[2], 0.0f));
    return collide_brush_segment(tw, bmin, bmax, nullptr, 0);
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
    TempBoxModel(*(const math::Position3*)&tw->size[0],
                 *(const math::Position3*)&tw->size[1], v7, 0);
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
DCGSet* TempBoxModel(const math::Position3& mins,
                     const math::Position3& maxs, int contents, int capsule)
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
    if (gBoxDCGSet->objects_m_count == 0
        && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                     "index >= 0 && index < size()", "invalid index"))
        __debugbreak();
    cdl_object_t* m_elements =
        (cdl_object_t*)gBoxDCGSet->objects_m_elements;
    m_elements->center[0] = (mins.v.m128_f32[0] + maxs.v.m128_f32[0])
        * 0.5f;
    m_elements->center[1] = (mins.v.m128_f32[1] + maxs.v.m128_f32[1])
        * 0.5f;
    m_elements->center[2] = (mins.v.m128_f32[2] + maxs.v.m128_f32[2])
        * 0.5f;
    m_elements->box_radius[0] = maxs.v.m128_f32[0] - m_elements->center[0];
    m_elements->box_radius[1] = maxs.v.m128_f32[1] - m_elements->center[1];
    m_elements->box_radius[2] = maxs.v.m128_f32[2] - m_elements->center[2];
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
// TestBoundingBoxInCapsule - ea: 0x623320 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x00623320
void TestBoundingBoxInCapsule(traceWork_t* tw)
{
    math::Position3 v2;
    v2.v = gBoxDCGSet->max.v;
    __m128 v3 = _mm_mul_ps(_mm_add_ps(gBoxDCGSet->min.v, v2.v),
                           _mm_set1_ps(0.5f));
    math::Position3 size[2];
    size[0].v = _mm_sub_ps(v2.v, v3);
    tw->start.v = _mm_sub_ps(tw->start.v, v3);
    tw->end.v = _mm_sub_ps(tw->end.v, v3);
    float v5 = size[0].v.m128_f32[2];
    float v6 = size[0].v.m128_f32[0];
    tw->sphere_use = 1;
    if (v6 > v5)
        v6 = v5;
    tw->sphere_radius = v6;
    tw->sphere_halfheight = v5;
    tw->sphere_offset.v.m128_f32[0] = 0.0f;
    tw->sphere_offset.v.m128_f32[1] = 0.0f;
    tw->sphere_offset.v.m128_f32[2] = v5 - tw->sphere_radius;
    int v7 = TempBoxModelContents();
    TempBoxModel(*(const math::Position3*)&tw->size[0],
                 *(const math::Position3*)&tw->size[1], v7, 0);
    cdl_object_t* objects = (cdl_object_t*)gBoxDCGSet->objects_m_elements;
    {
        AeAssert::gCurrentAuthor = AeAssert::JSV;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
        AeAssert::gCurrentLine = 77;
        AeAssert::gCurrentExpr = "index < size()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
        if (gBoxDCGSet->objects_m_count == 0
            && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                         "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
    }
    const cdl_object_t& obj = objects[0];
    math::Position3 bmax;
    bmax.v = _mm_add_ps(
        _mm_setr_ps(obj.center[0], obj.center[1], obj.center[2], 0.0f),
        _mm_setr_ps(obj.box_radius[0], obj.box_radius[1], obj.box_radius[2],
                    0.0f));
    math::Position3 bmin;
    bmin.v = _mm_sub_ps(
        _mm_setr_ps(obj.center[0], obj.center[1], obj.center[2], 0.0f),
        _mm_setr_ps(obj.box_radius[0], obj.box_radius[1], obj.box_radius[2],
                    0.0f));
    TestBoxInBrush(tw, bmin, bmax, nullptr, 0,
                   (unsigned int)obj.cflags);
}

// ============================================================================
// TestBoxInBox - ea: 0x61D5C0 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x0061D5C0
void TestBoxInBox(traceWork_t* tw, const math::Position3& bmin,
                  const math::Position3& bmax, unsigned int cflags)
{
    if ((__fpclass(tw->start.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(tw->start.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(tw->start.v.m128_f32[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 2179;
        AeAssert::gCurrentExpr =
            "!IS_NAN((tw->start)[0]) && !IS_NAN((tw->start)[1]) && !IS_NAN((tw->start)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((__fpclass(tw->end.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(tw->end.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(tw->end.v.m128_f32[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 2180;
        AeAssert::gCurrentExpr =
            "!IS_NAN((tw->end)[0]) && !IS_NAN((tw->end)[1]) && !IS_NAN((tw->end)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((_mm_movemask_ps(
             _mm_cmplt_ps(
                 _mm_max_ps(
                     _mm_sub_ps(bmin.v, tw->bounds[1].v),
                     _mm_sub_ps(tw->bounds[0].v, bmax.v)),
                 _mm_setzero_ps()))
         & 7) == 7)
    {
        tw->trace_allsolid = 1;
        tw->trace_startsolid = 1;
        tw->trace_fraction = 0.0f;
        tw->trace_contents = (int)cflags;
    }
}

// ============================================================================
// post_process - ea: 0x633290 (CollisionMgr.cpp)
// Fills proximity_data boxes/brushes/polies from the visitor's filtered lists.
// ============================================================================
// ea: 0x00633290
void rtree_visitor_t::post_process(int bi, proximity_data_t& proximity_data)
{
    cmgr_mem_ctx_t ctx;
    math::Position3* cg_verts = alloc_verts();

    int nobjects = this->objects_m_alloc_count;
    for (int ti = 0; ti < nobjects; ++ti)
    {
        if ((ti < 0 || ti >= this->objects_m_alloc_count)
            && _tlAssert(
                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                108, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        unsigned int index = this->objects_m_slot_array[ti];
        CGBank* bank = this->bank;
        if (index >= (unsigned int)bank->objects.m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 233;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (index >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        const cdl_object_t* obj =
            &((cdl_object_t*)bank->objects.m_elements)[index];
        int type = CGBank_get_type(bank, index);
        if (type == 1)
        {
            // brush
            if (proximity_data.brushes_count != 256)
            {
                if (proximity_data.brushes_count >= 256
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                        44, "m_alloc_count < m_slot_array_size",
                        "phys_array overflow"))
                    __debugbreak();
                int c = proximity_data.brushes_count;
                proximity_data.brushes_count = c + 1;
                proxy_obj_t v;
                v.oi = (uint16_t)index;
                v.bi = (uint8_t)bi;
                v.ti = 0xFF;
                proximity_data.brushes_slot[c] = v;
            }
        }
        else if (type != 0)
        {
            // patch
            unsigned int pi = index - (unsigned int)bank->nbrushes
                - (unsigned int)bank->nboxes;
            unpack(*bank, pi, cg_verts);
            if (pi >= (unsigned int)bank->patches.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            cdl_patch_t* patch =
                &((cdl_patch_t*)bank->patches.m_elements)[pi];
            unsigned int first_index = (unsigned int)patch->first_index;
            if (first_index >= (unsigned int)bank->patch_inds.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            const unsigned char* pvi =
                &((const unsigned char*)bank->patch_inds.m_elements)
                    [first_index];
            unsigned int num_inds = (unsigned int)patch->num_inds;
            for (unsigned int k = 0; k < num_inds; ++k)
            {
                math::Position3 v0 = cg_verts[pvi[3 * k + 0]];
                math::Position3 v1 = cg_verts[pvi[3 * k + 1]];
                math::Position3 v2 = cg_verts[pvi[3 * k + 2]];
                math::Position3 vmin;
                math::Position3 vmax;
                vmin.v = _mm_min_ps(v0.v, _mm_min_ps(v1.v, v2.v));
                vmax.v = _mm_max_ps(v0.v, _mm_max_ps(v1.v, v2.v));
                if ((_mm_movemask_ps(_mm_cmplt_ps(
                         _mm_max_ps(
                             _mm_sub_ps(vmin.v, proximity_data.hi.v),
                             _mm_sub_ps(proximity_data.lo.v, vmax.v)),
                         _mm_setzero_ps()))
                     & 7) == 7)
                {
                    if (proximity_data.polies_count != 128)
                    {
                        if (proximity_data.polies_count >= 128
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                                44, "m_alloc_count < m_slot_array_size",
                                "phys_array overflow"))
                            __debugbreak();
                        int c = proximity_data.polies_count;
                        ++proximity_data.polies_count;
                        bounded_proxy_obj_t v;
                        v.oi = (uint16_t)index;
                        v.bi = (uint8_t)bi;
                        v.ti = (uint8_t)k;
                        v.min[0] = vmin.v.m128_f32[0];
                        v.min[1] = vmin.v.m128_f32[1];
                        v.min[2] = vmin.v.m128_f32[2];
                        v.max[0] = vmax.v.m128_f32[0];
                        v.max[1] = vmax.v.m128_f32[1];
                        v.max[2] = vmax.v.m128_f32[2];
                        v.cflags = obj->cflags;
                        proximity_data.polies_slot[c] = v;
                    }
                }
            }
        }
        else if (proximity_data.boxes_count != 256)
        {
            // box
            if (proximity_data.boxes_count >= 256
                && _tlAssert(
                    "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                    44, "m_alloc_count < m_slot_array_size",
                    "phys_array overflow"))
                __debugbreak();
            int c = proximity_data.boxes_count;
            proximity_data.boxes_count = c + 1;
            proxy_obj_t v;
            v.oi = (uint16_t)index;
            v.bi = (uint8_t)bi;
            v.ti = 0xFF;
            proximity_data.boxes_slot[c] = v;
        }
    }
}

// ============================================================================
// TestInLeaf (CGBank) - ea: 0x623530 (CollisionMgr.cpp CGBank leaf sweep)
// ============================================================================
// ea: 0x00623530
void TestInLeaf(traceWork_t* tw, const CGBank* bank,
                const rtree_visitor_t* visitor)
{
    cmgr_mem_ctx_t ctx;
    math::Position3* verts = alloc_verts();

    unsigned int nboxes = (unsigned int)visitor->boxes_m_alloc_count;
    if (nboxes != 0)
    {
        for (unsigned int i = 0; i < nboxes; ++i)
        {
            if (((i & 0x80000000) != 0 || (int)i >= visitor->boxes_m_alloc_count)
                && _tlAssert(
                    "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                    114, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            unsigned int index =
                (unsigned int)visitor->boxes_m_slot_array[i];
            if (index >= (unsigned int)bank->objects.m_count)
            {
                AeAssert::gCurrentAuthor = AeAssert::JSV;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                AeAssert::gCurrentLine = 233;
                AeAssert::gCurrentExpr = "index < size()";
                if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                    __debugbreak();
                if (index >= (unsigned int)bank->objects.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
            }
            cdl_object_t* obj =
                &((cdl_object_t*)bank->objects.m_elements)[index];
            math::Position3 vmin;
            math::Position3 vmax;
            vmax.v = _mm_add_ps(
                _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f),
                _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                            obj->box_radius[2], 0.0f));
            vmin.v = _mm_sub_ps(
                _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f),
                _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                            obj->box_radius[2], 0.0f));
            TestBoxInBox(tw, vmin, vmax, (unsigned int)obj->cflags);
            if (tw->trace_allsolid != 0)
                return;
        }
    }

    unsigned int nbrushes = (unsigned int)visitor->brushes_m_alloc_count;
    if (nbrushes != 0)
    {
        for (unsigned int i = 0; i < nbrushes; ++i)
        {
            if (((i & 0x80000000) != 0
                 || (int)i >= visitor->brushes_m_alloc_count)
                && _tlAssert(
                    "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                    114, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            unsigned int index =
                (unsigned int)visitor->brushes_m_slot_array[i];
            if (index >= (unsigned int)bank->objects.m_count)
            {
                AeAssert::gCurrentAuthor = AeAssert::JSV;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                AeAssert::gCurrentLine = 233;
                AeAssert::gCurrentExpr = "index < size()";
                if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                    __debugbreak();
                if (index >= (unsigned int)bank->objects.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
            }
            cdl_object_t* obj =
                &((cdl_object_t*)bank->objects.m_elements)[index];
            unsigned int brush_index = index - (unsigned int)bank->nboxes;
            if (brush_index >= (unsigned int)bank->brushes.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            cdl_brush_t* brush =
                &((cdl_brush_t*)bank->brushes.m_elements)[brush_index];
            unsigned int first_side = (unsigned int)brush->first_side;
            if (first_side >= (unsigned int)bank->brush_sides.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            const cdlPlane* sides =
                &((const cdlPlane*)bank->brush_sides.m_elements)[first_side];
            math::Position3 vmin;
            math::Position3 vmax;
            vmax.v = _mm_add_ps(
                _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f),
                _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                            obj->box_radius[2], 0.0f));
            vmin.v = _mm_sub_ps(
                _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f),
                _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                            obj->box_radius[2], 0.0f));
            TestBoxInBrush(tw, vmin, vmax, sides,
                           (unsigned int)brush->num_sides,
                           (unsigned int)obj->cflags);
            if (tw->trace_allsolid != 0)
                return;
        }
    }

    if (tw->isPoint != 0)
        return;

    float radius = tw->sphere_radius;
    math::Position3 c;
    c.v = tw->start.v;
    unsigned int npatches = (unsigned int)visitor->patches_m_alloc_count;
    if (npatches != 0)
    {
        for (unsigned int i = 0; i < npatches; ++i)
        {
            if (((i & 0x80000000) != 0
                 || (int)i >= visitor->patches_m_alloc_count)
                && _tlAssert(
                    "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                    114, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            unsigned int index =
                (unsigned int)visitor->patches_m_slot_array[i];
            unsigned int pi = index - (unsigned int)bank->nbrushes
                - (unsigned int)bank->nboxes;
            unpack(*bank, pi, verts);
            if (pi >= (unsigned int)bank->patches.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            cdl_patch_t* patch =
                &((cdl_patch_t*)bank->patches.m_elements)[pi];
            unsigned int first_side = (unsigned int)patch->first_index;
            if (first_side >= (unsigned int)bank->patch_inds.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            const unsigned char* pvi =
                &((const unsigned char*)bank->patch_inds.m_elements)
                    [first_side];
            unsigned int num_inds = (unsigned int)patch->num_inds;
            if (num_inds == 0)
                continue;
            for (unsigned int k = 0; 3 * k < num_inds; ++k)
            {
                math::Position3 v0 = verts[pvi[3 * k + 0]];
                math::Position3 v1 = verts[pvi[3 * k + 1]];
                math::Position3 v2 = verts[pvi[3 * k + 2]];
                math::Vector4 plane = calc_normal(v0, v1, v2);
                if (collide_sphere_poly(c, radius, v0, v1, v2, plane))
                {
                    tw->trace_fraction = 0.0f;
                    tw->trace_allsolid = 1;
                    tw->trace_startsolid = 1;
                    return;
                }
            }
        }
    }
}

// ============================================================================
// collide_sphere - ea: 0x628EB0 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x00628EB0
void collide_sphere(const math::Position3& sphere_center, float sphere_radius,
                    math::Position3* hitp, math::Dir3* hitn, int* hitc)
{
    cmgr_mem_ctx_t ctx;
    math::Position3* cg_verts = alloc_verts();

    math::Position3 radius_vec;
    radius_vec.v = _mm_setr_ps(sphere_radius, sphere_radius, sphere_radius,
                               0.0f);
    math::Position3 lo;
    math::Position3 hi;
    lo.v = _mm_sub_ps(sphere_center.v, radius_vec.v);
    hi.v = _mm_add_ps(sphere_center.v, radius_vec.v);
    *hitc = 0;

    CGBankManager* mgr = (CGBankManager*)CGBankManager::sInst;
    for (int bi = 0; bi < mgr->mCount; ++bi)
    {
        if (bi > 0x62)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        CGBank* bank = mgr->mBankArray[bi];
        if (bank == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
            AeAssert::gCurrentLine = 4205;
            AeAssert::gCurrentExpr = "bank";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid bank"))
                __debugbreak();
        }
        if ((_mm_movemask_ps(_mm_cmplt_ps(
                 _mm_max_ps(_mm_sub_ps(bank->min.v, hi.v),
                            _mm_sub_ps(lo.v, bank->max.v)),
                 _mm_setzero_ps()))
             & 7) != 7)
            continue;

        rtree_visitor_t visitor(bank);
        traverse_rtree(lo, hi, bank->rtree_root, visitor);
        visitor.filter_objects(-1);

        int npatches = visitor.patches_m_alloc_count;
        for (int i = 0; i < npatches; ++i)
        {
            if ((i < 0 || i >= visitor.patches_m_alloc_count)
                && _tlAssert(
                    "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                    108, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            unsigned int index = visitor.patches_m_slot_array[i];
            unsigned int pi = index - (unsigned int)bank->nboxes
                - (unsigned int)bank->nbrushes;
            unpack(*bank, pi, cg_verts);
            if (pi >= (unsigned int)bank->patches.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            cdl_patch_t* patch =
                &((cdl_patch_t*)bank->patches.m_elements)[pi];
            unsigned int first_index = (unsigned int)patch->first_index;
            if (first_index >= (unsigned int)bank->patch_inds.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            const unsigned char* pvi =
                &((const unsigned char*)bank->patch_inds.m_elements)
                    [first_index];
            unsigned int num_inds = (unsigned int)patch->num_inds;
            if (num_inds == 0)
                continue;
            for (unsigned int k = 0; 3 * k < num_inds; ++k)
            {
                math::Position3 v0 = cg_verts[pvi[3 * k + 0]];
                math::Position3 v1 = cg_verts[pvi[3 * k + 1]];
                math::Position3 v2 = cg_verts[pvi[3 * k + 2]];
                math::Vector4 plane = calc_normal(v0, v1, v2);
                if (collide_sphere_triangle(sphere_center, sphere_radius, v0,
                                            v1, v2, *(const math::Dir3*)&plane,
                                            &hitp[*hitc], &hitn[*hitc]))
                {
                    ++*hitc;
                }
            }
        }
    }
}

// ============================================================================
// collide_ray - ea: 0x628AC0 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x00628AC0
bool collide_ray(const math::Position3& p0, const math::Dir3& u0,
                 math::Dir3* normal, float* t_)
{
    cmgr_mem_ctx_t ctx;
    math::Position3* cg_verts = alloc_verts();

    math::Position3 p1;
    p1.v = _mm_add_ps(p0.v, u0.v);
    math::Position3 lo;
    math::Position3 hi;
    lo.v = _mm_min_ps(p0.v, p1.v);
    hi.v = _mm_max_ps(p0.v, p1.v);
    *t_ = 1.0f;
    bool hit = false;

    CGBankManager* mgr = (CGBankManager*)CGBankManager::sInst;
    for (int bi = 0; bi < mgr->mCount; ++bi)
    {
        if (bi > 0x62)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        CGBank* bank = mgr->mBankArray[bi];
        if ((_mm_movemask_ps(_mm_cmplt_ps(
                 _mm_max_ps(_mm_sub_ps(bank->min.v, hi.v),
                            _mm_sub_ps(lo.v, bank->max.v)),
                 _mm_setzero_ps()))
             & 7) != 7)
            continue;

        rtree_visitor_t visitor(bank);
        traverse_rtree(lo, hi, bank->rtree_root, visitor);
        visitor.filter_objects(-1);

        int npatches = visitor.patches_m_alloc_count;
        for (int i = 0; i < npatches; ++i)
        {
            if ((i < 0 || i >= visitor.patches_m_alloc_count)
                && _tlAssert(
                    "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                    108, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            unsigned int index = visitor.patches_m_slot_array[i];
            unsigned int pi = index - (unsigned int)bank->nboxes
                - (unsigned int)bank->nbrushes;
            unpack(*bank, pi, cg_verts);
            if (pi >= (unsigned int)bank->patches.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            cdl_patch_t* patch =
                &((cdl_patch_t*)bank->patches.m_elements)[pi];
            unsigned int first_index = (unsigned int)patch->first_index;
            if (first_index >= (unsigned int)bank->patch_inds.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            const unsigned char* pvi =
                &((const unsigned char*)bank->patch_inds.m_elements)
                    [first_index];
            unsigned int num_inds = (unsigned int)patch->num_inds;
            if (num_inds == 0)
                continue;
            for (unsigned int k = 0; 3 * k < num_inds; ++k)
            {
                math::Position3 v0 = cg_verts[pvi[3 * k + 0]];
                math::Position3 v1 = cg_verts[pvi[3 * k + 1]];
                math::Position3 v2 = cg_verts[pvi[3 * k + 2]];
                math::Vector4 plane = calc_normal(v0, v1, v2);
                __m128 v32 = _mm_mul_ps(plane.v, u0.v);
                float ndot =
                    v32.m128_f32[0]
                    + (v32.m128_f32[1] + v32.m128_f32[2]);
                if (ndot < 0.0f
                    && collide_ray_triangle(p0, u0, v0, v1, v2, *t_, t_))
                {
                    normal->v = plane.v;
                    hit = true;
                }
            }
        }
    }
    return hit;
}

// ============================================================================
// sight_trace_point - ea: 0x6253B0 (CollisionMgr.cpp)
// ============================================================================
extern bool collide_box_segment(traceWork_t* tw,
                                const math::Position3& bmin,
                                const math::Position3& bmax);
// ea: 0x006253B0
bool sight_trace_point(traceWork_t* tw, const math::Position3& p0,
                       const math::Position3& p1)
{
    cmgr_mem_ctx_t ctx;
    math::Position3* verts = alloc_verts();

    math::Dir3 dir;
    dir.v = _mm_sub_ps(p1.v, p0.v);
    __m128 v7 = _mm_mul_ps(dir.v, dir.v);
    float len2 = v7.m128_f32[0] + (v7.m128_f32[1] + v7.m128_f32[2]);
    if (len2 < 0.0099999998f || len2 > 1680999900.0f)
        return 0;

    // 1/sqrt(len2) via Newton iteration (rsqrt approximation)
    float inv_len =
        1597463007 - ((int)(len2 * 0.0000015625f) >> 1);
    inv_len = (1.5f - ((inv_len * inv_len)
                       * ((len2 * 0.0000015625f) * 0.5f)))
        * inv_len;

    math::Dir3 step;
    step.v = _mm_mul_ps(dir.v, _mm_set1_ps(inv_len));

    math::Dir3 ones;
    ones.v = _mm_setr_ps(1.0f, 1.0f, 1.0f, 0.0f);

    math::Position3 cur = p0;
    float frac = 0.0f;
    while (1)
    {
        math::Position3 prev = cur;
        frac += inv_len;
        cur.v = _mm_add_ps(prev.v, step.v);
        if (frac > 1.0f)
            cur = p1;

        math::Position3 lo;
        math::Position3 hi;
        lo.v = _mm_sub_ps(_mm_min_ps(prev.v, cur.v), ones.v);
        hi.v = _mm_add_ps(_mm_max_ps(prev.v, cur.v), ones.v);

        CGBankManager* mgr = (CGBankManager*)CGBankManager::sInst;
        for (int bi = 0; bi < mgr->mCount; ++bi)
        {
            if (bi > 0x62)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 31;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            CGBank* bank = mgr->mBankArray[bi];
            if (!intersect_segment_aabb(prev, cur, lo, hi, bank->min,
                                        bank->max))
                continue;

            rtree_visitor_t visitor(bank);
            traverse_rtree(lo, hi, bank->rtree_root, visitor);
            visitor.filter_objects(tw->contents);

            // Boxes
            int nboxes = visitor.objects_m_alloc_count;
            for (int i = 0; i < nboxes; ++i)
            {
                if ((i < 0 || i >= visitor.objects_m_alloc_count)
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                        108, "i >= 0 && i < m_alloc_count",
                        defaultFileName))
                    __debugbreak();
                unsigned int index =
                    (unsigned int)visitor.objects_m_slot_array[i];
                if (index >= (unsigned int)bank->objects.m_count)
                {
                    AeAssert::gCurrentAuthor = AeAssert::JSV;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                    AeAssert::gCurrentLine = 233;
                    AeAssert::gCurrentExpr = "index < size()";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(defaultFileName))
                        __debugbreak();
                    if (index >= (unsigned int)bank->objects.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                            "index >= 0 && index < size()",
                            "invalid index"))
                        __debugbreak();
                }
                cdl_object_t* obj =
                    &((cdl_object_t*)bank->objects.m_elements)[index];
                math::Position3 bmin;
                math::Position3 bmax;
                bmax.v = _mm_add_ps(
                    _mm_setr_ps(obj->center[0], obj->center[1],
                                obj->center[2], 0.0f),
                    _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                obj->box_radius[2], 0.0f));
                bmin.v = _mm_sub_ps(
                    _mm_setr_ps(obj->center[0], obj->center[1],
                                obj->center[2], 0.0f),
                    _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                obj->box_radius[2], 0.0f));
                collide_box_segment(tw, bmin, bmax);
                if (tw->trace_fraction < 1.0f)
                    return 1;
            }

            // Brushes
            int nbrushes = visitor.brushes_m_alloc_count;
            for (int i = 0; i < nbrushes; ++i)
            {
                if ((i < 0 || i >= visitor.brushes_m_alloc_count)
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                        108, "i >= 0 && i < m_alloc_count",
                        defaultFileName))
                    __debugbreak();
                unsigned int index =
                    (unsigned int)visitor.brushes_m_slot_array[i];
                if (index >= (unsigned int)bank->objects.m_count)
                {
                    AeAssert::gCurrentAuthor = AeAssert::JSV;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                    AeAssert::gCurrentLine = 233;
                    AeAssert::gCurrentExpr = "index < size()";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(defaultFileName))
                        __debugbreak();
                    if (index >= (unsigned int)bank->objects.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                            "index >= 0 && index < size()",
                            "invalid index"))
                        __debugbreak();
                }
                cdl_object_t* obj =
                    &((cdl_object_t*)bank->objects.m_elements)[index];
                unsigned int brush_index =
                    index - (unsigned int)bank->nboxes;
                if (brush_index >= (unsigned int)bank->brushes.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
                cdl_brush_t* brush =
                    &((cdl_brush_t*)bank->brushes.m_elements)[brush_index];
                unsigned int first_side = (unsigned int)brush->first_side;
                if (first_side >= (unsigned int)bank->brush_sides.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
                const cdlPlane* sides =
                    &((const cdlPlane*)bank->brush_sides.m_elements)
                        [first_side];
                math::Position3 bmin;
                math::Position3 bmax;
                bmax.v = _mm_add_ps(
                    _mm_setr_ps(obj->center[0], obj->center[1],
                                obj->center[2], 0.0f),
                    _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                obj->box_radius[2], 0.0f));
                bmin.v = _mm_sub_ps(
                    _mm_setr_ps(obj->center[0], obj->center[1],
                                obj->center[2], 0.0f),
                    _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                obj->box_radius[2], 0.0f));
                collide_brush_segment(tw, bmin, bmax, sides,
                                      (unsigned int)brush->num_sides);
                if (tw->trace_fraction < 1.0f)
                    return 1;
            }

            // Patches
            int npatches = visitor.patches_m_alloc_count;
            for (int i = 0; i < npatches; ++i)
            {
                if ((i < 0 || i >= visitor.patches_m_alloc_count)
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                        108, "i >= 0 && i < m_alloc_count",
                        defaultFileName))
                    __debugbreak();
                unsigned int index =
                    (unsigned int)visitor.patches_m_slot_array[i];
                if (index >= (unsigned int)bank->objects.m_count)
                {
                    AeAssert::gCurrentAuthor = AeAssert::JSV;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                    AeAssert::gCurrentLine = 233;
                    AeAssert::gCurrentExpr = "index < size()";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(defaultFileName))
                        __debugbreak();
                    if (index >= (unsigned int)bank->objects.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                            "index >= 0 && index < size()",
                            "invalid index"))
                        __debugbreak();
                }
                unsigned int pi = index - (unsigned int)bank->nboxes
                    - (unsigned int)bank->nbrushes;
                unpack(*bank, pi, verts);
                if (pi >= (unsigned int)bank->patches.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
                cdl_patch_t* patch =
                    &((cdl_patch_t*)bank->patches.m_elements)[pi];
                unsigned int first_index =
                    (unsigned int)patch->first_index;
                if (first_index >= (unsigned int)bank->patch_inds.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
                if (sight_trace_point_patch(
                        verts,
                        &((const unsigned char*)bank->patch_inds.m_elements)
                            [first_index],
                        patch->num_inds, p0, p1, dir))
                    return 1;
            }
        }
        if (frac >= 1.0f)
            return 0;
    }
}

// ============================================================================
// sight_trace_sphere - ea: 0x625EA0 (CollisionMgr.cpp)
// ============================================================================
extern void collide_box_velocity_sphere(traceWork_t* tw,
                                        const math::Position3& bmin,
                                        const math::Position3& bmax);
extern void collide_brush_velocity_sphere(traceWork_t* tw,
                                          const math::Position3& bmin,
                                          const math::Position3& bmax,
                                          const cdlPlane* sides,
                                          unsigned int nsides);
// ea: 0x00625EA0
bool sight_trace_sphere(traceWork_t* tw)
{
    cmgr_mem_ctx_t ctx;
    math::Position3* verts = alloc_verts();

    math::Position3 c0 = tw->start;
    math::Position3 c1 = tw->end;
    math::Dir3 dir;
    dir.v = _mm_sub_ps(c1.v, c0.v);
    __m128 v5 = _mm_mul_ps(dir.v, dir.v);
    float len2 = v5.m128_f32[0] + (v5.m128_f32[1] + v5.m128_f32[2]);
    if (len2 < 0.01f || len2 > 1680999900.0f)
        return 0;

    float inv_len =
        1597463007 - ((int)(len2 * 0.0000015625f) >> 1);
    inv_len = (1.5f - ((inv_len * inv_len)
                       * ((len2 * 0.0000015625f) * 0.5f)))
        * inv_len;

    math::Dir3 step;
    step.v = _mm_mul_ps(dir.v, _mm_set1_ps(inv_len));
    float seglen = sqrtf(len2);
    math::Dir3 ndir;
    ndir.v = _mm_div_ps(dir.v, _mm_set1_ps(seglen));

    math::Position3 cur = c0;
    float frac = 0.0f;
    while (1)
    {
        math::Position3 prev = cur;
        frac += inv_len;
        cur.v = _mm_add_ps(prev.v, step.v);
        if (frac > 1.0f)
            cur = c1;

        math::Dir3 rad = tw->sphere_radiusOffset;
        math::Position3 lo;
        math::Position3 hi;
        lo.v = _mm_sub_ps(_mm_min_ps(prev.v, cur.v), rad.v);
        hi.v = _mm_add_ps(_mm_max_ps(prev.v, cur.v), rad.v);

        CGBankManager* mgr = (CGBankManager*)CGBankManager::sInst;
        for (int bi = 0; bi < mgr->mCount; ++bi)
        {
            if (bi > 0x62)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 31;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            CGBank* bank = mgr->mBankArray[bi];
            if ((_mm_movemask_ps(_mm_cmplt_ps(
                     _mm_max_ps(_mm_sub_ps(bank->min.v, hi.v),
                                _mm_sub_ps(lo.v, bank->max.v)),
                     _mm_setzero_ps()))
                 & 7) != 7)
                continue;

            rtree_visitor_t visitor(bank);
            traverse_rtree(lo, hi, bank->rtree_root, visitor);
            visitor.filter_objects(tw->contents);

            if (tw->sphere_use != 0)
            {
                // Boxes (velocity sphere)
                int nboxes = visitor.objects_m_alloc_count;
                for (int i = 0; i < nboxes; ++i)
                {
                    if ((i < 0 || i >= visitor.objects_m_alloc_count)
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                            108, "i >= 0 && i < m_alloc_count",
                            defaultFileName))
                        __debugbreak();
                    unsigned int index =
                        (unsigned int)visitor.objects_m_slot_array[i];
                    if (index >= (unsigned int)bank->objects.m_count)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::JSV;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\cgbank.h";
                        AeAssert::gCurrentLine = 233;
                        AeAssert::gCurrentExpr = "index < size()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(defaultFileName))
                            __debugbreak();
                        if (index >= (unsigned int)bank->objects.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                89, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                    }
                    cdl_object_t* obj =
                        &((cdl_object_t*)bank->objects.m_elements)[index];
                    math::Position3 bmin;
                    math::Position3 bmax;
                    bmax.v = _mm_add_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    bmin.v = _mm_sub_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    collide_box_velocity_sphere(tw, bmin, bmax);
                    if (tw->trace_fraction < 1.0f)
                        return 1;
                }
                // Brushes (velocity sphere)
                int nbrushes = visitor.brushes_m_alloc_count;
                for (int i = 0; i < nbrushes; ++i)
                {
                    if ((i < 0 || i >= visitor.brushes_m_alloc_count)
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                            108, "i >= 0 && i < m_alloc_count",
                            defaultFileName))
                        __debugbreak();
                    unsigned int index =
                        (unsigned int)visitor.brushes_m_slot_array[i];
                    if (index >= (unsigned int)bank->objects.m_count)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::JSV;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\cgbank.h";
                        AeAssert::gCurrentLine = 233;
                        AeAssert::gCurrentExpr = "index < size()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(defaultFileName))
                            __debugbreak();
                        if (index >= (unsigned int)bank->objects.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                89, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                    }
                    cdl_object_t* obj =
                        &((cdl_object_t*)bank->objects.m_elements)[index];
                    unsigned int brush_index =
                        index - (unsigned int)bank->nboxes;
                    if (brush_index
                        >= (unsigned int)bank->brushes.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                            "index >= 0 && index < size()",
                            "invalid index"))
                        __debugbreak();
                    cdl_brush_t* brush =
                        &((cdl_brush_t*)bank->brushes.m_elements)
                            [brush_index];
                    unsigned int first_side =
                        (unsigned int)brush->first_side;
                    if (first_side
                        >= (unsigned int)bank->brush_sides.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                            "index >= 0 && index < size()",
                            "invalid index"))
                        __debugbreak();
                    const cdlPlane* sides =
                        &((const cdlPlane*)bank->brush_sides.m_elements)
                            [first_side];
                    math::Position3 bmin;
                    math::Position3 bmax;
                    bmax.v = _mm_add_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    bmin.v = _mm_sub_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    collide_brush_velocity_sphere(
                        tw, bmin, bmax, sides,
                        (unsigned int)brush->num_sides);
                    if (tw->trace_fraction < 1.0f)
                        return 1;
                }
            }
            else
            {
                // Boxes (segment)
                int nboxes = visitor.objects_m_alloc_count;
                for (int i = 0; i < nboxes; ++i)
                {
                    if ((i < 0 || i >= visitor.objects_m_alloc_count)
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                            108, "i >= 0 && i < m_alloc_count",
                            defaultFileName))
                        __debugbreak();
                    unsigned int index =
                        (unsigned int)visitor.objects_m_slot_array[i];
                    if (index >= (unsigned int)bank->objects.m_count)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::JSV;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\cgbank.h";
                        AeAssert::gCurrentLine = 233;
                        AeAssert::gCurrentExpr = "index < size()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(defaultFileName))
                            __debugbreak();
                        if (index >= (unsigned int)bank->objects.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                89, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                    }
                    cdl_object_t* obj =
                        &((cdl_object_t*)bank->objects.m_elements)[index];
                    math::Position3 bmin;
                    math::Position3 bmax;
                    bmax.v = _mm_add_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    bmin.v = _mm_sub_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    collide_box_segment(tw, bmin, bmax);
                    if (tw->trace_fraction < 1.0f)
                        return 1;
                }
                // Brushes (segment)
                int nbrushes = visitor.brushes_m_alloc_count;
                for (int i = 0; i < nbrushes; ++i)
                {
                    if ((i < 0 || i >= visitor.brushes_m_alloc_count)
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                            108, "i >= 0 && i < m_alloc_count",
                            defaultFileName))
                        __debugbreak();
                    unsigned int index =
                        (unsigned int)visitor.brushes_m_slot_array[i];
                    if (index >= (unsigned int)bank->objects.m_count)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::JSV;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\cgbank.h";
                        AeAssert::gCurrentLine = 233;
                        AeAssert::gCurrentExpr = "index < size()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(defaultFileName))
                            __debugbreak();
                        if (index >= (unsigned int)bank->objects.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                89, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                    }
                    cdl_object_t* obj =
                        &((cdl_object_t*)bank->objects.m_elements)[index];
                    unsigned int brush_index =
                        index - (unsigned int)bank->nboxes;
                    if (brush_index
                        >= (unsigned int)bank->brushes.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                            "index >= 0 && index < size()",
                            "invalid index"))
                        __debugbreak();
                    cdl_brush_t* brush =
                        &((cdl_brush_t*)bank->brushes.m_elements)
                            [brush_index];
                    unsigned int first_side =
                        (unsigned int)brush->first_side;
                    if (first_side
                        >= (unsigned int)bank->brush_sides.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                            "index >= 0 && index < size()",
                            "invalid index"))
                        __debugbreak();
                    const cdlPlane* sides =
                        &((const cdlPlane*)bank->brush_sides.m_elements)
                            [first_side];
                    math::Position3 bmin;
                    math::Position3 bmax;
                    bmax.v = _mm_add_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    bmin.v = _mm_sub_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    collide_brush_segment(tw, bmin, bmax, sides,
                                          (unsigned int)brush->num_sides);
                    if (tw->trace_fraction < 1.0f)
                        return 1;
                }
            }

            // Patch sweep (sphere or point with padded radius)
            float r = tw->sphere_use != 0
                ? tw->sphere_radius
                : tw->size[1].v.m128_f32[0];
            float startZ = tw->start.v.m128_f32[2];
            float endZ = tw->end.v.m128_f32[2];
            float zshift = tw->sphere_halfheight - r;
            tw->start.v.m128_f32[2] = startZ - zshift;
            tw->end.v.m128_f32[2] = endZ - zshift;

            math::Dir3 pad;
            pad.v = _mm_setr_ps(r + 0.001f, r + 0.001f, r + 0.001f,
                                0.0f);
            math::Position3 sc0 = tw->start;
            math::Position3 sc1 = tw->end;
            math::Position3 plo;
            math::Position3 phi;
            plo.v = _mm_sub_ps(_mm_min_ps(sc0.v, sc1.v), pad.v);
            phi.v = _mm_add_ps(_mm_max_ps(sc0.v, sc1.v), pad.v);
            math::Dir3 pdir;
            pdir.v = _mm_sub_ps(sc1.v, sc0.v);
            __m128 pv = _mm_mul_ps(pdir.v, pdir.v);
            float plen2 =
                pv.m128_f32[0] + (pv.m128_f32[1] + pv.m128_f32[2]);
            float plen = sqrtf(plen2);
            math::Dir3 pndir;
            pndir.v = _mm_div_ps(pdir.v, _mm_set1_ps(plen));

            int npatches = visitor.patches_m_alloc_count;
            for (int i = 0; i < npatches; ++i)
            {
                if ((i < 0 || i >= visitor.patches_m_alloc_count)
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                        108, "i >= 0 && i < m_alloc_count",
                        defaultFileName))
                    __debugbreak();
                unsigned int index =
                    (unsigned int)visitor.patches_m_slot_array[i];
                if (index >= (unsigned int)bank->objects.m_count)
                {
                    AeAssert::gCurrentAuthor = AeAssert::JSV;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                    AeAssert::gCurrentLine = 233;
                    AeAssert::gCurrentExpr = "index < size()";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(defaultFileName))
                        __debugbreak();
                    if (index >= (unsigned int)bank->objects.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                            "index >= 0 && index < size()",
                            "invalid index"))
                        __debugbreak();
                }
                unsigned int pi = index - (unsigned int)bank->nboxes
                    - (unsigned int)bank->nbrushes;
                unpack(*bank, pi, verts);
                if (pi >= (unsigned int)bank->patches.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
                cdl_patch_t* patch =
                    &((cdl_patch_t*)bank->patches.m_elements)[pi];
                unsigned int first_index =
                    (unsigned int)patch->first_index;
                if (first_index
                    >= (unsigned int)bank->patch_inds.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
                const unsigned char* inds =
                    &((const unsigned char*)bank->patch_inds.m_elements)
                        [first_index];
                unsigned int num_inds = (unsigned int)patch->num_inds;
                if (num_inds == 0)
                    continue;
                for (unsigned int k = 0; 3 * k < num_inds; ++k)
                {
                    math::Position3 v0 = verts[inds[3 * k + 0]];
                    math::Position3 v1 = verts[inds[3 * k + 1]];
                    math::Position3 v2 = verts[inds[3 * k + 2]];
                    math::Vector4 plane = calc_normal(v0, v1, v2);
                    float w = plane.v.m128_f32[3];
                    float dot0 = plane.v.m128_f32[0] * sc0.v.m128_f32[0]
                        + plane.v.m128_f32[1] * sc0.v.m128_f32[1]
                        + plane.v.m128_f32[2] * sc0.v.m128_f32[2];
                    float dot1 = plane.v.m128_f32[0] * sc1.v.m128_f32[0]
                        + plane.v.m128_f32[1] * sc1.v.m128_f32[1]
                        + plane.v.m128_f32[2] * sc1.v.m128_f32[2];
                    float rw = (r + 0.001f) - w;
                    if (dot0 <= rw || dot1 <= rw)
                    {
                        math::Position3 tmin;
                        math::Position3 tmax;
                        tmin.v = _mm_min_ps(v0.v, _mm_min_ps(v1.v, v2.v));
                        tmax.v = _mm_max_ps(v0.v, _mm_max_ps(v1.v, v2.v));
                        if ((_mm_movemask_ps(_mm_cmplt_ps(
                                 _mm_max_ps(
                                     _mm_sub_ps(tmin.v, phi.v),
                                     _mm_sub_ps(plo.v, tmax.v)),
                                 _mm_setzero_ps()))
                             & 7) == 7)
                        {
                            bool insolid = false;
                            if (collide_velocity_sphere_poly(
                                    sc0, sc1, pndir, r + 0.001f, v0, v1, v2,
                                    plane, insolid))
                            {
                                tw->start.v.m128_f32[2] = startZ;
                                tw->end.v.m128_f32[2] = endZ;
                                return 1;
                            }
                        }
                    }
                }
            }
            tw->start.v.m128_f32[2] = startZ;
            tw->end.v.m128_f32[2] = endZ;
        }
        if (frac >= 1.0f)
            return 0;
    }
}

// ============================================================================
// SightTrace / PATH_SightTrace - ea: 0x6288C0 / 0x628980
// ============================================================================
extern cdl_proftimer cdl_proftimer_sight_trace_point;   // game.o @ 0xF44308
extern cdl_proftimer cdl_proftimer_sight_trace_sphere;  // game.o @ 0xF3E960
extern void Com_Memset(void* dest, int val, unsigned int count);  // core.o
// Capsule sphere descriptor (CollisionMgr.h) - 48 bytes, verified vs the
// 12-dword `rep movsd` in SightTrace (0x6343AB) / Trace (0x640F99):
//   { Position3 offset; Dir3 radiusOffset; int use; float radius;
//     float halfheight; }
struct sphere_t {
    math::Position3 offset;  // +0x00
    math::Dir3 radiusOffset; // +0x10
    int    use;              // +0x20
    float  radius;           // +0x24
    float  halfheight;       // +0x28
};
static_assert(sizeof(sphere_t) == 0x30, "sphere_t size mismatch");

// ea: 0x006288C0
bool SightTrace(traceWork_t* tw, const math::Position3& p0,
                const math::Position3& p1)
{
    bool v4;
    if (tw->isPoint != 0)
    {
        cdl_proftimer_sight_trace_point.start();
        v4 = sight_trace_point(tw, p0, p1);
        cdl_proftimer_sight_trace_point.stop();
    }
    else
    {
        cdl_proftimer_sight_trace_sphere.start();
        v4 = sight_trace_sphere(tw);
        cdl_proftimer_sight_trace_sphere.stop();
    }
    return v4;
}

// ea: 0x006340A0 (DCGSet model variant, returns hit num)
extern int SightTraceThroughLeaf(traceWork_t* tw,
                                 const DCGSet* set);  // game.o 0x624070

int SightTrace(int oldHitNum, const math::Position3* start,
               const math::Position3* end, const math::Position3* mins,
               const math::Position3* maxs, DCGSet* model,
               const math::Position3* origin, int brushmask, int capsule,
               void* sphere)
{
    if ((__fpclass(start->v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(start->v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(start->v.m128_f32[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 2215;
        AeAssert::gCurrentExpr =
            "!IS_NAN((start)[0]) && !IS_NAN((start)[1]) && !IS_NAN((start)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((__fpclass(end->v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(end->v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(end->v.m128_f32[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 2216;
        AeAssert::gCurrentExpr =
            "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((__fpclass(mins->v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(mins->v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(mins->v.m128_f32[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 2217;
        AeAssert::gCurrentExpr =
            "!IS_NAN((mins)[0]) && !IS_NAN((mins)[1]) && !IS_NAN((mins)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((__fpclass(maxs->v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(maxs->v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(maxs->v.m128_f32[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 2218;
        AeAssert::gCurrentExpr =
            "!IS_NAN((maxs)[0]) && !IS_NAN((maxs)[1]) && !IS_NAN((maxs)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    traceWork_t tw;
    Com_Memset((unsigned int*)&tw, 0, sizeof(tw));
    tw.trace_fraction = 1.0f;
    __m128 center =
        _mm_mul_ps(_mm_add_ps(mins->v, maxs->v), _mm_set1_ps(0.5f));
    tw.bounds[0].v = _mm_sub_ps(mins->v, center);
    tw.bounds[1].v = _mm_sub_ps(maxs->v, center);
    __m128 v15 = _mm_add_ps(start->v, center);
    __m128 v14 = _mm_add_ps(end->v, center);
    tw.start.v = v15;
    tw.end.v = v14;
    tw.delta.v = _mm_sub_ps(v14, v15);
    __m128 sq = _mm_mul_ps(tw.delta.v, tw.delta.v);
    tw.deltaLenSqrd = sq.m128_f32[0] + sq.m128_f32[1] + sq.m128_f32[2];
    tw.contents = brushmask;
    float v18, v19;
    sphere_t* sp = (sphere_t*)sphere;
    if (sp != nullptr)
    {
        tw.sphere_offset.v = sp->offset.v;
        tw.sphere_use = sp->use;
        tw.sphere_radius = sp->radius;
        tw.sphere_halfheight = sp->halfheight;
        v18 = tw.bounds[1].v.m128_f32[1];
        v19 = tw.bounds[1].v.m128_f32[0];
    }
    else
    {
        tw.sphere_use = capsule;
        v18 = tw.bounds[1].v.m128_f32[1];
        v19 = tw.bounds[1].v.m128_f32[0];
        float v17 = tw.bounds[1].v.m128_f32[0] <= tw.bounds[1].v.m128_f32[1]
            ? tw.bounds[1].v.m128_f32[0]
            : tw.bounds[1].v.m128_f32[1];
        tw.sphere_halfheight = tw.bounds[1].v.m128_f32[1];
        tw.sphere_radius = v17;
        tw.sphere_offset.v.m128_f32[0] = 0.0f;
        tw.sphere_offset.v.m128_f32[1] = 0.0f;
        tw.sphere_offset.v.m128_f32[2] = tw.bounds[1].v.m128_f32[1] - v17;
        tw.sphere_offset.v.m128_f32[3] = 0.0f;
    }
    __m128 v20 = _mm_min_ps(tw.start.v, tw.end.v);
    __m128 v21 = _mm_max_ps(tw.start.v, tw.end.v);
    if (tw.sphere_use != 0)
    {
        __m128 v22 = _mm_add_ps(
            _mm_andnot_ps(_mm_set1_ps(-0.0f), tw.sphere_offset.v),
            _mm_set1_ps(tw.sphere_radius));
        tw.bounds[0].v = _mm_sub_ps(v20, v22);
        tw.bounds[1].v = _mm_add_ps(v21, v22);
    }
    else
    {
        tw.bounds[0].v = _mm_add_ps(v20, tw.bounds[0].v);
        tw.bounds[1].v = _mm_add_ps(v21, tw.bounds[1].v);
    }
    for (int i = 0; i < 8; ++i)
    {
        tw.offsets[i].v.m128_f32[0] =
            (i & 1) ? tw.bounds[1].v.m128_f32[0] : tw.bounds[0].v.m128_f32[0];
        tw.offsets[i].v.m128_f32[1] =
            (i & 2) ? tw.bounds[1].v.m128_f32[1] : tw.bounds[0].v.m128_f32[1];
        tw.offsets[i].v.m128_f32[2] =
            (i & 4) ? tw.bounds[1].v.m128_f32[2] : tw.bounds[0].v.m128_f32[2];
        tw.offsets[i].v.m128_f32[3] = 0.0f;
    }
    tw.isPoint =
        (tw.bounds[1].v.m128_f32[2] + tw.bounds[1].v.m128_f32[1]
         + tw.bounds[1].v.m128_f32[0]) == 0.0f;
    if (model == nullptr)
        return SightTrace(&tw, tw.start, tw.end) ? 1 : 0;
    if (model->id != 4094)
        return (int)SightTraceThroughLeaf(&tw, model);
    if ((TempBoxModelContents() & brushmask) == 0)
        return 0;
    if (tw.sphere_use != 0)
        return SightTraceCapsuleThroughCapsule(&tw);
    return SightTraceBoundingBoxThroughCapsule(&tw);
}

// ea: 0x00628980
int PATH_SightTrace(const math::Position3& start, const math::Position3& end)
{
    traceWork_t tw;
    Com_Memset((unsigned int*)&tw, 0, sizeof(tw));
    tw.start.v = start.v;
    tw.end.v = end.v;
    tw.trace_fraction = 1.0f;
    memset(&tw.bounds[0], 0, sizeof(tw.bounds));
    tw.delta.v = _mm_sub_ps(end.v, start.v);
    __m128 sq = _mm_mul_ps(tw.delta.v, tw.delta.v);
    tw.deltaLenSqrd = sq.m128_f32[0] + sq.m128_f32[1] + sq.m128_f32[2];
    tw.contents = 0x2800003;
    tw.bounds[0].v = _mm_min_ps(start.v, end.v);
    tw.bounds[1].v = _mm_max_ps(start.v, end.v);
    return sight_trace_point(&tw, start, end) ? 1 : 0;
}

// ============================================================================
// Trace / TraceSphere / TracePoint - ea: 0x640E90..0x641880 (CollisionMgr.cpp)
// ============================================================================
extern void TestBoundingBoxInCapsule(traceWork_t* tw);   // game.o
extern void PositionTest(traceWork_t* tw);               // game.o
extern void PositionTest(traceWork_t* tw,
                         const proximity_data_t& data);  // game.o
extern void TraceSphereThroughLeaf(traceWork_t* tw,
                                   const DCGSet* set);   // game.o
extern void TracePointThroughLeaf(traceWork_t* tw,
                                  const DCGSet* set);    // game.o
extern void TestInLeaf(traceWork_t* tw, const DCGSet* set);  // game.o
extern bool collide_velocity_sphere(traceWork_t* tw,
                                    const proximity_data_t& data);  // game.o
extern cdl_proftimer cdl_proftimer_trace_sphere_list;  // game.o @ 0xF3C308
extern cdl_proftimer cdl_proftimer_trace_point_list;   // game.o @ 0xF456B0
extern int TempBoxModelContents();                     // game.o
extern void TestCapsuleInCapsule(traceWork_t* tw);     // game.o
extern void TraceCapsuleThroughCapsule(traceWork_t* tw);   // game.o
extern void TraceBoundingBoxThroughCapsule(traceWork_t* tw);// game.o
extern void TraceThroughTree(traceWork_t* tw,
                             const math::Position3& p0,
                             const math::Position3& p1);  // game.o

// ea: 0x00640E90
void Trace(trace_t* results, const math::Position3& start,
           const math::Position3& end, const math::Position3& mins,
           const math::Position3& maxs, DCGSet* model, int brushmask,
           int capsule, sphere_t* sphere)
{
    if (sphere == nullptr)
        results->fraction = 1.0f;
    traceWork_t tw;
    Com_Memset((unsigned int*)&tw, 0, sizeof(tw));
    tw.trace_fraction = results->fraction;
    __m128 center =
        _mm_mul_ps(_mm_add_ps(mins.v, maxs.v), _mm_set1_ps(0.5f));
    tw.bounds[0].v = _mm_sub_ps(mins.v, center);
    tw.bounds[1].v = _mm_sub_ps(maxs.v, center);
    __m128 v15 = _mm_add_ps(start.v, center);
    __m128 v14 = _mm_add_ps(end.v, center);
    tw.start.v = v15;
    tw.end.v = v14;
    tw.delta.v = _mm_sub_ps(v14, v15);
    __m128 sq = _mm_mul_ps(tw.delta.v, tw.delta.v);
    tw.deltaLenSqrd = sq.m128_f32[0] + sq.m128_f32[1] + sq.m128_f32[2];
    tw.contents = brushmask;
    tw.trace_decal_radius = results->decal_radius;
    tw.trace_check_decal = results->check_decal;
    int v21;
    float v18, v19, v20;
    if (sphere != nullptr)
    {
        tw.sphere_offset.v = sphere->offset.v;
        tw.sphere_radius = sphere->radius;
        tw.sphere_halfheight = sphere->halfheight;
        tw.sphere_use = 1;
        v21 = 1;
        v19 = tw.bounds[1].v.m128_f32[1];
        v20 = tw.bounds[1].v.m128_f32[0];
        v18 = tw.sphere_radius;
    }
    else
    {
        v21 = capsule;
        tw.sphere_use = capsule;
        v19 = tw.bounds[1].v.m128_f32[1];
        v20 = tw.bounds[1].v.m128_f32[0];
        v18 = tw.bounds[1].v.m128_f32[0] <= tw.bounds[1].v.m128_f32[1]
            ? tw.bounds[1].v.m128_f32[0]
            : tw.bounds[1].v.m128_f32[1];
        tw.sphere_halfheight = tw.bounds[1].v.m128_f32[1];
        tw.sphere_radius = v18;
        tw.sphere_offset.v.m128_f32[0] = 0.0f;
        tw.sphere_offset.v.m128_f32[1] = 0.0f;
        tw.sphere_offset.v.m128_f32[2] = tw.bounds[1].v.m128_f32[1] - v18;
        tw.sphere_offset.v.m128_f32[3] = 0.0f;
    }
    __m128 v22 = _mm_min_ps(tw.start.v, tw.end.v);
    __m128 v23 = _mm_max_ps(tw.start.v, tw.end.v);
    __m128 v24 = _mm_add_ps(
        _mm_andnot_ps(_mm_set1_ps(-0.0f), tw.sphere_offset.v),
        _mm_set1_ps(v18));
    tw.bounds[0].v = _mm_sub_ps(v22, v24);
    tw.bounds[1].v = _mm_add_ps(v23, v24);
    // offsets[0..7] (+0x60) = 8 corner offsets from bounds (size[0] +0x40,
    // size[1] +0x50): corners at (b0/b1 per axis), mirroring the original
    // q1-style corner table. The disasm only reads +0x60/+0x70/... when
    // capsule==0 (bounds box). Keep them as the 8 combinations.
    for (int i = 0; i < 8; ++i)
    {
        tw.offsets[i].v.m128_f32[0] =
            (i & 1) ? tw.bounds[1].v.m128_f32[0]
                    : tw.bounds[0].v.m128_f32[0];
        tw.offsets[i].v.m128_f32[1] =
            (i & 2) ? tw.bounds[1].v.m128_f32[1]
                    : tw.bounds[0].v.m128_f32[1];
        tw.offsets[i].v.m128_f32[2] =
            (i & 4) ? tw.bounds[1].v.m128_f32[2]
                    : tw.bounds[0].v.m128_f32[2];
        tw.offsets[i].v.m128_f32[3] = 0.0f;
    }
    bool zeroSize = start.v.m128_f32[0] == end.v.m128_f32[0]
        && start.v.m128_f32[1] == end.v.m128_f32[1]
        && start.v.m128_f32[2] == end.v.m128_f32[2];
    if (zeroSize)
    {
        if (model != nullptr)
        {
            if (model->id == 4094)
            {
                if ((TempBoxModelContents() & brushmask) != 0)
                {
                    if (v21 != 0)
                        TestCapsuleInCapsule(&tw);
                    else
                        TestBoundingBoxInCapsule(&tw);
                }
            }
            else
            {
                TestInLeaf(&tw, model);
            }
        }
        else
        {
            PositionTest(&tw);
        }
    }
    else
    {
        tw.isPoint =
            (tw.bounds[1].v.m128_f32[2]
             + tw.bounds[1].v.m128_f32[1]
             + tw.bounds[1].v.m128_f32[0]) == 0.0f;
        if (model != nullptr)
        {
            if (model->id == 4094)
            {
                if ((TempBoxModelContents() & brushmask) != 0)
                {
                    if (v21 != 0)
                        TraceCapsuleThroughCapsule(&tw);
                    else
                        TraceBoundingBoxThroughCapsule(&tw);
                }
            }
            else if (v21 != 0)
            {
                TraceSphereThroughLeaf(&tw, model);
            }
            else
            {
                TracePointThroughLeaf(&tw, model);
            }
        }
        else
        {
            TraceThroughTree(&tw, tw.start, tw.end);
        }
    }
    tw.trace_endpos.v = _mm_add_ps(
        start.v,
        _mm_mul_ps(tw.delta.v, _mm_set1_ps(tw.trace_fraction)));
    if ((__fpclass(tw.trace_endpos.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(tw.trace_endpos.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(tw.trace_endpos.v.m128_f32[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 1458;
        AeAssert::gCurrentExpr =
            "!IS_NAN((tw.trace.endpos)[0]) && !IS_NAN((tw.trace.endpos)[1]) && !IS_NAN((tw.trace.endpos)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((__fpclass(tw.trace_normal[0]) & 0x297) != 0
        || (__fpclass(tw.trace_normal[1]) & 0x297) != 0
        || (__fpclass(tw.trace_normal[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 1459;
        AeAssert::gCurrentExpr =
            "!IS_NAN((tw.trace.normal)[0]) && !IS_NAN((tw.trace.normal)[1]) && !IS_NAN((tw.trace.normal)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((__fpclass(tw.trace_fraction) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 1460;
        AeAssert::gCurrentExpr = "!IS_NAN(tw.trace.fraction)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    results->endpos = tw.trace_endpos;
    results->normal.v = _mm_loadu_ps(tw.trace_normal);
    results->fraction = tw.trace_fraction;
    results->surfaceFlags = tw.trace_surfaceFlags;
    results->contents = tw.trace_contents;
    results->allsolid = tw.trace_allsolid != 0;
    results->startsolid = tw.trace_startsolid != 0;
}

// ============================================================================
// TraceSphere / TracePoint - ea: 0x641470 / 0x641880 (CollisionMgr.cpp)
// ============================================================================

// ea: 0x00641470
void TraceSphere(const proximity_data_t& data, trace_t* results,
                 const math::Position3& start, const math::Position3& end,
                 const math::Position3& mins, const math::Position3& maxs,
                 int brushmask)
{
    cdl_proftimer_trace_sphere_list.start();
    results->fraction = 1.0f;
    traceWork_t tw;
    Com_Memset((unsigned int*)&tw, 0, sizeof(tw));
    tw.trace_fraction = results->fraction;
    __m128 center =
        _mm_mul_ps(_mm_add_ps(mins.v, maxs.v), _mm_set1_ps(0.5f));
    tw.bounds[0].v = _mm_sub_ps(mins.v, center);
    tw.bounds[1].v = _mm_sub_ps(maxs.v, center);
    __m128 v12 = _mm_add_ps(start.v, center);
    __m128 v13 = _mm_add_ps(end.v, center);
    tw.start.v = v12;
    tw.end.v = v13;
    tw.delta.v = _mm_sub_ps(v13, v12);
    __m128 sq = _mm_mul_ps(tw.delta.v, tw.delta.v);
    tw.deltaLenSqrd = sq.m128_f32[0] + sq.m128_f32[1] + sq.m128_f32[2];
    tw.contents = brushmask;
    tw.sphere_use = 1;
    float v15 = tw.bounds[1].v.m128_f32[0] > tw.bounds[1].v.m128_f32[1]
        ? tw.bounds[1].v.m128_f32[1]
        : tw.bounds[1].v.m128_f32[0];
    tw.sphere_halfheight = tw.bounds[1].v.m128_f32[1];
    tw.sphere_radius = v15;
    tw.sphere_offset.v.m128_f32[0] = 0.0f;
    tw.sphere_offset.v.m128_f32[1] = 0.0f;
    tw.sphere_offset.v.m128_f32[2] = tw.bounds[1].v.m128_f32[1] - v15;
    tw.sphere_offset.v.m128_f32[3] = 0.0f;
    __m128 v16 = _mm_min_ps(tw.start.v, tw.end.v);
    __m128 v17 = _mm_max_ps(tw.start.v, tw.end.v);
    __m128 v18 = _mm_add_ps(
        _mm_andnot_ps(_mm_set1_ps(-0.0f), tw.sphere_offset.v),
        _mm_set1_ps(v15));
    tw.bounds[0].v = _mm_sub_ps(v16, v18);
    tw.bounds[1].v = _mm_add_ps(v17, v18);
    tw.isPoint =
        (tw.bounds[1].v.m128_f32[2] + tw.bounds[1].v.m128_f32[1]
         + tw.bounds[1].v.m128_f32[0]) == 0.0f;
    bool zeroSize = start.v.m128_f32[0] == end.v.m128_f32[0]
        && start.v.m128_f32[1] == end.v.m128_f32[1]
        && start.v.m128_f32[2] == end.v.m128_f32[2];
    if (zeroSize)
        PositionTest(&tw, data);
    else
        collide_velocity_sphere(&tw, data);
    math::Position3 endpos;
    endpos.v = _mm_add_ps(
        start.v,
        _mm_mul_ps(tw.delta.v, _mm_set1_ps(tw.trace_fraction)));
    if ((__fpclass(endpos.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(endpos.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(endpos.v.m128_f32[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 1521;
        AeAssert::gCurrentExpr =
            "!IS_NAN((tw.trace.endpos)[0]) && !IS_NAN((tw.trace.endpos)[1]) && !IS_NAN((tw.trace.endpos)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((__fpclass(tw.trace_normal[0]) & 0x297) != 0
        || (__fpclass(tw.trace_normal[1]) & 0x297) != 0
        || (__fpclass(tw.trace_normal[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 1522;
        AeAssert::gCurrentExpr =
            "!IS_NAN((tw.trace.normal)[0]) && !IS_NAN((tw.trace.normal)[1]) && !IS_NAN((tw.trace.normal)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((__fpclass(tw.trace_fraction) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 1523;
        AeAssert::gCurrentExpr = "!IS_NAN(tw.trace.fraction)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    results->endpos = endpos;
    results->normal.v = _mm_loadu_ps(tw.trace_normal);
    results->fraction = tw.trace_fraction;
    results->surfaceFlags = tw.trace_surfaceFlags;
    results->contents = tw.trace_contents;
    results->allsolid = tw.trace_allsolid != 0;
    results->startsolid = tw.trace_startsolid != 0;
    cdl_proftimer_trace_sphere_list.stop();
}

// ea: 0x00641880
void TracePoint(const proximity_data_t& data, trace_t* results,
                const math::Position3& start, const math::Position3& end,
                int brushmask)
{
    cdl_proftimer_trace_point_list.start();
    results->fraction = 1.0f;
    traceWork_t tw;
    Com_Memset((unsigned int*)&tw, 0, sizeof(tw));
    tw.contents = brushmask;
    tw.start.v = start.v;
    tw.end.v = end.v;
    tw.trace_fraction = 1.0f;
    tw.delta.v = _mm_sub_ps(end.v, start.v);
    __m128 sq = _mm_mul_ps(tw.delta.v, tw.delta.v);
    tw.deltaLenSqrd = sq.m128_f32[0] + sq.m128_f32[1] + sq.m128_f32[2];
    tw.bounds[0].v = _mm_min_ps(start.v, end.v);
    tw.bounds[1].v = _mm_max_ps(start.v, end.v);
    tw.isPoint = 1;
    cdl_cinfo1 cinfo;
    int sflags = 0;
    int cflags = 0;
    if (collide_segment(data, &tw, start, end, cinfo, sflags, cflags)
        && tw.trace_fraction > sqrtf(
            (cinfo.pi.v.m128_f32[0] - start.v.m128_f32[0])
                * (cinfo.pi.v.m128_f32[0] - start.v.m128_f32[0])
            + (cinfo.pi.v.m128_f32[1] - start.v.m128_f32[1])
                * (cinfo.pi.v.m128_f32[1] - start.v.m128_f32[1])
            + (cinfo.pi.v.m128_f32[2] - start.v.m128_f32[2])
                * (cinfo.pi.v.m128_f32[2] - start.v.m128_f32[2]))
            / sqrtf(tw.deltaLenSqrd))
    {
        tw.trace_surfaceFlags = sflags;
        tw.trace_contents = cflags;
        __m128 a = _mm_mul_ps(cinfo.pi.v, cinfo.ni.v);
        float proj0 = a.m128_f32[0] + a.m128_f32[1] + a.m128_f32[2];
        float neg = -proj0;
        __m128 b = _mm_mul_ps(start.v, cinfo.ni.v);
        float proj1 = b.m128_f32[0] + b.m128_f32[1] + b.m128_f32[2];
        __m128 c = _mm_mul_ps(end.v, cinfo.ni.v);
        float proj2 = c.m128_f32[0] + c.m128_f32[1] + c.m128_f32[2];
        tw.trace_normal[0] = cinfo.ni.v.m128_f32[0];
        tw.trace_normal[1] = cinfo.ni.v.m128_f32[1];
        tw.trace_normal[2] = cinfo.ni.v.m128_f32[2];
        tw.trace_normal[3] = cinfo.ni.v.m128_f32[3];
        tw.trace_fraction = (proj1 + neg - 0.125f)
            / (proj1 + neg - (proj2 + neg));
    }
    math::Position3 endpos;
    endpos.v = _mm_add_ps(
        start.v,
        _mm_mul_ps(tw.delta.v, _mm_set1_ps(tw.trace_fraction)));
    if ((__fpclass(endpos.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(endpos.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(endpos.v.m128_f32[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 1590;
        AeAssert::gCurrentExpr =
            "!IS_NAN((tw.trace.endpos)[0]) && !IS_NAN((tw.trace.endpos)[1]) && !IS_NAN((tw.trace.endpos)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((__fpclass(tw.trace_normal[0]) & 0x297) != 0
        || (__fpclass(tw.trace_normal[1]) & 0x297) != 0
        || (__fpclass(tw.trace_normal[2]) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 1591;
        AeAssert::gCurrentExpr =
            "!IS_NAN((tw.trace.normal)[0]) && !IS_NAN((tw.trace.normal)[1]) && !IS_NAN((tw.trace.normal)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if ((__fpclass(tw.trace_fraction) & 0x297) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 1592;
        AeAssert::gCurrentExpr = "!IS_NAN(tw.trace.fraction)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
            __debugbreak();
    }
    results->endpos = endpos;
    results->normal.v = _mm_loadu_ps(tw.trace_normal);
    results->fraction = tw.trace_fraction;
    results->surfaceFlags = tw.trace_surfaceFlags;
    results->contents = tw.trace_contents;
    results->allsolid = tw.trace_allsolid != 0;
    results->startsolid = tw.trace_startsolid != 0;
    cdl_proftimer_trace_point_list.stop();
}

static cdl_proftimer cdl_proftimer_vsphere_poly;   // game.o @ 0xF439B8
static cdl_proftimer cdl_proftimer_vsphere_patch;  // game.o @ 0xF3BFE0
extern cdl_proftimer cdl_proftimer_temp0;          // game.o @ 0xF4EB18

// ============================================================================
// collide_velocity_sphere - ea: 0x627250 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x00627250
bool collide_velocity_sphere(traceWork_t* tw)
{
    cmgr_mem_ctx_t ctx;
    math::Position3* cg_verts = alloc_verts();

    math::Position3 c0 = tw->start;
    math::Position3 c1 = tw->end;
    math::Dir3 dir;
    dir.v = _mm_sub_ps(c1.v, c0.v);
    __m128 v5 = _mm_mul_ps(dir.v, dir.v);
    float len2 = v5.m128_f32[0] + (v5.m128_f32[1] + v5.m128_f32[2]);
    if (len2 < 0.001f || len2 > 1680999900.0f)
        return false;

    // 1/sqrt(len2) via Newton iteration (rsqrt approximation)
    float inv_len =
        1597463007 - ((int)(len2 * 0.0000015625f) >> 1);
    inv_len = (1.5f - ((inv_len * inv_len)
                       * ((len2 * 0.0000015625f) * 0.5f)))
        * inv_len;

    math::Dir3 step;
    step.v = _mm_mul_ps(dir.v, _mm_set1_ps(inv_len));
    float seglen = sqrtf(len2);
    math::Dir3 ndir;
    ndir.v = _mm_div_ps(dir.v, _mm_set1_ps(seglen));

    bool res = false;
    cdl_object_t* hitObj = nullptr;
    math::Position3 cur = c0;
    float frac = 0.0f;
    for (;;)
    {
        math::Position3 prev = cur;
        frac += inv_len;
        cur.v = _mm_add_ps(prev.v, step.v);
        if (frac > 1.0f)
            cur = c1;

        math::Dir3 rad = tw->sphere_radiusOffset;
        math::Position3 lo;
        math::Position3 hi;
        lo.v = _mm_sub_ps(_mm_min_ps(prev.v, cur.v), rad.v);
        hi.v = _mm_add_ps(_mm_max_ps(prev.v, cur.v), rad.v);

        CGBankManager* mgr = (CGBankManager*)CGBankManager::sInst;
        for (int bi = 0; bi < mgr->mCount; ++bi)
        {
            if (bi > 0x62)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 31;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            CGBank* bank = mgr->mBankArray[bi];
            if ((_mm_movemask_ps(_mm_cmplt_ps(
                     _mm_max_ps(_mm_sub_ps(bank->min.v, hi.v),
                                _mm_sub_ps(lo.v, bank->max.v)),
                     _mm_setzero_ps()))
                 & 7) != 7)
                continue;

            rtree_visitor_t visitor(bank);
            traverse_rtree(lo, hi, bank->rtree_root, visitor);
            visitor.filter_objects(tw->contents);

            if (tw->sphere_use != 0)
            {
                int nboxes = visitor.objects_m_alloc_count;
                for (int i = 0; i < nboxes; ++i)
                {
                    if ((i < 0 || i >= visitor.objects_m_alloc_count)
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                            108, "i >= 0 && i < m_alloc_count",
                            defaultFileName))
                        __debugbreak();
                    unsigned int index =
                        (unsigned int)visitor.objects_m_slot_array[i];
                    if (index >= (unsigned int)bank->objects.m_count)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::JSV;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\cgbank.h";
                        AeAssert::gCurrentLine = 233;
                        AeAssert::gCurrentExpr = "index < size()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(defaultFileName))
                            __debugbreak();
                        if (index >= (unsigned int)bank->objects.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                89, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                    }
                    cdl_object_t* obj =
                        &((cdl_object_t*)bank->objects.m_elements)[index];
                    math::Position3 bmin;
                    math::Position3 bmax;
                    bmax.v = _mm_add_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    bmin.v = _mm_sub_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    collide_box_velocity_sphere(tw, bmin, bmax);
                    if (tw->trace_fraction == 0.0f)
                    {
                        hitObj = obj;
                        goto HIT_OBJ;
                    }
                }
                int nbrushes = visitor.brushes_m_alloc_count;
                for (int i = 0; i < nbrushes; ++i)
                {
                    if ((i < 0 || i >= visitor.brushes_m_alloc_count)
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                            108, "i >= 0 && i < m_alloc_count",
                            defaultFileName))
                        __debugbreak();
                    unsigned int index =
                        (unsigned int)visitor.brushes_m_slot_array[i];
                    if (index >= (unsigned int)bank->objects.m_count)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::JSV;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\cgbank.h";
                        AeAssert::gCurrentLine = 233;
                        AeAssert::gCurrentExpr = "index < size()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(defaultFileName))
                            __debugbreak();
                        if (index >= (unsigned int)bank->objects.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                89, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                    }
                    cdl_object_t* obj =
                        &((cdl_object_t*)bank->objects.m_elements)[index];
                    unsigned int brush_index =
                        index - (unsigned int)bank->nboxes;
                    if (brush_index
                        >= (unsigned int)bank->brushes.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                            "index >= 0 && index < size()",
                            "invalid index"))
                        __debugbreak();
                    cdl_brush_t* brush =
                        &((cdl_brush_t*)bank->brushes.m_elements)
                            [brush_index];
                    unsigned int first_side =
                        (unsigned int)brush->first_side;
                    if (first_side
                        >= (unsigned int)bank->brush_sides.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                            "index >= 0 && index < size()",
                            "invalid index"))
                        __debugbreak();
                    const cdlPlane* sides =
                        &((const cdlPlane*)bank->brush_sides.m_elements)
                            [first_side];
                    math::Position3 bmin;
                    math::Position3 bmax;
                    bmax.v = _mm_add_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    bmin.v = _mm_sub_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    collide_brush_velocity_sphere(
                        tw, bmin, bmax, sides,
                        (unsigned int)brush->num_sides);
                    if (tw->trace_fraction == 0.0f)
                    {
                        hitObj = obj;
                        goto HIT_OBJ;
                    }
                }
            }
            else
            {
                int nboxes = visitor.objects_m_alloc_count;
                for (int i = 0; i < nboxes; ++i)
                {
                    if ((i < 0 || i >= visitor.objects_m_alloc_count)
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                            108, "i >= 0 && i < m_alloc_count",
                            defaultFileName))
                        __debugbreak();
                    unsigned int index =
                        (unsigned int)visitor.objects_m_slot_array[i];
                    if (index >= (unsigned int)bank->objects.m_count)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::JSV;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\cgbank.h";
                        AeAssert::gCurrentLine = 233;
                        AeAssert::gCurrentExpr = "index < size()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(defaultFileName))
                            __debugbreak();
                        if (index >= (unsigned int)bank->objects.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                89, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                    }
                    cdl_object_t* obj =
                        &((cdl_object_t*)bank->objects.m_elements)[index];
                    math::Position3 bmin;
                    math::Position3 bmax;
                    bmax.v = _mm_add_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    bmin.v = _mm_sub_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    collide_box_segment(tw, bmin, bmax);
                    if (tw->trace_fraction == 0.0f)
                    {
                        hitObj = obj;
                        goto HIT_OBJ;
                    }
                }
                int nbrushes = visitor.brushes_m_alloc_count;
                for (int i = 0; i < nbrushes; ++i)
                {
                    if ((i < 0 || i >= visitor.brushes_m_alloc_count)
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                            108, "i >= 0 && i < m_alloc_count",
                            defaultFileName))
                        __debugbreak();
                    unsigned int index =
                        (unsigned int)visitor.brushes_m_slot_array[i];
                    if (index >= (unsigned int)bank->objects.m_count)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::JSV;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\cgbank.h";
                        AeAssert::gCurrentLine = 233;
                        AeAssert::gCurrentExpr = "index < size()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(defaultFileName))
                            __debugbreak();
                        if (index >= (unsigned int)bank->objects.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                89, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                    }
                    cdl_object_t* obj =
                        &((cdl_object_t*)bank->objects.m_elements)[index];
                    unsigned int brush_index =
                        index - (unsigned int)bank->nboxes;
                    if (brush_index
                        >= (unsigned int)bank->brushes.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                            "index >= 0 && index < size()",
                            "invalid index"))
                        __debugbreak();
                    cdl_brush_t* brush =
                        &((cdl_brush_t*)bank->brushes.m_elements)
                            [brush_index];
                    unsigned int first_side =
                        (unsigned int)brush->first_side;
                    if (first_side
                        >= (unsigned int)bank->brush_sides.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                            "index >= 0 && index < size()",
                            "invalid index"))
                        __debugbreak();
                    const cdlPlane* sides =
                        &((const cdlPlane*)bank->brush_sides.m_elements)
                            [first_side];
                    math::Position3 bmin;
                    math::Position3 bmax;
                    bmax.v = _mm_add_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    bmin.v = _mm_sub_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    collide_brush_segment(tw, bmin, bmax, sides,
                                          (unsigned int)brush->num_sides);
                    if (tw->trace_fraction == 0.0f)
                    {
                        hitObj = obj;
                        goto HIT_OBJ;
                    }
                }
            }

            // Patch sweep (sphere or point with padded radius)
            float r = tw->sphere_use != 0
                ? tw->sphere_radius
                : tw->size[1].v.m128_f32[0];
            float startZ = tw->start.v.m128_f32[2];
            float endZ = tw->end.v.m128_f32[2];
            float zshift = tw->sphere_halfheight - r;
            tw->start.v.m128_f32[2] = startZ - zshift;
            tw->end.v.m128_f32[2] = endZ - zshift;

            math::Dir3 pad;
            pad.v = _mm_setr_ps(r + 0.001f, r + 0.001f, r + 0.001f,
                                0.0f);
            math::Position3 sc0 = tw->start;
            math::Position3 sc1 = tw->end;
            math::Position3 plo;
            math::Position3 phi;
            plo.v = _mm_sub_ps(_mm_min_ps(sc0.v, sc1.v), pad.v);
            phi.v = _mm_add_ps(_mm_max_ps(sc0.v, sc1.v), pad.v);
            math::Dir3 pdir;
            pdir.v = _mm_sub_ps(sc1.v, sc0.v);
            __m128 pv = _mm_mul_ps(pdir.v, pdir.v);
            float plen2 =
                pv.m128_f32[0] + (pv.m128_f32[1] + pv.m128_f32[2]);
            float plen = sqrtf(plen2);
            math::Dir3 pndir;
            pndir.v = _mm_div_ps(pdir.v, _mm_set1_ps(plen));

            int npatches = visitor.patches_m_alloc_count;
            for (int i = 0; i < npatches; ++i)
            {
                if ((i < 0 || i >= visitor.patches_m_alloc_count)
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                        108, "i >= 0 && i < m_alloc_count",
                        defaultFileName))
                    __debugbreak();
                unsigned int index =
                    (unsigned int)visitor.patches_m_slot_array[i];
                if (index >= (unsigned int)bank->objects.m_count)
                {
                    AeAssert::gCurrentAuthor = AeAssert::JSV;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                    AeAssert::gCurrentLine = 233;
                    AeAssert::gCurrentExpr = "index < size()";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(defaultFileName))
                        __debugbreak();
                    if (index >= (unsigned int)bank->objects.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                            "index >= 0 && index < size()",
                            "invalid index"))
                        __debugbreak();
                }
                unsigned int pi = index - (unsigned int)bank->nboxes
                    - (unsigned int)bank->nbrushes;
                unpack(*bank, pi, cg_verts);
                if (pi >= (unsigned int)bank->patches.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
                cdl_patch_t* patch =
                    &((cdl_patch_t*)bank->patches.m_elements)[pi];
                unsigned int first_index =
                    (unsigned int)patch->first_index;
                if (first_index
                    >= (unsigned int)bank->patch_inds.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
                const unsigned char* inds =
                    &((const unsigned char*)bank->patch_inds.m_elements)
                        [first_index];
                unsigned int num_inds = (unsigned int)patch->num_inds;
                if (num_inds == 0)
                    continue;
                cdl_object_t* obj =
                    &((cdl_object_t*)bank->objects.m_elements)[index];
                for (unsigned int k = 0; 3 * k < num_inds; ++k)
                {
                    math::Position3 v0 = cg_verts[inds[3 * k + 0]];
                    math::Position3 v1 = cg_verts[inds[3 * k + 1]];
                    math::Position3 v2 = cg_verts[inds[3 * k + 2]];
                    math::Vector4 plane = calc_normal(v0, v1, v2);
                    float w = plane.v.m128_f32[3];
                    float dot0 = plane.v.m128_f32[0] * sc0.v.m128_f32[0]
                        + plane.v.m128_f32[1] * sc0.v.m128_f32[1]
                        + plane.v.m128_f32[2] * sc0.v.m128_f32[2];
                    float dot1 = plane.v.m128_f32[0] * sc1.v.m128_f32[0]
                        + plane.v.m128_f32[1] * sc1.v.m128_f32[1]
                        + plane.v.m128_f32[2] * sc1.v.m128_f32[2];
                    float rw = (r + 0.001f) - w;
                    if (dot0 <= rw || dot1 <= rw)
                    {
                        math::Position3 tmin;
                        math::Position3 tmax;
                        tmin.v = _mm_min_ps(v0.v, _mm_min_ps(v1.v, v2.v));
                        tmax.v = _mm_max_ps(v0.v, _mm_max_ps(v1.v, v2.v));
                        if ((_mm_movemask_ps(_mm_cmplt_ps(
                                 _mm_max_ps(
                                     _mm_sub_ps(tmin.v, phi.v),
                                     _mm_sub_ps(plo.v, tmax.v)),
                                 _mm_setzero_ps()))
                             & 7) == 7)
                        {
                            cdl_proftimer_vsphere_poly.start();
                            bool insolid = false;
                            bool polyHit = collide_velocity_sphere_poly(
                                sc0, sc1, pndir, r + 0.001f, v0, v1, v2,
                                plane, insolid);
                            cdl_proftimer_vsphere_poly.stop();
                            if (polyHit)
                            {
                                math::Dir3 d;
                                d.v = _mm_sub_ps(sc1.v, sc0.v);
                                __m128 d2 = _mm_mul_ps(d.v, d.v);
                                float dist2 = d2.m128_f32[0]
                                    + (d2.m128_f32[1] + d2.m128_f32[2]);
                                float rr = sqrtf(dist2) / seglen;
                                if (insolid || rr <= 0.0000099999997f)
                                {
                                    tw->trace_normal[0] =
                                        plane.v.m128_f32[0];
                                    tw->trace_normal[1] =
                                        plane.v.m128_f32[1];
                                    tw->trace_normal[2] =
                                        plane.v.m128_f32[2];
                                    tw->trace_normal[3] =
                                        plane.v.m128_f32[3];
                                    tw->trace_fraction = 0.0f;
                                    tw->trace_startsolid = 1;
                                    tw->trace_surfaceFlags = obj->sflags;
                                    tw->trace_contents = obj->cflags;
                                    tw->start.v.m128_f32[2] = startZ;
                                    tw->end.v.m128_f32[2] = endZ;
                                    cdl_proftimer_vsphere_patch.stop();
                                    return true;
                                }
                                if (rr <= tw->trace_fraction)
                                {
                                    tw->trace_normal[0] =
                                        plane.v.m128_f32[0];
                                    tw->trace_normal[1] =
                                        plane.v.m128_f32[1];
                                    tw->trace_normal[2] =
                                        plane.v.m128_f32[2];
                                    tw->trace_normal[3] =
                                        plane.v.m128_f32[3];
                                    tw->trace_fraction =
                                        rr - 0.0000099999997f;
                                    tw->trace_surfaceFlags = obj->sflags;
                                    tw->trace_contents = obj->cflags;
                                    res = true;
                                    float dlen = sqrtf(dist2);
                                    pndir.v =
                                        _mm_div_ps(d.v, _mm_set1_ps(dlen));
                                }
                            }
                        }
                    }
                }
            }
            tw->start.v.m128_f32[2] = startZ;
            tw->end.v.m128_f32[2] = endZ;
        }
        if (frac >= 1.0f)
            return res;
    }

HIT_OBJ:
    tw->trace_surfaceFlags = hitObj->sflags;
    tw->trace_contents = hitObj->cflags;
    return true;
}

// ============================================================================
// PositionTest - ea: 0x633770 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x00633770
void PositionTest(traceWork_t* tw)
{
    math::Position3 lo;
    math::Position3 hi;
    lo.v = _mm_sub_ps(_mm_add_ps(tw->start.v, tw->size[0].v),
                      _mm_setr_ps(1.0f, 1.0f, 1.0f, 0.0f));
    hi.v = _mm_add_ps(_mm_add_ps(tw->start.v, tw->size[1].v),
                      _mm_setr_ps(1.0f, 1.0f, 1.0f, 0.0f));

    CGBankManager* mgr = (CGBankManager*)CGBankManager::sInst;
    for (int i = 0; i < mgr->mCount; ++i)
    {
        if (i > 0x62)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        CGBank* bank = mgr->mBankArray[i];
        if ((_mm_movemask_ps(_mm_cmplt_ps(
                 _mm_max_ps(_mm_sub_ps(bank->min.v, hi.v),
                            _mm_sub_ps(lo.v, bank->max.v)),
                 _mm_setzero_ps()))
             & 7) != 7)
            continue;
        rtree_visitor_t visitor(bank);
        traverse_rtree(lo, hi, bank->rtree_root, visitor);
        visitor.filter_objects(tw->contents);
        TestInLeaf(tw, bank, &visitor);
        if (tw->trace_allsolid != 0)
            return;
    }
}

// ============================================================================
// PositionTest (proximity) - ea: 0x633900 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x00633900
void PositionTest(traceWork_t* tw, const proximity_data_t& data)
{
    int nboxes = data.boxes_count;
    for (int i = 0; i < nboxes; ++i)
    {
        if ((i < 0 || i >= data.boxes_count)
            && _tlAssert(
                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                114, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        const proxy_obj_t& slot = data.boxes_slot[i];
        int bi = slot.bi;
        if (bi >= 99)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        CGBank* bank =
            ((CGBankManager*)CGBankManager::sInst)->mBankArray[bi];
        unsigned int oi = slot.oi;
        if (oi >= (unsigned int)bank->objects.m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 233;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (oi >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        cdl_object_t* obj =
            &((cdl_object_t*)bank->objects.m_elements)[oi];
        math::Position3 bmin;
        math::Position3 bmax;
        bmax.v = _mm_add_ps(
            _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                        0.0f),
            _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                        obj->box_radius[2], 0.0f));
        bmin.v = _mm_sub_ps(
            _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                        0.0f),
            _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                        obj->box_radius[2], 0.0f));
        TestBoxInBox(tw, bmin, bmax, (unsigned int)obj->cflags);
        if (tw->trace_allsolid != 0)
            return;
    }

    int nbrushes = data.brushes_count;
    for (int i = 0; i < nbrushes; ++i)
    {
        if ((i < 0 || i >= data.brushes_count)
            && _tlAssert(
                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                114, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        const proxy_obj_t& slot = data.brushes_slot[i];
        int bi = slot.bi;
        if (bi >= 99)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        CGBank* bank =
            ((CGBankManager*)CGBankManager::sInst)->mBankArray[bi];
        unsigned int oi = slot.oi;
        if (oi >= (unsigned int)bank->objects.m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 233;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (oi >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        cdl_object_t* obj =
            &((cdl_object_t*)bank->objects.m_elements)[oi];
        unsigned int brush_index = oi - (unsigned int)bank->nboxes;
        if (brush_index >= (unsigned int)bank->brushes.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        cdl_brush_t* brush =
            &((cdl_brush_t*)bank->brushes.m_elements)[brush_index];
        unsigned int first_side = (unsigned int)brush->first_side;
        if (first_side >= (unsigned int)bank->brush_sides.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        math::Position3 bmin;
        math::Position3 bmax;
        bmax.v = _mm_add_ps(
            _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                        0.0f),
            _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                        obj->box_radius[2], 0.0f));
        bmin.v = _mm_sub_ps(
            _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                        0.0f),
            _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                        obj->box_radius[2], 0.0f));
        TestBoxInBrush(tw, bmin, bmax,
                       &((const cdlPlane*)bank->brush_sides.m_elements)
                           [first_side],
                       (unsigned int)brush->num_sides,
                       (unsigned int)obj->cflags);
        if (tw->trace_allsolid != 0)
            return;
    }

    int npolies = data.polies_count;
    for (int i = 0; i < npolies; ++i)
    {
        if ((i < 0 || i >= data.polies_count)
            && _tlAssert(
                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                114, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        const bounded_proxy_obj_t& slot = data.polies_slot[i];
        int bi = slot.bi;
        if (bi >= 99)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        CGBank* bank =
            ((CGBankManager*)CGBankManager::sInst)->mBankArray[bi];
        unsigned int oi = slot.oi;
        if (oi >= (unsigned int)bank->objects.m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 233;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (oi >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        unsigned int pi =
            oi - (unsigned int)bank->nbrushes - (unsigned int)bank->nboxes;
        if (pi >= (unsigned int)bank->patches.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        if (pi >= (unsigned int)bank->gjk_patches.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        const cdl_vinfo_t* vinfo =
            &((const cdl_vinfo_t*)bank->gjk_patches.m_elements)[pi];
        unsigned int ind =
            (unsigned int)((cdl_patch_t*)bank->patches.m_elements)[pi]
                .first_index
            + 3 * (unsigned int)slot.ti;
        if (ind >= (unsigned int)bank->patch_inds.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        math::Position3 v0;
        math::Position3 v1;
        math::Position3 v2;
        unpack_poly(
            bank, vinfo,
            &((const unsigned char*)bank->patch_inds.m_elements)[ind], v0,
            v1, v2);
        math::Vector4 plane = calc_normal(v0, v1, v2);
        if (collide_sphere_poly(tw->start, tw->sphere_radius, v0, v1, v2,
                                plane))
        {
            tw->trace_allsolid = 1;
            tw->trace_startsolid = 1;
            tw->trace_fraction = 0.0f;
            break;
        }
    }
}

// ============================================================================
// collide_box_segment (math) - ea: 0x60C720 (CollisionMgr.cpp slab sweep)
// ============================================================================
// ea: 0x0060C720
bool collide_box_segment(const math::Position3& p0,
                         const math::Position3& p1,
                         const math::Position3& bmin,
                         const math::Position3& bmax, float& t,
                         math::Position3* normal)
{
    float v7 = t;             // max distance
    float enter = 0.0f;       // enter fraction
    float sign = -1.0f;
    math::Position3 nrm;
    nrm.v = _mm_setzero_ps();
    int allInside = 1;        // v21
    int setNormal = 0;        // v22
    int pass = 0;             // v20[40]

    while (2)
    {
        const math::Position3* bounds =
            pass == 0 ? &bmin : &bmax;
        for (int i = 0; i < 12; i += 4)
        {
            int axis = i / 4;
            float p0d = (p0.v.m128_f32[axis] - bounds->v.m128_f32[axis])
                * sign;
            float p1d = (p1.v.m128_f32[axis] - bounds->v.m128_f32[axis])
                * sign;
            if (p0d <= 0.0f)
            {
                if (p1d > 0.0f)
                {
                    float v18 = p0d - p1d;
                    allInside = 0;
                    if (p0d > v18 * v7)
                    {
                        float tt = p0d / v18;
                        if (enter >= tt)
                            return false;
                        v7 = tt;
                    }
                }
            }
            else
            {
                float v15 = p0d - p1d;
                if (p1d > 0.0f)
                {
                    if (v15 <= 0.0f || p1d >= 0.125f)
                        return false;
                    allInside = 0;
                }
                float v16 = p0d - 0.125f;
                if (v16 > v15 * enter)
                {
                    float tt = v16 / v15;
                    if (tt >= v7)
                        return false;
                    enter = tt;
                }
                else if (setNormal != 0)
                {
                    continue;
                }
                nrm.v = _mm_setzero_ps();
                nrm.v.m128_f32[axis] = sign;
                setNormal = 1;
            }
        }
        if (pass != 0)
            break;
        sign = 1.0f;
        pass = 1;
    }
    if (setNormal != 0)
    {
        t = enter;
        if (normal != nullptr)
            *normal = nrm;
    }
    else
    {
        if (allInside != 0)
            t = 0.0f;
        if (normal != nullptr)
        {
            normal->v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
            return true;
        }
    }
    return true;
}

// ============================================================================
// collide_box_segment (traceWork) - ea: 0x60C900 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x0060C900
bool collide_box_segment(traceWork_t* tw, const math::Position3& bmin,
                         const math::Position3& bmax)
{
    float v49[16];  // [4]=start, [0]=end, [8]=bound, [12]=lead normal
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
    int allInside = 1;
    int setNormal = 0;
    v49[12] = 0.0f;
    int pass = 0;
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
                    allInside = 0;
                    if (v17 > ((v17 - v18) * fraction))
                    {
                        float v22 = v17 / (v17 - v18);
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
                    allInside = 0;
                }
                float v20 = v17 - 0.125f;
                if (v20 <= (v19 * v13))
                {
                    if (setNormal != 0)
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
                setNormal = 1;
                v49[i + 12] = v9;
            }
        LABEL_17:
            v15 += 4;
        }
        if (pass == 0)
        {
            v9 = 1.0f;
            v49[8] = bmax.v.m128_f32[0];
            v49[9] = bmax.v.m128_f32[1];
            v49[10] = bmax.v.m128_f32[2];
            v49[11] = bmax.v.m128_f32[3];
            pass = 1;
            continue;
        }
        break;
    }
    if (setNormal != 0)
    {
        float dot = v49[12] * v49[12] + v49[13] * v49[13]
                  + v49[14] * v49[14] + v49[15] * v49[15];
        if (fabsf(sqrtf(dot) - 1.0f) >= 0.0099999998f)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
            AeAssert::gCurrentLine = 844;
            AeAssert::gCurrentExpr = "fabsf(Abs(leadNormal) - 1.0f) < .01f";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
            v13 = delta;
        }
        tw->trace_fraction = v13;
        tw->trace_normal[0] = v49[12];
        tw->trace_normal[1] = v49[13];
        tw->trace_normal[2] = v49[14];
        tw->trace_normal[3] = v49[15];
        return true;
    }
    tw->trace_startsolid = 1;
    if (allInside != 0)
    {
        tw->trace_allsolid = 1;
        tw->trace_fraction = 0.0f;
        return true;
    }
    return true;
}

// ============================================================================
// collide_segment (traceWork dispatch) - ea: 0x6243F0 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x006243F0
bool collide_segment(traceWork_t* tw, const math::Position3& p0,
                     const math::Position3& p1, cdl_cinfo1& cinfo,
                     int& surfaceFlags, int& contentFlags)
{
    cmgr_mem_ctx_t memctx;
    math::Position3* verts = alloc_verts();

    __m128 delta = _mm_sub_ps(p1.v, p0.v);
    __m128 d2 = _mm_mul_ps(delta, delta);
    float len2 = d2.m128_f32[0] + (d2.m128_f32[1] + d2.m128_f32[2]);
    if (len2 < 0.001f || len2 > 1680999900.0f)
        return false;

    // Fast inverse sqrt of len2 * (1/800)^2 -> per-step fraction for 800-unit
    // sub-segments (magic constant 0x5F3759DF pattern).
    float x = len2 * 0.0000015625f;
    float v12 = x * 0.5f;
    unsigned int ix = 1597463007 - ((*(int*)&x) >> 1);
    float v13 = *(float*)&ix;
    float rdt = (1.5f - ((v13 * v13) * v12)) * v13;
    __m128 step = _mm_mul_ps(delta, _mm_set1_ps(rdt));
    const float ones = 1.0f;

    math::Position3 cur;
    cur.v = p0.v;
    float cur_dt = 0.0f;
    int hit = 0;      // BYTE2 of the cur_dt state byte in the binary
    int decalOk = 0;  // HIBYTE of the cur_dt state byte in the binary

    while (1)
    {
        math::Position3 prev;
        prev.v = cur.v;
        float next_dt = cur_dt + rdt;
        math::Position3 next;
        next.v = _mm_add_ps(cur.v, step);
        if (next_dt > 1.0f)
            next.v = p1.v;
        cinfo.pi.v = next.v;  // stale prefill; overwritten on hit

        math::Position3 lo;
        math::Position3 hi;
        lo.v = _mm_sub_ps(_mm_min_ps(prev.v, next.v), _mm_set1_ps(ones));
        hi.v = _mm_add_ps(_mm_max_ps(prev.v, next.v), _mm_set1_ps(ones));

        CGBankManager* mgr = (CGBankManager*)CGBankManager::sInst;
        if (mgr->mCount > 0)
        {
            for (int bi = 0; bi < mgr->mCount; ++bi)
            {
                if (bi > 0x62)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                    AeAssert::gCurrentLine = 31;
                    AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("out of bounds"))
                        __debugbreak();
                }
                CGBank* bank = mgr->mBankArray[bi];
                if (!intersect_segment_aabb(prev, next, lo, hi, bank->min,
                                            bank->max))
                {
                    continue;
                }

                rtree_visitor_t visitor(bank);
                traverse_rtree(lo, hi, bank->rtree_root, visitor);
                visitor.filter_objects(tw->contents);

                // boxes
                int nboxes = visitor.boxes_m_alloc_count;
                for (int i = 0; i < nboxes; ++i)
                {
                    if ((i < 0 || i >= nboxes)
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                            108, "i >= 0 && i < m_alloc_count",
                            defaultFileName))
                        __debugbreak();
                    unsigned int oi = visitor.boxes_m_slot_array[i];
                    if (oi >= (unsigned int)bank->objects.m_count)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::JSV;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\cgbank.h";
                        AeAssert::gCurrentLine = 233;
                        AeAssert::gCurrentExpr = "index < size()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(defaultFileName))
                            __debugbreak();
                        if (oi >= (unsigned int)bank->objects.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                89, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                    }
                    cdl_object_t* obj =
                        &((cdl_object_t*)bank->objects.m_elements)[oi];
                    math::Position3 bmin;
                    math::Position3 bmax;
                    bmin.v = _mm_sub_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    bmax.v = _mm_add_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    if (collide_box_segment(tw, bmin, bmax))
                    {
                        __m128 p = _mm_add_ps(
                            tw->start.v,
                            _mm_mul_ps(
                                _mm_sub_ps(tw->end.v, tw->start.v),
                                _mm_set1_ps(tw->trace_fraction)));
                        __m128 v35 = _mm_sub_ps(tw->start.v, p);
                        __m128 v36 = _mm_mul_ps(v35, v35);
                        float dist2 = v36.m128_f32[0]
                                    + (v36.m128_f32[1] + v36.m128_f32[2]);
                        if (len2 > dist2)
                        {
                            cinfo.pi.v = p;
                            cinfo.ni.v = _mm_setr_ps(
                                tw->trace_normal[0], tw->trace_normal[1],
                                tw->trace_normal[2], tw->trace_normal[3]);
                            surfaceFlags = obj->sflags;
                            contentFlags = obj->cflags;
                            len2 = dist2;
                            if (cinfo.ni.v.m128_f32[0] != 0.0f
                                || cinfo.ni.v.m128_f32[1] != 0.0f
                                || cinfo.ni.v.m128_f32[2] != 0.0f)
                            {
                                hit = 1;
                            }
                            if (tw->trace_check_decal)
                            {
                                decalOk = can_place_decal(
                                    cinfo.pi, cinfo.ni, bmin, bmax, nullptr,
                                    0, tw->trace_decal_radius);
                            }
                        }
                    }
                    if (tw->trace_fraction == 0.0f)
                    {
                        tw->trace_check_decal = decalOk != 0;
                        return true;
                    }
                }

                // brushes
                int nbrushes = visitor.brushes_m_alloc_count;
                for (int i = 0; i < nbrushes; ++i)
                {
                    if ((i < 0 || i >= nbrushes)
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                            108, "i >= 0 && i < m_alloc_count",
                            defaultFileName))
                        __debugbreak();
                    unsigned int oi = visitor.brushes_m_slot_array[i];
                    if (oi >= (unsigned int)bank->objects.m_count)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::JSV;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\cgbank.h";
                        AeAssert::gCurrentLine = 233;
                        AeAssert::gCurrentExpr = "index < size()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(defaultFileName))
                            __debugbreak();
                        if (oi >= (unsigned int)bank->objects.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                89, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                    }
                    cdl_object_t* obj =
                        &((cdl_object_t*)bank->objects.m_elements)[oi];
                    unsigned int brush_index =
                        oi - (unsigned int)bank->nboxes;
                    if (brush_index >= (unsigned int)bank->brushes.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                            "index >= 0 && index < size()", "invalid index"))
                        __debugbreak();
                    cdl_brush_t* brush =
                        &((cdl_brush_t*)bank->brushes.m_elements)
                            [brush_index];
                    unsigned int first_side =
                        (unsigned int)brush->first_side;
                    if (first_side >= (unsigned int)bank->brush_sides.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                            "index >= 0 && index < size()", "invalid index"))
                        __debugbreak();
                    math::Position3 bmin;
                    math::Position3 bmax;
                    bmin.v = _mm_sub_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    bmax.v = _mm_add_ps(
                        _mm_setr_ps(obj->center[0], obj->center[1],
                                    obj->center[2], 0.0f),
                        _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                    obj->box_radius[2], 0.0f));
                    if (collide_brush_segment(
                            tw, bmin, bmax,
                            &((const cdlPlane*)bank->brush_sides.m_elements)
                                [first_side],
                            (unsigned int)brush->num_sides))
                    {
                        __m128 p = _mm_add_ps(
                            tw->start.v,
                            _mm_mul_ps(
                                _mm_sub_ps(tw->end.v, tw->start.v),
                                _mm_set1_ps(tw->trace_fraction)));
                        __m128 v55 = _mm_sub_ps(tw->start.v, p);
                        __m128 v56 = _mm_mul_ps(v55, v55);
                        float dist2 = v56.m128_f32[0]
                                    + (v56.m128_f32[1] + v56.m128_f32[2]);
                        if (len2 > dist2)
                        {
                            cinfo.pi.v = p;
                            cinfo.ni.v = _mm_setr_ps(
                                tw->trace_normal[0], tw->trace_normal[1],
                                tw->trace_normal[2], tw->trace_normal[3]);
                            surfaceFlags = obj->sflags;
                            contentFlags = obj->cflags;
                            len2 = dist2;
                            if (cinfo.ni.v.m128_f32[0] != 0.0f
                                || cinfo.ni.v.m128_f32[1] != 0.0f
                                || cinfo.ni.v.m128_f32[2] != 0.0f)
                            {
                                hit = 1;
                            }
                            if (tw->trace_check_decal)
                            {
                                decalOk = can_place_decal(
                                    cinfo.pi, cinfo.ni, bmin, bmax,
                                    &((const cdlPlane*)
                                          bank->brush_sides.m_elements)
                                        [first_side],
                                    (unsigned int)brush->num_sides,
                                    tw->trace_decal_radius);
                            }
                        }
                    }
                    if (tw->trace_fraction == 0.0f)
                    {
                        tw->trace_check_decal = decalOk != 0;
                        return true;
                    }
                }

                // patches
                int npatches = visitor.patches_m_alloc_count;
                for (int i = 0; i < npatches; ++i)
                {
                    if ((i < 0 || i >= npatches)
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                            108, "i >= 0 && i < m_alloc_count",
                            defaultFileName))
                        __debugbreak();
                    unsigned int oi = visitor.patches_m_slot_array[i];
                    if (oi >= (unsigned int)bank->objects.m_count)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::JSV;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\cgbank.h";
                        AeAssert::gCurrentLine = 233;
                        AeAssert::gCurrentExpr = "index < size()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(defaultFileName))
                            __debugbreak();
                        if (oi >= (unsigned int)bank->objects.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                89, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                    }
                    cdl_object_t* obj =
                        &((cdl_object_t*)bank->objects.m_elements)[oi];
                    if (intersect_segment_aabb(prev, next, lo, hi, bank->min,
                                               bank->max))
                    {
                        unsigned int pi = oi - (unsigned int)bank->nbrushes
                                        - (unsigned int)bank->nboxes;
                        unpack(*bank, pi, verts);
                        if (pi >= (unsigned int)bank->patches.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                91, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                        cdl_patch_t* patch =
                            &((cdl_patch_t*)bank->patches.m_elements)[pi];
                        unsigned int first_index =
                            (unsigned int)patch->first_index;
                        if (first_index
                                >= (unsigned int)bank->patch_inds.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                91, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                        float t = 1.0f;
                        math::Position3 normal;
                        int tid = -1;
                        if (collide_segment(
                                *obj, (const math::Dir3*)verts,
                                &((const unsigned char*)
                                      bank->patch_inds.m_elements)
                                    [first_index],
                                0, (int)patch->num_inds, p0, p1, t, normal,
                                &tid))
                        {
                            __m128 p = _mm_add_ps(
                                p0.v, _mm_mul_ps(delta, _mm_set1_ps(t)));
                            __m128 v67 = _mm_sub_ps(p0.v, p);
                            __m128 v68 = _mm_mul_ps(v67, v67);
                            float dist2 = v68.m128_f32[0]
                                        + (v68.m128_f32[1] + v68.m128_f32[2]);
                            if (len2 > dist2)
                            {
                                cinfo.pi.v = p;
                                cinfo.ni.v = normal.v;
                                surfaceFlags = obj->sflags;
                                contentFlags = obj->cflags;
                                len2 = dist2;
                                hit = 1;
                                if (tw->trace_check_decal)
                                {
                                    decalOk = can_place_decal(
                                        cinfo.pi, (const math::Dir3*)verts,
                                        &((const unsigned char*)
                                              bank->patch_inds.m_elements)
                                            [first_index],
                                        tid, tw->trace_decal_radius);
                                }
                            }
                        }
                    }
                }
            }
        }
        if (hit != 0)
        {
            tw->trace_check_decal = decalOk != 0;
            return true;
        }
        if (next_dt >= 1.0f)
            break;
        cur.v = next.v;
        cur_dt = next_dt;
    }
    tw->trace_check_decal = 0;
    return false;
}

// ============================================================================
// TraceThroughTree - ea: 0x6286E0 (CollisionMgr.cpp)
// ============================================================================
// cdl profile timers (game.o BSS)
cdl_proftimer cdl_proftimer_collide_segment;  // game.o @ 0xF442F0
cdl_proftimer cdl_proftimer_collide_sphere;   // game.o @ 0xF3EAC0

// ea: 0x006286E0
void TraceThroughTree(traceWork_t* tw, const math::Position3& p0,
                      const math::Position3& p1)
{
    if (tw->isPoint != 0)
    {
        __m128 v4 = _mm_sub_ps(p1.v, p0.v);
        __m128 v5 = _mm_mul_ps(v4, v4);
        float len2 = v5.m128_f32[0] + (v5.m128_f32[1] + v5.m128_f32[2]);
        cdl_proftimer_collide_segment.start();
        cdl_cinfo1 cinfo;
        int surfaceFlags;
        int contentFlags;
        bool hit = collide_segment(tw, p0, p1, cinfo, surfaceFlags,
                                   contentFlags);
        cdl_proftimer_collide_segment.stop();
        if (hit)
        {
            tw->trace_surfaceFlags = surfaceFlags;
            tw->trace_contents = contentFlags;
            __m128 v9 = _mm_sub_ps(cinfo.pi.v, p0.v);
            __m128 v10 = _mm_mul_ps(v9, v9);
            float dist2 = v10.m128_f32[0] + (v10.m128_f32[1] + v10.m128_f32[2]);
            if (tw->trace_fraction > sqrtf(dist2) / sqrtf(len2))
            {
                __m128 npi = _mm_mul_ps(cinfo.ni.v, cinfo.pi.v);
                float npi_dot = npi.m128_f32[0] + (npi.m128_f32[1] + npi.m128_f32[2]);
                float v12 = 0.0f - npi_dot;
                __m128 np0 = _mm_mul_ps(cinfo.ni.v, p0.v);
                float np0_dot = np0.m128_f32[0] + (np0.m128_f32[1] + np0.m128_f32[2]);
                __m128 np1 = _mm_mul_ps(cinfo.ni.v, p1.v);
                float np1_dot = np1.m128_f32[0] + (np1.m128_f32[1] + np1.m128_f32[2]);
                tw->trace_fraction =
                    ((np0_dot + v12) - 0.125f)
                    / ((np0_dot + v12) - (np1_dot + v12));
                tw->trace_normal[0] = cinfo.ni.v.m128_f32[0];
                tw->trace_normal[1] = cinfo.ni.v.m128_f32[1];
                tw->trace_normal[2] = cinfo.ni.v.m128_f32[2];
                tw->trace_normal[3] = cinfo.ni.v.m128_f32[3];
            }
        }
    }
    else
    {
        cdl_proftimer_collide_sphere.start();
        collide_velocity_sphere(tw);
        cdl_proftimer_collide_sphere.stop();
    }
}

// ============================================================================
// SightTraceXFormed - ea: 0x636890 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x00636890
int SightTraceXFormed(int hitNum, const math::Position3& start,
                      const math::Position3& end,
                      const math::Position3& mins,
                      const math::Position3& maxs, DCGSet* model,
                      int brushmask, const math::Position3& origin,
                      const math::Position3& angles, int capsule)
{
    __m128 center = _mm_mul_ps(_mm_add_ps(mins.v, maxs.v),
                               _mm_set1_ps(0.5f));
    math::Position3 mins2;
    mins2.v = _mm_sub_ps(mins.v, center);
    math::Position3 maxs2;
    maxs2.v = _mm_sub_ps(maxs.v, center);
    math::Position3 start2;
    start2.v = _mm_sub_ps(_mm_add_ps(start.v, center), origin.v);
    math::Position3 end2;
    end2.v = _mm_sub_ps(_mm_add_ps(end.v, center), origin.v);

    bool hasAngles = model != nullptr && model->id != 4095
        && (angles.v.m128_f32[0] != 0.0f
            || angles.v.m128_f32[1] != 0.0f
            || angles.v.m128_f32[2] != 0.0f);
    float half = maxs2.v.m128_f32[2]
               - (maxs2.v.m128_f32[0] <= maxs2.v.m128_f32[2]
                      ? maxs2.v.m128_f32[0]
                      : maxs2.v.m128_f32[2]);

    sphere_t sphere;
    sphere.offset.v = _mm_setzero_ps();
    sphere.radiusOffset.v = _mm_setzero_ps();
    sphere.use = capsule;
    sphere.radius = maxs2.v.m128_f32[0] <= maxs2.v.m128_f32[2]
                        ? maxs2.v.m128_f32[0]
                        : maxs2.v.m128_f32[2];
    sphere.halfheight = maxs2.v.m128_f32[2];

    if (hasAngles)
    {
        float forward[3];
        float right[3];
        float up[3];
        AngleVectors(angles, forward, right, up);
        VectorInverse(right);
        float matrix[3][3];
        matrix[0][0] = forward[0];
        matrix[0][1] = forward[1];
        matrix[0][2] = forward[2];
        matrix[1][0] = right[0];
        matrix[1][1] = right[1];
        matrix[1][2] = right[2];
        matrix[2][0] = up[0];
        matrix[2][1] = up[1];
        matrix[2][2] = up[2];
        RotatePoint(start2, (math::Position3*)matrix);
        RotatePoint(end2, (math::Position3*)matrix);
        sphere.offset.v.m128_f32[0] = forward[2] * half;
        sphere.offset.v.m128_f32[1] = -right[2] * half;
        sphere.offset.v.m128_f32[2] = up[2] * half;
    }
    else
    {
        sphere.offset.v.m128_f32[2] = half;
    }
    return SightTrace(hitNum, &start2, &end2, &mins2, &maxs2, model, &origin,
                      brushmask, capsule, &sphere);
}

// ============================================================================
// TraceXFormed - ea: 0x641D40 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x00641D40
void TraceXFormed(trace_t* results, const math::Position3& start,
                  const math::Position3& end, const math::Position3& mins,
                  const math::Position3& maxs, DCGSet* model, int brushmask,
                  const math::Position3& origin,
                  const math::Position3& angles, int capsule)
{
    __m128 center = _mm_mul_ps(_mm_add_ps(mins.v, maxs.v),
                               _mm_set1_ps(0.5f));
    math::Position3 mins2;
    mins2.v = _mm_sub_ps(mins.v, center);
    math::Position3 maxs2;
    maxs2.v = _mm_sub_ps(maxs.v, center);
    math::Position3 start2;
    start2.v = _mm_sub_ps(_mm_add_ps(start.v, center), origin.v);
    math::Position3 end2;
    end2.v = _mm_sub_ps(_mm_add_ps(end.v, center), origin.v);

    bool hasAngles = model != nullptr && model->id != 4095
        && (angles.v.m128_f32[0] != 0.0f
            || angles.v.m128_f32[1] != 0.0f
            || angles.v.m128_f32[2] != 0.0f);
    float half = maxs2.v.m128_f32[2]
               - (maxs2.v.m128_f32[0] <= maxs2.v.m128_f32[2]
                      ? maxs2.v.m128_f32[0]
                      : maxs2.v.m128_f32[2]);

    trace_t localTrace;
    localTrace.mEntity.mHandle.mVal = 0;
    localTrace.partName.mHash = 0;

    sphere_t sphere;
    sphere.offset.v = _mm_setzero_ps();
    sphere.radiusOffset.v = _mm_setzero_ps();
    sphere.use = capsule;
    sphere.radius = maxs2.v.m128_f32[0] <= maxs2.v.m128_f32[2]
                        ? maxs2.v.m128_f32[0]
                        : maxs2.v.m128_f32[2];
    sphere.halfheight = maxs2.v.m128_f32[2];

    float matrix[3][3];
    if (hasAngles)
    {
        float forward[3];
        float right[3];
        float up[3];
        AngleVectors(angles, forward, right, up);
        VectorInverse(right);
        matrix[0][0] = forward[0];
        matrix[0][1] = forward[1];
        matrix[0][2] = forward[2];
        matrix[1][0] = right[0];
        matrix[1][1] = right[1];
        matrix[1][2] = right[2];
        matrix[2][0] = up[0];
        matrix[2][1] = up[1];
        matrix[2][2] = up[2];
        RotatePoint(start2, (math::Position3*)matrix);
        RotatePoint(end2, (math::Position3*)matrix);
        sphere.offset.v.m128_f32[0] = forward[2] * half;
        sphere.offset.v.m128_f32[1] = -right[2] * half;
        sphere.offset.v.m128_f32[2] = up[2] * half;
    }
    else
    {
        sphere.offset.v.m128_f32[2] = half;
    }

    float savedFrac = results->fraction;
    localTrace.fraction = savedFrac;
    Trace(&localTrace, start2, end2, mins2, maxs2, model, brushmask, capsule,
          &sphere);
    if (hasAngles && results->fraction > savedFrac)
    {
        TransposeMatrix((math::Position3*)matrix,
                        (math::Position3*)&sphere);
        math::Dir3 normal = localTrace.normal;
        RotatePoint((math::Position3&)normal, (math::Position3*)&sphere);
        localTrace.normal.v = normal.v;
    }
    localTrace.endpos.v = _mm_add_ps(
        start.v, _mm_mul_ps(_mm_sub_ps(end.v, start.v),
                            _mm_set1_ps(savedFrac)));
    *results = localTrace;
}

// ============================================================================
// push_sphere_in_world - ea: 0x636AC0 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x00636AC0
bool push_sphere_in_world(math::Position3& pos, float radius,
                          const proximity_data_t& data,
                          TouchEntityData* entities)
{
    float v78[3] = { radius, radius + 35.0f, 72.0f - radius };
    bool hit = false;
    for (int i = 0; i < 3; ++i)
    {
        math::Position3 sphere_center = pos;
        sphere_center.v.m128_f32[2] += v78[i];

        // Entity brush models (world brush entities touching the sphere)
        if (entities != nullptr && entities->num > 0)
        {
            for (int j = 0; j < entities->num; ++j)
            {
                unsigned int handle =
                    entities->touch[j].mHandle.mVal;
                unsigned int idx = handle & 0xFFF;
                Entity* ent = nullptr;
                if (idx < 0x540
                    && (handle >> 12)
                        == (unsigned int)EntityHandleDb::sInst
                               .mElements[idx]
                               .mKey)
                {
                    ent = EntityHandleDb::sInst.mElements[idx].mObject;
                }
                if (ent == nullptr || ent->r.bmodel == nullptr)
                    continue;
                math::Mat43 mat = ent->CalcRotTranMat43();
                DCGSet* model = ent->r.bmodel;

                __m128 diff =
                    _mm_sub_ps(sphere_center.v, mat.w.v);
                __m128 offs =
                    _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(
                                _mm_shuffle_ps(diff, diff, 0),
                                mat.x.v),
                            _mm_mul_ps(
                                _mm_shuffle_ps(diff, diff, 0x55),
                                mat.y.v)),
                        _mm_mul_ps(
                            _mm_shuffle_ps(diff, diff, 0xAA),
                            mat.z.v));
                math::Position3 modelPos;
                modelPos.v = offs;

                int nbrushes = model->nbrushes;
                if (nbrushes != 0)
                {
                    cdl_object_t* objects =
                        (cdl_object_t*)model->objects_m_elements;
                    cdl_brush_t* brushes =
                        (cdl_brush_t*)model->brushes_m_elements;
                    cdlPlane* sides =
                        (cdlPlane*)model->brush_sides_m_elements;
                    for (int b = 0; b < nbrushes; ++b)
                    {
                        unsigned int oi =
                            (unsigned int)model->nboxes + (unsigned int)b;
                        if (oi >= (unsigned int)model->objects_m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                89, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                        cdl_object_t& obj = objects[oi];
                        cdl_brush_t& brush = brushes[b];
                        unsigned int first_side =
                            (unsigned int)brush.first_side;
                        if (first_side
                            >= (unsigned int)model->brush_sides_m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                89, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                        hit |= collide_sphere_brush(
                            modelPos, radius, obj, &sides[first_side],
                            (unsigned int)brush.num_sides, modelPos);
                    }
                }
                int nboxes = model->nboxes;
                if (nboxes != 0)
                {
                    cdl_object_t* objects =
                        (cdl_object_t*)model->objects_m_elements;
                    for (int b = 0; b < nboxes; ++b)
                    {
                        if ((unsigned int)b
                                >= (unsigned int)model->objects_m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                89, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                        hit |= collide_sphere_box(
                            modelPos, radius, objects[b], modelPos);
                    }
                }
                // Back to world space: M * model + origin
                sphere_center.v = _mm_add_ps(
                    _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(
                                _mm_shuffle_ps(modelPos.v, modelPos.v, 0),
                                mat.x.v),
                            _mm_mul_ps(
                                _mm_shuffle_ps(modelPos.v, modelPos.v, 0x55),
                                mat.y.v)),
                        _mm_mul_ps(
                            _mm_shuffle_ps(modelPos.v, modelPos.v, 0xAA),
                            mat.z.v)),
                    mat.w.v);
            }
        }

        // Proximity brushes
        for (int b = 0; b < data.brushes_count; ++b)
        {
            if ((b < 0 || b >= data.brushes_count)
                && _tlAssert(
                    "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                    114, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            const proxy_obj_t& slot = data.brushes_slot[b];
            unsigned int bi = slot.bi;
            if (bi >= 99)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 31;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            CGBank* bank =
                ((CGBankManager*)CGBankManager::sInst)->mBankArray[bi];
            unsigned int oi = slot.oi;
            if (oi >= (unsigned int)bank->objects.m_count)
            {
                AeAssert::gCurrentAuthor = AeAssert::JSV;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                AeAssert::gCurrentLine = 233;
                AeAssert::gCurrentExpr = "index < size()";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(defaultFileName))
                    __debugbreak();
                if (oi >= (unsigned int)bank->objects.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
            }
            cdl_object_t* obj =
                &((cdl_object_t*)bank->objects.m_elements)[oi];
            unsigned int brush_index = oi - (unsigned int)bank->nboxes;
            if (brush_index >= (unsigned int)bank->brushes.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            cdl_brush_t* brush =
                &((cdl_brush_t*)bank->brushes.m_elements)[brush_index];
            unsigned int first_side = (unsigned int)brush->first_side;
            if (first_side >= (unsigned int)bank->brush_sides.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            hit |= collide_sphere_brush(
                sphere_center, radius, *obj,
                &((const cdlPlane*)bank->brush_sides.m_elements)
                    [first_side],
                (unsigned int)brush->num_sides, sphere_center);
        }

        // Proximity boxes
        for (int b = 0; b < data.boxes_count; ++b)
        {
            if ((b < 0 || b >= data.boxes_count)
                && _tlAssert(
                    "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                    114, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            const proxy_obj_t& slot = data.boxes_slot[b];
            unsigned int bi = slot.bi;
            if (bi >= 99)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 31;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            CGBank* bank =
                ((CGBankManager*)CGBankManager::sInst)->mBankArray[bi];
            unsigned int oi = slot.oi;
            if (oi >= (unsigned int)bank->objects.m_count)
            {
                AeAssert::gCurrentAuthor = AeAssert::JSV;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                AeAssert::gCurrentLine = 233;
                AeAssert::gCurrentExpr = "index < size()";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(defaultFileName))
                    __debugbreak();
                if (oi >= (unsigned int)bank->objects.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
            }
            cdl_object_t* obj =
                &((cdl_object_t*)bank->objects.m_elements)[oi];
            hit |= collide_sphere_box(sphere_center, radius, *obj,
                                      sphere_center);
        }

        // Proximity polies
        for (int b = 0; b < data.polies_count; ++b)
        {
            if ((b < 0 || b >= data.polies_count)
                && _tlAssert(
                    "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                    114, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            const bounded_proxy_obj_t& slot = data.polies_slot[b];
            unsigned int bi = slot.bi;
            if (bi >= 99)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 31;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            CGBank* bank =
                ((CGBankManager*)CGBankManager::sInst)->mBankArray[bi];
            unsigned int oi = slot.oi;
            if (oi >= (unsigned int)bank->objects.m_count)
            {
                AeAssert::gCurrentAuthor = AeAssert::JSV;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                AeAssert::gCurrentLine = 233;
                AeAssert::gCurrentExpr = "index < size()";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(defaultFileName))
                    __debugbreak();
                if (oi >= (unsigned int)bank->objects.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
            }
            unsigned int pi = oi - (unsigned int)bank->nbrushes
                            - (unsigned int)bank->nboxes;
            if (pi >= (unsigned int)bank->patches.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            const cdl_patch_t* patch =
                &((const cdl_patch_t*)bank->patches.m_elements)[pi];
            if (pi >= (unsigned int)bank->gjk_patches.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            const cdl_vinfo_t* vinfo =
                &((const cdl_vinfo_t*)bank->gjk_patches.m_elements)[pi];
            unsigned int ind =
                (unsigned int)patch->first_index + 3 * (unsigned int)slot.ti;
            if (ind >= (unsigned int)bank->patch_inds.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            const unsigned char* pvi =
                &((const unsigned char*)bank->patch_inds.m_elements)[ind];
            const uint32_t* pverts =
                (const uint32_t*)bank->patch_verts.m_elements;
            float vbase[3] = { (float)vinfo->vbase[0],
                               (float)vinfo->vbase[1],
                               (float)vinfo->vbase[2] };
            math::Position3 v0;
            math::Position3 v1;
            math::Position3 v2;
            {
                unsigned int vertIdx =
                    (unsigned int)vinfo->first_vert + (unsigned int)pvi[0];
                if (vertIdx >= (unsigned int)bank->patch_verts.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
                uint32_t packed = pverts[vertIdx];
                v0.v = _mm_setr_ps(
                    (float)(packed & 0x7FF) * 0.25f + vbase[0],
                    (float)((packed >> 11) & 0x7FF) * 0.25f + vbase[1],
                    (float)(packed >> 22) * 0.25f + vbase[2], 0.0f);
            }
            {
                unsigned int vertIdx =
                    (unsigned int)vinfo->first_vert + (unsigned int)pvi[1];
                if (vertIdx >= (unsigned int)bank->patch_verts.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
                uint32_t packed = pverts[vertIdx];
                v1.v = _mm_setr_ps(
                    (float)(packed & 0x7FF) * 0.25f + vbase[0],
                    (float)((packed >> 11) & 0x7FF) * 0.25f + vbase[1],
                    (float)(packed >> 22) * 0.25f + vbase[2], 0.0f);
            }
            {
                unsigned int vertIdx =
                    (unsigned int)vinfo->first_vert + (unsigned int)pvi[2];
                if (vertIdx >= (unsigned int)bank->patch_verts.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
                uint32_t packed = pverts[vertIdx];
                v2.v = _mm_setr_ps(
                    (float)(packed & 0x7FF) * 0.25f + vbase[0],
                    (float)((packed >> 11) & 0x7FF) * 0.25f + vbase[1],
                    (float)(packed >> 22) * 0.25f + vbase[2], 0.0f);
            }
            math::Vector4 plane = calc_normal(v0, v1, v2);
            hit |= new_push_out_sphere_triangle(
                sphere_center, radius, v0, v1, v2, (math::Dir3&)plane,
                sphere_center);
        }

        pos = sphere_center;
        pos.v.m128_f32[2] -= v78[i];
    }
    return hit;
}

// ============================================================================
// collide_velocity_sphere (proximity) - ea: 0x635B70 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x00635B70
bool collide_velocity_sphere(traceWork_t* tw, const proximity_data_t& data)
{
    bool insolid = false;
    int frac = 0;

    // brushes
    int nbrushes = data.brushes_count;
    if (nbrushes != 0)
    {
        while (1)
        {
            if ((frac < 0 || frac >= data.brushes_count)
                && _tlAssert(
                    "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                    114, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            const proxy_obj_t& slot = data.brushes_slot[frac];
            unsigned int bi = slot.bi;
            if (bi >= 99)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 31;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            CGBank* bank =
                ((CGBankManager*)CGBankManager::sInst)->mBankArray[bi];
            unsigned int oi = slot.oi;
            if (oi >= (unsigned int)bank->objects.m_count)
            {
                AeAssert::gCurrentAuthor = AeAssert::JSV;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                AeAssert::gCurrentLine = 233;
                AeAssert::gCurrentExpr = "index < size()";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(defaultFileName))
                    __debugbreak();
                if (oi >= (unsigned int)bank->objects.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
            }
            cdl_object_t* obj =
                &((cdl_object_t*)bank->objects.m_elements)[oi];
            unsigned int brush_index = oi - (unsigned int)bank->nboxes;
            if (brush_index >= (unsigned int)bank->brushes.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            cdl_brush_t* brush =
                &((cdl_brush_t*)bank->brushes.m_elements)[brush_index];
            unsigned int first_side = (unsigned int)brush->first_side;
            if (first_side >= (unsigned int)bank->brush_sides.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            math::Position3 bmin;
            math::Position3 bmax;
            bmin.v = _mm_sub_ps(
                _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f),
                _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                            obj->box_radius[2], 0.0f));
            bmax.v = _mm_add_ps(
                _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f),
                _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                            obj->box_radius[2], 0.0f));
            float savedFrac = tw->trace_fraction;
            collide_brush_velocity_sphere(
                tw, bmin, bmax,
                &((const cdlPlane*)bank->brush_sides.m_elements)
                    [first_side],
                (unsigned int)brush->num_sides);
            if (tw->trace_fraction == 0.0f)
            {
                tw->trace_surfaceFlags = obj->sflags;
                tw->trace_contents = obj->cflags;
                return true;
            }
            if (savedFrac > tw->trace_fraction)
            {
                tw->trace_surfaceFlags = obj->sflags;
                tw->trace_contents = obj->cflags;
            }
            if (++frac >= data.brushes_count)
                break;
        }
    }

    // boxes
    frac = 0;
    int nboxes = data.boxes_count;
    if (nboxes != 0)
    {
        while (1)
        {
            if ((frac < 0 || frac >= data.boxes_count)
                && _tlAssert(
                    "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                    114, "i >= 0 && i < m_alloc_count", defaultFileName))
                __debugbreak();
            const proxy_obj_t& slot = data.boxes_slot[frac];
            unsigned int bi = slot.bi;
            if (bi >= 99)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 31;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            CGBank* bank =
                ((CGBankManager*)CGBankManager::sInst)->mBankArray[bi];
            unsigned int oi = slot.oi;
            if (oi >= (unsigned int)bank->objects.m_count)
            {
                AeAssert::gCurrentAuthor = AeAssert::JSV;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                AeAssert::gCurrentLine = 233;
                AeAssert::gCurrentExpr = "index < size()";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(defaultFileName))
                    __debugbreak();
                if (oi >= (unsigned int)bank->objects.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
            }
            cdl_object_t* obj =
                &((cdl_object_t*)bank->objects.m_elements)[oi];
            math::Position3 bmin;
            math::Position3 bmax;
            bmin.v = _mm_sub_ps(
                _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f),
                _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                            obj->box_radius[2], 0.0f));
            bmax.v = _mm_add_ps(
                _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f),
                _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                            obj->box_radius[2], 0.0f));
            float savedFrac = tw->trace_fraction;
            collide_box_velocity_sphere(tw, bmin, bmax);
            if (tw->trace_fraction == 0.0f)
            {
                tw->trace_surfaceFlags = obj->sflags;
                tw->trace_contents = obj->cflags;
                return true;
            }
            if (savedFrac > tw->trace_fraction)
            {
                tw->trace_surfaceFlags = obj->sflags;
                tw->trace_contents = obj->cflags;
            }
            if (++frac >= data.boxes_count)
                break;
        }
    }

    // polies
    float halfheight = tw->sphere_halfheight;
    float radius = tw->sphere_radius;
    float v31 = halfheight - radius;
    math::Position3 start2;
    start2.v = _mm_setr_ps(tw->start.v.m128_f32[0],
                            tw->start.v.m128_f32[1],
                            tw->start.v.m128_f32[2] - v31, 0.0f);
    math::Position3 end2;
    end2.v = _mm_setr_ps(tw->end.v.m128_f32[0],
                          tw->end.v.m128_f32[1],
                          tw->end.v.m128_f32[2] - v31, 0.0f);
    __m128 v34 = _mm_sub_ps(end2.v, start2.v);
    __m128 v35 = _mm_mul_ps(v34, v34);
    float len2 = v35.m128_f32[0] + (v35.m128_f32[1] + v35.m128_f32[2]);
    float seglen = sqrtf(len2);
    math::Dir3 ndir;
    ndir.v = _mm_div_ps(v34, _mm_set1_ps(seglen));
    float r = radius + 0.001f;
    int npolies = data.polies_count;
    frac = 0;
    if (npolies == 0)
        return insolid;
    while (1)
    {
        if ((frac < 0 || frac >= data.polies_count)
            && _tlAssert(
                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                114, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        const bounded_proxy_obj_t& slot = data.polies_slot[frac];
        unsigned int bi = slot.bi;
        if (bi >= 99)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        CGBank* bank =
            ((CGBankManager*)CGBankManager::sInst)->mBankArray[bi];
        unsigned int oi = slot.oi;
        if (oi >= (unsigned int)bank->objects.m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 233;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (oi >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        cdl_object_t* obj =
            &((cdl_object_t*)bank->objects.m_elements)[oi];
        unsigned int pi = oi - (unsigned int)bank->nbrushes
                        - (unsigned int)bank->nboxes;
        if (pi >= (unsigned int)bank->patches.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        if (pi >= (unsigned int)bank->gjk_patches.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        const cdl_vinfo_t* vinfo =
            &((const cdl_vinfo_t*)bank->gjk_patches.m_elements)[pi];
        const cdl_patch_t* patch =
            &((const cdl_patch_t*)bank->patches.m_elements)[pi];
        unsigned int ind = (unsigned int)patch->first_index
                         + 3 * (unsigned int)slot.ti;
        if (ind >= (unsigned int)bank->patch_inds.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        math::Position3 v0;
        math::Position3 v1;
        math::Position3 v2;
        unpack_poly(
            bank, vinfo,
            &((const unsigned char*)bank->patch_inds.m_elements)[ind], v0,
            v1, v2);
        math::Vector4 plane = calc_normal(v0, v1, v2);

        float w = plane.v.m128_f32[3];
        float dStart = plane.v.m128_f32[0] * start2.v.m128_f32[0]
                     + plane.v.m128_f32[1] * start2.v.m128_f32[1]
                     + plane.v.m128_f32[2] * start2.v.m128_f32[2];
        float dEnd = plane.v.m128_f32[0] * end2.v.m128_f32[0]
                   + plane.v.m128_f32[1] * end2.v.m128_f32[1]
                   + plane.v.m128_f32[2] * end2.v.m128_f32[2];
        if (!(dStart + w - r > 0.0f && dEnd + w - r > 0.0f
              && 1.0f + w - r > 0.0f))
        {
            cdl_proftimer_vsphere_poly.start();
            math::Position3 c1 = end2;
            bool polyInsolid = false;
            bool polyHit = collide_velocity_sphere_poly(
                start2, c1, ndir, r, v0, v1, v2, plane, polyInsolid);
            cdl_proftimer_vsphere_poly.stop();
            if (polyHit)
            {
                if (polyInsolid)
                {
                    tw->trace_normal[0] = plane.v.m128_f32[0];
                    tw->trace_normal[1] = plane.v.m128_f32[1];
                    tw->trace_normal[2] = plane.v.m128_f32[2];
                    tw->trace_normal[3] = plane.v.m128_f32[3];
                    tw->trace_fraction = 0.0f;
                    tw->trace_startsolid = 1;
                    tw->trace_surfaceFlags = obj->sflags;
                    tw->trace_contents = obj->cflags;
                    return true;
                }
                __m128 v50 = _mm_sub_ps(c1.v, start2.v);
                __m128 v51 = _mm_mul_ps(v50, v50);
                float dist2 = v51.m128_f32[0]
                            + (v51.m128_f32[1] + v51.m128_f32[2]);
                float d = sqrtf(dist2) / seglen;
                if (d > 0.0000099999997f)
                {
                    if (d <= tw->trace_fraction)
                    {
                        tw->trace_normal[0] = plane.v.m128_f32[0];
                        tw->trace_normal[1] = plane.v.m128_f32[1];
                        tw->trace_normal[2] = plane.v.m128_f32[2];
                        tw->trace_normal[3] = plane.v.m128_f32[3];
                        tw->trace_fraction = d - 0.0000099999997f;
                        tw->trace_surfaceFlags = obj->sflags;
                        tw->trace_contents = obj->cflags;
                        insolid = true;
                    }
                }
                else
                {
                    tw->trace_normal[0] = plane.v.m128_f32[0];
                    tw->trace_normal[1] = plane.v.m128_f32[1];
                    tw->trace_normal[2] = plane.v.m128_f32[2];
                    tw->trace_normal[3] = plane.v.m128_f32[3];
                    tw->trace_fraction = 0.0f;
                    tw->trace_startsolid = 1;
                    tw->trace_surfaceFlags = obj->sflags;
                    tw->trace_contents = obj->cflags;
                    return true;
                }
            }
        }
        if (++frac >= data.polies_count)
            return insolid;
    }
}

// ============================================================================
// collide_brush_segment (math) - ea: 0x61ACC0 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x0061ACC0
bool collide_brush_segment(const math::Position3& p0,
                           const math::Position3& p1,
                           const math::Position3& bmin,
                           const math::Position3& bmax,
                           const cdlPlane* sides, unsigned int nsides,
                           float& t, math::Position3* normal)
{
    float v9 = t;
    float enter = 0.0f;
    float delta = 0.0f;
    float sign = -1.0f;
    math::Position3 nrm;
    nrm.v = _mm_setzero_ps();
    int allInside = 1;
    int setNormal = 0;
    int pass = 0;

    while (2)
    {
        const math::Position3* bounds =
            pass == 0 ? &bmin : &bmax;
        for (int i = 0; i < 12; i += 4)
        {
            int axis = i / 4;
            float p0d = (p0.v.m128_f32[axis] - bounds->v.m128_f32[axis])
                * sign;
            float p1d = (p1.v.m128_f32[axis] - bounds->v.m128_f32[axis])
                * sign;
            if (p0d <= 0.0f)
            {
                if (p1d > 0.0f)
                {
                    allInside = 0;
                    if (p0d > (p0d - p1d) * v9)
                    {
                        float tt = p0d / (p0d - p1d);
                        if (enter >= tt)
                            return false;
                        v9 = tt;
                    }
                }
            }
            else
            {
                float v17 = p0d - p1d;
                if (p1d > 0.0f)
                {
                    if (v17 <= 0.0f || p1d >= 0.125f)
                        return false;
                    allInside = 0;
                }
                float v18 = p0d - 0.125f;
                if (v18 > v17 * enter)
                {
                    float tt = v18 / v17;
                    delta = tt;
                    if (tt >= v9)
                        return false;
                    enter = tt;
                }
                else if (setNormal != 0)
                {
                    continue;
                }
                nrm.v = _mm_setzero_ps();
                nrm.v.m128_f32[axis] = sign;
                setNormal = 1;
            }
        }
        if (pass != 0)
            break;
        sign = 1.0f;
        pass = 1;
    }

    for (unsigned int i = 0; i < nsides; ++i)
    {
        const cdlPlane& plane = sides[i];
        float offs = plane.packed[3];
        float d0 = plane.packed[0] * p0.v.m128_f32[0]
            + plane.packed[1] * p0.v.m128_f32[1]
            + plane.packed[2] * p0.v.m128_f32[2]
            - offs;
        float d1 = plane.packed[0] * p1.v.m128_f32[0]
            + plane.packed[1] * p1.v.m128_f32[1]
            + plane.packed[2] * p1.v.m128_f32[2]
            - offs;
        float v27 = d0;
        float v28 = d1;
        if (d0 <= 0.0f)
        {
            if (d1 > 0.0f)
            {
                float v35 = d0 - d1;
                allInside = 0;
                if (d0 > v35 * v9)
                {
                    float tt = d0 / v35;
                    if (enter >= tt)
                        return false;
                    v9 = tt;
                }
            }
        }
        else
        {
            float v29 = d0 - d1;
            if (d1 > 0.0f)
            {
                if (v29 <= 0.0f || d1 >= 0.125f)
                    return false;
                allInside = 0;
            }
            float v30 = d0 - 0.125f;
            if (v30 > v29 * enter)
            {
                float tt = v30 / v29;
                delta = tt;
                if (tt >= v9)
                    return false;
                enter = tt;
                nrm.v = _mm_setr_ps(plane.packed[0], plane.packed[1],
                                    plane.packed[2], 0.0f);
                setNormal = 1;
            }
            else if (setNormal == 0)
            {
                nrm.v = _mm_setr_ps(plane.packed[0], plane.packed[1],
                                    plane.packed[2], 0.0f);
                setNormal = 1;
            }
        }
    }
    if (setNormal != 0)
    {
        t = enter;
        if (normal != nullptr)
            *normal = nrm;
    }
    else
    {
        if (allInside != 0)
            t = 0.0f;
        if (normal != nullptr)
        {
            normal->v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
            return true;
        }
    }
    return true;
}

// ============================================================================
// collide_segment (proximity) - ea: 0x634620 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x00634620
bool collide_segment(const proximity_data_t& data, traceWork_t* tw,
                     const math::Position3& p0, const math::Position3& p1,
                     cdl_cinfo1& cinfo, int& sflags, int& cflags)
{
    __m128 v8 = _mm_sub_ps(p1.v, p0.v);
    __m128 v9 = _mm_mul_ps(v8, v8);
    float len2 = v9.m128_f32[0] + (v9.m128_f32[1] + v9.m128_f32[2]);
    if (len2 < 0.001f || len2 > 1680999900.0f)
        return false;

    bool hit = false;

    int nbrushes = data.brushes_count;
    for (int i = 0; i < nbrushes; ++i)
    {
        if ((i < 0 || i >= data.brushes_count)
            && _tlAssert(
                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                114, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        const proxy_obj_t& slot = data.brushes_slot[i];
        CGBank* bank =
            ((CGBankManager*)CGBankManager::sInst)->mBankArray[slot.bi];
        unsigned int oi = slot.oi;
        if (oi >= (unsigned int)bank->objects.m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 233;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (oi >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        cdl_object_t* obj =
            &((cdl_object_t*)bank->objects.m_elements)[oi];
        unsigned int brush_index = oi - (unsigned int)bank->nboxes;
        if (brush_index >= (unsigned int)bank->brushes.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        cdl_brush_t* brush =
            &((cdl_brush_t*)bank->brushes.m_elements)[brush_index];
        unsigned int first_side = (unsigned int)brush->first_side;
        if (first_side >= (unsigned int)bank->brush_sides.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        math::Position3 bmin;
        math::Position3 bmax;
        bmax.v = _mm_add_ps(
            _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                        0.0f),
            _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                        obj->box_radius[2], 0.0f));
        bmin.v = _mm_sub_ps(
            _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                        0.0f),
            _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                        obj->box_radius[2], 0.0f));
        if (collide_brush_segment(
                tw, bmin, bmax,
                &((const cdlPlane*)bank->brush_sides.m_elements)
                    [first_side],
                (unsigned int)brush->num_sides))
        {
            cinfo.ni.v = tw->trace_normal[0] != 0
                ? _mm_setr_ps(tw->trace_normal[0], tw->trace_normal[1],
                              tw->trace_normal[2], tw->trace_normal[3])
                : _mm_setzero_ps();
            cinfo.pi.v = _mm_add_ps(
                p0.v,
                _mm_mul_ps(_mm_sub_ps(p1.v, p0.v),
                           _mm_set1_ps(tw->trace_fraction)));
            sflags = obj->sflags;
            cflags = obj->cflags;
            hit = true;
        }
    }

    int nboxes = data.boxes_count;
    for (int i = 0; i < nboxes; ++i)
    {
        if ((i < 0 || i >= data.boxes_count)
            && _tlAssert(
                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                114, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        const proxy_obj_t& slot = data.boxes_slot[i];
        CGBank* bank =
            ((CGBankManager*)CGBankManager::sInst)->mBankArray[slot.bi];
        if (bank == nullptr)
            continue;
        unsigned int oi = slot.oi;
        if (oi >= (unsigned int)bank->objects.m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 233;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (oi >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        cdl_object_t* obj =
            &((cdl_object_t*)bank->objects.m_elements)[oi];
        math::Position3 bmin;
        math::Position3 bmax;
        bmax.v = _mm_add_ps(
            _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                        0.0f),
            _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                        obj->box_radius[2], 0.0f));
        bmin.v = _mm_sub_ps(
            _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                        0.0f),
            _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                        obj->box_radius[2], 0.0f));
        if (collide_box_segment(tw, bmin, bmax))
        {
            cinfo.ni.v = _mm_setr_ps(tw->trace_normal[0],
                                     tw->trace_normal[1],
                                     tw->trace_normal[2],
                                     tw->trace_normal[3]);
            cinfo.pi.v = _mm_add_ps(
                p0.v,
                _mm_mul_ps(_mm_sub_ps(p1.v, p0.v),
                           _mm_set1_ps(tw->trace_fraction)));
            sflags = obj->sflags;
            cflags = obj->cflags;
            hit = true;
        }
    }

    int npolies = data.polies_count;
    for (int i = 0; i < npolies; ++i)
    {
        if ((i < 0 || i >= data.polies_count)
            && _tlAssert(
                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                114, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        const bounded_proxy_obj_t& slot = data.polies_slot[i];
        CGBank* bank =
            ((CGBankManager*)CGBankManager::sInst)->mBankArray[slot.bi];
        unsigned int oi = slot.oi;
        if (oi >= (unsigned int)bank->objects.m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 233;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (oi >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        cdl_object_t* obj =
            &((cdl_object_t*)bank->objects.m_elements)[oi];
        unsigned int pi =
            oi - (unsigned int)bank->nbrushes - (unsigned int)bank->nboxes;
        if (pi >= (unsigned int)bank->patches.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        if (pi >= (unsigned int)bank->gjk_patches.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        const cdl_vinfo_t* vinfo =
            &((const cdl_vinfo_t*)bank->gjk_patches.m_elements)[pi];
        unsigned int ind =
            (unsigned int)((cdl_patch_t*)bank->patches.m_elements)[pi]
                .first_index
            + 3 * (unsigned int)slot.ti;
        if (ind >= (unsigned int)bank->patch_inds.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        math::Position3 v0;
        math::Position3 v1;
        math::Position3 v2;
        unpack_poly(
            bank, vinfo,
            &((const unsigned char*)bank->patch_inds.m_elements)[ind], v0,
            v1, v2);
        math::Vector4 plane = calc_normal(v0, v1, v2);

        float d0 = plane.v.m128_f32[0] * p0.v.m128_f32[0]
            + plane.v.m128_f32[1] * p0.v.m128_f32[1]
            + plane.v.m128_f32[2] * p0.v.m128_f32[2]
            + plane.v.m128_f32[3];
        float d1 = plane.v.m128_f32[0] * p1.v.m128_f32[0]
            + plane.v.m128_f32[1] * p1.v.m128_f32[1]
            + plane.v.m128_f32[2] * p1.v.m128_f32[2]
            + plane.v.m128_f32[3];
        if (d0 > 0.000099999997f && d1 <= 0.0f)
        {
            // closest point on segment to triangle plane
            math::Position3 cp;
            cp.v = _mm_div_ps(
                _mm_sub_ps(_mm_mul_ps(p0.v, _mm_set1_ps(d1)),
                           _mm_mul_ps(p1.v, _mm_set1_ps(d0))),
                _mm_set1_ps(d1 - d0));
            __m128 v52 = _mm_sub_ps(p0.v, cp.v);
            __m128 v53 = _mm_mul_ps(v52, v52);
            float d2 =
                v53.m128_f32[0] + (v53.m128_f32[1] + v53.m128_f32[2]);
            if (len2 > d2)
            {
                // barycentric edge tests of cp against the triangle
                __m128 v57 = _mm_sub_ps(v0.v, v1.v);
                __m128 v56 = _mm_sub_ps(cp.v, v2.v);
                __m128 v58 = _mm_sub_ps(v1.v, v2.v);
                __m128 v55 = _mm_sub_ps(cp.v, v0.v);
                __m128 v59 = _mm_sub_ps(v2.v, v0.v);
                __m128 v54 = _mm_sub_ps(cp.v, v1.v);
                __m128 a = _mm_mul_ps(
                    _mm_sub_ps(
                        _mm_mul_ps(_mm_shuffle_ps(v57, v57, 9),
                                   _mm_shuffle_ps(v56, v56, 18)),
                        _mm_mul_ps(_mm_shuffle_ps(v57, v57, 18),
                                   _mm_shuffle_ps(v56, v56, 9))),
                    plane.v);
                float s0 = a.m128_f32[0] + (a.m128_f32[1] + a.m128_f32[2]);
                __m128 b = _mm_mul_ps(
                    _mm_sub_ps(
                        _mm_mul_ps(_mm_shuffle_ps(v58, v58, 9),
                                   _mm_shuffle_ps(v55, v55, 18)),
                        _mm_mul_ps(_mm_shuffle_ps(v58, v58, 18),
                                   _mm_shuffle_ps(v55, v55, 9))),
                    plane.v);
                float s1 = b.m128_f32[0] + (b.m128_f32[1] + b.m128_f32[2]);
                __m128 c = _mm_mul_ps(
                    _mm_sub_ps(
                        _mm_mul_ps(_mm_shuffle_ps(v59, v59, 9),
                                   _mm_shuffle_ps(v54, v54, 18)),
                        _mm_mul_ps(_mm_shuffle_ps(v59, v59, 18),
                                   _mm_shuffle_ps(v54, v54, 9))),
                    plane.v);
                float s2 = c.m128_f32[0] + (c.m128_f32[1] + c.m128_f32[2]);
                if (s0 >= 0.0f && s1 >= 0.0f && s2 >= 0.0f)
                {
                    cinfo.ni.v = _mm_setr_ps(plane.v.m128_f32[0],
                                             plane.v.m128_f32[1],
                                             plane.v.m128_f32[2],
                                             plane.v.m128_f32[3]);
                    cinfo.pi = cp;
                    sflags = obj->sflags;
                    cflags = obj->cflags;
                    hit = true;
                }
            }
        }
    }
    return hit;
}

// ============================================================================
// collide_segment (triangle mesh) - ea: 0x61EB80 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x0061EB80
bool collide_segment(const cdl_object_t& obj, const math::Dir3* vert_list,
                     const unsigned char* index_list,
                     unsigned short first_vert, int num_indices,
                     const math::Position3& p0, const math::Position3& p1,
                     float& t, math::Position3& normal, int* tid)
{
    math::Dir3 center;
    center.v = _mm_setr_ps(obj.center[0], obj.center[1], obj.center[2],
                           0.0f);
    __m128 delta = _mm_sub_ps(p1.v, p0.v);
    __m128 toCenter = _mm_sub_ps(center.v, p0.v);
    __m128 c2 = _mm_mul_ps(toCenter, toCenter);
    float dist2 = c2.m128_f32[0] + c2.m128_f32[1] + c2.m128_f32[2];
    float r2 = obj.sphere_radius * obj.sphere_radius;
    if (dist2 > r2)
    {
        __m128 proj = _mm_mul_ps(toCenter, delta);
        float pd = proj.m128_f32[0] + proj.m128_f32[1] + proj.m128_f32[2];
        if (pd < 0.0f)
            return false;
        __m128 d2 = _mm_mul_ps(delta, delta);
        float dl = d2.m128_f32[0] + d2.m128_f32[1] + d2.m128_f32[2];
        float pc = pd * pd;
        if ((dl * dist2 - pc) > (dl * r2))
            return false;
    }
    bool hit = false;
    if (num_indices % 3 != 0
        && _tlAssert("c:\\cod\\code\\game\\CollisionMgr.cpp", 4340,
                     "num_indices % 3 == 0", defaultFileName))
        __debugbreak();
    if (tid != nullptr)
        *tid = -1;
    if (num_indices > 0)
    {
        for (int i = 0; i < num_indices; i += 3)
        {
            math::Dir3 v0 =
                vert_list[first_vert + index_list[i]];
            math::Dir3 v1 =
                vert_list[first_vert + index_list[i + 1]];
            math::Dir3 v2 =
                vert_list[first_vert + index_list[i + 2]];
            math::Position3 pv0;
            math::Position3 pv1;
            math::Position3 pv2;
            pv0.v = v0.v;
            pv1.v = v1.v;
            pv2.v = v2.v;
            math::Vector4 n = calc_normal(pv0, pv1, pv2);
            __m128 nd = _mm_mul_ps(delta, n.v);
            float ndv = nd.m128_f32[0] + nd.m128_f32[1] + nd.m128_f32[2];
            if (ndv >= 0.0f)
                continue;
            __m128 p0n = _mm_mul_ps(n.v, p0.v);
            float d0 = p0n.m128_f32[0] + p0n.m128_f32[1]
                + p0n.m128_f32[2] + n.v.m128_f32[3];
            __m128 p1n = _mm_mul_ps(n.v, p1.v);
            float d1 = p1n.m128_f32[0] + p1n.m128_f32[1]
                + p1n.m128_f32[2] + n.v.m128_f32[3];
            if (d0 * d1 >= 0.0f)
                continue;
            float frac = d0 / (d0 - d1);
            if (t > frac)
            {
                math::Position3 p;
                p.v = _mm_add_ps(p0.v, _mm_mul_ps(delta, _mm_set1_ps(frac)));
                __m128 e0 = _mm_sub_ps(v0.v, v1.v);
                __m128 e1 = _mm_sub_ps(v1.v, v2.v);
                __m128 e2 = _mm_sub_ps(v2.v, v0.v);
                __m128 q0 = _mm_sub_ps(p.v, v1.v);
                __m128 q1 = _mm_sub_ps(p.v, v2.v);
                __m128 q2 = _mm_sub_ps(p.v, v0.v);
                __m128 c0 = _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(e0, e0, 0x09),
                               _mm_shuffle_ps(q0, q0, 0x12)),
                    _mm_mul_ps(_mm_shuffle_ps(e0, e0, 0x12),
                               _mm_shuffle_ps(q0, q0, 0x09)));
                float b0 = c0.m128_f32[0] + c0.m128_f32[1]
                    + c0.m128_f32[2];
                __m128 c1 = _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(e1, e1, 0x09),
                               _mm_shuffle_ps(q1, q1, 0x12)),
                    _mm_mul_ps(_mm_shuffle_ps(e1, e1, 0x12),
                               _mm_shuffle_ps(q1, q1, 0x09)));
                float b1 = c1.m128_f32[0] + c1.m128_f32[1]
                    + c1.m128_f32[2];
                __m128 c2x = _mm_sub_ps(
                    _mm_mul_ps(_mm_shuffle_ps(e2, e2, 0x09),
                               _mm_shuffle_ps(q2, q2, 0x12)),
                    _mm_mul_ps(_mm_shuffle_ps(e2, e2, 0x12),
                               _mm_shuffle_ps(q2, q2, 0x09)));
                float b2 = c2x.m128_f32[0] + c2x.m128_f32[1]
                    + c2x.m128_f32[2];
                float bary[3] = { b2, b0, b1 };
                if ((_mm_movemask_ps(_mm_cmplt_ps(
                         _mm_setr_ps(bary[0], bary[1], bary[2], 0.0f),
                         _mm_setzero_ps()))
                     & 7) == 0)
                {
                    t = frac;
                    normal.v = n.v;
                    hit = true;
                    if (tid != nullptr)
                        *tid = i;
                }
            }
        }
    }
    return hit;
}

// ============================================================================
// collide_segment (proximity, void) - ea: 0x6350B0 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x006350B0
void collide_segment(const proximity_data_t& data,
                     const math::Position3& p0, const math::Position3& p1,
                     float& t, int& sflags, int& cflags,
                     cdl_poly_inl_t* poly)
{
    __m128 v8 = _mm_sub_ps(p1.v, p0.v);
    __m128 v9 = _mm_mul_ps(v8, v8);
    float len2 = v9.m128_f32[0] + (v9.m128_f32[1] + v9.m128_f32[2]);
    if (len2 < 0.001f || len2 > 1680999900.0f)
        return;

    int nbrushes = data.brushes_count;
    for (int i = 0; i < nbrushes; ++i)
    {
        if ((i < 0 || i >= data.brushes_count)
            && _tlAssert(
                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                114, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        const proxy_obj_t& slot = data.brushes_slot[i];
        CGBank* bank =
            ((CGBankManager*)CGBankManager::sInst)->mBankArray[slot.bi];
        unsigned int oi = slot.oi;
        if (oi >= (unsigned int)bank->objects.m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 233;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (oi >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        cdl_object_t* obj =
            &((cdl_object_t*)bank->objects.m_elements)[oi];
        unsigned int brush_index = oi - (unsigned int)bank->nboxes;
        if (brush_index >= (unsigned int)bank->brushes.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        cdl_brush_t* brush =
            &((cdl_brush_t*)bank->brushes.m_elements)[brush_index];
        unsigned int first_side = (unsigned int)brush->first_side;
        if (first_side >= (unsigned int)bank->brush_sides.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        math::Position3 bmin;
        math::Position3 bmax;
        bmax.v = _mm_add_ps(
            _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                        0.0f),
            _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                        obj->box_radius[2], 0.0f));
        bmin.v = _mm_sub_ps(
            _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                        0.0f),
            _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                        obj->box_radius[2], 0.0f));
        if (collide_brush_segment(
                p0, p1, bmin, bmax,
                &((const cdlPlane*)bank->brush_sides.m_elements)
                    [first_side],
                (unsigned int)brush->num_sides, t, nullptr))
        {
            sflags = obj->sflags;
            cflags = obj->cflags;
            if (t == 0.0f)
                return;
        }
    }

    int nboxes = data.boxes_count;
    for (int i = 0; i < nboxes; ++i)
    {
        if ((i < 0 || i >= data.boxes_count)
            && _tlAssert(
                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                114, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        const proxy_obj_t& slot = data.boxes_slot[i];
        CGBank* bank =
            ((CGBankManager*)CGBankManager::sInst)->mBankArray[slot.bi];
        unsigned int oi = slot.oi;
        if (oi >= (unsigned int)bank->objects.m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 233;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (oi >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        cdl_object_t* obj =
            &((cdl_object_t*)bank->objects.m_elements)[oi];
        math::Position3 bmin;
        math::Position3 bmax;
        bmax.v = _mm_add_ps(
            _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                        0.0f),
            _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                        obj->box_radius[2], 0.0f));
        bmin.v = _mm_sub_ps(
            _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                        0.0f),
            _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                        obj->box_radius[2], 0.0f));
        if (collide_box_segment(p0, p1, bmin, bmax, t, nullptr))
        {
            sflags = obj->sflags;
            cflags = obj->cflags;
            if (t == 0.0f)
                return;
        }
    }

    if (poly != nullptr)
        poly->valid = false;
    int npolies = data.polies_count;
    for (int i = 0; i < npolies; ++i)
    {
        if ((i < 0 || i >= data.polies_count)
            && _tlAssert(
                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                114, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        const bounded_proxy_obj_t& slot = data.polies_slot[i];
        CGBank* bank =
            ((CGBankManager*)CGBankManager::sInst)->mBankArray[slot.bi];
        unsigned int oi = slot.oi;
        if (oi >= (unsigned int)bank->objects.m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 233;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (oi >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        cdl_object_t* obj =
            &((cdl_object_t*)bank->objects.m_elements)[oi];
        unsigned int pi =
            oi - (unsigned int)bank->nbrushes - (unsigned int)bank->nboxes;
        if (pi >= (unsigned int)bank->patches.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        if (pi >= (unsigned int)bank->gjk_patches.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        const cdl_vinfo_t* vinfo =
            &((const cdl_vinfo_t*)bank->gjk_patches.m_elements)[pi];
        unsigned int ind =
            (unsigned int)((cdl_patch_t*)bank->patches.m_elements)[pi]
                .first_index
            + 3 * (unsigned int)slot.ti;
        if (ind >= (unsigned int)bank->patch_inds.m_count
            && _tlAssert(
                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        math::Position3 v0;
        math::Position3 v1;
        math::Position3 v2;
        unpack_poly(
            bank, vinfo,
            &((const unsigned char*)bank->patch_inds.m_elements)[ind], v0,
            v1, v2);
        math::Vector4 plane = calc_normal(v0, v1, v2);

        float d0 = plane.v.m128_f32[0] * p0.v.m128_f32[0]
            + plane.v.m128_f32[1] * p0.v.m128_f32[1]
            + plane.v.m128_f32[2] * p0.v.m128_f32[2]
            + plane.v.m128_f32[3];
        float d1 = plane.v.m128_f32[0] * p1.v.m128_f32[0]
            + plane.v.m128_f32[1] * p1.v.m128_f32[1]
            + plane.v.m128_f32[2] * p1.v.m128_f32[2]
            + plane.v.m128_f32[3];
        if (d0 <= 0.000099999997f || d1 > 0.0f)
            continue;
        math::Position3 cp;
        cp.v = _mm_div_ps(
            _mm_sub_ps(_mm_mul_ps(p0.v, _mm_set1_ps(d1)),
                       _mm_mul_ps(p1.v, _mm_set1_ps(d0))),
            _mm_set1_ps(d1 - d0));
        __m128 v52 = _mm_sub_ps(p0.v, cp.v);
        __m128 v53 = _mm_mul_ps(v52, v52);
        float dist2 =
            v53.m128_f32[0] + (v53.m128_f32[1] + v53.m128_f32[2]);
        if (len2 <= dist2)
            continue;

        __m128 v57 = _mm_sub_ps(v0.v, v1.v);
        __m128 v56 = _mm_sub_ps(cp.v, v2.v);
        __m128 v58 = _mm_sub_ps(v1.v, v2.v);
        __m128 v55 = _mm_sub_ps(cp.v, v0.v);
        __m128 v59 = _mm_sub_ps(v2.v, v0.v);
        __m128 v54 = _mm_sub_ps(cp.v, v1.v);
        __m128 a = _mm_mul_ps(
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(v57, v57, 9),
                           _mm_shuffle_ps(v56, v56, 18)),
                _mm_mul_ps(_mm_shuffle_ps(v57, v57, 18),
                           _mm_shuffle_ps(v56, v56, 9))),
            plane.v);
        float s0 = a.m128_f32[0] + (a.m128_f32[1] + a.m128_f32[2]);
        __m128 b = _mm_mul_ps(
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(v58, v58, 9),
                           _mm_shuffle_ps(v55, v55, 18)),
                _mm_mul_ps(_mm_shuffle_ps(v58, v58, 18),
                           _mm_shuffle_ps(v55, v55, 9))),
            plane.v);
        float s1 = b.m128_f32[0] + (b.m128_f32[1] + b.m128_f32[2]);
        __m128 c = _mm_mul_ps(
            _mm_sub_ps(
                _mm_mul_ps(_mm_shuffle_ps(v59, v59, 9),
                           _mm_shuffle_ps(v54, v54, 18)),
                _mm_mul_ps(_mm_shuffle_ps(v59, v59, 18),
                           _mm_shuffle_ps(v54, v54, 9))),
            plane.v);
        float s2 = c.m128_f32[0] + (c.m128_f32[1] + c.m128_f32[2]);
        if (s0 < 0.0f || s1 < 0.0f || s2 < 0.0f)
            continue;

        __m128 v61 = _mm_mul_ps(v52, plane.v);
        float v68 =
            v61.m128_f32[0] + (v61.m128_f32[1] + v61.m128_f32[2]);
        float tt = (v68 - 0.125f) / (d0 - d1);
        if (t > tt)
        {
            t = tt;
            sflags = obj->sflags;
            cflags = obj->cflags;
            if (poly != nullptr)
            {
                poly->v0[0] = v0.v.m128_f32[0];
                poly->v0[1] = v0.v.m128_f32[1];
                poly->v0[2] = v0.v.m128_f32[2];
                poly->v0[3] = v0.v.m128_f32[3];
                poly->v1[0] = v1.v.m128_f32[0];
                poly->v1[1] = v1.v.m128_f32[1];
                poly->v1[2] = v1.v.m128_f32[2];
                poly->v1[3] = v1.v.m128_f32[3];
                poly->v2[0] = v2.v.m128_f32[0];
                poly->v2[1] = v2.v.m128_f32[1];
                poly->v2[2] = v2.v.m128_f32[2];
                poly->v2[3] = v2.v.m128_f32[3];
                poly->n[0] = plane.v.m128_f32[0];
                poly->n[1] = plane.v.m128_f32[1];
                poly->n[2] = plane.v.m128_f32[2];
                poly->n[3] = plane.v.m128_f32[3];
                poly->sflags = sflags;
                poly->valid = true;
            }
        }
    }
}

// ============================================================================
// collide_segment_poly - ea: 0x61F010 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x0061F010
bool collide_segment_poly(const math::Position3& p0,
                          const math::Position3& p1, cdl_poly_inl_t& poly,
                          cdl_cinfo1& cinfo)
{
    if (!poly.valid)
        return false;

    const float* n = poly.n;
    float v5 = n[3];
    float v30 = n[0] * p0.v.m128_f32[0]
        + n[1] * p0.v.m128_f32[1]
        + n[2] * p0.v.m128_f32[2];
    float v29 = n[0] * p1.v.m128_f32[0]
        + n[1] * p1.v.m128_f32[1]
        + n[2] * p1.v.m128_f32[2];
    float v8 = v30 + v5;
    float v9 = v29 + v5;
    if (v8 <= 0.000099999997f || v9 > 0.0f)
    {
        poly.valid = false;
        return 0;
    }

    // closest point on segment to the triangle plane
    math::Position3 cp;
    cp.v = _mm_div_ps(
        _mm_sub_ps(_mm_mul_ps(p0.v, _mm_set1_ps(v9)),
                   _mm_mul_ps(p1.v, _mm_set1_ps(v8))),
        _mm_set1_ps(v9 - v8));

    const float* v0 = poly.v0;
    const float* v1 = poly.v1;
    const float* v2 = poly.v2;
    __m128 v16 = _mm_sub_ps(
        _mm_setr_ps(v0[0], v0[1], v0[2], v0[3]),
        _mm_setr_ps(v1[0], v1[1], v1[2], v1[3]));
    __m128 v17 = _mm_sub_ps(
        cp.v, _mm_setr_ps(v1[0], v1[1], v1[2], v1[3]));
    __m128 v18 = _mm_sub_ps(
        cp.v, _mm_setr_ps(v2[0], v2[1], v2[2], v2[3]));
    __m128 v19 = _mm_sub_ps(
        _mm_setr_ps(v1[0], v1[1], v1[2], v1[3]),
        _mm_setr_ps(v2[0], v2[1], v2[2], v2[3]));
    __m128 v20 = _mm_sub_ps(
        _mm_setr_ps(v2[0], v2[1], v2[2], v2[3]),
        _mm_setr_ps(v0[0], v0[1], v0[2], v0[3]));
    __m128 v21 = _mm_sub_ps(
        cp.v, _mm_setr_ps(v0[0], v0[1], v0[2], v0[3]));
    __m128 pn = _mm_setr_ps(n[0], n[1], n[2], n[3]);

    __m128 a = _mm_mul_ps(
        _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(v16, v16, 9),
                       _mm_shuffle_ps(v17, v17, 18)),
            _mm_mul_ps(_mm_shuffle_ps(v16, v16, 18),
                       _mm_shuffle_ps(v17, v17, 9))),
        pn);
    float s0 = a.m128_f32[0] + (a.m128_f32[1] + a.m128_f32[2]);
    __m128 b = _mm_mul_ps(
        _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(v19, v19, 9),
                       _mm_shuffle_ps(v18, v18, 18)),
            _mm_mul_ps(_mm_shuffle_ps(v19, v19, 18),
                       _mm_shuffle_ps(v18, v18, 9))),
        pn);
    float s1 = b.m128_f32[0] + (b.m128_f32[1] + b.m128_f32[2]);
    __m128 c = _mm_mul_ps(
        _mm_sub_ps(
            _mm_mul_ps(_mm_shuffle_ps(v20, v20, 9),
                       _mm_shuffle_ps(v21, v21, 18)),
            _mm_mul_ps(_mm_shuffle_ps(v20, v20, 18),
                       _mm_shuffle_ps(v21, v21, 9))),
        pn);
    float s2 = c.m128_f32[0] + (c.m128_f32[1] + c.m128_f32[2]);
    if (s0 < 0.0f || s1 < 0.0f || s2 < 0.0f)
    {
        poly.valid = false;
        return 0;
    }
    cinfo.ni.v = _mm_setr_ps(n[0], n[1], n[2], n[3]);
    cinfo.pi = cp;
    return 1;
}

// ============================================================================
// collide_sphere_box - ea: 0x61E890 (CollisionMgr.cpp)
// ============================================================================
float fudge_2;  // game.o @ 0xDF8D34
// ea: 0x0061E890
bool collide_sphere_box(const math::Position3& sphere_center,
                        float sphere_radius, const cdl_object_t& box,
                        math::Position3& new_sphere_center)
{
    float ext[3] = { box.box_radius[0], box.box_radius[1],
                     box.box_radius[2] };
    float center[3] = { box.center[0], box.center[1], box.center[2] };
    float delta[3] = { sphere_center.v.m128_f32[0] - center[0],
                       sphere_center.v.m128_f32[1] - center[1],
                       sphere_center.v.m128_f32[2] - center[2] };
    float dist[3] = { ext[0] + sphere_radius - fabsf(delta[0]),
                      ext[1] + sphere_radius - fabsf(delta[1]),
                      ext[2] + sphere_radius - fabsf(delta[2]) };
    if (dist[0] < 0.0f)
        return false;
    int best = 0;
    float bestDist = dist[0];
    for (int i = 1; i < 3; ++i)
    {
        if (dist[i] < 0.0f)
            return false;
        if (bestDist > dist[i])
        {
            bestDist = dist[i];
            best = i;
        }
    }
    float push = dist[best] + fudge_2;
    new_sphere_center = sphere_center;
    float axis = delta[best] >= 0.0f ? push : -push;
    new_sphere_center.v.m128_f32[best] = center[best] + axis;
    return true;
}

// ============================================================================
// collide_sphere_brush - ea: 0x61E660 (CollisionMgr.cpp)
// ============================================================================
float fudge_1;  // game.o @ 0xDF8D30
// ea: 0x0061E660
bool collide_sphere_brush(math::Position3& sphere_center, float sphere_radius,
                          const cdl_object_t& obj, const cdlPlane* sides,
                          unsigned int nsides,
                          math::Position3& new_sphere_center)
{
    float ext[3] = { obj.box_radius[0], obj.box_radius[1],
                     obj.box_radius[2] };
    float center[3] = { obj.center[0], obj.center[1], obj.center[2] };
    float delta[3] = { sphere_center.v.m128_f32[0] - center[0],
                       sphere_center.v.m128_f32[1] - center[1],
                       sphere_center.v.m128_f32[2] - center[2] };
    float dist[3] = { ext[0] + sphere_radius - fabsf(delta[0]),
                      ext[1] + sphere_radius - fabsf(delta[1]),
                      ext[2] + sphere_radius - fabsf(delta[2]) };
    if (dist[0] < 0.0f)
        return false;
    int best = 0;
    float bestDist = dist[0];
    for (int i = 1; i < 3; ++i)
    {
        if (dist[i] < 0.0f)
            return false;
        if (bestDist > dist[i])
        {
            bestDist = dist[i];
            best = i;
        }
    }
    int bestPlane = -1;
    for (unsigned int i = 0; i < nsides; ++i)
    {
        const cdlPlane& plane = sides[i];
        float offs = plane.packed[3];
        float dot = plane.packed[0] * sphere_center.v.m128_f32[0]
            + plane.packed[1] * sphere_center.v.m128_f32[1]
            + plane.packed[2] * sphere_center.v.m128_f32[2];
        float d = -(dot - offs - sphere_radius);
        if (d < 0.0f)
            return false;
        if (bestDist > d)
        {
            bestDist = d;
            bestPlane = (int)i;
        }
    }
    if (bestPlane != -1)
    {
        const cdlPlane& plane = sides[bestPlane];
        float push = fudge_1 + bestDist;
        new_sphere_center.v = _mm_add_ps(
            sphere_center.v,
            _mm_mul_ps(_mm_setr_ps(plane.packed[0], plane.packed[1],
                                   plane.packed[2], 0.0f),
                       _mm_set1_ps(push)));
        return true;
    }
    float push = fudge_1 + dist[best];
    new_sphere_center = sphere_center;
    float axis = delta[best] >= 0.0f ? push : -push;
    new_sphere_center.v.m128_f32[best] = center[best] + axis;
    return true;
}

// ============================================================================
// PM_UpdateMeleeAssistAim - ea: 0x62DE20 (bg_pmove.cpp melee assist)
// ============================================================================
extern vmCvar_t bg_meleeassistaspeed;  // ?bg_meleeassistaspeed@@3UvmCvar_t@@A (game.o @ 0xF441D0)
extern void vectosignedangles(const float* const vec,
                              float* const angles);  // q_math
// ea: 0x0062DE20
void PM_UpdateMeleeAssistAim(PlayerState* ps, int msec)
{
    unsigned int mVal = ps->mMeleeAssistTarget.mHandle.mVal;
    if (mVal == 0)
        return;
    unsigned int idx = mVal & 0xFFF;
    if (idx >= 0x540
        || mVal >> 12
            != (unsigned int)EntityHandleDb::sInst.mElements[idx].mKey)
        return;
    Entity* ent = EntityHandleDb::sInst.mElements[idx].mObject;
    if (ent == nullptr)
        return;

    const float* eo = ent->r.currentOrigin.v.m128_f32;
    float tgt[3] = { eo[0], eo[1], eo[2] };
    float my[3] = { ps->origin.v.m128_f32[0], ps->origin.v.m128_f32[1],
                    ps->origin.v.m128_f32[2] };
    float dir[3] = { tgt[0] - my[0], tgt[1] - my[1], tgt[2] - my[2] };
    VectorNormalize(dir);
    float want[3];
    vectosignedangles(dir, want);

    float dyaw = want[1] - ps->viewangles[1];
    float dpitch = want[2] - ps->viewangles[2];
    float step = (float)(bg_meleeassistaspeed.integer * msec) * 0.001f;
    float neg = -step;

    while (dyaw > 180.0f)
        dyaw -= 360.0f;
    while (dyaw < -180.0f)
        dyaw += 360.0f;
    if (dyaw < neg)
        dyaw = neg;
    else if (dyaw > step)
        dyaw = step;

    while (dpitch > 180.0f)
        dpitch -= 360.0f;
    while (dpitch < -180.0f)
        dpitch += 360.0f;
    if (dpitch < neg)
        dpitch = neg;
    else if (dpitch > step)
        dpitch = step;

    float yawAdj = want[0];
    ps->delta_angles[0] += (int)(yawAdj * 182.04445f);
    ps->delta_angles[1] += (int)(dyaw * 182.04445f);
    ps->delta_angles[2] += (int)(dpitch * 182.04445f);
    ps->viewangles[0] += yawAdj;
    ps->viewangles[1] += dyaw;
    ps->viewangles[2] += dpitch;
}

// ============================================================================
// TestInLeaf / SightTraceThroughLeaf - ea: 0x623D40 / 0x624070
// (CollisionMgr.cpp DCGSet leaf sweep)
// ============================================================================
extern bool collide_box_segment(traceWork_t* tw,
                                const math::Position3& bmin,
                                const math::Position3& bmax);

// ea: 0x00623D40
void TestInLeaf(traceWork_t* tw, const DCGSet* set)
{
    if (set == nullptr)
        return;
    cdl_object_t* objects = (cdl_object_t*)set->objects_m_elements;
    int nboxes = set->nboxes;
    if (nboxes != 0)
    {
        for (int i = 0; i < nboxes; ++i)
        {
            const cdl_object_t& obj = objects[i];
            math::Position3 vmin;
            math::Position3 vmax;
            vmin.v = _mm_sub_ps(_mm_setr_ps(obj.center[0], obj.center[1],
                                            obj.center[2], 0.0f),
                                _mm_setr_ps(obj.box_radius[0],
                                            obj.box_radius[1],
                                            obj.box_radius[2], 0.0f));
            vmax.v = _mm_add_ps(_mm_setr_ps(obj.center[0], obj.center[1],
                                            obj.center[2], 0.0f),
                                _mm_setr_ps(obj.box_radius[0],
                                            obj.box_radius[1],
                                            obj.box_radius[2], 0.0f));
            TestBoxInBox(tw, vmin, vmax, (unsigned int)obj.cflags);
            if (tw->trace_allsolid != 0)
                return;
        }
    }
    cdl_brush_t* brushes = (cdl_brush_t*)set->brushes_m_elements;
    int nbrushes = set->nbrushes;
    for (int i = 0; i < nbrushes; ++i)
    {
        const cdl_object_t& obj = objects[nboxes + i];
        const cdl_brush_t& brush = brushes[i];
        math::Position3 vmin;
        math::Position3 vmax;
        vmax.v = _mm_add_ps(_mm_setr_ps(obj.center[0], obj.center[1],
                                        obj.center[2], 0.0f),
                            _mm_setr_ps(obj.box_radius[0], obj.box_radius[1],
                                        obj.box_radius[2], 0.0f));
        vmin.v = _mm_sub_ps(_mm_setr_ps(obj.center[0], obj.center[1],
                                        obj.center[2], 0.0f),
                            _mm_setr_ps(obj.box_radius[0], obj.box_radius[1],
                                        obj.box_radius[2], 0.0f));
        cdlPlane* sides = (cdlPlane*)set->brush_sides_m_elements;
        TestBoxInBrush(tw, vmin, vmax, &sides[brush.first_side],
                       brush.num_sides, (unsigned int)obj.cflags);
        if (tw->trace_allsolid != 0)
            return;
    }
}

// ea: 0x00624070
int SightTraceThroughLeaf(traceWork_t* tw, const DCGSet* set)
{
    if (set == nullptr)
        return 0;
    cdl_object_t* objects = (cdl_object_t*)set->objects_m_elements;
    int nboxes = set->nboxes;
    for (int i = 0; i < nboxes; ++i)
    {
        const cdl_object_t& obj = objects[i];
        if ((obj.cflags & tw->trace_contents) != 0)
        {
            math::Position3 vmin;
            math::Position3 vmax;
            vmax.v = _mm_add_ps(_mm_setr_ps(obj.center[0], obj.center[1],
                                            obj.center[2], 0.0f),
                                _mm_setr_ps(obj.box_radius[0], obj.box_radius[1],
                                            obj.box_radius[2], 0.0f));
            vmin.v = _mm_sub_ps(_mm_setr_ps(obj.center[0], obj.center[1],
                                            obj.center[2], 0.0f),
                                _mm_setr_ps(obj.box_radius[0], obj.box_radius[1],
                                            obj.box_radius[2], 0.0f));
            if (collide_box_segment(tw, vmin, vmax) != 0)
                return 1;
        }
    }
    cdl_brush_t* brushes = (cdl_brush_t*)set->brushes_m_elements;
    int nbrushes = set->nbrushes;
    for (int i = 0; i < nbrushes; ++i)
    {
        const cdl_object_t& obj = objects[nboxes + i];
        if ((obj.cflags & tw->trace_contents) != 0)
        {
            const cdl_brush_t& brush = brushes[i];
            math::Position3 vmin;
            math::Position3 vmax;
            vmax.v = _mm_add_ps(_mm_setr_ps(obj.center[0], obj.center[1],
                                            obj.center[2], 0.0f),
                                _mm_setr_ps(obj.box_radius[0], obj.box_radius[1],
                                            obj.box_radius[2], 0.0f));
            vmin.v = _mm_sub_ps(_mm_setr_ps(obj.center[0], obj.center[1],
                                            obj.center[2], 0.0f),
                                _mm_setr_ps(obj.box_radius[0], obj.box_radius[1],
                                            obj.box_radius[2], 0.0f));
            cdlPlane* sides = (cdlPlane*)set->brush_sides_m_elements;
            if (collide_brush_segment(tw, vmin, vmax,
                                      &sides[brush.first_side],
                                      brush.num_sides) != 0)
                return 1;
        }
    }
    return 0;
}

// ============================================================================
// TracePointThroughLeaf - ea: 0x622FD0 (CollisionMgr.cpp DCGSet leaf sweep)
// ============================================================================
// ea: 0x00622FD0
void TracePointThroughLeaf(traceWork_t* tw, const DCGSet* set)
{
    cdl_object_t* objects = (cdl_object_t*)set->objects_m_elements;
    int nboxes = set->nboxes;
    if (nboxes != 0)
    {
        for (int i = 0; i < nboxes; ++i)
        {
            const cdl_object_t& obj = objects[i];
            if ((obj.cflags & tw->trace_contents) != 0)
            {
                math::Position3 vmin;
                math::Position3 vmax;
                vmax.v = _mm_add_ps(
                    _mm_setr_ps(obj.center[0], obj.center[1], obj.center[2],
                                0.0f),
                    _mm_setr_ps(obj.box_radius[0], obj.box_radius[1],
                                obj.box_radius[2], 0.0f));
                vmin.v = _mm_sub_ps(
                    _mm_setr_ps(obj.center[0], obj.center[1], obj.center[2],
                                0.0f),
                    _mm_setr_ps(obj.box_radius[0], obj.box_radius[1],
                                obj.box_radius[2], 0.0f));
                collide_box_segment(tw, vmin, vmax);
                if (tw->trace_fraction == 0.0f)
                    return;
            }
        }
    }
    cdl_brush_t* brushes = (cdl_brush_t*)set->brushes_m_elements;
    int nbrushes = set->nbrushes;
    for (int i = 0; i < nbrushes; ++i)
    {
        const cdl_object_t& obj = objects[nboxes + i];
        if ((tw->trace_contents & obj.cflags) != 0)
        {
            const cdl_brush_t& brush = brushes[i];
            math::Position3 vmin;
            math::Position3 vmax;
            vmax.v = _mm_add_ps(
                _mm_setr_ps(obj.center[0], obj.center[1], obj.center[2],
                            0.0f),
                _mm_setr_ps(obj.box_radius[0], obj.box_radius[1],
                            obj.box_radius[2], 0.0f));
            vmin.v = _mm_sub_ps(
                _mm_setr_ps(obj.center[0], obj.center[1], obj.center[2],
                            0.0f),
                _mm_setr_ps(obj.box_radius[0], obj.box_radius[1],
                            obj.box_radius[2], 0.0f));
            cdlPlane* sides = (cdlPlane*)set->brush_sides_m_elements;
            collide_brush_segment(tw, vmin, vmax, &sides[brush.first_side],
                                  brush.num_sides);
            if (tw->trace_fraction == 0.0f)
                return;
        }
    }
}

// ============================================================================
// collide_box_velocity_sphere - ea: 0x60CB80 (CollisionMgr.cpp slab sweep)
// ============================================================================
// ea: 0x0060CB80
void collide_box_velocity_sphere(traceWork_t* tw,
                                 const math::Position3& bmin,
                                 const math::Position3& bmax)
{
    float v49[16];  // [0]=bound, [4]=start, [8]=end, [12]=lead normal
    v49[8] = bmin.v.m128_f32[0];
    v49[9] = bmin.v.m128_f32[1];
    v49[10] = bmin.v.m128_f32[2];
    v49[11] = bmin.v.m128_f32[3];
    float fraction = tw->trace_fraction;
    float v10 = -1.0f;
    v49[4] = tw->start.v.m128_f32[0];
    v49[5] = tw->start.v.m128_f32[1];
    v49[6] = tw->start.v.m128_f32[2];
    v49[7] = tw->start.v.m128_f32[3];
    v49[0] = tw->end.v.m128_f32[0];
    v49[1] = tw->end.v.m128_f32[1];
    v49[2] = tw->end.v.m128_f32[2];
    v49[3] = tw->end.v.m128_f32[3];
    float v13 = 0.0f;
    int v11 = 1;
    int v12 = 0;
    v49[12] = 0.0f;
    v49[13] = 0.0f;
    v49[14] = 0.0f;
    v49[15] = 0.0f;
    int pass = 0;
    while (1)
    {
        const float* radiusOffset = tw->sphere_radiusOffset.v.m128_f32;
        for (int i = 0; i < 3; ++i)
        {
            float v15 = (v49[4 + i] - v49[8 + i]) * v10 - radiusOffset[i];
            float v16 = (v49[i] - v49[8 + i]) * v10 - radiusOffset[i];
            if (v15 <= 0.0f)
            {
                if (v16 > 0.0f)
                {
                    float v19 = v15 - v16;
                    v11 = 0;
                    if (v15 > v19 * fraction)
                    {
                        fraction = v15 / v19;
                        if (v13 >= v15 / v19)
                            return;
                    }
                }
            }
            else
            {
                float v17 = v15 - v16;
                if (v16 > 0.0f)
                {
                    if (v17 <= 0.0f || v16 >= 0.125f)
                        return;
                    v11 = 0;
                }
                float v18 = v15 - 0.125f;
                if (v18 > v17 * v13)
                {
                    v13 = v18 / v17;
                    if (v18 / v17 >= fraction)
                        return;
                }
                else if (v12 == 0)
                {
                    v49[12] = 0.0f;
                    v49[13] = 0.0f;
                    v49[14] = 0.0f;
                    v49[15] = 0.0f;
                    v12 = 1;
                    v49[12 + i] = v10;
                }
            }
        }
        if (pass == 0)
        {
            v10 = 1.0f;
            v49[8] = bmax.v.m128_f32[0];
            v49[9] = bmax.v.m128_f32[1];
            v49[10] = bmax.v.m128_f32[2];
            v49[11] = bmax.v.m128_f32[3];
            pass = 1;
            continue;
        }
        break;
    }
    if (v12 != 0)
    {
        tw->trace_fraction = v13;
        tw->trace_normal[0] = v49[12];
        tw->trace_normal[1] = v49[13];
        tw->trace_normal[2] = v49[14];
        tw->trace_normal[3] = v49[15];
    }
    else
    {
        tw->trace_startsolid = 1;
        if (v11 != 0)
        {
            tw->trace_allsolid = 1;
            tw->trace_fraction = 0.0f;
        }
    }
}

// ============================================================================
// TraceSphereThroughLeaf - ea: 0x622C80 (CollisionMgr.cpp DCGSet leaf sweep)
// ============================================================================
// ea: 0x00622C80
void TraceSphereThroughLeaf(traceWork_t* tw, const DCGSet* set)
{
    cdl_object_t* objects = (cdl_object_t*)set->objects_m_elements;
    int nboxes = set->nboxes;
    if (nboxes != 0)
    {
        for (int i = 0; i < nboxes; ++i)
        {
            const cdl_object_t& obj = objects[i];
            if ((obj.cflags & tw->trace_contents) != 0)
            {
                math::Position3 vmin;
                math::Position3 vmax;
                vmax.v = _mm_add_ps(
                    _mm_setr_ps(obj.center[0], obj.center[1], obj.center[2],
                                0.0f),
                    _mm_setr_ps(obj.box_radius[0], obj.box_radius[1],
                                obj.box_radius[2], 0.0f));
                vmin.v = _mm_sub_ps(
                    _mm_setr_ps(obj.center[0], obj.center[1], obj.center[2],
                                0.0f),
                    _mm_setr_ps(obj.box_radius[0], obj.box_radius[1],
                                obj.box_radius[2], 0.0f));
                collide_box_velocity_sphere(tw, vmin, vmax);
                if (tw->trace_fraction == 0.0f)
                    return;
            }
        }
    }
    cdl_brush_t* brushes = (cdl_brush_t*)set->brushes_m_elements;
    int nbrushes = set->nbrushes;
    for (int i = 0; i < nbrushes; ++i)
    {
        const cdl_object_t& obj = objects[nboxes + i];
        if ((tw->trace_contents & obj.cflags) != 0)
        {
            const cdl_brush_t& brush = brushes[i];
            math::Position3 vmin;
            math::Position3 vmax;
            vmax.v = _mm_add_ps(
                _mm_setr_ps(obj.center[0], obj.center[1], obj.center[2],
                            0.0f),
                _mm_setr_ps(obj.box_radius[0], obj.box_radius[1],
                            obj.box_radius[2], 0.0f));
            vmin.v = _mm_sub_ps(
                _mm_setr_ps(obj.center[0], obj.center[1], obj.center[2],
                            0.0f),
                _mm_setr_ps(obj.box_radius[0], obj.box_radius[1],
                            obj.box_radius[2], 0.0f));
            cdlPlane* sides = (cdlPlane*)set->brush_sides_m_elements;
            collide_brush_velocity_sphere(tw, vmin, vmax,
                                          &sides[brush.first_side],
                                          brush.num_sides);
            if (tw->trace_fraction == 0.0f)
                return;
        }
    }
}

// ============================================================================
// ClipHandleToDCGSet - ea: 0x622C00 (CollisionMgr.cpp)
// ============================================================================
class DCGBank;
class DCGBankManager {
public:
    static DCGBankManager* sInst;  // ?sInst@DCGBankManager@@2PAV1@A @ 0xF4F43C
    const DCGSet* GetDCGSet(TPakId pakId, int handle);  // ?GetDCGSet@DCGBankManager@@QBEPBVDCGSet@@W4TPakId@@H@Z
    void*   mBankArray[99];         // +0x04 (0x18C bytes; DCGBank* per pak)
    struct TempDCGSet {
        uint16_t nboxes;          // +0x00
        uint16_t nbrushes;        // +0x02
        int      objects_m_count;      // +0x04
        void*    objects_m_elements;   // +0x08
        int      brushes_m_count;      // +0x0C
        void*    brushes_m_elements;   // +0x10
        int      gjk_brushes_m_count;  // +0x14
        void*    gjk_brushes_m_elements; // +0x18
        int      brush_sides_m_count;  // +0x1C
        void*    brush_sides_m_elements; // +0x20
        int      brush_verts_m_count;  // +0x24
        void*    brush_verts_m_elements; // +0x28
        uint8_t  _pad2C[0x30 - 0x2C];
        math::Position3 min;      // +0x30
        math::Position3 max;      // +0x40
        math::Position3 center;   // +0x50
        float    radius;          // +0x60
        float    radius2;         // +0x64
        int      id;              // +0x68
        TempDCGSet();   // ??0TempDCGSet@DCGBankManager@@QAE@XZ (game.o 0x638800)
        ~TempDCGSet()   // ??1TempDCGSet@DCGBankManager@@QAE@XZ (inline COMDAT 0x661AB0)
        {
            if (this->brush_verts_m_elements != nullptr)
                tlMemFree(this->brush_verts_m_elements);
            this->brush_verts_m_elements = nullptr;
            this->brush_verts_m_count = 0;
            if (this->brush_sides_m_elements != nullptr)
                tlMemFree(this->brush_sides_m_elements);
            this->brush_sides_m_elements = nullptr;
            this->brush_sides_m_count = 0;
            if (this->gjk_brushes_m_elements != nullptr)
                tlMemFree(this->gjk_brushes_m_elements);
            this->gjk_brushes_m_elements = nullptr;
            this->gjk_brushes_m_count = 0;
            if (this->brushes_m_elements != nullptr)
                tlMemFree(this->brushes_m_elements);
            this->brushes_m_elements = nullptr;
            this->brushes_m_count = 0;
            if (this->objects_m_elements != nullptr)
                tlMemFree(this->objects_m_elements);
            this->objects_m_elements = nullptr;
            this->objects_m_count = 0;
        }
    };
    TempDCGSet mBoxDCGSet;       // +0x190 (0x6C bytes)
private:
    void AddBank(TPakId pakId, DCGBank* bank);  // ?AddBank@DCGBankManager@@AAEXW4TPakId@@PAVDCGBank@@@Z (game.o 0x61FC60)
    virtual void UnloadBank(TPakId pakId);      // ?UnloadBank@DCGBankManager@@EAEXW4TPakId@@@Z (game.o 0x61FCD0)
public:
    void DecodeDCGBank(const char* name, unsigned char* data, int size,
                       TPakId pakId);          // ?DecodeDCGBank@DCGBankManager@@QAEXPBDPAEHW4TPakId@@@Z (game.o 0x629DF0)
    DCGBankManager();             // ??0DCGBankManager@@QAE@XZ (game.o 0x6388F0)
    virtual ~DCGBankManager();     // ??1DCGBankManager@@UAE@XZ (game.o 0x629D90)
};
DCGBankManager* DCGBankManager::sInst = nullptr;
extern void AssetBankSet_Dtor(void* self);   // AssetBankSet::~AssetBankSet
extern void AssetBankSet_ctor(void* self);   // AssetBankSet::AssetBankSet
extern const void* DCGBank_get_set(void* bank, int id);  // ?get_set@DCGBank@@QBEPBVDCGSet@@H@Z
extern void GetPakPrerequisites(TPakId pakId,
                                void* prereqs);  // ?GetPakPrerequisites@@YAXW4TPakId@@AAV?$ae_sized_array@W4TPakId@@$0CA@@@@Z @ 0x64DC0

// ea: 0x0061FCF0
const DCGSet* DCGBankManager::GetDCGSet(TPakId pakId, int handle)
{
    TPakId curPak = CurPakId();
    const DCGSet* set = nullptr;
    if (this->mBankArray[(int)pakId] != nullptr)
    {
        set = (const DCGSet*)DCGBank_get_set(this->mBankArray[(int)pakId],
                                             handle);
        if (set != nullptr)
            goto prereq;
    }
    if (this->mBankArray[(int)curPak] != nullptr)
    {
        set = (const DCGSet*)DCGBank_get_set(this->mBankArray[(int)curPak],
                                             handle);
        if (set != nullptr)
            goto prereq;
    }
    {
        int globalPak = *(int*)((char*)PakManager::sInst + 0x30);
        if (this->mBankArray[globalPak] != nullptr)
            set = (const DCGSet*)DCGBank_get_set(
                this->mBankArray[globalPak], handle);
        else
            set = nullptr;
    }
prereq:
    {
        ae_sized_array<TPakId, 32> prereqs;
        prereqs.m_size = 0;
        GetPakPrerequisites(pakId, &prereqs);
        if (set == nullptr && prereqs.m_size > 0)
        {
            for (int v9 = 0; v9 < prereqs.m_size; ++v9)
            {
                if (v9 >= 0x20)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                    AeAssert::gCurrentLine = 154;
                    AeAssert::gCurrentExpr =
                        "idx >= 0 && idx < _CAPACITY";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("out of bounds"))
                        __debugbreak();
                }
                if (prereqs.m_elements[v9] != PAK_ID_INVALID)
                {
                    void* bank = this->mBankArray[
                        (int)prereqs.m_elements[v9]];
                    if (bank != nullptr)
                    {
                        set = (const DCGSet*)DCGBank_get_set(bank, handle);
                        if (set != nullptr)
                            return set;
                    }
                    else
                    {
                        set = nullptr;
                    }
                }
            }
        }
    }
    return set;
}

extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* msg);

// ea: 0x00638800
DCGBankManager::TempDCGSet::TempDCGSet()
{
    this->nboxes = 1;
    this->nbrushes = 0;
    this->objects_m_count = 0;
    this->objects_m_elements = nullptr;
    void* v2 = tlMemAlloc(0x24, 4, 0);
    this->objects_m_elements = v2;
    if (v2 != nullptr)
    {
        this->objects_m_count = 1;
    }
    else if (_tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 61,
                       "0", "cdl_mem_pool overflow."))
    {
        __debugbreak();
    }
    this->brushes_m_count = 0;
    this->brushes_m_elements = nullptr;
    this->gjk_brushes_m_count = 0;
    this->gjk_brushes_m_elements = nullptr;
    this->brush_sides_m_count = 0;
    this->brush_sides_m_elements = nullptr;
    this->brush_verts_m_count = 0;
    this->brush_verts_m_elements = nullptr;
    memset(&this->center, 0, sizeof(this->center));
    memset(&this->min, 0, sizeof(this->min));
    memset(&this->max, 0, sizeof(this->max));
    this->id = 0;
    this->radius = 0.0f;
    this->radius2 = 0.0f;
}

// ea: 0x006388F0
DCGBankManager::DCGBankManager()
{
    AssetBankSet_ctor(this);
    for (int i = 0; i < 99; ++i)
        this->mBankArray[i] = nullptr;
    gBoxDCGSet = (DCGSet*)&this->mBoxDCGSet;
}

// ea: 0x00629D90
DCGBankManager::~DCGBankManager()
{
    AssetBankSet_Dtor(this);
}

// ea: 0x0061FC60
void DCGBankManager::AddBank(TPakId pakId, DCGBank* bank)
{
    if (this->mBankArray[(int)pakId] != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.cpp";
        AeAssert::gCurrentLine = 179;
        AeAssert::gCurrentExpr = "mBankArray[(int)pakId] == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("bank already loaded!"))
            __debugbreak();
    }
    this->mBankArray[(int)pakId] = bank;
}

// ea: 0x0061FCD0
void DCGBankManager::UnloadBank(TPakId pakId)
{
    this->mBankArray[(int)pakId] = nullptr;
}

// ============================================================================
// DCGBankManager::DecodeDCGBank - ea: 0x629DF0 (cgbank.cpp)
// ============================================================================
extern void DCGBank_load_inplace(void* self, char* base, int* offs);
    // ?load_inplace@DCGBank@@QAEXPADAAH@Z (inplace_xboxr)
extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* msg);  // core/tl_system.cpp

// ea: 0x00629DF0
void DCGBankManager::DecodeDCGBank(const char* name, unsigned char* data,
                                   int size, TPakId pakId)
{
    int offs = 8;
    DCGBank_load_inplace(data, (char*)data, &offs);
    if (offs != size)
    {
        AeAssert::gCurrentAuthor = AeAssert::JSV;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.cpp";
        AeAssert::gCurrentLine = 149;
        AeAssert::gCurrentExpr = "offs == size";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("oops"))
            __debugbreak();
    }
    this->AddBank(pakId, (DCGBank*)data);
    unsigned int n = *(unsigned int*)data;
    int i = 0;
    if (n != 0)
    {
        int pakIda = 0;
        do
        {
            if (i >= n
                && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                             "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            unsigned char* dcg =
                (unsigned char*)(*(unsigned int*)(data + 4) + pakIda);
            unsigned int objCount = *(unsigned int*)(dcg + 4);
            unsigned int j = 0;
            if (objCount != 0)
            {
                int off = 0;
                do
                {
                    if (j >= objCount
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                            "index >= 0 && index < size()", "invalid index"))
                        __debugbreak();
                    unsigned int* el =
                        (unsigned int*)(*(unsigned int*)(dcg + 8) + off);
                    int v14 = *el;
                    if ((0x1000000 & v14) != 0)
                        *el = v14 & 0xFEFFFFFF;
                    if ((*el & 0x4000) != 0)
                        *el = 0x1000000 | *el & 0xFFFFBFFF;
                    ++j;
                    off += 0x24;
                } while (j < *(unsigned int*)(dcg + 4));
            }
            n = *(unsigned int*)data;
            ++i;
            pakIda += 0x70;
        } while (i < n);
    }
}

class GdbFile {
public:
    void* mLayout;   // +0x00 (InplaceTree<uint,uint>*)
    void* mRecords;  // +0x04 (InplaceVector<GdbFileSet::Value>*)
};
struct GdbFileSet;
struct GdbFileBank;
class PakFile;

class GdbFileManager {
private:
    GdbFileManager();  // ??0GdbFileManager@@AAE@XZ (game.o 0x629920)
    virtual ~GdbFileManager();  // ??1GdbFileManager@@EAE@XZ (game.o 0x61F790)
public:
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pakId, PakFile* pakFile);  // ?DecodeBank@GdbFileManager@@QAEXPBDPAEHW4TPakId@@PAVPakFile@@@Z (game.o 0x629940)
    GdbFile GetGdbFile(TPakId pakId, const char* name, const char* type);
        // ?GetGdbFile@GdbFileManager@@QAE?AVGdbFile@@W4TPakId@@PBD1@Z (game.o 0x638750)
    static GdbFileManager* sInst;  // ?sInst@GdbFileManager@@2PAV1@A @ 0xF4F434
};
GdbFileManager* GdbFileManager::sInst = nullptr;

// ea: 0x00622C00
DCGSet* ClipHandleToDCGSet(TPakId pakId, int handle)
{
    if (handle == 0)
        return nullptr;
    if (handle != 4095 && handle != 4094)
        return (DCGSet*)DCGBankManager::sInst->GetDCGSet(pakId, handle);
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
    AeAssert::gCurrentLine = 1224;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning(
            " COME SEE STAVRO IMMEDIATELY IF YOU GET THIS!!!! "))
        __debugbreak();
    return gBoxDCGSet;
}

// ============================================================================
// unpack (game.o) - ea: 0x622B60 (CollisionMgr.cpp)
// ============================================================================
extern void unpack(const cdl_vinfo_t* vinfo, const cdl_array_t* verts,
                   math::Dir3* vert_list);  // physics.o 0x6FF680

// ea: 0x00622B60
void unpack(const CGBank& bank, unsigned int pi, math::Position3* verts)
{
    if (pi >= (unsigned int)bank.gjk_patches.m_count
        && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                     "index >= 0 && index < size()", "invalid index"))
        __debugbreak();
    cdl_vinfo_t* v3 = &((cdl_vinfo_t*)bank.gjk_patches.m_elements)[pi];
    if (v3->num_verts >= 0x20)
    {
        AeAssert::gCurrentAuthor = AeAssert::JSV;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\CollisionMgr.cpp";
        AeAssert::gCurrentLine = 321;
        AeAssert::gCurrentExpr = "vinfo.num_verts < 32";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("max number of verts per primitive exceeded."))
            __debugbreak();
    }
    unpack(v3, &bank.patch_verts, (math::Dir3*)verts);
}

// ============================================================================
// CM_ValidateAllWorldSectors - ea: 0x633010 / _r: 0x632F40 (cm_world.cpp)
// ============================================================================
// ea: 0x00632F40
void CM_ValidateAllWorldSectors_r(WorldSector* node)
{
    if (node == nullptr)
        return;
    WorldSector* v1 = node;
    while (1)
    {
        EntityShared* i = (EntityShared*)v1->entities;
        for (; i != nullptr; i = i->nextEntityInWorldSector)
        {
            unsigned int v3 = i[1].svFlags & 0xFFF;
            Entity* mObject = nullptr;
            if (v3 < 0x540
                && i[1].svFlags >> 12
                    == (unsigned int)EntityHandleDb::sInst.mElements[v3].mKey)
                mObject = EntityHandleDb::sInst.mElements[v3].mObject;
            if ((Entity*)((char*)i - 0xE0) != mObject)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
                AeAssert::gCurrentLine = 394;
                AeAssert::gCurrentExpr =
                    "check->GetEntity() == *(check->GetEntity()->GetHandle())";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Bad/Deleted entity in world sector."))
                    __debugbreak();
            }
        }
        CM_ValidateAllWorldSectors_r((WorldSector*)v1->child[0]);
        if (v1->child[1] == nullptr)
            break;
        v1 = (WorldSector*)v1->child[1];
    }
}

// ea: 0x00633010
void CM_ValidateAllWorldSectors()
{
    CM_ValidateAllWorldSectors_r(&pcm.worldSectorHead);
}

// ============================================================================
// CM_AreaEntities - ea: 0x6331A0 / _r: 0x633020 (cm_world.cpp)
// ============================================================================
struct areaParms_t {
    math::Position3 mins;       // +0x00
    math::Position3 maxs;       // +0x10
    DbLinkedHandle<EntityHandleDb, Entity>* list;  // +0x20
    int count;                  // +0x24
    int maxcount;               // +0x28
    int contentmask;            // +0x2C
};

// ea: 0x00633020
void CM_AreaEntities_r(WorldSector* node, areaParms_t* ap)
{
    WorldSector* v2 = node;
    if ((node->contentsEntities & ap->contentmask) != 0)
    {
        while (1)
        {
            EntityShared* entities = (EntityShared*)v2->entities;
            if (entities != nullptr)
            {
                while (1)
                {
                    unsigned int v4 = entities[1].svFlags & 0xFFF;
                    Entity* mObject = nullptr;
                    if (v4 < 0x540
                        && entities[1].svFlags >> 12
                            == (unsigned int)EntityHandleDb::sInst
                                   .mElements[v4].mKey)
                        mObject = EntityHandleDb::sInst.mElements[v4].mObject;
                    if ((Entity*)((char*)entities - 0xE0) != mObject)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\cm_world.cpp";
                        AeAssert::gCurrentLine = 663;
                        AeAssert::gCurrentExpr =
                            "check->GetEntity() == *(check->GetEntity()->GetHandle())";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(
                                "Bad/Deleted entity in world sector."))
                            __debugbreak();
                    }
                    if ((entities->contents & ap->contentmask) != 0
                        && (_mm_movemask_ps(
                                _mm_cmplt_ps(
                                    _mm_max_ps(
                                        _mm_sub_ps(ap->mins.v,
                                                   entities->absmax.v),
                                        _mm_sub_ps(entities->absmin.v,
                                                   ap->maxs.v)),
                                    _mm_setzero_ps()))
                            & 7) == 7)
                    {
                        int count = ap->count;
                        if (count == ap->maxcount)
                        {
                            Com_DPrintf("CM_AreaEntities: MAXCOUNT\n");
                            return;
                        }
                        ap->list[count].mHandle.mVal =
                            (unsigned int)entities[1].svFlags;
                        ++ap->count;
                    }
                    entities = entities->nextEntityInWorldSector;
                    if (entities == nullptr)
                    {
                        v2 = node;
                        break;
                    }
                }
            }
            if (ap->maxs.v.m128_f32[v2->axis] > v2->dist)
            {
                CM_AreaEntities_r((WorldSector*)v2->child[0], ap);
                v2 = node;
            }
            if (v2->dist <= ap->mins.v.m128_f32[v2->axis])
                break;
            node = (WorldSector*)v2->child[1];
            if ((node->contentsEntities & ap->contentmask) == 0)
                break;
            v2 = (WorldSector*)v2->child[1];
        }
    }
}

// ea: 0x006331A0
int CM_AreaEntities(const math::Position3& mins,
                    const math::Position3& maxs,
                    DbLinkedHandle<EntityHandleDb, Entity>* entityList,
                    int maxcount, int contentmask)
{
    areaParms_t ap;
    ap.mins.v = mins.v;
    ap.maxs.v = maxs.v;
    ap.list = entityList;
    ap.count = 0;
    ap.maxcount = maxcount;
    ap.contentmask = contentmask;
    CM_AreaEntities_r(&pcm.worldSectorHead, &ap);
    return ap.count;
}

// ============================================================================
// CM_TransformedPointContents - ea: 0x632E00 (cm_load.cpp)
// ============================================================================
extern void AngleVectors(const math::Position3& angles, float* const forward,
                         float* const right, float* const up);  // core.o
extern void traverse_rtree(const math::Position3& p0,
                           const math::Position3& p1,
                           const rtree_root_t& root,
                           subdivision_visitor& visitor);  // physics.o 0x6F6620

// ============================================================================
// CM_PointContents - ea: 0x632500 (cm_test.cpp)
// ============================================================================
// ea: 0x00632500
int CM_PointContents(const math::Position3& p, DCGSet* model)
{
    if (g_bspTree->mNodes.mSize == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_test.cpp";
        AeAssert::gCurrentLine = 221;
        AeAssert::gCurrentExpr = "g_bspTree->mNodes.size()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int contents = 0;
    if (model != nullptr)
    {
        unsigned int nboxes = (unsigned int)model->nboxes;
        if (nboxes != 0)
        {
            for (unsigned int i = 0; i < nboxes; ++i)
            {
                if (i >= (unsigned int)model->objects_m_count)
                {
                    AeAssert::gCurrentAuthor = AeAssert::JSV;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                    AeAssert::gCurrentLine = 77;
                    AeAssert::gCurrentExpr = "index < size()";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(defaultFileName))
                        __debugbreak();
                    if (i >= (unsigned int)model->objects_m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                            "index >= 0 && index < size()", "invalid index"))
                        __debugbreak();
                }
                cdl_object_t* obj =
                    &((cdl_object_t*)model->objects_m_elements)[i];
                math::Position3 bmin;
                math::Position3 bmax;
                bmax.v = _mm_add_ps(
                    _mm_setr_ps(obj->center[0], obj->center[1],
                                obj->center[2], 0.0f),
                    _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                obj->box_radius[2], 0.0f));
                bmin.v = _mm_sub_ps(
                    _mm_setr_ps(obj->center[0], obj->center[1],
                                obj->center[2], 0.0f),
                    _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                obj->box_radius[2], 0.0f));
                if (TestPointInBox(p, bmin, bmax))
                    contents |= obj->cflags;
            }
        }
        unsigned int nbrushes = (unsigned int)model->nbrushes;
        if (nbrushes != 0)
        {
            for (unsigned int i = 0; i < nbrushes; ++i)
            {
                unsigned int obj_index = i + (unsigned int)model->nboxes;
                if (obj_index >= (unsigned int)model->objects_m_count)
                {
                    AeAssert::gCurrentAuthor = AeAssert::JSV;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                    AeAssert::gCurrentLine = 77;
                    AeAssert::gCurrentExpr = "index < size()";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(defaultFileName))
                        __debugbreak();
                    if (obj_index >= (unsigned int)model->objects_m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                            "index >= 0 && index < size()", "invalid index"))
                        __debugbreak();
                }
                cdl_object_t* obj =
                    &((cdl_object_t*)model->objects_m_elements)[obj_index];
                if (i >= (unsigned int)model->brushes_m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
                cdl_brush_t* brush =
                    &((cdl_brush_t*)model->brushes_m_elements)[i];
                unsigned int first_side = (unsigned int)brush->first_side;
                if (first_side >= (unsigned int)model->brush_sides_m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
                math::Position3 bmin;
                math::Position3 bmax;
                bmax.v = _mm_add_ps(
                    _mm_setr_ps(obj->center[0], obj->center[1],
                                obj->center[2], 0.0f),
                    _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                obj->box_radius[2], 0.0f));
                bmin.v = _mm_sub_ps(
                    _mm_setr_ps(obj->center[0], obj->center[1],
                                obj->center[2], 0.0f),
                    _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                                obj->box_radius[2], 0.0f));
                const cdlPlane* sides =
                    &((const cdlPlane*)model->brush_sides_m_elements)
                        [first_side];
                if (TestPointInBrush(p, bmin, bmax, sides,
                                     (unsigned int)brush->num_sides))
                    contents |= obj->cflags;
            }
            return contents;
        }
        return contents;
    }

    math::Position3 pos;
    pos.v = _mm_setr_ps(p.v.m128_f32[0], p.v.m128_f32[1],
                        p.v.m128_f32[2], 0.0f);
    CGBankManager* mgr = (CGBankManager*)CGBankManager::sInst;
    for (int i = 0; i < mgr->mCount; ++i)
    {
        if (i > 0x62)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        CGBank* bank = mgr->mBankArray[i];
        if (TestPointInBox(pos, bank->min, bank->max))
        {
            math::Position3 lo;
            math::Position3 hi;
            lo.v = _mm_sub_ps(pos.v, _mm_setr_ps(1.0f, 1.0f, 1.0f, 0.0f));
            hi.v = _mm_add_ps(pos.v, _mm_setr_ps(1.0f, 1.0f, 1.0f, 0.0f));
            rtree_visitor_t visitor(bank);
            traverse_rtree(lo, hi, bank->rtree_root, visitor);
            visitor.filter_objects(-1);

            int nobjects = visitor.objects_m_alloc_count;
            for (int j = 0; j < nobjects; ++j)
            {
                if ((j < 0 || j >= visitor.objects_m_alloc_count)
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                        108, "i >= 0 && i < m_alloc_count", defaultFileName))
                    __debugbreak();
                unsigned int index =
                    (unsigned int)(uint16_t)visitor.objects_m_slot_array[j];
                if (index >= (unsigned int)bank->objects.m_count)
                {
                    AeAssert::gCurrentAuthor = AeAssert::JSV;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                    AeAssert::gCurrentLine = 233;
                    AeAssert::gCurrentExpr = "index < size()";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(defaultFileName))
                        __debugbreak();
                    if (index >= (unsigned int)bank->objects.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                            "index >= 0 && index < size()", "invalid index"))
                        __debugbreak();
                }
                contents |=
                    ((cdl_object_t*)bank->objects.m_elements)[index].cflags;
            }

            int nbrushes = visitor.brushes_m_alloc_count;
            for (int j = 0; j < nbrushes; ++j)
            {
                if ((j < 0 || j >= visitor.brushes_m_alloc_count)
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                        108, "i >= 0 && i < m_alloc_count", defaultFileName))
                    __debugbreak();
                unsigned int obj_index =
                    (unsigned int)(uint16_t)visitor.brushes_m_slot_array[j];
                if (obj_index >= (unsigned int)bank->objects.m_count)
                {
                    AeAssert::gCurrentAuthor = AeAssert::JSV;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                    AeAssert::gCurrentLine = 233;
                    AeAssert::gCurrentExpr = "index < size()";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(defaultFileName))
                        __debugbreak();
                    if (obj_index >= (unsigned int)bank->objects.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                            "index >= 0 && index < size()", "invalid index"))
                        __debugbreak();
                }
                cdl_object_t* obj =
                    &((cdl_object_t*)bank->objects.m_elements)[obj_index];
                unsigned int bi =
                    obj_index - (unsigned int)bank->nboxes;
                if (bi >= (unsigned int)bank->brushes.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
                cdl_brush_t* brush =
                    &((cdl_brush_t*)bank->brushes.m_elements)[bi];
                int num_sides = (int)brush->num_sides;
                int side;
                for (side = 0; side < num_sides; ++side)
                {
                    unsigned int side_index =
                        (unsigned int)side + (unsigned int)brush->first_side;
                    if (side_index >= (unsigned int)bank->brush_sides.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                            "index >= 0 && index < size()", "invalid index"))
                        __debugbreak();
                    const cdlPlane* plane =
                        &((const cdlPlane*)bank->brush_sides.m_elements)
                            [side_index];
                    const float* pv = (const float*)plane->packed;
                    float dot = pos.v.m128_f32[0] * pv[0]
                        + pos.v.m128_f32[1] * pv[1]
                        + pos.v.m128_f32[2] * pv[2];
                    if (dot > pv[3])
                        break;
                }
                if (side == num_sides)
                    contents |= obj->cflags;
            }
        }
    }
    return contents;
}

// ea: 0x00632E00
int CM_TransformedPointContents(const math::Position3& p, DCGSet* model,
                                const math::Position3& origin,
                                const math::Position3& angles)
{
    float local[3];
    local[0] = p.v.m128_f32[0] - origin.v.m128_f32[0];
    local[1] = p.v.m128_f32[1] - origin.v.m128_f32[1];
    local[2] = p.v.m128_f32[2] - origin.v.m128_f32[2];
    if (model->id != 4095
        && (angles.v.m128_f32[0] != 0.0f
            || angles.v.m128_f32[1] != 0.0f
            || angles.v.m128_f32[2] != 0.0f))
    {
        float right[3];
        float up[3];
        float forward[3];
        AngleVectors(angles, forward, right, up);
        float v6 = local[1];
        float v7 = (local[2] * right[2]) + (local[1] * right[1])
            + (local[0] * right[0]);
        float v11 = 0.0f - ((local[2] * up[2]) + (local[1] * up[1])
                            + (up[0] * local[0]));
        float v8 = (local[2] * forward[2]) + (forward[1] * v6)
            + (forward[0] * local[0]);
        local[0] = v7;
        local[1] = v11;
        local[2] = v8;
    }
    return CM_PointContents(*(const math::Position3*)local, model);
}

// ============================================================================
// CM_PointTraceToEntities - ea: 0x633230 (cm_world.cpp)
// ============================================================================
extern void SV_PointTraceToEntity(pointtrace_t* clip,
                                  EntityShared* check);  // sv.o 0x521E10

// ea: 0x00633230
void CM_PointTraceToEntities(pointtrace_t* clip,
                             const TouchEntityData& entities)
{
    int v2 = 0;
    if (entities.num > 0)
    {
        const DbLinkedHandle<EntityHandleDb, Entity>* touch =
            entities.touch;
        do
        {
            unsigned int v4 = touch->mHandle.mVal & 0xFFF;
            Entity* mObject = nullptr;
            if (v4 < 0x540
                && touch->mHandle.mVal >> 12
                    == (unsigned int)EntityHandleDb::sInst.mElements[v4].mKey)
                mObject = EntityHandleDb::sInst.mElements[v4].mObject;
            SV_PointTraceToEntity(clip, &mObject->r);
            ++v2;
            ++touch;
        } while (v2 < entities.num);
    }
}

// ============================================================================
// intersect_segment_aabb - ea: 0x61EA00 (cm_world.cpp)
// ============================================================================
// ea: 0x0061EA00
bool intersect_segment_aabb(const math::Position3& p0,
                            const math::Position3& p1,
                            const math::Position3& lo,
                            const math::Position3& hi,
                            const math::Position3& bmin,
                            const math::Position3& bmax)
{
    if ((_mm_movemask_ps(
             _mm_cmplt_ps(
                 _mm_max_ps(_mm_sub_ps(bmin.v, hi.v),
                            _mm_sub_ps(lo.v, bmax.v)),
                 _mm_setzero_ps()))
         & 7) == 7)
    {
        float v6 = 1.0f;
        float v7 = 0.0f;
        int v8 = 2;
        const float* v9 = &bmin.v.m128_f32[2];
        while (1)
        {
            float v10 = *v9 - p0.v.m128_f32[v8];
            float v11 = *v9 - p1.v.m128_f32[v8];
            if (v10 <= 0.0f)
            {
                if (v11 > 0.0f && v10 > (v10 - v11) * v6)
                {
                    v6 = v10 / (v10 - v11);
                    if (v7 >= v6)
                        return 0;
                }
            }
            else if (v10 > (v10 - v11) * v7 + 0.125f)
            {
                v7 = (v10 - 0.125f) / (v10 - v11);
                if (v7 >= v6)
                    return 0;
            }
            --v8;
            --v9;
            if (v8 < 0)
            {
                const float* v12 = &bmax.v.m128_f32[2];
                int v13 = 2;
                while (1)
                {
                    float v14 = p1.v.m128_f32[v13] - *v12;
                    float v15 = p0.v.m128_f32[v13] - *v12;
                    if (v15 <= 0.0f)
                    {
                        if (v14 > 0.0f && v15 > (v15 - v14) * v6)
                        {
                            v6 = v15 / (v15 - v14);
                            if (v7 >= v6)
                                return 0;
                        }
                    }
                    else if (v15 > (v15 - v14) * v7)
                    {
                        v7 = v15 / (v15 - v14);
                        if (v7 >= v6)
                            return 0;
                    }
                    --v13;
                    --v12;
                    if (v13 < 0)
                        return 1;
                }
            }
        }
    }
    return 0;
}

// ============================================================================
// CM_PointTraceToEntities (context version) - ea: 0x622790 / _r: 0x622500
// ============================================================================
// ea: 0x00622500
void CM_PointTraceToEntities_r(pointtrace_t* clip, WorldSector* node,
                               float p1f, float p2f,
                               const math::Position3* p1,
                               const math::Position3* p2,
                               collision_context_t* context)
{
    if (p1f < clip->trace.fraction
        && (clip->contentmask & node->contentsEntities) != 0)
    {
        int axis = node->axis;
        float v9 = p1->v.m128_f32[axis] - node->dist;
        float v10 = p2->v.m128_f32[axis] - node->dist;
        float v17 = v9;
        if (v9 < 0.0f || v10 < 0.0f)
        {
            if (v9 > 0.0f || v10 > 0.0f)
            {
                float frac = v9 / (v9 - v10);
                if (frac < 0.0f)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\cm_world.cpp";
                    AeAssert::gCurrentLine = 1433;
                    AeAssert::gCurrentExpr = "frac >= 0";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                    v9 = v17;
                }
                if (frac > 1.0f)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\cm_world.cpp";
                    AeAssert::gCurrentLine = 1434;
                    AeAssert::gCurrentExpr = "frac <= 1.f";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                    v9 = v17;
                }
                float midF = (p2f - p1f) * frac + p1f;
                math::Position3 mid;
                mid.v = _mm_add_ps(
                    p1->v, _mm_mul_ps(_mm_sub_ps(p2->v, p1->v),
                                      _mm_set1_ps(frac)));
                int side = v9 < 0.0f;
                CM_PointTraceToEntities_r(clip, (WorldSector*)node->child[side],
                                          p1f, midF, p1, &mid, context);
                CM_PointTraceToEntities_r(clip,
                                          (WorldSector*)node->child[1 - side],
                                          midF, p2f, &mid, p2, context);
            }
            else
            {
                CM_PointTraceToEntities_r(clip, (WorldSector*)node->child[1],
                                          p1f, p2f, p1, p2, context);
            }
        }
        else
        {
            CM_PointTraceToEntities_r(clip, (WorldSector*)node->child[0],
                                      p1f, p2f, p1, p2, context);
        }
        EntityShared* entities = (EntityShared*)node->entities;
        math::Position3 lo;
        math::Position3 hi;
        lo.v = _mm_min_ps(clip->start.v, clip->end.v);
        hi.v = _mm_max_ps(clip->start.v, clip->end.v);
        for (; entities != nullptr; entities = entities->nextEntityInWorldSector)
        {
            if (intersect_segment_aabb(clip->start, clip->end, lo, hi,
                                       entities->absmin, entities->absmax)
                && !context->filter((Entity*)((char*)entities - 0xE0)))
            {
                SV_PointTraceToEntity(clip, entities);
            }
        }
    }
}

// ea: 0x00622790
void CM_PointTraceToEntities(pointtrace_t* clip,
                             const collision_context_t& context)
{
    if (clip->trace.fraction > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 1476;
        AeAssert::gCurrentExpr = "clip->trace.fraction <= 1.0f";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("%f", clip->trace.fraction))
            __debugbreak();
    }
    CM_PointTraceToEntities_r(clip, &pcm.worldSectorHead, 0.0f,
                              clip->trace.fraction, &clip->start, &clip->end,
                              (collision_context_t*)&context);
}

// ============================================================================
// CM_PointSightTraceToEntities - ea: 0x622AB0 / _r: 0x622820 (cm_world.cpp)
// ============================================================================
extern int SV_PointSightTraceToEntity(sightpointtrace_t* clip,
                                      EntityShared* check);  // sv.o 0x5225B0

// ea: 0x00622820
int CM_PointSightTraceToEntities_r(sightpointtrace_t* clip,
                                   WorldSector* node, float p1f, float p2f,
                                   const math::Position3* p1,
                                   const math::Position3* p2,
                                   collision_context_t* context)
{
    if ((clip->contentmask & node->contentsEntities) == 0)
        return 0;
    int axis = node->axis;
    float v9 = p1->v.m128_f32[axis] - node->dist;
    float v10 = p2->v.m128_f32[axis] - node->dist;
    float v18 = v9;
    int result;
    if (v9 >= 0.0f && v10 >= 0.0f)
    {
        result = CM_PointSightTraceToEntities_r(clip,
                                                (WorldSector*)node->child[0],
                                                p1f, p2f, p1, p2, context);
        if (result != 0)
            return result;
        goto process;
    }
    if (v9 <= 0.0f && v10 <= 0.0f)
    {
        result = CM_PointSightTraceToEntities_r(clip,
                                                (WorldSector*)node->child[1],
                                                p1f, p2f, p1, p2, context);
        if (result != 0)
            return result;
        goto process;
    }
    float frac = v9 / (v9 - v10);
    if (frac < 0.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 1515;
        AeAssert::gCurrentExpr = "frac >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        v9 = v18;
    }
    if (frac > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 1516;
        AeAssert::gCurrentExpr = "frac <= 1.f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        v9 = v18;
    }
    float midF = (p2f - p1f) * frac + p1f;
    math::Position3 mid;
    mid.v = _mm_add_ps(p1->v, _mm_mul_ps(_mm_sub_ps(p2->v, p1->v),
                                         _mm_set1_ps(frac)));
    int side = v9 < 0.0f;
    result = CM_PointSightTraceToEntities_r(clip,
                                            (WorldSector*)node->child[side],
                                            p1f, midF, p1, &mid, context);
    if (result != 0)
        return result;
    result = CM_PointSightTraceToEntities_r(
        clip, (WorldSector*)node->child[1 - side], midF, p2f, &mid, p2,
        context);
    if (result != 0)
        return result;
process:
    EntityShared* entities = (EntityShared*)node->entities;
    math::Position3 lo;
    math::Position3 hi;
    lo.v = _mm_min_ps(clip->start.v, clip->end.v);
    hi.v = _mm_max_ps(clip->start.v, clip->end.v);
    if (entities == nullptr)
        return 0;
    do
    {
        if (intersect_segment_aabb(clip->start, clip->end, lo, hi,
                                   entities->absmin, entities->absmax)
            && !context->filter((Entity*)((char*)entities - 0xE0)))
        {
            result = SV_PointSightTraceToEntity(clip, entities);
            if (result != 0)
                return result;
        }
        entities = entities->nextEntityInWorldSector;
    } while (entities != nullptr);
    return 0;
}

// ea: 0x00622AB0
int CM_PointSightTraceToEntities(sightpointtrace_t* clip,
                                 const collision_context_t& context)
{
    return CM_PointSightTraceToEntities_r(
        clip, &pcm.worldSectorHead, 0.0f, 1.0f, &clip->start, &clip->end,
        (collision_context_t*)&context);
}

// ============================================================================
// CM_PointTraceStaticModels - ea: 0x6223E0 / _r: 0x619FE0 (cm_world.cpp)
// ============================================================================
struct locTraceWork_t {
    trace_t          trace;     // +0x00
    math::Position3  start;     // +0x50
    math::Position3  end;       // +0x60
    int              contents;  // +0x70
};

extern int CM_TraceBox(const math::Position3& start,
                       const math::Position3& end,
                       const math::Position3& mins,
                       const math::Position3& maxs,
                       float fraction);  // sv.o

// ea: 0x00619FE0
void CM_PointTraceStaticModels_r(locTraceWork_t* tw, WorldSector* node,
                                 float p1f, float p2f,
                                 const math::Position3* p1,
                                 const math::Position3* p2)
{
    if (p1f < tw->trace.fraction
        && (node->contentsStaticModels & tw->contents) != 0)
    {
        int axis = node->axis;
        float v8 = p1->v.m128_f32[axis] - node->dist;
        float v9 = p2->v.m128_f32[axis] - node->dist;
        float v20 = v8;
        if (v8 < 0.0f || v9 < 0.0f)
        {
            if (v8 > 0.0f || v9 > 0.0f)
            {
                float v10 = v8 - v9;
                if (v10 == 0.0f)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\cm_world.cpp";
                    AeAssert::gCurrentLine = 915;
                    AeAssert::gCurrentExpr = "t1 - t2";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                float frac = v8 / v10;
                if (frac < 0.0f)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\cm_world.cpp";
                    AeAssert::gCurrentLine = 917;
                    AeAssert::gCurrentExpr = "frac >= 0.0f";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                    v8 = v20;
                }
                if (frac > 1.0f)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\cm_world.cpp";
                    AeAssert::gCurrentLine = 918;
                    AeAssert::gCurrentExpr = "frac <= 1.0f";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                    v8 = v20;
                }
                float midF = (p2f - p1f) * frac + p1f;
                math::Position3 mid;
                mid.v = _mm_add_ps(
                    p1->v, _mm_mul_ps(_mm_sub_ps(p2->v, p1->v),
                                      _mm_set1_ps(frac)));
                int side = v8 < 0.0f;
                CM_PointTraceStaticModels_r(tw,
                                            (WorldSector*)node->child[side],
                                            p1f, midF, p1, &mid);
                CM_PointTraceStaticModels_r(
                    tw, (WorldSector*)node->child[1 - side], midF, p2f, &mid,
                    p2);
            }
            else
            {
                CM_PointTraceStaticModels_r(tw,
                                            (WorldSector*)node->child[1],
                                            p1f, p2f, p1, p2);
            }
        }
        else
        {
            CM_PointTraceStaticModels_r(tw,
                                        (WorldSector*)node->child[0],
                                        p1f, p2f, p1, p2);
        }
        StaticModel* i = (StaticModel*)node->staticModels;
        for (; i != nullptr; i = i->nextModel)
        {
            if ((i->xmodel->contents & tw->contents) != 0)
            {
                float fraction = tw->trace.fraction;
                math::Position3 v15;
                math::Position3 v16;
                v15.v = _mm_setr_ps(i->absmin[0], i->absmin[1], i->absmin[2],
                                    0.0f);
                v16.v = _mm_setr_ps(i->absmax[0], i->absmax[1], i->absmax[2],
                                    0.0f);
                if (CM_TraceBox(tw->start, tw->end, v15, v16,
                                fraction) == 0)
                    CM_TraceStaticModel(i, &tw->trace, tw->start, tw->end,
                                        tw->contents);
            }
        }
    }
}

// ea: 0x006223E0
void CM_PointTraceStaticModels(trace_t* results,
                               const math::Position3& start,
                               const math::Position3& end,
                               const collision_context_t& context)
{
    locTraceWork_t v9;
    memset(&v9, 0, sizeof(v9));
    int contentmask = context.contentmask;
    v9.trace.fraction = results->fraction;
    v9.start.v = start.v;
    v9.end.v = end.v;
    v9.contents = contentmask;
    CM_PointTraceStaticModels_r(&v9, &pcm.worldSectorHead, 0.0f,
                                v9.trace.fraction, &v9.start, &v9.end);
    if (results->fraction > v9.trace.fraction)
    {
        v9.trace.endpos.v.m128_f32[0] =
            (end.v.m128_f32[0] - start.v.m128_f32[0]) * v9.trace.fraction
            + start.v.m128_f32[0];
        v9.trace.endpos.v.m128_f32[1] =
            (end.v.m128_f32[1] - start.v.m128_f32[1]) * v9.trace.fraction
            + start.v.m128_f32[1];
        v9.trace.endpos.v.m128_f32[2] =
            (end.v.m128_f32[2] - start.v.m128_f32[2]) * v9.trace.fraction
            + start.v.m128_f32[2];
        *results = v9.trace;
    }
}

// ============================================================================
// GdbFileManager ctor/dtor - ea: 0x629920 / 0x61F790
// ============================================================================
extern void* InplaceAssetBankSet_GdbFileBank_ctor(void* self);  // streamer.o

// ea: 0x00629920
GdbFileManager::GdbFileManager()
{
    InplaceAssetBankSet_GdbFileBank_ctor(this);
}

// ea: 0x0061F790
GdbFileManager::~GdbFileManager()
{
}

extern void InplaceAssetBank_GdbFileSet_Fixup(void* data);   // streamer.o
extern void InplaceAssetBankSet_GdbFileBank_AddBank(void* self, TPakId pakId,
                                                   void* data);  // streamer.o

// ea: 0x00629940
void GdbFileManager::DecodeBank(const char* name, unsigned char* data, int size,
                                TPakId pakId, PakFile* pakFile)
{
    InplaceAssetBank_GdbFileSet_Fixup(data);
    InplaceAssetBankSet_GdbFileBank_AddBank(this, pakId, data);
}

// ============================================================================
// GdbFileManager::GetGdbFile - ea: 0x638750 + C bridge for cross-TU callers
// ============================================================================
extern void InplaceAssetBankSet_Find_GdbFileBank(
    void* self, void* result, const char* pakId, const char* key,
    int type, void* foundPakId);
    // InplaceAssetBankSet<GdbFileBank>::Find<char const *,IVPointer<GdbFileSet>> @ 0x429A88
extern void** InplaceTree_Find_GdbFileRecords(
    void* self, const char* const* key);
    // InplaceTree<InplaceString,InplaceVector<GdbFileSet::Value> const *>::Find<char const *> @ 0x41672B

// ea: 0x00638750
GdbFile GdbFileManager::GetGdbFile(TPakId pakId, const char* name,
                                   const char* type)
{
    GdbFile result;
    IVPointer<GdbFileSet> xm;
    InplaceAssetBankSet_Find_GdbFileBank(this, &xm, name, type, 0, nullptr);
    ValidatePakId((TPakId)xm.mPakId);
    GdbFileSet* mValue = xm.mValue;
    if (mValue != nullptr)
    {
        ValidatePakId((TPakId)xm.mPakId);
        void** v7 = InplaceTree_Find_GdbFileRecords(
            (char*)mValue + 0x0C, &name);
        ValidatePakId((TPakId)xm.mPakId);
        if (v7 != nullptr)
        {
            result.mLayout = (char*)mValue + 0x04;
            result.mRecords = *v7;
            return result;
        }
    }
    result.mLayout = nullptr;
    result.mRecords = nullptr;
    return result;
}

// C-style bridge (effect_events.cpp / common.cpp)
extern void* GdbFileManager_sInst;  // ?sInst@GdbFileManager@@2PAV1@A
void* GdbFileManager_GetGdbFile(void* mgr, TPakId pakId, const char* name,
                                const char* type)
{
    static GdbFile s_result;
    s_result = ((GdbFileManager*)mgr)->GetGdbFile(pakId, name, type);
    return &s_result;
}

// ea: 0x00638720
void DecodeGDB(const char* name, unsigned char* data, int size, TPakId pakId,
               PakFile* pakFile)
{
    InplaceAssetBank_GdbFileSet_Fixup(data);
    InplaceAssetBankSet_GdbFileBank_AddBank(GdbFileManager::sInst, pakId,
                                            data);
}

// ea: 0x006387E0
void DecodeDCGBank(const char* name, unsigned char* data, int size,
                   TPakId pakId, PakFile* pakFile)
{
    DCGBankManager::sInst->DecodeDCGBank(name, data, size, pakId);
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

// ============================================================================
// Static-model tracing (cm_staticmodel.cpp / cm_world.cpp)
// ============================================================================
StaticModel* g_static_model;  // ?g_static_model@@3PAVStaticModel@@A (game.o)

// ea: 0x0060AF40
void CM_CreateStaticModel(const char* name, TPakId pakId,
                          float* const axis, float* const origin,
                          float* const scale)
{
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_staticmodel.cpp";
    AeAssert::gCurrentLine = 22;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("dead code"))
        __debugbreak();
}

// ea: 0x00618980
void CM_TraceStaticModel(StaticModel* sm, trace_t* results,
                         const math::Position3& start,
                         const math::Position3& end, int contentmask)
{
    float fraction = results->fraction;
    trace_t v25;
    v25.mEntity.mHandle.mVal = 0;
    v25.partName.mHash = 0;
    v25.fraction = fraction;

    XModel* xmodel = sm->xmodel;
    int v10 = 0;
    if (xmodel->lod[0] == nullptr)
    {
        XModelLod** lod = xmodel->lod;
        do
        {
            ++lod;
            ++v10;
        } while (*lod == nullptr);
    }
    unsigned int mSize;
    if (xmodel->lod[v10]->xmodelParts != nullptr)
    {
        int v12 = 0;
        if (xmodel->lod[0] == nullptr)
        {
            XModelLod** v13 = xmodel->lod;
            XModelLod* v14;
            do
            {
                v14 = v13[1];
                ++v13;
                ++v12;
            } while (v14 == nullptr);
        }
        mSize = xmodel->lod[v12]->xmodelParts->mHierarchy.mSize;
    }
    else
    {
        mSize = 0;
    }
    DObjSkelMat* mat = (DObjSkelMat*)_alloca(mSize << 6);

    IVPointer<XModel> v21;
    v21.mPakId = sm->pakId;
    v21.mValue = xmodel;
    XModelGetBasePose(v21, mat, nullptr);

    float v28[3];
    v28[0] = start.v.m128_f32[0] - sm->origin[0];
    v28[1] = start.v.m128_f32[1] - sm->origin[1];
    v28[2] = start.v.m128_f32[2] - sm->origin[2];
    const float (*invAxis)[3] = sm->invAxis;
    float v24[3];
    MatrixTransformVector(v28, sm->invAxis, v24);
    v28[0] = end.v.m128_f32[0] - sm->origin[0];
    v28[1] = end.v.m128_f32[1] - sm->origin[1];
    v28[2] = end.v.m128_f32[2] - sm->origin[2];
    float v23[3];
    MatrixTransformVector(v28, invAxis, v23);

    IVPointer<XModel> v20;
    v20.mPakId = sm->pakId;
    v20.mValue = sm->xmodel;
    if (XModelTraceLine(v20, &v25, mat, v24, v23, contentmask) >= 0)
    {
        g_static_model = sm;
        float v17 = (end.v.m128_f32[0] - start.v.m128_f32[0]) * v25.fraction
            + start.v.m128_f32[0];
        float v18 = (end.v.m128_f32[1] - start.v.m128_f32[1]) * v25.fraction
            + start.v.m128_f32[1];
        float v19 = (end.v.m128_f32[2] - start.v.m128_f32[2]) * v25.fraction
            + start.v.m128_f32[2];
        v25.mEntity.mHandle.mVal = 0;
        v25.endpos.v.m128_f32[0] = v17;
        v25.endpos.v.m128_f32[1] = v18;
        v25.endpos.v.m128_f32[2] = v19;
        float v26[3];
        MatrixTransposeTransformVector(v25.normal.v.m128_f32, invAxis, v26);
        VectorNormalize(v26);
        v25.normal.v.m128_f32[0] = v26[0];
        v25.normal.v.m128_f32[1] = v26[1];
        v25.normal.v.m128_f32[2] = v26[2];
        *results = v25;
    }
}

// ============================================================================
// World-sector entity traversal (cm_world.cpp) - capsule + sight clip
// ============================================================================

// ea: 0x00619AF0
void CM_CapsuleAreaEntities(TouchEntityData& entities, WorldSector* node,
                            float p1f, float p2f,
                            const math::Position3& p1,
                            const math::Position3& p2, float radius,
                            const collision_context_t& context)
{
    if (p1f >= 1.0f)
        return;
    if ((node->contentsEntities & context.contentmask) == 0)
        return;

    int axis = node->axis;
    float v13 = p1.v.m128_f32[axis] - node->dist;
    float v14 = p2.v.m128_f32[axis] - node->dist;

    if (v13 >= radius && v14 >= radius)
    {
        CM_CapsuleAreaEntities(entities, node->child[0], p1f, p2f, p1, p2,
                               radius, context);
        goto process_entities;
    }
    if (-radius >= v13 && -radius >= v14)
    {
        CM_CapsuleAreaEntities(entities, node->child[1], p1f, p2f, p1, p2,
                               radius, context);
        goto process_entities;
    }

    int side;
    float frac;
    float frac2;
    if (v14 > v13)
    {
        float invDist = 1.0f / (v13 - v14);
        if (invDist >= 0.0f)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
            AeAssert::gCurrentLine = 755;
            AeAssert::gCurrentExpr = "invDist < 0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        float t1 = v13 - radius;
        frac2 = (v13 + radius) * invDist;
        side = 1;
        if (t1 >= 0.0f)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
            AeAssert::gCurrentLine = 758;
            AeAssert::gCurrentExpr = "t1 - radius < 0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        frac = t1 * invDist;
    }
    else if (v13 > v14)
    {
        float invDist = 1.0f / (v13 - v14);
        if (invDist <= 0.0f)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
            AeAssert::gCurrentLine = 764;
            AeAssert::gCurrentExpr = "invDist > 0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        float t1 = v13 + radius;
        frac2 = (v13 - radius) * invDist;
        side = 0;
        if (t1 <= 0.0f)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
            AeAssert::gCurrentLine = 767;
            AeAssert::gCurrentExpr = "t1 + radius > 0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        frac = t1 * invDist;
    }
    else
    {
        side = 0;
        frac2 = 0.0f;
        frac = 1.0f;
    }
    if (frac < 0.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 777;
        AeAssert::gCurrentExpr = "frac >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (frac > 1.0f)
        frac = 1.0f;

    math::Position3 mid;
    mid.v.m128_f32[0] = p1.v.m128_f32[0]
        + (p2.v.m128_f32[0] - p1.v.m128_f32[0]) * frac;
    mid.v.m128_f32[1] = p1.v.m128_f32[1]
        + (p2.v.m128_f32[1] - p1.v.m128_f32[1]) * frac;
    mid.v.m128_f32[2] = p1.v.m128_f32[2]
        + (p2.v.m128_f32[2] - p1.v.m128_f32[2]) * frac;
    float deltaF = p2f - p1f;
    CM_CapsuleAreaEntities(entities, node->child[side], p1f,
                           p1f + deltaF * frac, p1, mid, radius, context);
    if (frac2 > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 786;
        AeAssert::gCurrentExpr = "frac2 <= 1.0f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (frac2 < 0.0f)
        frac2 = 0.0f;
    mid.v.m128_f32[0] = p1.v.m128_f32[0]
        + (p2.v.m128_f32[0] - p1.v.m128_f32[0]) * frac2;
    mid.v.m128_f32[1] = p1.v.m128_f32[1]
        + (p2.v.m128_f32[1] - p1.v.m128_f32[1]) * frac2;
    mid.v.m128_f32[2] = p1.v.m128_f32[2]
        + (p2.v.m128_f32[2] - p1.v.m128_f32[2]) * frac2;
    CM_CapsuleAreaEntities(entities, node->child[1 - side],
                           p1f + deltaF * frac2, p2f, mid, p2, radius,
                           context);

process_entities:
    EntityShared* v22 = node->entities;
    if (v22 != nullptr)
    {
        do
        {
            if (v22->absmin.v.m128_f32[0] - radius
                        < entities.maxs.v.m128_f32[0]
                && v22->absmin.v.m128_f32[1] - radius
                       < entities.maxs.v.m128_f32[1]
                && v22->absmin.v.m128_f32[2] - radius
                       < entities.maxs.v.m128_f32[2]
                && entities.mins.v.m128_f32[0]
                       < v22->absmax.v.m128_f32[0] + radius
                && entities.mins.v.m128_f32[1]
                       < v22->absmax.v.m128_f32[1] + radius
                && entities.mins.v.m128_f32[2]
                       < v22->absmax.v.m128_f32[2] + radius
                && !context.filter(
                       (Entity*)((char*)v22 - 0xE0)))
            {
                if (entities.num == 128)
                    return;
                entities.touch[entities.num++].mHandle.mVal =
                    (unsigned int)v22[1].svFlags;
            }
            v22 = v22->nextEntityInWorldSector;
        } while (v22 != nullptr);
    }
}

// ea: 0x0061A320
static int CM_ClipSightTraceToEntities_r(sightclip_t* clip, WorldSector* node,
                                         float p1f, float p2f,
                                         const math::Position3* p1,
                                         const math::Position3* p2,
                                         const collision_context_t* context)
{
    int result;
    if ((clip->contentmask & node->contentsEntities) == 0)
        return 0;

    int axis = node->axis;
    float v9 = p1->v.m128_f32[axis] - node->dist;
    float v12 = p2->v.m128_f32[axis] - node->dist;
    float offset = clip->outerSize.v.m128_f32[axis];

    if (v9 >= offset && v12 >= offset)
    {
        result = CM_ClipSightTraceToEntities_r(clip, node->child[0], p1f,
                                               p2f, p1, p2, context);
        if (result != 0)
            return result;
        goto process_entities;
    }
    if (-offset >= v9 && -offset >= v12)
    {
        result = CM_ClipSightTraceToEntities_r(clip, node->child[1], p1f,
                                               p2f, p1, p2, context);
        if (result != 0)
            return result;
        goto process_entities;
    }

    int side;
    float frac;
    float frac2;
    if (v12 <= v9)
    {
        if (v9 <= v12)
        {
            side = 0;
            frac2 = 0.0f;
            frac = 1.0f;
        }
        else
        {
            float invDist = 1.0f / (v9 - v12);
            if (invDist <= 0.0f)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
                AeAssert::gCurrentLine = 1331;
                AeAssert::gCurrentExpr = "invDist > 0";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            float t1 = v9 + offset;
            frac2 = (v9 - offset) * invDist;
            side = 0;
            if (t1 <= 0.0f)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
                AeAssert::gCurrentLine = 1334;
                AeAssert::gCurrentExpr = "t1 + offset > 0";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            frac = t1 * invDist;
        }
    }
    else
    {
        float invDist = 1.0f / (v9 - v12);
        if (invDist >= 0.0f)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
            AeAssert::gCurrentLine = 1322;
            AeAssert::gCurrentExpr = "invDist < 0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        float t1 = v9 - offset;
        frac2 = (v9 + offset) * invDist;
        side = 1;
        if (t1 >= 0.0f)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
            AeAssert::gCurrentLine = 1325;
            AeAssert::gCurrentExpr = "t1 - offset < 0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        frac = t1 * invDist;
    }
    if (frac < 0.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 1344;
        AeAssert::gCurrentExpr = "frac >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (frac > 1.0f)
        frac = 1.0f;

    math::Position3 mid;
    mid.v.m128_f32[0] = p1->v.m128_f32[0]
        + (p2->v.m128_f32[0] - p1->v.m128_f32[0]) * frac;
    mid.v.m128_f32[1] = p1->v.m128_f32[1]
        + (p2->v.m128_f32[1] - p1->v.m128_f32[1]) * frac;
    mid.v.m128_f32[2] = p1->v.m128_f32[2]
        + (p2->v.m128_f32[2] - p1->v.m128_f32[2]) * frac;
    float deltaF = p2f - p1f;
    result = CM_ClipSightTraceToEntities_r(clip, node->child[side], p1f,
                                           p1f + deltaF * frac, p1, &mid,
                                           context);
    if (result != 0)
        return result;
    if (frac2 > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 1355;
        AeAssert::gCurrentExpr = "frac2 <= 1.0f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (frac2 < 0.0f)
        frac2 = 0.0f;
    mid.v.m128_f32[0] = p1->v.m128_f32[0]
        + (p2->v.m128_f32[0] - p1->v.m128_f32[0]) * frac2;
    mid.v.m128_f32[1] = p1->v.m128_f32[1]
        + (p2->v.m128_f32[1] - p1->v.m128_f32[1]) * frac2;
    mid.v.m128_f32[2] = p1->v.m128_f32[2]
        + (p2->v.m128_f32[2] - p1->v.m128_f32[2]) * frac2;
    result = CM_ClipSightTraceToEntities_r(clip, node->child[1 - side],
                                           p1f + deltaF * frac2, p2f, &mid,
                                           p2, context);
    if (result != 0)
        return result;

process_entities:
    math::Position3 pmin;
    math::Position3 pmax;
    for (int i = 0; i < 3; ++i)
    {
        float lo = clip->start.v.m128_f32[i];
        float hi = clip->end.v.m128_f32[i];
        if (hi < lo)
        {
            lo = hi;
            hi = clip->start.v.m128_f32[i];
        }
        pmin.v.m128_f32[i] = lo - clip->outerSize.v.m128_f32[i];
        pmax.v.m128_f32[i] = hi + clip->outerSize.v.m128_f32[i];
    }
    EntityShared* entities = node->entities;
    if (entities == nullptr)
        return 0;
    do
    {
        if (pmin.v.m128_f32[0] < entities->absmax.v.m128_f32[0]
            && pmin.v.m128_f32[1] < entities->absmax.v.m128_f32[1]
            && pmin.v.m128_f32[2] < entities->absmax.v.m128_f32[2]
            && entities->absmin.v.m128_f32[0] < pmax.v.m128_f32[0]
            && entities->absmin.v.m128_f32[1] < pmax.v.m128_f32[1]
            && entities->absmin.v.m128_f32[2] < pmax.v.m128_f32[2]
            && !context->filter((Entity*)((char*)entities - 0xE0)))
        {
            result = SV_ClipSightToEntity(clip, entities);
            if (result != 0)
                return result;
        }
        entities = entities->nextEntityInWorldSector;
    } while (entities != nullptr);
    return 0;
}

// ea: 0x0061A800
int CM_ClipSightTraceToEntities(sightclip_t* clip,
                                const collision_context_t& context)
{
    return CM_ClipSightTraceToEntities_r(clip, &pcm.worldSectorHead, 0.0f,
                                         1.0f, &clip->start, &clip->end,
                                         &context);
}

// ============================================================================
// Static-model link/unlink family (cm_world.cpp)
// ============================================================================

// ea: 0x0060B080
static WorldSector* CM_AllocWorldSector(float* mins, float* maxs)
{
    WorldSector* freeHead = pcm.freeHead;
    if (pcm.freeHead == nullptr)
        return nullptr;
    float size[2];
    size[0] = maxs[0] - mins[0];
    size[1] = maxs[1] - mins[1];
    int v5 = size[1] >= size[0];
    if (size[v5] <= 512.0f)
        return nullptr;
    pcm.freeHead = pcm.freeHead->parent;
    if (freeHead->contentsStaticModels != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 51;
        AeAssert::gCurrentExpr = "!node->contentsStaticModels";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (freeHead->contentsEntities != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 52;
        AeAssert::gCurrentExpr = "!node->contentsEntities";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (freeHead->entities != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 53;
        AeAssert::gCurrentExpr = "!node->entities";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (freeHead->staticModels != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 54;
        AeAssert::gCurrentExpr = "!node->staticModels";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    freeHead->axis = v5;
    freeHead->dist = (mins[v5] + maxs[v5]) * 0.5f;
    freeHead->child[0] = &pcm.dummyNode;
    freeHead->child[1] = &pcm.dummyNode;
    return freeHead;
}

// ea: 0x0060B5C0
static void CM_SortNode(WorldSector* node, float* mins, float* maxs)
{
    int axis = node->axis;
    float dist = node->dist;

    EntityShared* prevEnt = nullptr;
    EntityShared* ent = node->entities;
    while (ent != nullptr)
    {
        if (ent->linkmin[axis] > dist)
        {
            WorldSector* v6 = node->child[0];
            if (v6 == &pcm.dummyNode)
            {
                v6 = CM_AllocWorldSector(mins, maxs);
                if (v6 == nullptr)
                    return;
                node->child[0] = v6;
                v6->parent = node;
            }
            EntityShared* nextEnt = ent->nextEntityInWorldSector;
            if (prevEnt != nullptr)
            {
                if (prevEnt->nextEntityInWorldSector != ent)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
                    AeAssert::gCurrentLine = 258;
                    AeAssert::gCurrentExpr =
                        "!prevEnt || (prevEnt->nextEntityInWorldSector == ent)";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
            }
            else if (node->entities != ent)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
                AeAssert::gCurrentLine = 257;
                AeAssert::gCurrentExpr = "prevEnt || (node->entities == ent)";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            ent->worldSector = v6;
            ent->nextEntityInWorldSector = v6->entities;
            v6->contentsEntities |= ent->contents;
            v6->entities = ent;
            ent = nextEnt;
            if (prevEnt != nullptr)
                prevEnt->nextEntityInWorldSector = nextEnt;
            else
                node->entities = nextEnt;
        }
        else if (dist > ent->linkmax[axis])
        {
            WorldSector* v6 = node->child[1];
            if (v6 == &pcm.dummyNode)
            {
                v6 = CM_AllocWorldSector(mins, maxs);
                if (v6 == nullptr)
                    return;
                node->child[1] = v6;
                v6->parent = node;
            }
            EntityShared* nextEnt = ent->nextEntityInWorldSector;
            if (prevEnt != nullptr)
            {
                if (prevEnt->nextEntityInWorldSector != ent)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
                    AeAssert::gCurrentLine = 258;
                    AeAssert::gCurrentExpr =
                        "!prevEnt || (prevEnt->nextEntityInWorldSector == ent)";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
            }
            else if (node->entities != ent)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
                AeAssert::gCurrentLine = 257;
                AeAssert::gCurrentExpr = "prevEnt || (node->entities == ent)";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            ent->worldSector = v6;
            ent->nextEntityInWorldSector = v6->entities;
            v6->contentsEntities |= ent->contents;
            v6->entities = ent;
            ent = nextEnt;
            if (prevEnt != nullptr)
                prevEnt->nextEntityInWorldSector = nextEnt;
            else
                node->entities = nextEnt;
        }
        else
        {
            prevEnt = ent;
            ent = ent->nextEntityInWorldSector;
        }
    }

    StaticModel* prevSm = nullptr;
    StaticModel* sm = node->staticModels;
    while (sm != nullptr)
    {
        if (sm->absmin[axis] > dist)
        {
            WorldSector* v12 = node->child[0];
            if (v12 == &pcm.dummyNode)
            {
                v12 = CM_AllocWorldSector(mins, maxs);
                if (v12 == nullptr)
                    return;
                node->child[0] = v12;
                v12->parent = node;
            }
            StaticModel* nextSm = sm->nextModel;
            if (prevSm != nullptr)
            {
                if (prevSm->nextModel != sm)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
                    AeAssert::gCurrentLine = 314;
                    AeAssert::gCurrentExpr =
                        "!prevStaticModel || (prevStaticModel->nextModel == staticModel)";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
            }
            else if (node->staticModels != sm)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
                AeAssert::gCurrentLine = 313;
                AeAssert::gCurrentExpr =
                    "prevStaticModel || (node->staticModels == staticModel)";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            sm->nextModel = v12->staticModels;
            v12->staticModels = sm;
            v12->contentsStaticModels |= sm->xmodel->contents;
            sm = nextSm;
            if (prevSm != nullptr)
                prevSm->nextModel = nextSm;
            else
                node->staticModels = nextSm;
        }
        else if (dist > sm->absmax[axis])
        {
            WorldSector* v12 = node->child[1];
            if (v12 == &pcm.dummyNode)
            {
                v12 = CM_AllocWorldSector(mins, maxs);
                if (v12 == nullptr)
                    return;
                node->child[1] = v12;
                v12->parent = node;
            }
            StaticModel* nextSm = sm->nextModel;
            if (prevSm != nullptr)
            {
                if (prevSm->nextModel != sm)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
                    AeAssert::gCurrentLine = 314;
                    AeAssert::gCurrentExpr =
                        "!prevStaticModel || (prevStaticModel->nextModel == staticModel)";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
            }
            else if (node->staticModels != sm)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
                AeAssert::gCurrentLine = 313;
                AeAssert::gCurrentExpr =
                    "prevStaticModel || (node->staticModels == staticModel)";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            sm->nextModel = v12->staticModels;
            v12->staticModels = sm;
            v12->contentsStaticModels |= sm->xmodel->contents;
            sm = nextSm;
            if (prevSm != nullptr)
                prevSm->nextModel = nextSm;
            else
                node->staticModels = nextSm;
        }
        else
        {
            prevSm = sm;
            sm = sm->nextModel;
        }
    }
}

// ea: 0x0060BB20
void CM_LinkStaticModel(StaticModel* staticModel)
{
    int contents = staticModel->xmodel->contents;
    if (contents == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 515;
        AeAssert::gCurrentExpr = "contents";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float mins[2];
    float maxs[2];
    mins[0] = g_bspTree->mins[0];
    mins[1] = g_bspTree->mins[1];
    maxs[0] = g_bspTree->maxs[0];
    maxs[1] = g_bspTree->maxs[1];
    WorldSector* i = &pcm.worldSectorHead;
    while (1)
    {
        float dist;
        int axis;
        while (1)
        {
            if (i == &pcm.dummyNode)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
                AeAssert::gCurrentLine = 523;
                AeAssert::gCurrentExpr = "node != &pcm.dummyNode";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            dist = i->dist;
            i->contentsStaticModels |= contents;
            axis = i->axis;
            if (staticModel->absmin[axis] <= dist)
                break;
            mins[axis] = dist;
            if (i->child[0] == &pcm.dummyNode)
                goto LABEL_16;
            i = i->child[0];
        }
        if (dist <= staticModel->absmax[axis])
            break;
        maxs[axis] = dist;
        if (i->child[1] == &pcm.dummyNode)
            break;
        i = i->child[1];
    }
LABEL_16:
    staticModel->nextModel = i->staticModels;
    i->staticModels = staticModel;
    CM_SortNode(i, mins, maxs);
    EntityShared* entities = (EntityShared*)i->entities;
    if (entities != nullptr && entities == entities->nextEntityInWorldSector)
    {
        AeAssert::gCurrentAuthor = AeAssert::JSV;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 561;
        AeAssert::gCurrentExpr =
            "!node || !node->entities || ( node->entities != node->entities->nextEntityInWorldSector )";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("cycle in entities list"))
            __debugbreak();
    }
}

// ============================================================================
// CM_UnlinkEntity - ea: 0x60B230 (cm_world.cpp)
// ============================================================================
// ea: 0x0060B230
void CM_UnlinkEntity(EntityShared* ent)
{
    WorldSector* node = ent->worldSector;
    if (node == nullptr)
        return;
    ent->worldSector = nullptr;
    if (node == &pcm.dummyNode)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 115;
        AeAssert::gCurrentExpr = "node != &pcm.dummyNode";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (ent == ent->nextEntityInWorldSector)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 117;
        AeAssert::gCurrentExpr = "ent != ent->nextEntityInWorldSector";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Entity links to itself!"))
            __debugbreak();
    }
    EntityShared* scan = node->entities;
    if (scan == ent)
    {
        node->entities = ent->nextEntityInWorldSector;
    }
    else
    {
        while (scan != nullptr)
        {
            if (scan->nextEntityInWorldSector == ent)
            {
                scan->nextEntityInWorldSector = ent->nextEntityInWorldSector;
                goto FOUND;
            }
            scan = scan->nextEntityInWorldSector;
        }
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 127;
        AeAssert::gCurrentExpr = "scan";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
FOUND:
    for (EntityShared* i = node->entities; i != nullptr;
         i = i->nextEntityInWorldSector)
    {
        if (i == ent)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
            AeAssert::gCurrentLine = 141;
            AeAssert::gCurrentExpr = "scan != ent";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("found twice!"))
                __debugbreak();
        }
    }
    if (node->entities == nullptr)
    {
        while (node->staticModels == nullptr
               && node->child[0] == &pcm.dummyNode
               && node->child[1] == &pcm.dummyNode)
        {
            if (node->contentsStaticModels != 0)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
                AeAssert::gCurrentLine = 150;
                AeAssert::gCurrentExpr = "!node->contentsStaticModels";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            WorldSector* parent = node->parent;
            node->contentsEntities = 0;
            if (parent == nullptr)
            {
                if (node != &pcm.worldSectorHead)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\cm_world.cpp";
                    AeAssert::gCurrentLine = 156;
                    AeAssert::gCurrentExpr = "node == &pcm.worldSectorHead";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                goto DONE;
            }
            node->parent = pcm.freeHead;
            pcm.freeHead = node;
            if (parent->child[0] == node)
                parent->child[0] = &pcm.dummyNode;
            else
            {
                if (parent->child[1] != node)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\cm_world.cpp";
                    AeAssert::gCurrentLine = 169;
                    AeAssert::gCurrentExpr = "parentNode->child[1] == node";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                parent->child[1] = &pcm.dummyNode;
            }
            node = parent;
            if (parent->entities != nullptr)
                goto DONE;
        }
    }
    do
    {
    DONE:
        if (node->child[0] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
            AeAssert::gCurrentLine = 180;
            AeAssert::gCurrentExpr = "node->child[0]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("child[0] is null, unexpected"))
                __debugbreak();
        }
        if (node->child[1] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
            AeAssert::gCurrentLine = 181;
            AeAssert::gCurrentExpr = "node->child[1]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("child[1] is null, unexpected"))
                __debugbreak();
        }
        int contentsEntities = 0;
        if (node->child[0] != nullptr)
            contentsEntities = node->child[0]->contentsEntities;
        if (node->child[1] != nullptr)
            contentsEntities |= node->child[1]->contentsEntities;
        for (EntityShared* j = node->entities; j != nullptr;
             j = j->nextEntityInWorldSector)
            contentsEntities |= j->contents;
        node->contentsEntities = contentsEntities;
        node = node->parent;
    } while (node != nullptr);
}

// ============================================================================
// CM_LinkEntity - ea: 0x60B8E0 (cm_world.cpp)
// ============================================================================
// ea: 0x0060B8E0
void CM_LinkEntity(EntityShared* ent, const float* const absmin,
                   const float* const absmax)
{
    int contents = ent->contents;
    if (contents == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 418;
        AeAssert::gCurrentExpr = "contents";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }

    float mins[2];
    float maxs[2];
    WorldSector* node = &pcm.worldSectorHead;
    for (;;)
    {
        mins[0] = g_bspTree->mins[0];
        mins[1] = g_bspTree->mins[1];
        maxs[0] = g_bspTree->maxs[0];
        maxs[1] = g_bspTree->maxs[1];
        for (;;)
        {
            float dist;
            int axis;
            while (1)
            {
                if (node == &pcm.dummyNode)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\cm_world.cpp";
                    AeAssert::gCurrentLine = 428;
                    AeAssert::gCurrentExpr = "node != &pcm.dummyNode";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                dist = node->dist;
                node->contentsEntities |= contents;
                axis = node->axis;
                if (absmin[axis] <= dist)
                    break;
                mins[axis] = dist;
                if (node->child[0] == &pcm.dummyNode)
                    goto LABEL_21;
                node = node->child[0];
            }
            if (dist <= absmax[axis])
                break;
            maxs[axis] = dist;
            if (node->child[1] == &pcm.dummyNode)
                goto LABEL_21;
            node = node->child[1];
        }
        if (node == ent->worldSector
            && (~contents & ent->linkcontents) == 0)
        {
            ent->linkcontents = contents;
            ent->linkmin[0] = absmin[0];
            ent->linkmin[1] = absmin[1];
            ent->linkmax[0] = absmax[0];
            ent->linkmax[1] = absmax[1];
            return;
        }
    LABEL_21:
        WorldSector* worldSector = ent->worldSector;
        if (worldSector == nullptr)
            break;
        if (node == worldSector && (~contents & ent->linkcontents) == 0)
            goto LABEL_26;
        CM_UnlinkEntity(ent);
    }
    ent->worldSector = node;
    ent->nextEntityInWorldSector = node->entities;
    node->entities = ent;
LABEL_26:
    ent->linkcontents = contents;
    ent->linkmin[0] = absmin[0];
    ent->linkmin[1] = absmin[1];
    ent->linkmax[0] = absmax[0];
    ent->linkmax[1] = absmax[1];
    CM_SortNode(node, mins, maxs);
    EntityShared* entities = node->entities;
    if (entities != nullptr
        && entities == entities->nextEntityInWorldSector)
    {
        AeAssert::gCurrentAuthor = AeAssert::JSV;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 489;
        AeAssert::gCurrentExpr =
            "!node || !node->entities || ( node->entities != node->entities->nextEntityInWorldSector )";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("cycle in entities list"))
            __debugbreak();
    }
}

// ea: 0x0060AF90
int CM_UnlinkStaticModels(TPakId pakId, WorldSector* node)
{
    StaticModel* staticModels = node->staticModels;
    node->contentsStaticModels = 0;
    StaticModel* v3 = nullptr;
    while (staticModels != nullptr)
    {
        StaticModel* nextModel = staticModels->nextModel;
        if (staticModels->pakId == pakId)
        {
            if (v3 != nullptr)
                v3->nextModel = nextModel;
            else
                node->staticModels = nextModel;
        }
        else
        {
            node->contentsStaticModels |= staticModels->xmodel->contents;
            v3 = staticModels;
        }
        staticModels = nextModel;
    }
    if (node->child[0] != nullptr)
        node->contentsStaticModels |=
            CM_UnlinkStaticModels(pakId, node->child[0]);
    if (node->child[1] != nullptr)
        node->contentsStaticModels |=
            CM_UnlinkStaticModels(pakId, node->child[1]);
    return node->contentsStaticModels;
}

// ea: 0x0060B020
void CM_DestroyStaticModels(TPakId pakId)
{
    CM_UnlinkStaticModels(pakId, &pcm.worldSectorHead);
}

// ============================================================================
// CM_ misc query helpers (cm_load.cpp)
// ============================================================================

// ea: 0x006093B0
void CM_FreeLump()
{
    mem_heap_free(com_lumpBuf);
}

// ea: 0x006093C0
int CM_NumClusters()
{
    return g_bspTree->numClusters;
}

// ============================================================================
// CM_ClusterPVS - ea: 0x60B040 (cm_load.cpp)
// ============================================================================
// ea: 0x0060B040
uint8_t* CM_ClusterPVS(int cluster)
{
    if (cluster >= 0 && cluster < g_bspTree->numClusters && pcm.vised != 0)
        return &pcm.visibility[cluster * pcm.clusterBytes];
    return pcm.visibility;
}

// ============================================================================
// CM_TraceBox - ea: 0x60BCD0 (cm_world.cpp)
// ============================================================================
// ea: 0x0060BCD0
int CM_TraceBox(const math::Position3& start, const math::Position3& end,
                const math::Position3& mins, const math::Position3& maxs,
                float fraction)
{
    float bounds[3] = { mins.v.m128_f32[0], mins.v.m128_f32[1],
                        mins.v.m128_f32[2] };
    float sign = -1.0f;
    float enterFrac = 0.0f;
    float leaveFrac = fraction;

    while (2)
    {
        if ((__fpclass(bounds[0]) & 0x297) != 0
            || (__fpclass(bounds[1]) & 0x297) != 0
            || (__fpclass(bounds[2]) & 0x297) != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
            AeAssert::gCurrentLine = 832;
            AeAssert::gCurrentExpr =
                "!IS_NAN((bounds)[0]) && !IS_NAN((bounds)[1]) && !IS_NAN((bounds)[2])";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        for (int i = 0; i < 3; ++i)
        {
            float v11 = end.v.m128_f32[i] - bounds[i];
            float v12 = (start.v.m128_f32[i] - bounds[i]) * sign;
            float v13 = v11 * sign;
            if (v12 <= 0.0f)
            {
                if (v13 <= 0.0f)
                    continue;
                float v18 = v12 - v13;
                if (v18 >= 0.0f)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\cm_world.cpp";
                    AeAssert::gCurrentLine = 856;
                    AeAssert::gCurrentExpr = "dist < 0";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("old cod assert"))
                        __debugbreak();
                }
                if (v12 > v18 * leaveFrac)
                {
                    float tt = v12 / v18;
                    bool ok = enterFrac < tt;
                    leaveFrac = tt;
                    if (!ok)
                        return 1;
                }
                continue;
            }
            if (v13 > 0.0f)
                return 1;
            float v14 = v12 - v13;
            if (v14 <= 0.0f)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
                AeAssert::gCurrentLine = 844;
                AeAssert::gCurrentExpr = "dist > 0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            float v15 = v12 - 0.125f;
            if (v15 > v14 * enterFrac)
            {
                float tt = v15 / v14;
                bool ok = tt < leaveFrac;
                enterFrac = tt;
                if (!ok)
                    return 1;
            }
        }
        if (sign == 1.0f)
            return 0;
        sign = 1.0f;
        bounds[0] = maxs.v.m128_f32[0];
        bounds[1] = maxs.v.m128_f32[1];
        bounds[2] = maxs.v.m128_f32[2];
    }
}

// ============================================================================
// filter_proximity_data - ea: 0x61DC00 (CollisionMgr.cpp)
// ============================================================================
// ea: 0x0061DC00
void filter_proximity_data(const math::Position3& lo,
                           const math::Position3& hi, int contents,
                           const proximity_data_t& in,
                           proximity_data_t& out)
{
    out.boxes_count = 0;
    out.brushes_count = 0;
    out.polies_count = 0;
    out.lo.v = lo.v;
    out.hi.v = hi.v;

    int nbrushes = in.brushes_count;
    for (int i = 0; i < nbrushes; ++i)
    {
        if ((i < 0 || i >= in.brushes_count)
            && _tlAssert(
                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                114, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        const proxy_obj_t& slot = in.brushes_slot[i];
        if (slot.bi >= 99)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        CGBank* bank =
            ((CGBankManager*)CGBankManager::sInst)->mBankArray[slot.bi];
        unsigned int oi = slot.oi;
        if (oi >= (unsigned int)bank->objects.m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 233;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (oi >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        cdl_object_t* obj =
            &((cdl_object_t*)bank->objects.m_elements)[oi];
        if ((obj->cflags & contents) != 0)
        {
            math::Position3 bmin;
            math::Position3 bmax;
            bmax.v = _mm_add_ps(
                _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f),
                _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                            obj->box_radius[2], 0.0f));
            bmin.v = _mm_sub_ps(
                _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f),
                _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                            obj->box_radius[2], 0.0f));
            if ((_mm_movemask_ps(_mm_cmplt_ps(
                     _mm_max_ps(_mm_sub_ps(bmin.v, hi.v),
                                _mm_sub_ps(lo.v, bmax.v)),
                     _mm_setzero_ps()))
                 & 7) == 7)
            {
                if (out.brushes_count >= 256
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                        44, "m_alloc_count < m_slot_array_size",
                        "phys_array overflow"))
                    __debugbreak();
                out.brushes_slot[out.brushes_count++] = slot;
            }
        }
    }

    int nboxes = in.boxes_count;
    for (int i = 0; i < nboxes; ++i)
    {
        if ((i < 0 || i >= in.boxes_count)
            && _tlAssert(
                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                114, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        const proxy_obj_t& slot = in.boxes_slot[i];
        if (slot.bi >= 99)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        CGBank* bank =
            ((CGBankManager*)CGBankManager::sInst)->mBankArray[slot.bi];
        unsigned int oi = slot.oi;
        if (oi >= (unsigned int)bank->objects.m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 233;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (oi >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        cdl_object_t* obj =
            &((cdl_object_t*)bank->objects.m_elements)[oi];
        if ((obj->cflags & contents) != 0)
        {
            math::Position3 bmin;
            math::Position3 bmax;
            bmax.v = _mm_add_ps(
                _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f),
                _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                            obj->box_radius[2], 0.0f));
            bmin.v = _mm_sub_ps(
                _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f),
                _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                            obj->box_radius[2], 0.0f));
            if ((_mm_movemask_ps(_mm_cmplt_ps(
                     _mm_max_ps(_mm_sub_ps(bmin.v, hi.v),
                                _mm_sub_ps(lo.v, bmax.v)),
                     _mm_setzero_ps()))
                 & 7) == 7)
            {
                if (out.boxes_count >= 256
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                        44, "m_alloc_count < m_slot_array_size",
                        "phys_array overflow"))
                    __debugbreak();
                out.boxes_slot[out.boxes_count++] = slot;
            }
        }
    }

    int npolies = in.polies_count;
    for (int i = 0; i < npolies; ++i)
    {
        if ((i < 0 || i >= in.polies_count)
            && _tlAssert(
                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                114, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        const bounded_proxy_obj_t& slot = in.polies_slot[i];
        if ((slot.cflags & contents) != 0)
        {
            math::Position3 bmin;
            math::Position3 bmax;
            bmin.v = _mm_setr_ps(slot.min[0], slot.min[1], slot.min[2],
                                 0.0f);
            bmax.v = _mm_setr_ps(slot.max[0], slot.max[1], slot.max[2],
                                 0.0f);
            if ((_mm_movemask_ps(_mm_cmplt_ps(
                     _mm_max_ps(_mm_sub_ps(bmin.v, hi.v),
                                _mm_sub_ps(lo.v, bmax.v)),
                     _mm_setzero_ps()))
                 & 7) == 7)
            {
                if (out.polies_count >= 128
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                        44, "m_alloc_count < m_slot_array_size",
                        "phys_array overflow"))
                    __debugbreak();
                out.polies_slot[out.polies_count++] = slot;
            }
        }
    }
}

// ============================================================================
// filter_proximity_brushes - ea: 0x61E210 (CollisionMgr.cpp)
// Same as filter_proximity_data but only boxes/brushes (no polies).
// ============================================================================
// ea: 0x0061E210
void filter_proximity_brushes(const math::Position3& lo,
                              const math::Position3& hi, int contents,
                              const proximity_data_t& in,
                              proximity_data_t& out)
{
    out.brushes_count = 0;
    out.boxes_count = 0;

    int nbrushes = in.brushes_count;
    for (int i = 0; i < nbrushes; ++i)
    {
        if ((i < 0 || i >= in.brushes_count)
            && _tlAssert(
                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                114, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        const proxy_obj_t& slot = in.brushes_slot[i];
        if (slot.bi >= 99)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        CGBank* bank =
            ((CGBankManager*)CGBankManager::sInst)->mBankArray[slot.bi];
        unsigned int oi = slot.oi;
        if (oi >= (unsigned int)bank->objects.m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 233;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (oi >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        cdl_object_t* obj =
            &((cdl_object_t*)bank->objects.m_elements)[oi];
        if ((obj->cflags & contents) != 0)
        {
            math::Position3 bmin;
            math::Position3 bmax;
            bmax.v = _mm_add_ps(
                _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f),
                _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                            obj->box_radius[2], 0.0f));
            bmin.v = _mm_sub_ps(
                _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f),
                _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                            obj->box_radius[2], 0.0f));
            if ((_mm_movemask_ps(_mm_cmplt_ps(
                     _mm_max_ps(_mm_sub_ps(bmin.v, hi.v),
                                _mm_sub_ps(lo.v, bmax.v)),
                     _mm_setzero_ps()))
                 & 7) == 7)
            {
                if (out.brushes_count >= 256
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                        44, "m_alloc_count < m_slot_array_size",
                        "phys_array overflow"))
                    __debugbreak();
                out.brushes_slot[out.brushes_count++] = slot;
            }
        }
    }

    int nboxes = in.boxes_count;
    for (int i = 0; i < nboxes; ++i)
    {
        if ((i < 0 || i >= in.boxes_count)
            && _tlAssert(
                "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                114, "i >= 0 && i < m_alloc_count", defaultFileName))
            __debugbreak();
        const proxy_obj_t& slot = in.boxes_slot[i];
        if (slot.bi >= 99)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        CGBank* bank =
            ((CGBankManager*)CGBankManager::sInst)->mBankArray[slot.bi];
        unsigned int oi = slot.oi;
        if (oi >= (unsigned int)bank->objects.m_count)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
            AeAssert::gCurrentLine = 233;
            AeAssert::gCurrentExpr = "index < size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
                __debugbreak();
            if (oi >= (unsigned int)bank->objects.m_count
                && _tlAssert(
                    "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                    "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
        }
        cdl_object_t* obj =
            &((cdl_object_t*)bank->objects.m_elements)[oi];
        if ((obj->cflags & contents) != 0)
        {
            math::Position3 bmin;
            math::Position3 bmax;
            bmax.v = _mm_add_ps(
                _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f),
                _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                            obj->box_radius[2], 0.0f));
            bmin.v = _mm_sub_ps(
                _mm_setr_ps(obj->center[0], obj->center[1], obj->center[2],
                            0.0f),
                _mm_setr_ps(obj->box_radius[0], obj->box_radius[1],
                            obj->box_radius[2], 0.0f));
            if ((_mm_movemask_ps(_mm_cmplt_ps(
                     _mm_max_ps(_mm_sub_ps(bmin.v, hi.v),
                                _mm_sub_ps(lo.v, bmax.v)),
                     _mm_setzero_ps()))
                 & 7) == 7)
            {
                if (out.boxes_count >= 256
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\physics\\include\\phys_array_base.inc",
                        44, "m_alloc_count < m_slot_array_size",
                        "phys_array overflow"))
                    __debugbreak();
                out.boxes_slot[out.boxes_count++] = slot;
            }
        }
    }
}

// ============================================================================
// query_proximity_data - ea: 0x6366C0 (CollisionMgr.cpp)
// ============================================================================
extern cdl_proftimer cdl_proftimer_proxy_queries;  // game.o @ 0xF4EA48
extern struct cdl_profcounter { int value; unsigned int _pad[3]; }
    cdl_profcounter_temp0;  // game.o @ 0xF44CE8

// ea: 0x006366C0
void query_proximity_data(const math::Position3& lo,
                          const math::Position3& hi,
                          proximity_data_t& out)
{
    cdl_proftimer_proxy_queries.start();
    ++cdl_profcounter_temp0.value;
    out.boxes_count = 0;
    out.brushes_count = 0;
    out.polies_count = 0;
    out.lo.v = lo.v;
    out.hi.v = hi.v;
    CGBankManager* mgr = (CGBankManager*)CGBankManager::sInst;
    for (int i = 0; i < mgr->mCount; ++i)
    {
        if (i > 0x62)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 31;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        CGBank* bank = mgr->mBankArray[i];
        if ((_mm_movemask_ps(_mm_cmplt_ps(
                 _mm_max_ps(_mm_sub_ps(bank->min.v, hi.v),
                            _mm_sub_ps(lo.v, bank->max.v)),
                 _mm_setzero_ps()))
             & 7) == 7)
        {
            rtree_visitor_t visitor(bank);
            traverse_rtree(lo, hi, bank->rtree_root, visitor);
            visitor.post_process(i, out);
        }
    }
    cdl_proftimer_proxy_queries.stop();
}

// ============================================================================
// AddLeanToPosition - ea: 0x61FBA0 (g_weapon.cpp lean offset)
// ============================================================================
extern void AnglesToRight(const float* const angles,
                          float* const right);  // q_math
// ea: 0x0061FBA0
void AddLeanToPosition(float* const vPosition, float fViewYaw,
                       float fLeanFrac, float fViewRoll, float fLeanDist)
{
    if (fLeanFrac == 0.0f)
        return;
    float fLean = (2.0f - fabsf(fLeanFrac)) * fLeanFrac;
    float vAng[3] = { 0.0f, fViewYaw, fLean * fViewRoll };
    float vRight[3];
    AnglesToRight(vAng, vRight);
    vPosition[0] += vRight[0] * (fLean * fLeanDist);
    vPosition[1] += vRight[1] * (fLean * fLeanDist);
    vPosition[2] += vRight[2] * (fLean * fLeanDist);
}

// ============================================================================
// CM_ClipMoveToEntities - ea: 0x60BF60 (cm_world.cpp)
// ============================================================================
// ea: 0x0060BF60
void CM_ClipMoveToEntities(moveclip_t* clip,
                           const collision_context_t& context)
{
    if (clip->trace.fraction > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cm_world.cpp";
        AeAssert::gCurrentLine = 1251;
        AeAssert::gCurrentExpr = "clip->trace.fraction <= 1.f";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("%f", clip->trace.fraction))
            __debugbreak();
    }
    cdl_proftimer_temp0.start();
    cdl_proftimer_temp0.stop();
}

// ea: 0x006093D0
void CM_ModelBounds(DCGSet* mod, math::Position3& mins,
                    math::Position3& maxs)
{
    if (mod != nullptr)
    {
        if ((mins.v.m128_f32[0] == 0.0f && mins.v.m128_f32[1] == 0.0f
             && mins.v.m128_f32[2] == 0.0f)
            || (maxs.v.m128_f32[0] == 0.0f && maxs.v.m128_f32[1] == 0.0f
                && maxs.v.m128_f32[2] == 0.0f))
        {
            mins = mod->min;
            maxs = mod->max;
        }
        else
        {
            mins.v = _mm_min_ps(mins.v, mod->min.v);
            maxs.v = _mm_max_ps(maxs.v, mod->max.v);
        }
    }
}

// ============================================================================
// CGBankManager::DebugRender - ea: 0x646700 (cgbank.cpp)
// ============================================================================
// File statics (verified against IDA)
static bool          render_tmp;    // @ 0xDF8E98
static float         nlen;          // @ 0xDF8E90
static bool          render_normal; // @ 0xF592C0
static unsigned int  dword_F592BC;  // @ 0xF592BC (min/max_thresh init flag)
static std::pair<int, int> min_thresh;  // @ 0xF592B0
static std::pair<int, int> max_thresh;  // @ 0xF592A4
static int           sample_size;   // @ 0xDF8E8C
static float         rr;            // @ 0xDF8E88
static float         rr_0;          // @ 0xDF8E94
static float         basex;         // @ 0xDF8E80
static float         basey;         // @ 0xDF8E84
static float         offs;          // @ 0xDF8E7C
static float         fscale;        // @ 0xDF8E68
static float         psize;         // @ 0xDF8E6C
static float         statsbasex;    // @ 0xDF8E74
static float         statsbasey;    // @ 0xDF8E70
static float         fscale_0;      // @ 0xDF8E78
static char          buf[128];      // @ 0xF59208

// debug_brush twin of g_entity_misc.cpp (same layout; game.o data type)
struct DebugColor {
    float r, g, b, a;
};
struct cdlBrushView {
    uint8_t _pad[0x60];
};
struct debug_brush {
    const cdlBrushView* brush;  // +0x00
    math::Mat43        mat;     // +0x10
    DebugColor         color;   // +0x50
};
static_assert(sizeof(debug_brush) == 0x60, "debug_brush size mismatch");

// Render-layer externs (shared with g_entity_misc.cpp render_brush port)
extern ae_vector<debug_brush> debug_brushes;
    // ?debug_brushes@@3V?$ae_vector@Udebug_brush@@@@A (game.o)
extern void render_brush(const math::Position3& bmin,
                         const math::Position3& bmax, const cdlPlane* sides,
                         unsigned int nsides,
                         const Color& color);  // game.o 0x638A10
extern int nglGetScreenWidth();   // ngl_xboxr
extern int nglGetScreenHeight();  // ngl_xboxr
extern void nglGetStringDimensions(nglFont* font, unsigned int* width,
                                   unsigned int* height, float scaleX,
                                   float scaleY, const char* fmt, ...);
struct nglFont;
extern nglFont* nglSysFont;  // ?nglSysFont@@3PAUnglFont@@A (ngl_font.o)
extern void* nglListAlloc(unsigned int Bytes,
                          unsigned int Alignment);  // inline 0x660140
extern void mem_heap_free(void* ptr);  // mem_heap

// Minimal ngl type views (full definitions in ngl_dx_gpu.h)
struct nglShaderParamSet {
    unsigned char mData[4];
    static unsigned int NumParams;
};
struct gpuVertexFormat {
    int          VertexSize;
    const void*  Elements;
    void*        VertexDeclaration;
};
struct nglMeshSection;
struct nglMesh;
struct nglMaterial;
extern gpuVertexFormat cddebug_vertex_format;
extern void setup_color(const Color& i_col, nglShaderParamSet& o_params);
extern nglMesh* auxCreateScratchMesh(int flags, int num);
extern nglMeshSection* nglCreateScratchSection(
    int Prim, int NIndices, int NVertices, gpuVertexFormat* VertexFormat);
extern void nglAddMeshSection(nglMesh* Mesh, nglMeshSection* Section,
                              nglMaterial* Material, int Flags);
extern void* nglLockSectionIndices(nglMeshSection* Section);
extern void* nglLockSectionVertices(nglMeshSection* Section);
extern nglMesh* auxCloseScratchMesh(nglMesh* m);
struct nglMeshParams;
struct nglShaderParamSet;
struct nglMeshNode;
extern nglMeshNode* nglListAddMesh(nglMesh* Mesh,
                                   const math::Mat43& LocalToWorld,
                                   nglMeshParams* MeshParams,
                                   nglShaderParamSet* ShaderParams,
                                   void (*fn)(nglMeshNode*));
extern void j_nullsub_67(nglMeshSection* Section);
extern void j_nullsub_27(nglMeshSection* Section);
extern unsigned char* nglListWork;
extern unsigned char* nglListWorkPos;
extern int nglListWorkSize;
extern int nglLastListAllocWarnFrame;
extern int nglFrame;
extern void tlFatal(const char* fmt, ...);
extern void* DebugRender_sInst;  // ?sInst@DebugRender@@2V1@A @ 0xF74D20
extern int printf(const char* fmt, ...);

// ea: 0x00646700
void CGBankManager::DebugRender()
{
    cmgr_mem_ctx_t ctx;
    math::Position3* cg_verts = alloc_verts();
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player == nullptr)
        return;
    math::Position3 pos = Player->r.currentOrigin;
    nglGetScreenWidth();
    float nbanks = (float)nglGetScreenHeight();
    if (render_tmp)
    {
        for (int i = 0; i < debug_brushes.mSize; ++i)
            debug_brushes.mElements[i];
    }
    debug_brushes.mSize = 0;  // resize(0)

    if ((this->mDebugRenderMode & 1) != 0)
    {
        CGBankManager* mgr = (CGBankManager*)CGBankManager::sInst;
        for (int bi = 0; bi < mgr->mCount; ++bi)
        {
            if (bi > 0x62)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 31;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            CGBank* bank = mgr->mBankArray[bi];
            if (bank == nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::JSV;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.cpp";
                AeAssert::gCurrentLine = 723;
                AeAssert::gCurrentExpr = "bank";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("invalid bank"))
                    __debugbreak();
            }
            if ((_mm_movemask_ps(_mm_cmplt_ps(bank->min.v, pos.v)) & 7) == 7
                && (_mm_movemask_ps(_mm_cmplt_ps(pos.v, bank->max.v)) & 7)
                    == 7)
            {
                for (int oi = 0; oi < bank->objects.m_count; ++oi)
                {
                    if (oi >= bank->objects.m_count)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::JSV;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\cgbank.h";
                        AeAssert::gCurrentLine = 233;
                        AeAssert::gCurrentExpr = "index < size()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(defaultFileName))
                            __debugbreak();
                    }
                    if (oi >= bank->objects.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                            89, "index >= 0 && index < size()",
                            "invalid index"))
                        __debugbreak();
                    cdl_object_t* obj =
                        &((cdl_object_t*)bank->objects.m_elements)[oi];
                    // Debug color packed into the object index bytes.
                    float colorR =
                        (float)((uintptr_t)obj & 0xFF) * 0.0040000002f;
                    float colorG =
                        (float)(((uintptr_t)obj >> 4) & 0xFF)
                        * 0.0040000002f;
                    float colorB =
                        (float)(((uintptr_t)obj >> 8) & 0xFF)
                        * 0.0040000002f;
                    math::Position3 center;
                    center.v.m128_f32[0] = obj->center[0];
                    center.v.m128_f32[1] = obj->center[1];
                    center.v.m128_f32[2] = obj->center[2];
                    center.v.m128_f32[3] = 0.0f;
                    math::Position3 boxr;
                    boxr.v.m128_f32[0] = obj->box_radius[0];
                    boxr.v.m128_f32[1] = obj->box_radius[1];
                    boxr.v.m128_f32[2] = obj->box_radius[2];
                    boxr.v.m128_f32[3] = 0.0f;
                    int type = CGBank_get_type(bank, oi);
                    if (type == 1)
                    {
                        int brushIndex = oi - bank->nboxes;
                        if (brushIndex >= bank->brushes.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                91, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                        cdl_brush_t* brush =
                            &((cdl_brush_t*)bank->brushes.m_elements)
                                [brushIndex];
                        int first_side = brush->first_side;
                        if (first_side >= bank->brush_sides.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                91, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                        math::Position3 bmax;
                        bmax.v = _mm_add_ps(center.v, boxr.v);
                        math::Position3 bmin;
                        bmin.v = _mm_sub_ps(center.v, boxr.v);
                        Color color(colorR, colorG, colorB, 1.0f);
                        render_brush(
                            bmin, bmax,
                            &((cdlPlane*)bank->brush_sides.m_elements)
                                [first_side],
                            brush->num_sides, color);
                    }
                    else if (type != 0)
                    {
                        int patchIndex = oi - bank->nboxes - bank->nbrushes;
                        unpack(*bank, patchIndex, cg_verts);
                        if (patchIndex >= bank->patches.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                91, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                        cdl_patch_t* patch =
                            &((cdl_patch_t*)bank->patches.m_elements)
                                [patchIndex];
                        unsigned int first_index = patch->first_index;
                        unsigned int num_inds = patch->num_inds;
                        if (first_index >= bank->patch_inds.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                91, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                        if (num_inds != 0)
                        {
                            int tris = num_inds / 3;
                            int nVertices = 3 * tris;
                            int nIndices = 5 * tris - 2;
                            nglMesh* mesh =
                                auxCreateScratchMesh(0x40000, 1);
                            nglMeshSection* section =
                                nglCreateScratchSection(
                                    6, nIndices, nVertices,
                                    &cddebug_vertex_format);
                            nglAddMeshSection(
                                mesh, section,
                                *(nglMaterial**)((char*)DebugRender_sInst
                                                 + 0xC),
                                1);
                            unsigned short* indices =
                                (unsigned short*)nglLockSectionIndices(
                                    section);
                            float* verts =
                                (float*)nglLockSectionVertices(section);
                            unsigned char* inds =
                                (unsigned char*)bank->patch_inds.m_elements
                                + first_index;
                            int v = 0;
                            for (int t = 0; t < tris; ++t)
                            {
                                math::Position3 va = cg_verts[inds[3 * t + 0]];
                                math::Position3 vb = cg_verts[inds[3 * t + 1]];
                                math::Position3 vc = cg_verts[inds[3 * t + 2]];
                                if (v > 0)
                                {
                                    indices[0] = (unsigned short)(v - 1);
                                    indices[1] = (unsigned short)v;
                                    indices += 2;
                                }
                                verts[0] = va.v.m128_f32[0];
                                verts[1] = va.v.m128_f32[1];
                                verts[2] = va.v.m128_f32[2];
                                indices[0] = (unsigned short)v;
                                verts[3] = vb.v.m128_f32[0];
                                verts[4] = vb.v.m128_f32[1];
                                verts[5] = vb.v.m128_f32[2];
                                indices[1] = (unsigned short)(v + 1);
                                verts[6] = vc.v.m128_f32[0];
                                verts[7] = vc.v.m128_f32[1];
                                verts[8] = vc.v.m128_f32[2];
                                indices[2] = (unsigned short)(v + 2);
                                indices += 3;
                                verts += 9;
                                v += 3;
                                j_nullsub_67(section);
                                j_nullsub_27(section);
                                if (render_normal)
                                {
                                    math::Vector4 plane =
                                        calc_normal(va, vb, vc);
                                    math::Position3 center;
                                    center.v = _mm_mul_ps(
                                        _mm_add_ps(
                                            _mm_add_ps(va.v, vb.v), vc.v),
                                        _mm_set1_ps(0.33333334f));
                                    math::Position3 end;
                                    end.v = _mm_add_ps(
                                        center.v,
                                        _mm_mul_ps(plane.v,
                                                   _mm_set1_ps(nlen)));
                                    float ncol[4] = { 1.0f, 0.0f, 0.0f,
                                                      1.0f };
                                    DebugRender::RenderLine(
                                        center, end, Color(ncol[0], ncol[1], ncol[2], ncol[3]), 5.0f);
                                }
                            }
                            unsigned char* v58 =
                                (unsigned char*)(~7
                                                 & ((uintptr_t)nglListWorkPos
                                                    + 7));
                            unsigned int v59 =
                                4 * nglShaderParamSet::NumParams + 8;
                            if (v58 + v59 <= nglListWork + nglListWorkSize)
                            {
                                nglListWorkPos = v58 + v59;
                            }
                            else
                            {
                                if (nglLastListAllocWarnFrame != nglFrame)
                                {
                                    tlFatal(
                                        "Render list allocation overflow. "
                                        "Reserved = %d Requested = %d "
                                        "Free = %d.\n",
                                        nglListWorkSize,
                                        4 * nglShaderParamSet::NumParams + 8,
                                        nglListWork + nglListWorkSize - v58);
                                    nglLastListAllocWarnFrame = nglFrame;
                                }
                                v58 = nullptr;
                            }
                            nglShaderParamSet* npolies =
                                (nglShaderParamSet*)v58;
                            *(unsigned int*)v58 = 0;
                            *(unsigned int*)(v58 + 4) = 0;
                            Color pcol((float)v, colorG, colorB, 1.0f);
                            setup_color(pcol, *npolies);
                            math::Mat43 identity;
                            identity.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f,
                                                       0.0f);
                            identity.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f,
                                                       0.0f);
                            identity.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f,
                                                       0.0f);
                            identity.w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f,
                                                       1.0f);
                            nglMesh* m = auxCloseScratchMesh(mesh);
                            nglListAddMesh(m, identity, nullptr, npolies,
                                           nullptr);
                        }
                        else
                        {
                            Color boxcol(colorR, colorG, colorB, 1.0f);
                            math::Position3 bmax;
                            bmax.v = _mm_add_ps(center.v, boxr.v);
                            math::Position3 bmin;
                            bmin.v = _mm_sub_ps(center.v, boxr.v);
                            DebugRender::RenderBox(bmin, bmax, boxcol);
                        }
                    }
                }
            }
        }
    }

    if ((this->mDebugRenderMode & 6) != 0)
    {
        CGBankManager* mgr = (CGBankManager*)CGBankManager::sInst;
        int bankCount = mgr->mCount;
        float ext = 5.0f;
        math::Position3 pmin;
        pmin.v = _mm_sub_ps(pos.v, _mm_set1_ps(ext));
        math::Position3 pmax;
        pmax.v = _mm_add_ps(pos.v, _mm_set1_ps(ext));
        int bestCount = -1;
        int bestBank = -1;
        for (int bi = 0; bi < bankCount; ++bi)
        {
            if (bi > 0x62)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 31;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            CGBank* bank = mgr->mBankArray[bi];
            if ((_mm_movemask_ps(_mm_cmplt_ps(
                     _mm_max_ps(
                         _mm_sub_ps(bank->rtree_root.region_center.v,
                                    pmax.v),
                         _mm_sub_ps(
                             pmin.v,
                             bank->rtree_root.region_halfsize_inv32k.v)),
                     _mm_setzero_ps()))
                 & 7)
                == 7)
            {
                rtree_visitor_t visitor(bank);
                traverse_rtree(pmin, pmax, bank->rtree_root, visitor);
                if (visitor.objects_m_alloc_count > bestCount)
                {
                    bestCount = visitor.objects_m_alloc_count;
                    bestBank = bi;
                }
            }
        }
        if (bestBank != -1)
        {
            if ((dword_F592BC & 1) == 0)
            {
                dword_F592BC |= 1u;
                min_thresh = std::make_pair(5, 40);
            }
            if ((dword_F592BC & 2) == 0)
            {
                dword_F592BC |= 2u;
                max_thresh = std::make_pair(20, 100);
            }
            if (bestBank > 0x62)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 31;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            CGBank* bank = mgr->mBankArray[bestBank];
            math::Position3 mn = bank->min;
            math::Position3 mx = bank->max;
            math::Position3 extents;
            extents.v = _mm_sub_ps(mx.v, mn.v);
            int x = (sample_size + (int)extents.v.m128_f32[0]) / sample_size;
            int y = (sample_size + (int)extents.v.m128_f32[1]) / sample_size;
            std::vector<std::pair<int, int>> grid(x * y);
            for (int oi = 0; oi < bank->objects.m_count; ++oi)
            {
                if (oi >= bank->objects.m_count)
                {
                    AeAssert::gCurrentAuthor = AeAssert::JSV;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
                    AeAssert::gCurrentLine = 233;
                    AeAssert::gCurrentExpr = "index < size()";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(defaultFileName))
                        __debugbreak();
                }
                if (oi >= bank->objects.m_count
                    && _tlAssert(
                        "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                        "index >= 0 && index < size()", "invalid index"))
                    __debugbreak();
                cdl_object_t* obj =
                    &((cdl_object_t*)bank->objects.m_elements)[oi];
                int type = CGBank_get_type(bank, oi);
                if (type != 0)
                {
                    if (type == 1)
                    {
                        math::Position3 center;
                        center.v.m128_f32[0] = obj->center[0];
                        center.v.m128_f32[1] = obj->center[1];
                        center.v.m128_f32[2] = obj->center[2];
                        math::Position3 boxr;
                        boxr.v.m128_f32[0] = obj->box_radius[0];
                        boxr.v.m128_f32[1] = obj->box_radius[1];
                        boxr.v.m128_f32[2] = obj->box_radius[2];
                        math::Position3 mins;
                        mins.v = _mm_max_ps(
                            _mm_sub_ps(center.v, boxr.v), mn.v);
                        math::Position3 maxs;
                        maxs.v = _mm_max_ps(
                            _mm_add_ps(center.v, boxr.v), mn.v);
                        int col0 = (int)(mins.v.m128_f32[0] / sample_size);
                        int col1 = (int)(maxs.v.m128_f32[0] / sample_size);
                        int row0 = (int)(mins.v.m128_f32[1] / sample_size);
                        int row1 = (int)(maxs.v.m128_f32[1] / sample_size);
                        for (int c = col0; c <= col1; ++c)
                        {
                            if (c >= x)
                                break;
                            for (int r = row0; r <= row1; ++r)
                            {
                                if (r >= y)
                                    break;
                                ++grid[c + x * r].first;
                            }
                        }
                    }
                    else
                    {
                        int patchIndex =
                            oi - bank->nboxes - bank->nbrushes;
                        unpack(*bank, patchIndex, cg_verts);
                        if (patchIndex >= bank->patches.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                91, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                        cdl_patch_t* patch =
                            &((cdl_patch_t*)bank->patches.m_elements)
                                [patchIndex];
                        unsigned int first_index = patch->first_index;
                        unsigned int num_inds = patch->num_inds;
                        if (first_index >= bank->patch_inds.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                91, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                        if (num_inds != 0)
                        {
                            unsigned char* inds =
                                (unsigned char*)bank->patch_inds.m_elements
                                + first_index;
                            int tris = (num_inds - 1) / 3 + 1;
                            for (int t = 0; t < tris; ++t)
                            {
                                math::Position3 va = cg_verts[inds[3 * t + 0]];
                                math::Position3 vb = cg_verts[inds[3 * t + 1]];
                                math::Position3 vc = cg_verts[inds[3 * t + 2]];
                                math::Position3 lo;
                                lo.v = _mm_min_ps(
                                    _mm_min_ps(va.v, vb.v), vc.v);
                                math::Position3 hi;
                                hi.v = _mm_max_ps(
                                    _mm_max_ps(va.v, vb.v), vc.v);
                                int col0 = (int)(lo.v.m128_f32[0]
                                                 / sample_size);
                                int col1 = (int)(hi.v.m128_f32[0]
                                                 / sample_size);
                                int row0 = (int)(lo.v.m128_f32[1]
                                                 / sample_size);
                                int row1 = (int)(hi.v.m128_f32[1]
                                                 / sample_size);
                                int density = 0;
                                for (int c = col0; c <= col1; ++c)
                                {
                                    if (c >= x)
                                        break;
                                    for (int r = row0; r <= row1; ++r)
                                    {
                                        if (r >= y)
                                            break;
                                        int v = grid[c + x * r].second;
                                        if (density < v)
                                            density = v;
                                        ++grid[c + x * r].second;
                                    }
                                }
                                printf("[%s:%d] JIV STUB\n",
                                       "c:\\cod\\code\\game\\cgbank.cpp",
                                       964);
                                nglMesh* mesh =
                                    auxCreateScratchMesh(0x40000, 1);
                                nglMeshSection* section =
                                    nglCreateScratchSection(
                                        6, 3, 3, &cddebug_vertex_format);
                                nglAddMeshSection(
                                    mesh, section,
                                    *(nglMaterial**)((char*)DebugRender_sInst
                                                     + 0xC),
                                    1);
                                unsigned short* indices =
                                    (unsigned short*)nglLockSectionIndices(
                                        section);
                                float* verts =
                                    (float*)nglLockSectionVertices(section);
                                verts[0] = va.v.m128_f32[0];
                                verts[1] = va.v.m128_f32[1];
                                verts[2] = va.v.m128_f32[2];
                                indices[0] = 0;
                                verts[3] = vb.v.m128_f32[0];
                                verts[4] = vb.v.m128_f32[1];
                                verts[5] = vb.v.m128_f32[2];
                                indices[1] = 1;
                                verts[6] = vc.v.m128_f32[0];
                                verts[7] = vc.v.m128_f32[1];
                                verts[8] = vc.v.m128_f32[2];
                                indices[2] = 2;
                                j_nullsub_67(section);
                                j_nullsub_27(section);
                                unsigned char* v156 =
                                    (unsigned char*)(~7
                                                     & ((uintptr_t)
                                                            nglListWorkPos
                                                        + 7));
                                unsigned int v157 =
                                    4 * nglShaderParamSet::NumParams + 8;
                                if (v156 + v157
                                    <= nglListWork + nglListWorkSize)
                                {
                                    nglListWorkPos = v156 + v157;
                                }
                                else
                                {
                                    if (nglLastListAllocWarnFrame
                                        != nglFrame)
                                    {
                                        tlFatal(
                                            "Render list allocation "
                                            "overflow. Reserved = %d "
                                            "Requested = %d Free = %d.\n",
                                            nglListWorkSize,
                                            4 * nglShaderParamSet::NumParams
                                                + 8,
                                            nglListWork + nglListWorkSize
                                                - v156);
                                        nglLastListAllocWarnFrame =
                                            nglFrame;
                                    }
                                    v156 = nullptr;
                                }
                                nglShaderParamSet* npolies =
                                    (nglShaderParamSet*)v156;
                                *(unsigned int*)v156 = 0;
                                *(unsigned int*)(v156 + 4) = 0;
                                int second = density;
                                if (second > max_thresh.second)
                                    second = max_thresh.second;
                                Color pcol;
                                if (second >= min_thresh.second)
                                {
                                    pcol = Color(
                                        (float)second
                                            / (float)max_thresh.second,
                                        0.0f, 0.0f, 1.0f);
                                }
                                else
                                {
                                    pcol = Color(0.75f, 0.75f, 0.75f,
                                                 1.0f);
                                }
                                setup_color(pcol, *npolies);
                                math::Mat43 identity;
                                identity.x.v = _mm_setr_ps(1.0f, 0.0f,
                                                           0.0f, 0.0f);
                                identity.y.v = _mm_setr_ps(0.0f, 1.0f,
                                                           0.0f, 0.0f);
                                identity.z.v = _mm_setr_ps(0.0f, 0.0f,
                                                           1.0f, 0.0f);
                                identity.w.v = _mm_setr_ps(0.0f, 0.0f,
                                                           0.0f, 1.0f);
                                nglMesh* m = auxCloseScratchMesh(mesh);
                                nglListAddMesh(m, identity, nullptr,
                                               npolies, nullptr);
                            }
                        }
                    }
                }
            }

            if ((this->mDebugRenderMode & 2) != 0)
            {
                for (int oi = 0; oi < bank->objects.m_count; ++oi)
                {
                    if (oi >= bank->objects.m_count)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::JSV;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\cgbank.h";
                        AeAssert::gCurrentLine = 233;
                        AeAssert::gCurrentExpr = "index < size()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert(defaultFileName))
                            __debugbreak();
                    }
                    if (oi >= bank->objects.m_count
                        && _tlAssert(
                            "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                            89, "index >= 0 && index < size()",
                            "invalid index"))
                        __debugbreak();
                    cdl_object_t* obj =
                        &((cdl_object_t*)bank->objects.m_elements)[oi];
                    int type = CGBank_get_type(bank, oi);
                    if (type != 0)
                    {
                        math::Position3 center;
                        center.v.m128_f32[0] = obj->center[0];
                        center.v.m128_f32[1] = obj->center[1];
                        center.v.m128_f32[2] = obj->center[2];
                        math::Position3 boxr;
                        boxr.v.m128_f32[0] = obj->box_radius[0];
                        boxr.v.m128_f32[1] = obj->box_radius[1];
                        boxr.v.m128_f32[2] = obj->box_radius[2];
                        math::Position3 bmax;
                        bmax.v = _mm_add_ps(center.v, boxr.v);
                        math::Position3 bmin;
                        bmin.v = _mm_sub_ps(center.v, boxr.v);
                        math::Position3 clamped;
                        clamped.v = _mm_max_ps(
                            _mm_min_ps(pos.v, bmax.v), bmin.v);
                        math::Position3 d;
                        d.v = _mm_sub_ps(clamped.v, pos.v);
                        float dist2 =
                            d.v.m128_f32[0] * d.v.m128_f32[0]
                            + d.v.m128_f32[1] * d.v.m128_f32[1]
                            + d.v.m128_f32[2] * d.v.m128_f32[2];
                        if (dist2 <= rr * rr)
                        {
                            if (type == 1)
                            {
                                int brushIndex = oi - bank->nboxes;
                                if (brushIndex >= bank->brushes.m_count
                                    && _tlAssert(
                                        "c:\\cod\\code\\tl\\cdl\\source"
                                        "\\cdl_mem.h",
                                        91, "index >= 0 && index < size()",
                                        "invalid index"))
                                    __debugbreak();
                                cdl_brush_t* brush =
                                    &((cdl_brush_t*)bank->brushes
                                          .m_elements)[brushIndex];
                                int first_side = brush->first_side;
                                if (first_side
                                        >= bank->brush_sides.m_count
                                    && _tlAssert(
                                        "c:\\cod\\code\\tl\\cdl\\source"
                                        "\\cdl_mem.h",
                                        91,
                                        "index >= 0 && index < size()",
                                        "invalid index"))
                                    __debugbreak();
                                Color color(0.75f, 0.75f, 0.75f, 1.0f);
                                render_brush(
                                    bmin, bmax,
                                    &((cdlPlane*)bank->brush_sides
                                          .m_elements)[first_side],
                                    brush->num_sides, color);
                            }
                            else
                            {
                                int patchIndex =
                                    oi - bank->nboxes - bank->nbrushes;
                                unpack(*bank, patchIndex, cg_verts);
                                if (patchIndex >= bank->patches.m_count
                                    && _tlAssert(
                                        "c:\\cod\\code\\tl\\cdl\\source"
                                        "\\cdl_mem.h",
                                        91,
                                        "index >= 0 && index < size()",
                                        "invalid index"))
                                    __debugbreak();
                                cdl_patch_t* patch =
                                    &((cdl_patch_t*)bank->patches
                                          .m_elements)[patchIndex];
                                unsigned int first_index =
                                    patch->first_index;
                                unsigned int num_inds = patch->num_inds;
                                if (first_index >= bank->patch_inds.m_count
                                    && _tlAssert(
                                        "c:\\cod\\code\\tl\\cdl\\source"
                                        "\\cdl_mem.h",
                                        91,
                                        "index >= 0 && index < size()",
                                        "invalid index"))
                                    __debugbreak();
                                if (num_inds != 0)
                                {
                                    unsigned char* inds =
                                        (unsigned char*)
                                            bank->patch_inds.m_elements
                                        + first_index;
                                    int tris = (num_inds - 1) / 3 + 1;
                                    for (int t = 0; t < tris; ++t)
                                    {
                                        math::Position3 va =
                                            cg_verts[inds[3 * t + 0]];
                                        math::Position3 vb =
                                            cg_verts[inds[3 * t + 1]];
                                        math::Position3 vc =
                                            cg_verts[inds[3 * t + 2]];
                                        math::Position3 lo;
                                        lo.v = _mm_min_ps(
                                            _mm_min_ps(va.v, vb.v), vc.v);
                                        math::Position3 hi;
                                        hi.v = _mm_max_ps(
                                            _mm_max_ps(va.v, vb.v), vc.v);
                                        int col0 =
                                            (int)(lo.v.m128_f32[0]
                                                  / sample_size);
                                        int col1 =
                                            (int)(hi.v.m128_f32[0]
                                                  / sample_size);
                                        int row0 =
                                            (int)(lo.v.m128_f32[1]
                                                  / sample_size);
                                        int row1 =
                                            (int)(hi.v.m128_f32[1]
                                                  / sample_size);
                                        int density = 0;
                                        for (int c = col0; c <= col1; ++c)
                                        {
                                            if (c >= x)
                                                break;
                                            for (int r = row0; r <= row1;
                                                 ++r)
                                            {
                                                if (r >= y)
                                                    break;
                                                int v =
                                                    grid[c + x * r].second;
                                                if (density < v)
                                                    density = v;
                                            }
                                        }
                                        printf("[%s:%d] JIV STUB\n",
                                               "c:\\cod\\code\\game"
                                               "\\cgbank.cpp",
                                               964);
                                        nglMesh* mesh =
                                            auxCreateScratchMesh(0x40000,
                                                                 1);
                                        nglMeshSection* section =
                                            nglCreateScratchSection(
                                                6, 3, 3,
                                                &cddebug_vertex_format);
                                        nglAddMeshSection(
                                            mesh, section,
                                            *(nglMaterial**)(
                                                (char*)DebugRender_sInst
                                                + 0xC),
                                            1);
                                        unsigned short* indices =
                                            (unsigned short*)
                                                nglLockSectionIndices(
                                                    section);
                                        float* verts =
                                            (float*)
                                                nglLockSectionVertices(
                                                    section);
                                        verts[0] = va.v.m128_f32[0];
                                        verts[1] = va.v.m128_f32[1];
                                        verts[2] = va.v.m128_f32[2];
                                        indices[0] = 0;
                                        verts[3] = vb.v.m128_f32[0];
                                        verts[4] = vb.v.m128_f32[1];
                                        verts[5] = vb.v.m128_f32[2];
                                        indices[1] = 1;
                                        verts[6] = vc.v.m128_f32[0];
                                        verts[7] = vc.v.m128_f32[1];
                                        verts[8] = vc.v.m128_f32[2];
                                        indices[2] = 2;
                                        j_nullsub_67(section);
                                        j_nullsub_27(section);
                                        unsigned char* v156 =
                                            (unsigned char*)(~7
                                                             & ((uintptr_t)
                                                                    nglListWorkPos
                                                                + 7));
                                        unsigned int v157 =
                                            4
                                                * nglShaderParamSet::
                                                      NumParams
                                            + 8;
                                        if (v156 + v157
                                            <= nglListWork
                                                   + nglListWorkSize)
                                        {
                                            nglListWorkPos = v156 + v157;
                                        }
                                        else
                                        {
                                            if (nglLastListAllocWarnFrame
                                                != nglFrame)
                                            {
                                                tlFatal(
                                                    "Render list "
                                                    "allocation overflow. "
                                                    "Reserved = %d "
                                                    "Requested = %d "
                                                    "Free = %d.\n",
                                                    nglListWorkSize,
                                                    4
                                                        * nglShaderParamSet::
                                                              NumParams
                                                        + 8,
                                                    nglListWork
                                                            + nglListWorkSize
                                                        - v156);
                                                nglLastListAllocWarnFrame =
                                                    nglFrame;
                                            }
                                            v156 = nullptr;
                                        }
                                        nglShaderParamSet* npolies =
                                            (nglShaderParamSet*)v156;
                                        *(unsigned int*)v156 = 0;
                                        *(unsigned int*)(v156 + 4) = 0;
                                        int second = density;
                                        if (second > max_thresh.second)
                                            second = max_thresh.second;
                                        Color pcol;
                                        if (second >= min_thresh.second)
                                        {
                                            pcol = Color(
                                                (float)second
                                                    / (float)max_thresh
                                                          .second,
                                                0.0f, 0.0f, 1.0f);
                                        }
                                        else
                                        {
                                            pcol = Color(0.75f, 0.75f,
                                                         0.75f, 1.0f);
                                        }
                                        setup_color(pcol, *npolies);
                                        math::Mat43 identity;
                                        identity.x.v = _mm_setr_ps(
                                            1.0f, 0.0f, 0.0f, 0.0f);
                                        identity.y.v = _mm_setr_ps(
                                            0.0f, 1.0f, 0.0f, 0.0f);
                                        identity.z.v = _mm_setr_ps(
                                            0.0f, 0.0f, 1.0f, 0.0f);
                                        identity.w.v = _mm_setr_ps(
                                            0.0f, 0.0f, 0.0f, 1.0f);
                                        nglMesh* m =
                                            auxCloseScratchMesh(mesh);
                                        nglListAddMesh(m, identity,
                                                       nullptr, npolies,
                                                       nullptr);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if ((this->mDebugRenderMode & 4) != 0)
            {
                float scale = (float)nbanks / (this->scale * 41.0f);
                int polies = 0;
                for (int w = 0; w < x; ++w)
                {
                    for (int h = 0; h < y; ++h)
                    {
                        int brushes = grid[w + x * h].first;
                        int pls = grid[w + x * h].second;
                        float density =
                            brushes <= max_thresh.first
                                ? (float)brushes / (float)max_thresh.first
                                : 1.0f;
                        float den2 =
                            pls <= max_thresh.second
                                ? (float)pls / (float)max_thresh.second
                                : 1.0f;
                        if (den2 <= density)
                            den2 = density;
                        float col[4];
                        if (brushes >= min_thresh.first
                            || pls >= min_thresh.second)
                        {
                            col[0] = den2;
                            col[1] = 0.0f;
                            col[2] = 0.0f;
                            col[3] = 0.60000002f;
                        }
                        else
                        {
                            col[0] = 0.75f;
                            col[1] = 0.75f;
                            col[2] = 1.0f;
                            col[3] = 0.6f;
                        }
                        float l = basex
                                  + ((float)(bank->min.v.m128_f32[0]
                                             + sample_size * w)
                                     - pos.v.m128_f32[0])
                                        * scale;
                        float t = basey
                                  + ((float)(bank->min.v.m128_f32[1]
                                             + sample_size * h)
                                     - pos.v.m128_f32[1])
                                        * scale;
                        float r = l + sample_size * scale;
                        float b = t + sample_size * scale;
                        DebugRender::RenderQuad2D(l + offs, t + offs,
                                                  r - offs, b - offs, 1.0f,
                                                  Color(col[0], col[1], col[2], col[3]));
                        sprintf(buf, "%d", pls);
                        unsigned int wText = 0;
                        unsigned int hText = 0;
                        nglGetStringDimensions(nglSysFont, &wText, &hText,
                                               fscale_0, fscale_0, buf);
                        float cellCenter =
                            sample_size * scale * 0.5f;
                        float xc = l + cellCenter - wText * 0.5f;
                        float yc = t + cellCenter - hText * 0.5f;
                        float xr = wText + xc;
                        if (statsbasex - 20.0f > xr
                            || yc > statsbasey + 120.0f)
                        {
                            float wcol[4] = { 0.98039216f, 0.98039216f,
                                              0.98039216f, 1.0f };
                            DebugRender::RenderText(
                                buf, (int)xc,
                                (int)(yc - (float)(hText >> 1)), Color(wcol[0], wcol[1], wcol[2], wcol[3]),
                                0.0f, fscale_0);
                            sprintf(buf, "%d", brushes);
                            DebugRender::RenderText(
                                buf, (int)xc,
                                (int)(yc + (float)(hText >> 1)), Color(wcol[0], wcol[1], wcol[2], wcol[3]),
                                0.0f, fscale_0);
                        }
                    }
                }
                float linecol[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
                DebugRender::RenderQuad2D(basex, basey,
                                          psize + basex, basey - psize,
                                          1.0f, Color(linecol[0], linecol[1], linecol[2], linecol[3]));
                float bgcol[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                DebugRender::RenderQuad2D(statsbasex - 10.0f,
                                          statsbasey - 10.0f,
                                          statsbasex + 200.0f,
                                          statsbasey + 110.0f, 1.0f,
                                          Color(bgcol[0], bgcol[1], bgcol[2], bgcol[3]));
                unsigned int wFont = 0;
                unsigned int hFont = 0;
                nglGetStringDimensions(nglSysFont, &wFont, &hFont, fscale,
                                       fscale, "fGgW");
                sprintf(buf, "pos (%.0f,%.0f,%.0f)",
                        pos.v.m128_f32[0], pos.v.m128_f32[1],
                        pos.v.m128_f32[2]);
                float wcol[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
                DebugRender::RenderText(buf, (int)statsbasex,
                                        (int)statsbasey, Color(wcol[0], wcol[1], wcol[2], wcol[3]), 0.0f,
                                        fscale);
                int brushes = 0;
                int triCount = 0;
                math::Position3 boxMin;
                boxMin.v = _mm_sub_ps(pos.v, _mm_set1_ps(
                    (float)(sample_size / 2)));
                math::Position3 boxMax;
                boxMax.v = _mm_add_ps(pos.v, _mm_set1_ps(
                    (float)(sample_size / 2)));
                for (int bi = 0; bi < bankCount; ++bi)
                {
                    if (bi > 0x62)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                        AeAssert::gCurrentLine = 31;
                        AeAssert::gCurrentExpr =
                            "idx >= 0 && idx < _SIZE";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("out of bounds"))
                            __debugbreak();
                    }
                    CGBank* b2 = mgr->mBankArray[bi];
                    for (int oi = 0; oi < b2->objects.m_count; ++oi)
                    {
                        if (oi >= b2->objects.m_count)
                        {
                            AeAssert::gCurrentAuthor = AeAssert::JSV;
                            AeAssert::gCurrentFile =
                                "c:\\cod\\code\\game\\cgbank.h";
                            AeAssert::gCurrentLine = 233;
                            AeAssert::gCurrentExpr = "index < size()";
                            if (!AeAssert::IsIgnored()
                                && AeAssert::Assert(defaultFileName))
                                __debugbreak();
                        }
                        if (oi >= b2->objects.m_count
                            && _tlAssert(
                                "c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h",
                                89, "index >= 0 && index < size()",
                                "invalid index"))
                            __debugbreak();
                        cdl_object_t* obj =
                            &((cdl_object_t*)b2->objects.m_elements)[oi];
                        int type = CGBank_get_type(b2, oi);
                        if (type == 1)
                        {
                            math::Position3 center;
                            center.v.m128_f32[0] = obj->center[0];
                            center.v.m128_f32[1] = obj->center[1];
                            center.v.m128_f32[2] = obj->center[2];
                            math::Position3 boxr;
                            boxr.v.m128_f32[0] = obj->box_radius[0];
                            boxr.v.m128_f32[1] = obj->box_radius[1];
                            boxr.v.m128_f32[2] = obj->box_radius[2];
                            math::Position3 bmax;
                            bmax.v = _mm_add_ps(center.v, boxr.v);
                            math::Position3 bmin;
                            bmin.v = _mm_sub_ps(center.v, boxr.v);
                            if (bmin.v.m128_f32[0]
                                        <= boxMax.v.m128_f32[0]
                                    && bmin.v.m128_f32[1]
                                        <= boxMax.v.m128_f32[1]
                                    && bmin.v.m128_f32[2]
                                        <= boxMax.v.m128_f32[2]
                                    && bmax.v.m128_f32[0]
                                        >= boxMin.v.m128_f32[0]
                                    && bmax.v.m128_f32[1]
                                        >= boxMin.v.m128_f32[1]
                                    && bmax.v.m128_f32[2]
                                        >= boxMin.v.m128_f32[2])
                                ++brushes;
                        }
                        else if (type == 2)
                        {
                            int patchIndex =
                                oi - b2->nboxes - b2->nbrushes;
                            unpack(*b2, patchIndex, cg_verts);
                            if (patchIndex >= b2->patches.m_count
                                && _tlAssert(
                                    "c:\\cod\\code\\tl\\cdl\\source"
                                    "\\cdl_mem.h",
                                    91, "index >= 0 && index < size()",
                                    "invalid index"))
                                __debugbreak();
                            cdl_patch_t* patch =
                                &((cdl_patch_t*)b2->patches.m_elements)
                                    [patchIndex];
                            unsigned int first_index = patch->first_index;
                            unsigned int num_inds = patch->num_inds;
                            if (first_index >= b2->patch_inds.m_count
                                && _tlAssert(
                                    "c:\\cod\\code\\tl\\cdl\\source"
                                    "\\cdl_mem.h",
                                    91, "index >= 0 && index < size()",
                                    "invalid index"))
                                __debugbreak();
                            if (num_inds != 0)
                            {
                                unsigned char* inds =
                                    (unsigned char*)b2->patch_inds
                                        .m_elements
                                    + first_index;
                                int tris = (num_inds - 1) / 3 + 1;
                                for (int t = 0; t < tris; ++t)
                                {
                                    math::Position3 va =
                                        cg_verts[inds[3 * t + 0]];
                                    math::Position3 vb =
                                        cg_verts[inds[3 * t + 1]];
                                    math::Position3 vc =
                                        cg_verts[inds[3 * t + 2]];
                                    math::Position3 lo;
                                    lo.v = _mm_min_ps(
                                        _mm_min_ps(va.v, vb.v), vc.v);
                                    math::Position3 hi;
                                    hi.v = _mm_max_ps(
                                        _mm_max_ps(va.v, vb.v), vc.v);
                                    if (lo.v.m128_f32[0]
                                                <= boxMax.v.m128_f32[0]
                                            && lo.v.m128_f32[1]
                                                <= boxMax.v.m128_f32[1]
                                            && hi.v.m128_f32[0]
                                                >= boxMin.v.m128_f32[0]
                                            && hi.v.m128_f32[1]
                                                >= boxMin.v.m128_f32[1])
                                        ++triCount;
                                }
                            }
                        }
                    }
                }
                sprintf(buf, "brushes: %d", brushes);
                DebugRender::RenderText(
                    buf, (int)statsbasex,
                    (int)statsbasey + (int)hFont, Color(wcol[0], wcol[1], wcol[2], wcol[3]), 0.0f, fscale);
                sprintf(buf, "polies: %d", triCount);
                DebugRender::RenderText(
                    buf, (int)statsbasex,
                    (int)statsbasey + 2 * (int)hFont, Color(wcol[0], wcol[1], wcol[2], wcol[3]), 0.0f,
                    fscale);
                sprintf(buf, "box size(units): %d", sample_size);
                DebugRender::RenderText(
                    buf, (int)statsbasex,
                    (int)statsbasey + 3 * (int)hFont, Color(wcol[0], wcol[1], wcol[2], wcol[3]), 0.0f,
                    fscale);
                sprintf(buf, "brush limit: %d", max_thresh.first);
                DebugRender::RenderText(
                    buf, (int)statsbasex,
                    (int)statsbasey + 4 * (int)hFont, Color(wcol[0], wcol[1], wcol[2], wcol[3]), 0.0f,
                    fscale);
                sprintf(buf, "polies limit: %d", max_thresh.second);
                DebugRender::RenderText(
                    buf, (int)statsbasex,
                    (int)statsbasey + 5 * (int)hFont, Color(wcol[0], wcol[1], wcol[2], wcol[3]), 0.0f,
                    fscale);
            }
        }
    }
}
