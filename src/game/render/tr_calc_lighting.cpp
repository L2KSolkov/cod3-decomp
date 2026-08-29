// ============================================================================
// tr_calc_lighting.cpp - render.o calc_lighting (0x6CEFA0)
// Deliberately includes only math_types.h so nglShaderParamSet can be a
// struct (U tag) matching the original binary's map mangle.
// ============================================================================

#include "core/math_types.h"
#include "engine/broc_types.h"
#include "ngl/ngl_lighting.h"
#include "ngl/ngl_scene.h"

#include <stdint.h>

namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmtstring, ...);
}

struct nglShaderParamSet {
    unsigned int* Array;             // +0x00
};
class Entity;

class tagInfoLocal {
public:
    Entity* parent;                  // +0x00
    Entity* next;                    // +0x04
};
class trRefEntity {
public:
    uint8_t _pad[0x148];
    float lastPos[3];                // +0x148
    bool moved;                      // +0x154
};
class Entity {
public:
    uint8_t _pad0[0xE0];
    struct refEntityLocal {
        math::Position3 currentOrigin;  // +0x00
        math::Position3 currentAngles;  // +0x10
    } r;                             // +0xE0
    uint8_t _pad1[0x2C4 - 0x110];
    int flags;                       // +0x2C4
    uint8_t _pad2[0x3BC - 0x2C8];
    short cell_index;                // +0x3BC
    uint8_t _pad3[0x3D4 - 0x3BE];
    tagInfoLocal* tagInfo;           // +0x3D4
    trRefEntity& GetRenderEntity();
};

struct BspCellLocal {
    uint8_t _pad[0x48];
    void* mLgridToc;                 // +0x48
};
struct BspTree {
    uint8_t _pad[0x18];
    struct {
        int mSize;                   // +0x18
        BspCellLocal* mList;         // +0x1C
    } mCells;                        // InplaceVector<BspCell>
};
extern BspTree* g_bspTree;           // ?g_bspTree@@3PAUBspTree@@A
extern void ModelLightingHack();     // ?ModelLightingHack@@YAXXZ
extern unsigned int cdSimpleAlphaAlphaParamID;  // tr_tiny.cpp

namespace LightGrid { struct TOC; }
class LightGridData;
class LightGridMgr {
public:
    static LightGridMgr* sInst;  // ?sInst@LightGridMgr@@2PAV1@A
    LightGrid::TOC* GetLightGrid(const math::Position3& pos, int* cell);
    void SampleLightGrid(const LightGrid::TOC& toc, int cell,
                         const math::Position3& pos, LightGridData* out);
};

// ea: 0x006CEFA0
nglLightContext* calc_lighting(Entity* entity, const math::Mat43& matrix,
                               float alpha, nglShaderParamSet& shaderParams)
{
    nglLightContext* ctx = nglCreateLightContext();
    float v30 = matrix.w.v.m128_f32[0];
    float v31 = matrix.w.v.m128_f32[1];
    float v32 = matrix.w.v.m128_f32[2];
    int flags = 0;
    if (entity != nullptr)
    {
        flags = entity->flags;
        tagInfoLocal* tagInfo = entity->tagInfo;
        if (tagInfo != nullptr)
        {
            Entity* parent = tagInfo->parent;
            v30 = parent->r.currentOrigin.v.m128_f32[0];
            v31 = parent->r.currentOrigin.v.m128_f32[1];
            v32 = parent->r.currentOrigin.v.m128_f32[2];
            flags = parent->flags;
        }
    }
    if ((flags & 0x800000) == 0)
    {
        if ((flags & 0x200000) != 0)
        {
            ModelLightingHack();
        }
        else
        {
            void* toc = nullptr;
            int cellNum = 0;
            if (entity != nullptr)
            {
                cellNum = entity->cell_index;
                if (cellNum != -1)
                    toc = g_bspTree->mCells.mList[cellNum].mLgridToc;
            }
            else
            {
                toc = LightGridMgr::sInst->GetLightGrid(
                    *(math::Position3*)&v30, &cellNum);
            }
            if (toc != nullptr)
            {
                if (entity != nullptr)
                {
                    trRefEntity& re = entity->GetRenderEntity();
                    bool moved = re.lastPos[0] != v30 || re.lastPos[1] != v31
                                || re.lastPos[2] != v32;
                    re.moved = moved;
                    if (moved)
                    {
                        re.lastPos[0] = v30;
                        re.lastPos[1] = v31;
                        re.lastPos[2] = v32;
                    }
                    LightGridMgr::sInst->SampleLightGrid(
                        *(LightGrid::TOC*)toc, entity->cell_index,
                        *(math::Position3*)&v30, nullptr);
                }
                else
                {
                    LightGridMgr::sInst->SampleLightGrid(
                        *(LightGrid::TOC*)toc, cellNum,
                        *(math::Position3*)&v30, nullptr);
                }
            }
            else
            {
                ModelLightingHack();
            }
        }
    }

    // Re-submit the scene's light nodes into the newly created build context.
    // The release binary walks category 6 using the context object as its
    // intrusive-list sentinel before publishing the context shader parameter.
    nglLightContext* buildLightContext = nglBuildScene->LightContext;
    nglLightNode* light = buildLightContext->Head.Next[6];
    nglLightNode* sentinel = reinterpret_cast<nglLightNode*>(buildLightContext);
    while (light != sentinel)
    {
        nglListAddLight(light->Type, light->NodeData, 0xFFFFFFFFu);
        light = light->Next[6];
    }

    unsigned int* Array = shaderParams.Array;
    unsigned int id = nglLightContextParamID;
    if (id < 32)
        Array[0] |= 1u << id;
    else
        Array[1] |= 1u << (id - 32);
    shaderParams.Array[id + 2] = (unsigned int)ctx;
    if (alpha > 0.0f)
    {
        unsigned int* arr = shaderParams.Array;
        unsigned int aid = cdSimpleAlphaAlphaParamID;
        if (aid < 32)
            arr[0] |= 1u << aid;
        else
            arr[1] |= 1u << (aid - 32);
        shaderParams.Array[aid + 2] = *(unsigned int*)&alpha;
    }
    return ctx;
}

