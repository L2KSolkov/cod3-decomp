// ============================================================================
// tr_staticmodel.cpp - render.o static-model cell insertion (tr_staticmodel.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/ngl_scene.h"

#include <stdint.h>
#include <string.h>

// AeAssert (game.o)
namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1, JRS = 3 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmtstring, ...);
}

class StaticModel;
class PoolAllocator;
class XModel;
struct XModelLod;
class XModelParts;
class LightGridMgr;
struct nglMesh;

namespace LightGrid {
struct TOC;
}

enum TPakId {
    PAK_ID_INVALID = 0xFFFFFFFFu,
    PAK_ID_MIN = 0,
    PAK_ID_MAX = 0x63,
};

// trStaticModelList_t (8 bytes)
struct trStaticModelList_t {
    StaticModel* model;          // +0x00
    trStaticModelList_t* next;   // +0x04
    static PoolAllocator* sAllocator;  // ?sAllocator@trStaticModelList_t@@2PAVPoolAllocator@@A @ 0xF7443C
    static void SetAllocator(PoolAllocator* a);
};

class PoolAllocator {
public:
    void* Allocate(unsigned int s, bool forceHeapAlloc);  // ?Allocate@PoolAllocator@@QAEPAXI_N@Z
};

// BspCell view (staticModels +0x34)
struct BspCell {
    uint8_t _pad[0x34];
    trStaticModelList_t* staticModels;  // +0x34
};

// BspTree view (mCells +0x18)
class BspTree {
public:
    uint8_t _pad[0x18];
    unsigned int mCellsSize;   // +0x18
    BspCell* mCellsList;       // +0x1C
};

// world_t view (bspTree +0x100)
struct world_t {
    uint8_t _pad[0x100];
    BspTree* bspTree;          // +0x100
};

PoolAllocator* trStaticModelList_t::sAllocator;

// ea: 0x004B5300
void trStaticModelList_t::SetAllocator(PoolAllocator* a)
{
    trStaticModelList_t::sAllocator = a;
}
int g_staticCount;                            // ?g_staticCount@@3HA @ 0xEAECD0

// ============================================================================
// Static model / XModel views (IDA layouts)
// ============================================================================
class LightGridData {
public:
    math::Position3::Packed m_ambientColor;   // +0x00
    math::Vector4::Packed m_directionalColor[3];  // +0x0C
    math::Dir3::Packed m_directionalDir[3];       // +0x24
    int m_numDirectional;                         // +0x3C
};
class StaticModel {
public:
    LightGridData lgridData;                 // +0x00
    unsigned int lgridDataInitialized;       // +0x40
    XModel* xmodel;                          // +0x44
    float axis[3][3];                        // +0x48
    float origin[3];                         // +0x60
    float scale;                             // +0x6C
};
class XModelParts {
public:
    uint8_t _pad[0x10];
    struct Hierarchy {
        uint8_t _pad[8];
        unsigned int mSize;                  // +0x08 (InplaceVector)
        void* mList;                         // +0x0C
    } mHierarchy;                            // +0x10
    uint8_t _pad2[0x28 - 0x20];
    struct MeshPtrs {
        unsigned int mSize;                  // +0x28
        nglMesh** mList;                     // +0x2C
    } mMeshPtrs;                             // +0x28
};
struct XModelLod {
    float dist;                              // +0x00
    uint8_t filename[4];                     // +0x04 (InplaceString)
    XModelParts* xmodelParts;                // +0x08
};
class XModel {
public:
    math::Position3 mins;                    // +0x00
    math::Position3 maxs;                    // +0x10
    XModelParts* parts;                      // +0x20
    XModelLod* lod[5];                       // +0x24
    uint8_t _pad[0x48 - 0x38];
    const char* mStr;                        // +0x48 (InplaceString)
};

template <class T>
class IVPointer {
public:
    T* mValue;                               // +0x00
    TPakId mPakId;                            // +0x04
};

struct DObjSkelMat {
    float axis[3][4];                        // +0x00
    float origin[4];                         // +0x30
};
static_assert(sizeof(DObjSkelMat) == 0x40, "DObjSkelMat size mismatch");
static DObjSkelMat boneMtxList_0[40];
static nglMeshParams scale_params;

