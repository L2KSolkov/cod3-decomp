// ============================================================================
// tr_xmodel_cluster.cpp - render.o shadow/xmodel surface helpers
// do_shadow (0x6CED60) + static GetScaledMeshLOD (0x6BE740).
// ============================================================================

#include "core/math_types.h"

#include <stdint.h>
#include <math.h>

namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1, JRS = 3 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmtstring, ...);
}

struct nglScene;
struct nglMesh;
class nglMeshParams;
struct nglShaderParamSet;
class nglMeshNode;
class DObj;
class XModel;
class XModelParts;
struct XModelLod;

struct nglSceneLocal {
    uint8_t _pad[0x8C];
    math::Position3 ViewPos;         // +0x8C
    math::Mat44 WorldToView;         // +0x9C
};
extern nglScene* nglBuildScene;      // ?nglBuildScene@@3PAUnglScene@@A

// ---- static helpers (not public render.o symbols) ----
float gShadowCastCullAbove = 40.0f;   // static render.o
float gShadowCastCullBelow = 100.0f;  // static render.o
extern nglScene* gProjShadowScene;    // ?gProjShadowScene@@3PAUnglScene@@A
extern nglMeshNode* nglListAddMesh(nglMesh* mesh, const math::Mat43& localToWorld,
                                   nglMeshParams* meshParams,
                                   nglShaderParamSet* shaderParams,
                                   void (__cdecl* fn)(nglMeshNode*));  // ?nglListAddMesh@@YAPAVnglMeshNode@@...@@Z
extern nglScene* nglListSelectScene(nglScene* scene);  // ?nglListSelectScene@@YAPAUnglScene@@PAU1@@Z
extern math::Position3* auxGetSphereCenter(math::Position3* result, nglMesh* mesh);  // ?auxGetSphereCenter@@YAPAVPosition3@math@@PAV12@PAUnglMesh@@@Z
extern float auxGetSphereRadius(nglMesh* mesh);  // ?auxGetSphereRadius@@YAMPAUnglMesh@@@Z
extern void ValidatePakId(int pakId);  // ?ValidatePakId@@YAXW4TPakId@@@Z
extern math::Mat43* nglListAddMesh_GetScaledMatrix(const math::Mat43& m,
                                                   nglMeshParams* p,
                                                   float* scale);  // ?nglListAddMesh_GetScaledMatrix@@YAPAVMat43@math@@ABV12@PAVnglMeshParams@@PAM@Z
extern void DObj_SetLODOverride(DObj* obj, int lod);  // ?SetLODOverride@DObj@@QAEXH@Z

struct nglMeshLocal {
    uint8_t _pad[0x0C];
    math::Vector4 Sphere;            // +0x0C
    struct LODLocal {
        float Range;                 // +0x00
        void* Mesh;                  // +0x04
    } LODs[1];                       // +0x1C
    int NLODs;                       // (tail)
};

// GetScaledMeshLOD - ea: 0x006BE740 (static render.o helper)
static int GetScaledMeshLOD(nglMesh* Mesh, const math::Mat43* LocalToWorld,
                            nglMeshParams* MeshParams, float a4)
{
    if ((*(unsigned int*)MeshParams & 2) != 0)
        LocalToWorld = nglListAddMesh_GetScaledMatrix(
            *LocalToWorld, MeshParams, nullptr);
    __m128 v4 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(((nglMeshLocal*)Mesh)->Sphere.v,
                                      ((nglMeshLocal*)Mesh)->Sphere.v, 0),
                       LocalToWorld->x.v),
            _mm_mul_ps(_mm_shuffle_ps(((nglMeshLocal*)Mesh)->Sphere.v,
                                      ((nglMeshLocal*)Mesh)->Sphere.v, 0x55),
                       LocalToWorld->y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(((nglMeshLocal*)Mesh)->Sphere.v,
                                      ((nglMeshLocal*)Mesh)->Sphere.v, 0xAA),
                       LocalToWorld->z.v),
            LocalToWorld->w.v));
    __m128 v5 = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v4, v4, 0),
                       ((nglSceneLocal*)nglBuildScene)->WorldToView.x.v),
            _mm_mul_ps(_mm_shuffle_ps(v4, v4, 0x55),
                       ((nglSceneLocal*)nglBuildScene)->WorldToView.y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(v4, v4, 0xAA),
                       ((nglSceneLocal*)nglBuildScene)->WorldToView.z.v),
            ((nglSceneLocal*)nglBuildScene)->WorldToView.w.v));
    float v6 = _mm_shuffle_ps(v5, v5, 0xAA).m128_f32[0] * a4;
    int result = 0;
    int v8 = ((nglMeshLocal*)Mesh)->NLODs - 1;
    if (v8 >= 4)
    {
        float* v9 = &((nglMeshLocal*)Mesh)->LODs[1].Range;
        while (v6 > *(v9 - 2))
        {
            if (v6 <= *v9)
                return ++result;
            if (v6 <= v9[2])
            {
                result += 2;
                return result;
            }
            if (v6 <= v9[4])
            {
                result += 3;
                return result;
            }
            result += 4;
            v9 += 8;
            if (result >= (((nglMeshLocal*)Mesh)->NLODs - 4))
                break;
        }
    }
    if (result < v8)
    {
        float* p_Range = &((nglMeshLocal*)Mesh)->LODs[result].Range;
        do
        {
            if (v6 <= *p_Range)
                break;
            ++result;
            p_Range += 2;
        } while (result < v8);
    }
    return result;
}