class scr_vehicle_t {
public:
    uint8_t _pad[0x51C];
    float treadTime;                 // +0x51C
    float treadTime2;                // +0x520
};
// Entity::scr_vehicle @ +0x260 (already in Entity above? add accessor)
extern void* nglListAlloc(unsigned int size, unsigned int align);  // ?nglListAlloc@@YAPAXII@Z
extern void make_rotate(math::Mat43& m, int axis, float angle);  // ?make_rotate@@YAXAAVMat43@math@@HM@Z
extern unsigned int TextureMatrixParamID;    // ?TextureMatrixParamID@@3IA
extern unsigned int isRotatingTextureParamID;  // ?isRotatingTextureParamID@@3IA

static unsigned int g_tag_tread_left;
static unsigned int g_tag_tread_right;
static unsigned int g_tag_left_gear_hash;
static unsigned int g_tag_right_gear_hash;
static int g_tagInit;

// ============================================================================
// AddTextureMatrix - ea: 0x006CE850
// ============================================================================
bool AddTextureMatrix(Entity* ent, unsigned int boneNameHash,
                      nglShaderParamSet& shaderParams)
{
    if ((g_tagInit & 1) == 0)
    {
        g_tagInit |= 1;
        g_tag_tread_left = HashString::CalcHash("tag_tread_left");
    }
    if ((g_tagInit & 2) == 0)
    {
        g_tagInit |= 2;
        g_tag_tread_right = HashString::CalcHash("tag_tread_right");
    }
    if ((g_tagInit & 4) == 0)
    {
        g_tagInit |= 4;
        g_tag_left_gear_hash = HashString::CalcHash("tag_gear_left");
    }
    if ((g_tagInit & 8) == 0)
    {
        g_tagInit |= 8;
        g_tag_right_gear_hash = HashString::CalcHash("tag_gear_right");
    }

    scr_vehicle_t* veh = (scr_vehicle_t*)*(void**)((char*)ent + 0x260);
    float v5;
    if (boneNameHash == g_tag_tread_left)
        v5 = veh->treadTime2 * 0.001f;
    else if (boneNameHash == g_tag_tread_right)
        v5 = veh->treadTime * 0.001f;
    else if (boneNameHash == g_tag_left_gear_hash
             || boneNameHash == g_tag_right_gear_hash)
        v5 = 0.0f;
    else
        v5 = veh->treadTime * 0.001f;

    math::Mat43* m = (math::Mat43*)nglListAlloc(0x40u, 0x10u);
    m->x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    m->y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    m->z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    m->w.v = _mm_setr_ps(0.0f, 0.0f, v5, 0.0f);

    float rotation = 0.0f;
    if (boneNameHash == g_tag_left_gear_hash)
    {
        rotation = veh->treadTime2 * 0.0014f;
        make_rotate(*m, 2, rotation);
        // original also mirrors the rotation axis; keep the simple rotation
    }
    else if (boneNameHash == g_tag_right_gear_hash)
    {
        rotation = veh->treadTime * 0.0014f;
        make_rotate(*m, 2, rotation);
    }

    unsigned int* Array = shaderParams.Array;
    unsigned int id = TextureMatrixParamID;
    Array[0] |= (1u << id);
    shaderParams.Array[id + 2] = (unsigned int)m;
    unsigned int* arr = shaderParams.Array;
    unsigned int rid = isRotatingTextureParamID;
    if (rid < 32)
        arr[0] |= 1u << rid;
    else
        arr[1] |= 1u << (rid - 32);
    shaderParams.Array[rid + 2] = boneNameHash == g_tag_left_gear_hash
                                || boneNameHash == g_tag_right_gear_hash
        ? 1u : 0u;
    return true;
}
