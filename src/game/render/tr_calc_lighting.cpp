// ============================================================================
// tr_calc_lighting.cpp - render.o calc_lighting (0x6CEFA0)
// Deliberately includes only math_types.h so nglShaderParamSet can be a
// struct (U tag) matching the original binary's map mangle.
// ============================================================================

#include "core/math_types.h"

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
struct nglLightContext;
class Entity;

class tagInfoLocal {
public:
    Entity* parent;                  // +0x00
    Entity* next;                    // +0x04
};
struct trRefEntityLocal {
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
};

struct BspCellLocal {
    uint8_t _pad[0x48];
    void* mLgridToc;                 // +0x48
};
struct BspTreeLocal2 {
    uint8_t _pad[0x18];
    BspCellLocal* mCellsList;        // +0x18
    unsigned int mCellsSize;         // +0x1C
};
extern BspTreeLocal2* g_bspTree;     // ?g_bspTree@@3PAUBspTree@@A
extern void ModelLightingHack();     // ?ModelLightingHack@@YAXXZ
extern nglLightContext* nglCreateLightContext();  // ?nglCreateLightContext@@YAPAUnglLightContext@@XZ
extern void nglListAddLight(int type, void* data, int unknown);  // ?nglListAddLight@@YAXW4nglLightType@@PAXH@Z
extern trRefEntityLocal& Entity_GetRenderEntity(Entity* self);  // ?GetRenderEntity@Entity@@QAEAAVtrRefEntity@@XZ
extern unsigned int nglLightContextParamID;   // ngl_lighting.cpp
extern unsigned int cdSimpleAlphaAlphaParamID;  // tr_tiny.cpp

class LightGridMgr2 {
public:
    void* GetLightGrid(const math::Position3& pos, int* cell);  // ?GetLightGrid@LightGridMgr@@QAEPBUTOC@1@ABVPosition3@math@@PAH@Z
    void SampleLightGrid(void* toc, int cell, const math::Position3& pos,
                         void* out);  // ?SampleLightGrid@LightGridMgr@@QAEXABUTOC@1@HABVPosition3@math@@PAVMat44@4@2@Z
};
extern LightGridMgr2* LightGridMgr_sInst;  // ?sInst@LightGridMgr@@2V1@A

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
                    toc = g_bspTree->mCellsList[cellNum].mLgridToc;
            }
            else
            {
                toc = LightGridMgr_sInst->GetLightGrid(
                    *(math::Position3*)&v30, &cellNum);
            }
            if (toc != nullptr)
            {
                if (entity != nullptr)
                {
                    trRefEntityLocal& re = Entity_GetRenderEntity(entity);
                    bool moved = re.lastPos[0] != v30 || re.lastPos[1] != v31
                                || re.lastPos[2] != v32;
                    re.moved = moved;
                    if (moved)
                    {
                        re.lastPos[0] = v30;
                        re.lastPos[1] = v31;
                        re.lastPos[2] = v32;
                    }
                    LightGridMgr_sInst->SampleLightGrid(
                        toc, entity->cell_index, *(math::Position3*)&v30, nullptr);
                }
                else
                {
                    LightGridMgr_sInst->SampleLightGrid(
                        toc, cellNum, *(math::Position3*)&v30, nullptr);
                }
            }
            else
            {
                ModelLightingHack();
            }
        }
    }
    unsigned int* Array = shaderParams.Array;
    unsigned int id = nglLightContextParamID;
    Array[0] |= (1u << id);
    Array[1] |= (1u << id) >> 32;
    shaderParams.Array[id + 2] = (unsigned int)ctx;
    if (alpha > 0.0f)
    {
        unsigned int* arr = shaderParams.Array;
        unsigned int aid = cdSimpleAlphaAlphaParamID;
        arr[0] |= (1u << aid);
        arr[1] |= (1u << aid) >> 32;
        shaderParams.Array[aid + 2] = *(unsigned int*)&alpha;
    }
    return ctx;
}
