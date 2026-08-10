// ============================================================================
// g_cm_load.cpp - game.o CM_ BSP leaf helpers (cm_load.cpp)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <malloc.h>
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
    int      numClusters;   // +0x58
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
extern char cmgr_memory_buffer[0x400];     // ?cmgr_memory_buffer@@3PADA (game.o)
extern bool tlScratchpadLocked;            // ?tlScratchpadLocked@@3_NA
extern bool g_in_cmgr_mem_context;         // ?g_in_cmgr_mem_context@@3_NA

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
    uint8_t _padF4[0xFC - 0xF4];
    uint8_t isPoint;               // +0xFC
    uint8_t _padFD[0x120 - 0xFD];
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
    TempBoxModel(&tw->size[0], &tw->size[1], v7, 0);
    cdl_object_t* objects = (cdl_object_t*)gBoxDCGSet->objects_m_elements;
    if (gBoxDCGSet->objects_m_count == 0)
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
            unpack(bank, pi, cg_verts);
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
            unpack(bank, pi, verts);
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
            unpack(bank, pi, cg_verts);
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
            unpack(bank, pi, cg_verts);
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
unsigned int SightTraceThroughLeaf(traceWork_t* tw, const DCGSet* set)
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
struct DCGBankManager {
    void* __vftable;             // +0x00
    static DCGBankManager* sInst;  // ?sInst@DCGBankManager@@2PAV1@A @ 0xF4F43C
    const DCGSet* GetDCGSet(TPakId pakId, int handle);  // ?GetDCGSet@DCGBankManager@@QBEPBVDCGSet@@W4TPakId@@H@Z
    uint8_t _pad[0x18C];           // +0x04
    void*   mBoxDCGSet;            // +0x190 (TempDCGSet)
    ~DCGBankManager();             // ??1DCGBankManager@@UAE@XZ (game.o 0x629D90)
};
DCGBankManager* DCGBankManager::sInst = nullptr;

extern void TempDCGSet_Dtor(void* self);     // DCGBankManager::TempDCGSet::~TempDCGSet
extern void AssetBankSet_Dtor(void* self);   // AssetBankSet::~AssetBankSet

// ea: 0x00629D90
DCGBankManager::~DCGBankManager()
{
    this->__vftable = 0;
    TempDCGSet_Dtor(&this->mBoxDCGSet);
    AssetBankSet_Dtor(this);
}

struct GdbFileManager {
    void* __vftable;  // +0x00
    GdbFileManager();  // ??0GdbFileManager@@AAE@XZ (game.o 0x629920)
    ~GdbFileManager(); // ??1GdbFileManager@@EAE@XZ (game.o 0x61F790)
    void DecodeBank(const char* name, void* data, int size, TPakId pakId,
                    void* pakFile);  // ?DecodeBank@GdbFileManager@@QAEXPBDPAEHW4TPakId@@PAVPakFile@@@Z (game.o 0x629940)
};

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
void unpack(const CGBank* bank, unsigned int pi, math::Position3* verts)
{
    if (pi >= (unsigned int)bank->gjk_patches.m_count
        && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 89,
                     "index >= 0 && index < size()", "invalid index"))
        __debugbreak();
    cdl_vinfo_t* v3 = &((cdl_vinfo_t*)bank->gjk_patches.m_elements)[pi];
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
    unpack(v3, &bank->patch_verts, (math::Dir3*)verts);
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
extern void AngleVectors(const math::Position3* angles, float* forward,
                         float* right, float* up);  // core.o
extern void traverse_rtree(const math::Position3& p0,
                           const math::Position3& p1,
                           const rtree_root_t& root,
                           subdivision_visitor& visitor);  // physics.o 0x6F6620

// ============================================================================
// CM_PointContents - ea: 0x632500 (cm_test.cpp)
// ============================================================================
// ea: 0x00632500
int CM_PointContents(const math::Position3* p, DCGSet* model)
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
                if (TestPointInBox(*p, bmin, bmax))
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
                if (TestPointInBrush(*p, bmin, bmax, sides,
                                     (unsigned int)brush->num_sides))
                    contents |= obj->cflags;
            }
            return contents;
        }
        return contents;
    }

    math::Position3 pos;
    pos.v = _mm_setr_ps(p->v.m128_f32[0], p->v.m128_f32[1],
                        p->v.m128_f32[2], 0.0f);
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
        AngleVectors(&angles, forward, right, up);
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
    return CM_PointContents((const math::Position3*)local, model);
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
                && !context->__vftable->filter(
                    context, (Entity*)((char*)entities - 0xE0)))
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
            && !context->__vftable->filter(
                context, (Entity*)((char*)entities - 0xE0)))
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

extern int CM_TraceBox(const math::Position3* start,
                       const math::Position3* end,
                       const math::Position3* mins,
                       const math::Position3* maxs,
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
                if (CM_TraceBox(&tw->start, &tw->end, &v15, &v16,
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
    this->__vftable = 0;
}

// ea: 0x0061F790
GdbFileManager::~GdbFileManager()
{
    this->__vftable = 0;
}

extern void InplaceAssetBank_GdbFileSet_Fixup(void* data);   // streamer.o
extern void InplaceAssetBankSet_GdbFileBank_AddBank(void* self, TPakId pakId,
                                                   void* data);  // streamer.o

// ea: 0x00629940
void GdbFileManager::DecodeBank(const char* name, void* data, int size,
                                TPakId pakId, void* pakFile)
{
    InplaceAssetBank_GdbFileSet_Fixup(data);
    InplaceAssetBankSet_GdbFileBank_AddBank(this, pakId, data);
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
void CM_CreateStaticModel(const char* name, TPakId pakId, float*& axis,
                          float*& origin, float*& scale)
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
                && !context.__vftable->filter(
                       const_cast<collision_context_t*>(&context),
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
            && !context->__vftable->filter(
                   const_cast<collision_context_t*>(context),
                   (Entity*)((char*)entities - 0xE0)))
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