extern void ValidatePakId(TPakId pakId);     // ?ValidatePakId@@YAXW4TPakId@@@Z
extern TPakId CurPakId();                    // ?CurPakId@@YA?AW4TPakId@@XZ
extern void XModelGetBasePose(IVPointer<XModel> model,
                              DObjSkelMat* out,
                              DObjSkelMat* out2);  // ?XModelGetBasePose@@YAXV?$IVPointer@VXModel@@@@PAUDObjSkelMat@@1@Z
extern void R_UseCachedLightSample(const LightGridData& data);  // ?R_UseCachedLightSample@@YAXABVLightGridData@@@Z
extern void auxSetScale(nglMeshParams* params, float x, float y, float z);  // ?auxSetScale@@YAXPAVnglMeshParams@@MMM@Z
extern nglMeshNode* _codListAddMesh(nglMesh* mesh, const math::Mat43& localToWorld,
                                    nglMeshParams* meshParams,
                                    nglShaderParamSet* shaderParams,
                                    void (__cdecl* fn)(nglMeshNode*));  // ?_codListAddMesh@@YAPAVnglMeshNode@@...@@Z
extern nglLightContext* nglCreateLightContext();  // ?nglCreateLightContext@@YAPAUnglLightContext@@XZ
extern unsigned int nglLightContextParamID;   // ?nglLightContextParamID@@3IA
extern unsigned int cdFlagRandomSeedID;       // ?cdFlagRandomSeedID@@3IA
extern unsigned int isRotatingTextureParamID; // ?isRotatingTextureParamID@@3IA
extern void* nglListAlloc(unsigned int size, unsigned int align);  // ?nglListAlloc@@YAPAXII@Z
extern void CG_DebugBox(const float* p1, const float* p2, const float* color,
                        int a4, int a5);           // ?CG_DebugBox@@YAXQBM00HH@Z
class LightGridMgr {
public:
    static LightGridMgr* sInst;              // ?sInst@LightGridMgr@@2V1@A
    LightGrid::TOC* GetLightGrid(const math::Position3& pos, int* cell);  // ?GetLightGrid@LightGridMgr@@QAEPAUTOC@LightGrid@@ABVPosition3@math@@PAH@Z
    void SampleLightGrid(const LightGrid::TOC& toc, int cell,
                         const math::Position3& pos,
                         LightGridData* out);  // ?SampleLightGrid@LightGridMgr@@QAEXABUTOC@LightGrid@@HABVPosition3@math@@PAVLightGridData@@@Z
};
struct vmCvar_t {
    uint8_t _pad[0x1C];
    float value;                             // +0x1C
    int integer;                             // +0x20
};
extern vmCvar_t g_drawEntBBoxes;             // ?g_drawEntBBoxes@@3UvmCvar_t@@A
extern const float colorWhite[4];