// ---- do_shadow - ea: 0x006CED60 ----
class DObjLocal {
public:
    uint8_t _pad[0x80];
    struct IVModel {
        void* mValue;                // +0x00
        int mPakId;                  // +0x04
    } models[8];                     // +0x80
    int mLOD;                        // +0xD8
    int mLODOverride;                // +0xDC
    int mLODAnim;                    // +0xE0
    unsigned int mFlags;             // +0xE4
};
struct XModelPartsLocal {
    uint8_t _pad[0x28];
    struct MeshPtrs {
        unsigned int mSize;          // +0x28
        nglMesh** mList;             // +0x2C
    } mMeshPtrs;                     // +0x28
};
struct XModelLodLocal {
    float dist;                      // +0x00
    uint8_t filename[4];             // +0x04
    XModelPartsLocal* xmodelParts;   // +0x08
};
struct XModelLocal {
    uint8_t _pad[0x24];
    XModelLodLocal* lod[5];          // +0x24
};
extern XModelParts* XModel_GetXModelParts(XModel* xmodel, int lodIndex);  // ?GetXModelParts@XModel@@QAEPAVXModelParts@@H@Z (g.o)

void do_shadow(DObj* obj, int model_index, int bone_index,
               nglMesh* mesh, const math::Mat43& matrix,
               const math::Mat43& worldMatrix, nglMeshParams& meshParams,
               nglShaderParamSet& shaderParams, nglMeshNode* node)
{
    (void)node;
    DObjLocal* dobj = (DObjLocal*)obj;
    float camZ = ((nglSceneLocal*)nglBuildScene)->ViewPos.v.m128_f32[2];
    float above = camZ + gShadowCastCullAbove;
    float mid = ((camZ - gShadowCastCullBelow) + (camZ + gShadowCastCullAbove)) * 0.5f;
    math::Position3 sphereCenter;
    auxGetSphereCenter(&sphereCenter, mesh);
    math::Position3 centerWorld;
    centerWorld.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(sphereCenter.v, sphereCenter.v, 0), matrix.x.v),
            _mm_mul_ps(_mm_shuffle_ps(sphereCenter.v, sphereCenter.v, 0x55), matrix.y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(sphereCenter.v, sphereCenter.v, 0xAA), matrix.z.v),
            matrix.w.v));
    float sphereZ = centerWorld.v.m128_f32[2];
    __m128 delta = _mm_sub_ps(((nglSceneLocal*)nglBuildScene)->ViewPos.v,
                              centerWorld.v);
    float distSq = delta.m128_f32[0] * delta.m128_f32[0]
                 + delta.m128_f32[1] * delta.m128_f32[1]
                 + delta.m128_f32[2] * delta.m128_f32[2];
    float radius = auxGetSphereRadius(mesh);
    if (above - mid < fabsf(sphereZ - mid)
        || distSq - radius * radius > 360000.0f)
    {
        *(unsigned int*)&meshParams |= 0x100u;
    }
    else
    {
        nglScene* oldScene = nglListSelectScene(gProjShadowScene);
        DObj_SetLODOverride(obj, 4);
        int lod;
        if (model_index != 0)
        {
            lod = 0x7FFFFFFF;
        }
        else if (dobj->mLODOverride < 0)
        {
            if (dobj->mLODAnim < 0)
                lod = dobj->mLOD;
            else
                lod = dobj->mLODAnim;
        }
        else
        {
            lod = dobj->mLODOverride;
        }
        XModel* mValue = (XModel*)dobj->models[model_index].mValue;
        int mPakId = dobj->models[model_index].mPakId;
        if (model_index == -1)
        {
            mValue = (XModel*)dobj->models[0].mValue;
            mPakId = dobj->models[0].mPakId;
        }
        ValidatePakId(mPakId);
        XModelParts* parts = XModel_GetXModelParts(mValue, lod);
        nglMesh* v17 =
            ((XModelPartsLocal*)parts)->mMeshPtrs.mList[bone_index];
        if (v17 == nullptr)
            v17 = mesh;
        *(unsigned int*)&meshParams |= 0x80u;
        ((unsigned int*)&meshParams)[3] =  // LOD @ +0x0C
            (unsigned int)GetScaledMeshLOD(v17, &worldMatrix, &meshParams, 100000.0f);
        nglListAddMesh(v17, worldMatrix, &meshParams, &shaderParams, nullptr);
        nglListSelectScene(oldScene);
        dobj->mLODOverride = -1;
    }
}