// ============================================================================
// R_AddStaticModelSurfaces - ea: 0x006D2D70
// ============================================================================
void R_AddStaticModelSurfaces(StaticModel* ent)
{
    IVPointer<XModel> ctx;
    ctx.mValue = ent->xmodel;
    ctx.mPakId = CurPakId();
    ValidatePakId(ctx.mPakId);

    int lodIdx = 0;
    if (ctx.mValue->lod[0] == nullptr)
    {
        do
        {
            ++lodIdx;
        } while (ctx.mValue->lod[lodIdx] == nullptr);
    }
    if (ctx.mValue->lod[lodIdx]->xmodelParts != nullptr)
    {
        int idx = 0;
        if (ctx.mValue->lod[0] == nullptr)
        {
            do
            {
                ++idx;
            } while (ctx.mValue->lod[idx] == nullptr);
        }
        if (ctx.mValue->lod[idx]->xmodelParts->mHierarchy.mSize > 40)
        {
            AeAssert::gCurrentAuthor = AeAssert::JRS;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_staticmodel.cpp";
            AeAssert::gCurrentLine = 92;
            AeAssert::gCurrentExpr = "xmod->GetNumBones() <= 40";
            if (!AeAssert::IsIgnored())
            {
                ValidatePakId(ctx.mPakId);
                if (AeAssert::Assert("static XModel %s has too many bones.",
                                     ctx.mValue->mStr))
                    __debugbreak();
            }
        }
    }

    ValidatePakId(ctx.mPakId);
    int lodIdx2 = 0;
    if (ctx.mValue->lod[0] == nullptr)
    {
        do
        {
            ++lodIdx2;
        } while (ctx.mValue->lod[lodIdx2] == nullptr);
    }
    if (ctx.mValue->lod[lodIdx2]->xmodelParts == nullptr)
        goto skip;
    {
        int idx = 0;
        if (ctx.mValue->lod[0] == nullptr)
        {
            do
            {
                ++idx;
            } while (ctx.mValue->lod[idx] == nullptr);
        }
        if (ctx.mValue->lod[idx]->xmodelParts->mHierarchy.mSize > 40)
            goto skip;
    }

    XModelGetBasePose(ctx, boneMtxList_0, boneMtxList_0);

    math::Mat43 localToWorld;
    localToWorld.x.v = _mm_setr_ps(ent->axis[0][0], ent->axis[0][1],
                                   ent->axis[0][2], 0.0f);
    localToWorld.y.v = _mm_setr_ps(ent->axis[1][0], ent->axis[1][1],
                                   ent->axis[1][2], 0.0f);
    localToWorld.z.v = _mm_setr_ps(ent->axis[2][0], ent->axis[2][1],
                                   ent->axis[2][2], 0.0f);
    localToWorld.w.v = _mm_setr_ps(ent->origin[0], ent->origin[1],
                                   ent->origin[2], 1.0f);

    nglLightContext* lightCtx = nglCreateLightContext();
    math::Position3 pos;
    pos.v = localToWorld.w.v;
    pos.v.m128_f32[3] = 0.0f;
    if (ent->lgridDataInitialized != 0)
    {
        if (ent->lgridDataInitialized == 2)
            R_UseCachedLightSample(ent->lgridData);
        goto lit;
    }
    ent->lgridDataInitialized = 1;
    int cell = -1;
    LightGrid::TOC* grid = LightGridMgr::sInst->GetLightGrid(pos, &cell);
    if (grid == nullptr)
    {
        math::Position3 probe = pos;
        probe.v.m128_f32[2] = -24.0f;
        grid = LightGridMgr::sInst->GetLightGrid(probe, &cell);
    }
    if (grid == nullptr)
    {
        math::Position3 probe = pos;
        probe.v.m128_f32[2] = 24.0f;
        grid = LightGridMgr::sInst->GetLightGrid(probe, &cell);
    }
    if (grid != nullptr)
    {
        if (cell >= 0)
        {
            LightGridMgr::sInst->SampleLightGrid(*grid, cell, pos,
                                                 &ent->lgridData);
            ent->lgridDataInitialized = 2;
            goto lit;
        }
    }
    else
    {
        ent->lgridDataInitialized = 0;
    }
    ent->lgridData.m_ambientColor.x = 1.0f;
    ent->lgridData.m_ambientColor.y = 1.0f;
    ent->lgridData.m_ambientColor.z = 1.0f;
lit:
    float scale = ent->scale;
    bool haveScale = false;
    if (scale != 1.0f)
    {
        scale_params.Flags = 2;
        auxSetScale(&scale_params, scale, scale, scale);
        haveScale = true;
    }

    nglShaderParamSet* shaderParams =
        (nglShaderParamSet*)nglListAlloc(4 * nglShaderParamSet::NumParams + 8, 8u);
    shaderParams->Array[0] = 0;
    shaderParams->Array[1] = 0;
    unsigned int id = nglLightContextParamID;
    shaderParams->Array[0] |= (1u << id);
    shaderParams->Array[1] |= (1u << id) >> 32;
    shaderParams->Array[id + 2] = (unsigned int)lightCtx;

    for (unsigned int bone = 0; ; ++bone)
    {
        ValidatePakId(ctx.mPakId);
        int li = 0;
        if (ctx.mValue->lod[0] == nullptr)
        {
            do
            {
                ++li;
            } while (ctx.mValue->lod[li] == nullptr);
        }
        XModelParts* parts = ctx.mValue->lod[li]->xmodelParts;
        unsigned int numBones = parts != nullptr ? parts->mHierarchy.mSize : 0;
        if (bone >= numBones)
            break;
        ValidatePakId(ctx.mPakId);
        int li2 = 0;
        if (ctx.mValue->lod[0] == nullptr)
        {
            do
            {
                ++li2;
            } while (ctx.mValue->lod[li2] == nullptr);
        }
        nglMesh* mesh =
            ctx.mValue->lod[li2]->xmodelParts->mMeshPtrs.mList[bone];
        if (mesh != nullptr)
        {
            unsigned int fid = cdFlagRandomSeedID;
            shaderParams->Array[0] |= (1u << fid);
            shaderParams->Array[1] |= (1u << fid) >> 32;
            shaderParams->Array[fid + 2] = (unsigned int)ent;

            // worldTransform = localToWorld * boneMat (rows x/y/z/w)
            math::Mat43 worldTransform;
            const math::Dir3* bx = (const math::Dir3*)&boneMtxList_0[bone].axis[0][0];
            const math::Dir3* by = (const math::Dir3*)&boneMtxList_0[bone].axis[1][0];
            const math::Dir3* bz = (const math::Dir3*)&boneMtxList_0[bone].axis[2][0];
            const math::Position3* bw =
                (const math::Position3*)&boneMtxList_0[bone].origin[0];
            worldTransform.x.v = _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(bx->v, bx->v, 0), localToWorld.x.v),
                           _mm_mul_ps(_mm_shuffle_ps(bx->v, bx->v, 0x55), localToWorld.y.v)),
                _mm_mul_ps(_mm_shuffle_ps(bx->v, bx->v, 0xAA), localToWorld.z.v));
            worldTransform.y.v = _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(by->v, by->v, 0), localToWorld.x.v),
                           _mm_mul_ps(_mm_shuffle_ps(by->v, by->v, 0x55), localToWorld.y.v)),
                _mm_mul_ps(_mm_shuffle_ps(by->v, by->v, 0xAA), localToWorld.z.v));
            worldTransform.z.v = _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(bz->v, bz->v, 0), localToWorld.x.v),
                           _mm_mul_ps(_mm_shuffle_ps(bz->v, bz->v, 0x55), localToWorld.y.v)),
                _mm_mul_ps(_mm_shuffle_ps(bz->v, bz->v, 0xAA), localToWorld.z.v));
            worldTransform.w.v = _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(bw->v, bw->v, 0), localToWorld.x.v),
                           _mm_mul_ps(_mm_shuffle_ps(bw->v, bw->v, 0x55), localToWorld.y.v)),
                _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(bw->v, bw->v, 0xAA), localToWorld.z.v),
                           localToWorld.w.v));

            unsigned int rid = isRotatingTextureParamID;
            shaderParams->Array[0] |= (1u << rid);
            shaderParams->Array[1] |= (1u << rid) >> 32;
            shaderParams->Array[rid + 2] = 0;

            nglMeshParams params;
            if (haveScale)
                params = scale_params;
            else
                memset(&params, 0, sizeof(params));
            _codListAddMesh(mesh, worldTransform, &params, shaderParams, nullptr);
        }
    }

    if (g_drawEntBBoxes.integer == 2)
    {
        ValidatePakId(ctx.mPakId);
        float mins[3] = {
            ctx.mValue->mins.v.m128_f32[0] + ent->origin[0],
            ctx.mValue->mins.v.m128_f32[1] + ent->origin[1],
            ctx.mValue->mins.v.m128_f32[2] + ent->origin[2],
        };
        ValidatePakId(ctx.mPakId);
        float maxs[3] = {
            ctx.mValue->maxs.v.m128_f32[0] + ent->origin[0],
            ctx.mValue->maxs.v.m128_f32[1] + ent->origin[1],
            ctx.mValue->maxs.v.m128_f32[2] + ent->origin[2],
        };
        CG_DebugBox(mins, maxs, colorWhite, 1, 0);
    }
skip:
    ;
}

// ============================================================================
// R_AddModelToCell - ea: 0x006C6CE0
// ============================================================================
void R_AddModelToCell(world_t* world, StaticModel* psm, int cellNum)
{
    if (psm == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_staticmodel.cpp";
        AeAssert::gCurrentLine = 26;
        AeAssert::gCurrentExpr = "psm";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (cellNum < 0 || cellNum >= (int)world->bspTree->mCellsSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_staticmodel.cpp";
        AeAssert::gCurrentLine = 27;
        AeAssert::gCurrentExpr =
            "cellNum >= 0 && cellNum < world->bspTree->mCells.size()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    BspCell* v3 = &world->bspTree->mCellsList[cellNum];
    StaticModel** p_model = &v3->staticModels->model;
    if (p_model == nullptr || *p_model != psm)
    {
        trStaticModelList_t* v5 = (trStaticModelList_t*)
            trStaticModelList_t::sAllocator->Allocate(8u, false);
        v5->model = psm;
        v5->next = v3->staticModels;
        v3->staticModels = v5;
    }
}
