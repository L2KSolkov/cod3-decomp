// ============================================================================
// tr_xmodel_cluster.cpp - render.o shadow/xmodel surface helpers
// do_shadow (0x6CED60) + static GetScaledMeshLOD (0x6BE740).
// ============================================================================

#include "core/math_types.h"

#include <stdint.h>
#include <math.h>
#include <string.h>
#include <intrin.h>

namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1, JRS = 3, JSV = 0xA };
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
struct nglShaderParamSet {
public:
    unsigned char mData[4];
    static unsigned int NumParams;  // ?NumParams@nglShaderParamSet@@2IA (ngl_dx_core.cpp)
};
static_assert(sizeof(nglShaderParamSet) == 4, "nglShaderParamSet size mismatch");
class nglMeshNode;
struct XModelLod;
struct DObjSkelMat;
struct nglLightContext;

// nglMeshParams (IDA type; size 0x20) - class V tag per binary manglings
class nglMeshParams {
public:
    unsigned int Flags;   // +0x00
    int NBones;           // +0x04
    void* Bones;          // +0x08
    unsigned int LOD;     // +0x0C
    float Scale[4];       // +0x10
};
static_assert(sizeof(nglMeshParams) == 0x20, "nglMeshParams size mismatch");

// IDA-verified views for the surface adders (tr_xmodel.cpp family)
struct trRefEntityLocal {
    uint8_t _pad[0xEC];
    float mScale;               // +0xEC
    uint8_t _pad2[0xFC - 0xF0];
    unsigned char iflIndex;     // +0xFC
};
struct SentientLocal {
    uint8_t _pad[0x04];
    int eTeam;                  // +0x04
};
class Entity {
public:
    uint8_t _pad0[0x25C];
    SentientLocal* sentient;        // +0x25C
    void* scr_vehicle;              // +0x260
    uint8_t _pad2[0x268 - 0x264];
    trRefEntityLocal* mRenderEntity;// +0x268
    uint8_t _pad3[0x278 - 0x26C];
    float modelscale;               // +0x278
    uint8_t _pad4[0x2C4 - 0x27C];
    int flags;                      // +0x2C4
    unsigned int mFlags;            // +0x2C8
};
class DObj {
public:
    uint8_t _pad0[0x40];
    struct PoseLocal {
        uint8_t _pad[0x04];
        int LOD;                    // +0x04 (nalBasePose::LOD)
    }* mPose[8];                    // +0x40
    uint8_t _pad60[0x80 - 0x60];
    struct Model {
        void* mValue;               // +0x00
        int mPakId;                 // +0x04
    } models[8];                    // +0x80
    uint8_t _padC0[0xCE - 0xC0];
    unsigned char numModels;        // +0xCE
    unsigned char numBones;         // +0xCF
    uint8_t _padD0[0xD8 - 0xD0];
    int mLOD;                       // +0xD8
    int mLODOverride;               // +0xDC
    int mLODAnim;                   // +0xE0
    unsigned int mFlags;            // +0xE4
    void SetLOD(int startLod);      // ?SetLOD@DObj@@QAEXH@Z
    void SetLODAnim(int startLod);  // ?SetLODAnim@DObj@@QAEXH@Z
    void SetLODOverride(int startLod);  // ?SetLODOverride@DObj@@QAEXH@Z
};
class XModelParts {
public:
    uint8_t _pad0[0x10];
    struct Hierarchy {
        unsigned int mSize;         // +0x10
        struct BoneHier {
            unsigned int mName;      // +0x00 (InplaceString::mStr)
            unsigned int mNameHash;  // +0x04
            int mParentIndex;        // +0x08
        }* mList;                    // +0x14
    } mHierarchy;
    uint8_t _pad2[0x28 - 0x18];
    struct MeshPtrs {
        unsigned int mSize;          // +0x28
        nglMesh** mList;             // +0x2C
    } mMeshPtrs;
};
class XModel {
public:
    uint8_t _pad[0x24];
    XModelLod* lod[5];              // +0x24
    XModelParts* GetXModelParts(int lodIndex);  // ?GetXModelParts@XModel@@QAEPAVXModelParts@@H@Z
    const XModelParts* GetXModelParts(int lodIndex) const;  // ?GetXModelParts@XModel@@QBEPBVXModelParts@@H@Z (g.o)
};

struct nglSceneLocal {
    uint8_t _pad[0x150];
    math::Mat44 WorldToView;         // +0x150
    uint8_t _pad2[0x250 - 0x190];
    math::Position3 ViewPos;         // +0x250
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
extern void* nglListAlloc(unsigned int size, unsigned int align);  // ?nglListAlloc@@YAPAXII@Z
extern nglMeshNode* _codListAddMesh(nglMesh* mesh,
                                    const math::Mat43& localToWorld,
                                    nglMeshParams* meshParams,
                                    nglShaderParamSet* shaderParams,
                                    void (__cdecl* fn)(nglMeshNode*));  // tr_dpvs.cpp
extern nglLightContext* calc_lighting(Entity* entity, const math::Mat43& matrix,
                                      float alpha,
                                      nglShaderParamSet& shaderParams);  // ?calc_lighting@@YAPAUnglLightContext@@PAVEntity@@ABVMat43@math@@MAAUnglShaderParamSet@@@Z
extern bool AddTextureMatrix(Entity* ent, unsigned int boneNameHash,
                             nglShaderParamSet& shaderParams);  // ?AddTextureMatrix@@YA_NPAVEntity@@IAAUnglShaderParamSet@@@Z
extern DObjSkelMat* DObjGetMatrixArray(const DObj* obj, int modelIndex);  // ?DObjGetMatrixArray@@YAPAUDObjSkelMat@@PBVDObj@@H@Z
extern unsigned int isRotatingTextureParamID;  // ?isRotatingTextureParamID@@3IA
extern unsigned int nglTextureFrameParamID;    // ?nglTextureFrameParamID@@3IA
extern int g_DOBJF_NOT_RENDERED_LAST_FRAME;    // ?g_DOBJF_NOT_RENDERED_LAST_FRAME@@3HA
extern float gZoomRatio;                       // ?gZoomRatio@@3MA (cg.o)
extern int auxGetNBones(nglMesh* m);           // ?auxGetNBones@@YAHPAUnglMesh@@@Z
extern void auxSetScale(nglMeshParams* meshParams, float sx, float sy,
                        float sz);             // ?auxSetScale@@YAXPAVnglMeshParams@@MMM@Z
extern float computeLOD(DObj* obj, const math::Position3& center);  // ?computeLOD@@YAMPAVDObj@@ABVPosition3@math@@@Z
extern nglLightContext* nglCreateLightContext();  // ?nglCreateLightContext@@YAPAUnglLightContext@@XZ
extern void ModelLightingHack();                  // ?ModelLightingHack@@YAXXZ
extern unsigned int nglLightContextParamID;       // ?nglLightContextParamID@@3IA
extern void render_view_model_arms(int client_index, nglMeshParams& meshParams,
                                   const math::Mat43& matrix,
                                   math::Mat43& worldTrasform);  // ?render_view_model_arms@@YAXHAAVnglMeshParams@@ABVMat43@math@@AAV23@@Z
extern void render_view_model_weapon(int client_index,
                                     nglMeshParams& meshParams,
                                     math::Mat43& modelTransform);  // ?render_view_model_weapon@@YAXHAAVnglMeshParams@@AAVMat43@math@@@Z

// trGlobals view (viewParms.zFar +0x180)
struct trGlobals_t {
    uint8_t _pad[0x10];
    struct {
        uint8_t _pad[0x180];
        float zFar;  // +0x180
    } viewParms;     // +0x10
};
extern trGlobals_t tr;    // ?tr@@3UtrGlobals_t@@A @ 0xF74DD0

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
    uint8_t _padC0[0xCE - 0xC0];
    unsigned char numModels;         // +0xCE
    unsigned char numBones;          // +0xCF
    uint8_t _padD0[0xD8 - 0xD0];
    int mLOD;                        // +0xD8
    int mLODOverride;                // +0xDC
    int mLODAnim;                    // +0xE0
    unsigned int mFlags;             // +0xE4
};

// Entity view (scr_vehicle +0x260) - class V tag
class EntityLocal {
public:
    uint8_t _pad[0x260];
    void* scr_vehicle;  // +0x260
};

struct DObjSkelMatLocal {
    float axis[3][4];  // +0x00
    float origin[4];   // +0x30
};
struct XModelPartsLocal {
    uint8_t _pad0[0x10];
    struct Hierarchy {
        unsigned int mSize;  // +0x10
        struct BoneHier {
            unsigned int mName;      // +0x00 (InplaceString::mStr)
            unsigned int mNameHash;  // +0x04
            int mParentIndex;        // +0x08
        }* mList;                    // +0x14
    } mHierarchy;
    uint8_t _pad2[0x28 - 0x18];
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
        obj->SetLODOverride(4);
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
        XModelParts* parts = mValue->GetXModelParts(lod);
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

// ============================================================================
// R_AddVehicleSurfaces - ea: 0x006D0B50
// ============================================================================
static math::Mat43 VehicleWorldMatrix(const DObjSkelMatLocal* bone,
                                      const math::Mat43& matrix)
{
    // world = bone * matrix (row-vector); w row = origin * axis + translation.
    math::Mat43 world;
    world.x.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(_mm_setr_ps(bone->axis[0][0],
                                                  bone->axis[0][1],
                                                  bone->axis[0][2], 0.0f),
                                      _mm_setr_ps(bone->axis[0][0],
                                                  bone->axis[0][1],
                                                  bone->axis[0][2], 0.0f),
                                      0),
                       matrix.x.v),
            _mm_mul_ps(_mm_shuffle_ps(_mm_setr_ps(bone->axis[0][0],
                                                  bone->axis[0][1],
                                                  bone->axis[0][2], 0.0f),
                                      _mm_setr_ps(bone->axis[0][0],
                                                  bone->axis[0][1],
                                                  bone->axis[0][2], 0.0f),
                                      0x55),
                       matrix.y.v)),
        _mm_mul_ps(_mm_shuffle_ps(_mm_setr_ps(bone->axis[0][0],
                                              bone->axis[0][1],
                                              bone->axis[0][2], 0.0f),
                                  _mm_setr_ps(bone->axis[0][0],
                                              bone->axis[0][1],
                                              bone->axis[0][2], 0.0f),
                                  0xAA),
                   matrix.z.v));
    world.y.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(_mm_setr_ps(bone->axis[1][0],
                                                  bone->axis[1][1],
                                                  bone->axis[1][2], 0.0f),
                                      _mm_setr_ps(bone->axis[1][0],
                                                  bone->axis[1][1],
                                                  bone->axis[1][2], 0.0f),
                                      0),
                       matrix.x.v),
            _mm_mul_ps(_mm_shuffle_ps(_mm_setr_ps(bone->axis[1][0],
                                                  bone->axis[1][1],
                                                  bone->axis[1][2], 0.0f),
                                      _mm_setr_ps(bone->axis[1][0],
                                                  bone->axis[1][1],
                                                  bone->axis[1][2], 0.0f),
                                      0x55),
                       matrix.y.v)),
        _mm_mul_ps(_mm_shuffle_ps(_mm_setr_ps(bone->axis[1][0],
                                              bone->axis[1][1],
                                              bone->axis[1][2], 0.0f),
                                  _mm_setr_ps(bone->axis[1][0],
                                              bone->axis[1][1],
                                              bone->axis[1][2], 0.0f),
                                  0xAA),
                   matrix.z.v));
    world.z.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(_mm_setr_ps(bone->axis[2][0],
                                                  bone->axis[2][1],
                                                  bone->axis[2][2], 0.0f),
                                      _mm_setr_ps(bone->axis[2][0],
                                                  bone->axis[2][1],
                                                  bone->axis[2][2], 0.0f),
                                      0),
                       matrix.x.v),
            _mm_mul_ps(_mm_shuffle_ps(_mm_setr_ps(bone->axis[2][0],
                                                  bone->axis[2][1],
                                                  bone->axis[2][2], 0.0f),
                                      _mm_setr_ps(bone->axis[2][0],
                                                  bone->axis[2][1],
                                                  bone->axis[2][2], 0.0f),
                                      0x55),
                       matrix.y.v)),
        _mm_mul_ps(_mm_shuffle_ps(_mm_setr_ps(bone->axis[2][0],
                                              bone->axis[2][1],
                                              bone->axis[2][2], 0.0f),
                                  _mm_setr_ps(bone->axis[2][0],
                                              bone->axis[2][1],
                                              bone->axis[2][2], 0.0f),
                                  0xAA),
                   matrix.z.v));
    world.w.v = _mm_add_ps(
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(_mm_setr_ps(bone->origin[0],
                                                  bone->origin[1],
                                                  bone->origin[2], 0.0f),
                                      _mm_setr_ps(bone->origin[0],
                                                  bone->origin[1],
                                                  bone->origin[2], 0.0f),
                                      0),
                       matrix.x.v),
            _mm_mul_ps(_mm_shuffle_ps(_mm_setr_ps(bone->origin[0],
                                                  bone->origin[1],
                                                  bone->origin[2], 0.0f),
                                      _mm_setr_ps(bone->origin[0],
                                                  bone->origin[1],
                                                  bone->origin[2], 0.0f),
                                      0x55),
                       matrix.y.v)),
        _mm_add_ps(
            _mm_mul_ps(_mm_shuffle_ps(_mm_setr_ps(bone->origin[0],
                                                  bone->origin[1],
                                                  bone->origin[2], 0.0f),
                                      _mm_setr_ps(bone->origin[0],
                                                  bone->origin[1],
                                                  bone->origin[2], 0.0f),
                                      0xAA),
                       matrix.z.v),
            matrix.w.v));
    return world;
}

int R_AddVehicleSurfaces(DObj* obj, Entity* entity, const math::Mat43& matrix,
                         float alpha, bool render_shadow)
{
    if (entity == nullptr || ((EntityLocal*)entity)->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JSV;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_xmodel.cpp";
        AeAssert::gCurrentLine = 1249;
        AeAssert::gCurrentExpr = "entity && entity->scr_vehicle";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("this function is for dobjs with valid entity"))
            __debugbreak();
    }
    nglShaderParamSet* shadowParams =
        (nglShaderParamSet*)nglListAlloc(4 * nglShaderParamSet::NumParams + 8, 8);
    *(unsigned int*)shadowParams = 0;
    *((unsigned int*)shadowParams + 1) = 0;
    calc_lighting(entity, matrix, alpha, *shadowParams);

    DObjLocal* dobj = (DObjLocal*)obj;
    nglMeshParams meshParams = {};
    meshParams.Flags = 0x80;
    int modelIndex = 0;
    if (dobj->numModels != 0)
    {
        while (1)
        {
            DObjSkelMatLocal* matrixArray =
                (DObjSkelMatLocal*)DObjGetMatrixArray(obj, modelIndex);
            int pakId = dobj->models[modelIndex].mPakId;
            XModel* xmodel = (XModel*)dobj->models[modelIndex].mValue;
            int mLOD;
            if (modelIndex != 0)
            {
                mLOD = -1;
            }
            else if (dobj->mLODOverride < 0)
            {
                if (dobj->mLODAnim < 0)
                    mLOD = dobj->mLOD;
                else
                    mLOD = dobj->mLODAnim;
            }
            else
            {
                mLOD = dobj->mLODOverride;
            }
            ValidatePakId(pakId);
            int lodIndex = mLOD;
            if (mLOD < 0)
            {
                lodIndex = 0;
                if (((XModelLocal*)xmodel)->lod[0] == nullptr)
                {
                    do
                    {
                        ++lodIndex;
                    } while (((XModelLocal*)xmodel)->lod[lodIndex] == nullptr);
                }
            }
            XModelPartsLocal* parts =
                (XModelPartsLocal*)((XModelLocal*)xmodel)->lod[lodIndex]
                    ->xmodelParts;
            int mi = 0;
            for (;;)
            {
                ValidatePakId(pakId);
                int v16 = mLOD;
                if (mLOD < 0)
                {
                    v16 = 0;
                    while (((XModelLocal*)xmodel)->lod[v16] == nullptr)
                        ++v16;
                }
                XModelPartsLocal* parts2 =
                    (XModelPartsLocal*)((XModelLocal*)xmodel)->lod[v16]
                        ->xmodelParts;
                unsigned int meshCount =
                    (parts2 != nullptr) ? parts2->mHierarchy.mSize : 0;
                if (mi >= (int)meshCount)
                    break;
                if (mi >= (int)parts->mMeshPtrs.mSize)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "../ae\\inplace/InplaceVector.h";
                    AeAssert::gCurrentLine = 81;
                    AeAssert::gCurrentExpr = "index < mSize";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Bounds check"))
                        __debugbreak();
                }
                nglMesh* mesh = parts->mMeshPtrs.mList[mi];
                if (mesh != nullptr)
                {
                    math::Mat43 worldMatrix =
                        VehicleWorldMatrix(&matrixArray[mi], matrix);
                    meshParams.LOD = (unsigned int)GetScaledMeshLOD(
                        mesh, &worldMatrix, &meshParams, gZoomRatio);
                    if (mi >= (int)parts->mHierarchy.mSize)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::JRS;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\XModelParts.h";
                        AeAssert::gCurrentLine = 216;
                        AeAssert::gCurrentExpr =
                            "i >= 0 && i < mHierarchy.size()";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Bad Bone Index"))
                            __debugbreak();
                    }
                    if (mi >= (int)parts->mHierarchy.mSize)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "../ae\\inplace/InplaceVector.h";
                        AeAssert::gCurrentLine = 91;
                        AeAssert::gCurrentExpr = "index < mSize";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Bounds check"))
                            __debugbreak();
                    }
                    unsigned int boneNameHash =
                        parts->mHierarchy.mList[mi].mNameHash;
                    nglShaderParamSet* drawParams =
                        (nglShaderParamSet*)nglListAlloc(
                            4 * nglShaderParamSet::NumParams + 8, 8);
                    memcpy(drawParams, shadowParams,
                           4 * nglShaderParamSet::NumParams + 8);
                    AddTextureMatrix(entity, boneNameHash, *drawParams);
                    nglMeshNode* node = _codListAddMesh(
                        mesh, worldMatrix, &meshParams, drawParams, nullptr);
                    dobj->mFlags &= ~(unsigned int)g_DOBJF_NOT_RENDERED_LAST_FRAME;
                    if (render_shadow)
                    {
                        unsigned int rid = isRotatingTextureParamID;
                        unsigned long long bit = 1ULL << rid;
                        *(unsigned int*)shadowParams |= (unsigned int)bit;
                        *((unsigned int*)shadowParams + 1) |=
                            (unsigned int)(bit >> 32);
                        ((unsigned int*)shadowParams)[2 + rid] = 0;
                        do_shadow(obj, modelIndex, mi, mesh, matrix,
                                  worldMatrix, meshParams, *shadowParams,
                                  node);
                    }
                }
                ++mi;
            }
            ++modelIndex;
            if (modelIndex >= dobj->numModels)
                break;
        }
    }
    return (dobj->mFlags & (unsigned int)g_DOBJF_NOT_RENDERED_LAST_FRAME) == 0;
}

// ============================================================================
// R_AddNonVehicleSurfaces - ea: 0x006D02D0
// ============================================================================
enum { TEAM_DEAD = 4, EF_NON_EXISTING = 0x40000000 };

int R_AddNonVehicleSurfaces(DObj* obj, Entity* entity,
                            const math::Mat43& matrix, float alpha,
                            bool render_shadow, bool maxLod)
{
    if (entity == nullptr || entity->scr_vehicle != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JSV;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_xmodel.cpp";
        AeAssert::gCurrentLine = 1053;
        AeAssert::gCurrentExpr = "entity && !entity->scr_vehicle";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("this function is for non-vehicle entities"))
            __debugbreak();
    }
    nglShaderParamSet* shaderParams =
        (nglShaderParamSet*)nglListAlloc(4 * nglShaderParamSet::NumParams + 8, 8);
    *(unsigned int*)shaderParams = 0;
    *((unsigned int*)shaderParams + 1) = 0;
    bool didCalcLighting = false;

    bool poseLodSet = false;
    if (entity->sentient != nullptr && entity->sentient->eTeam == TEAM_DEAD
        && (entity->flags & EF_NON_EXISTING) == 0)
    {
        DObj::PoseLocal* pose = obj->mPose[0];
        if (pose != nullptr)
        {
            int lodOverride = obj->mLODOverride;
            if (lodOverride < 0)
                lodOverride = obj->mLOD;
            if (lodOverride < pose->LOD)
            {
                obj->SetLODAnim(pose->LOD);
                poseLodSet = true;
            }
        }
    }
    if (!poseLodSet)
        obj->mLODAnim = -1;

    int v75 = ((entity->mFlags & 2) != 0) << 6;
    if (maxLod)
    {
        obj->SetLOD(0);
    }
    else
    {
        computeLOD(obj, *((const math::Position3*)&matrix.w.v));
    }

    nglMeshParams meshParams = {};
    int modelIndex = 0;
    if (obj->numModels != 0)
    {
        while (1)
        {
            DObjSkelMatLocal* matrixArray =
                (DObjSkelMatLocal*)DObjGetMatrixArray(obj, modelIndex);
            int pakId = obj->models[modelIndex].mPakId;
            XModel* xmodel = (XModel*)obj->models[modelIndex].mValue;
            int lod = obj->mLODOverride;
            if (lod >= 0 || (lod = obj->mLODAnim) >= 0
                || (lod = obj->mLOD) >= 0)
            {
                XModelLod** p = &xmodel->lod[lod];
                do
                {
                    ValidatePakId(pakId);
                    if (lod < 5 && *p != nullptr)
                        break;
                    --lod;
                    --p;
                } while (lod >= 0);
            }
            if (!didCalcLighting)
            {
                didCalcLighting = true;
                calc_lighting(entity, matrix, alpha, *shaderParams);
            }
            ValidatePakId(pakId);
            int v21 = lod;
            if (lod < 0)
            {
                v21 = 0;
                while (xmodel->lod[v21] == nullptr)
                    ++v21;
            }
            XModelParts* parts =
                (XModelParts*)((XModelLodLocal*)xmodel->lod[v21])->xmodelParts;
            ValidatePakId(pakId);
            int v24 = lod;
            if (lod < 0)
            {
                v24 = 0;
                while (xmodel->lod[v24] == nullptr)
                    ++v24;
            }
            int meshCount;
            if (((XModelLodLocal*)xmodel->lod[v24])->xmodelParts != nullptr)
                meshCount = (int)xmodel->GetXModelParts(lod)->mHierarchy.mSize;
            else
                meshCount = 0;

            int mi = 0;
            if (meshCount > 0)
            {
                do
                {
                    int count = mi;
                    if (mi >= (int)parts->mMeshPtrs.mSize)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "../ae\\inplace/InplaceVector.h";
                        AeAssert::gCurrentLine = 81;
                        AeAssert::gCurrentExpr = "index < mSize";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Bounds check"))
                            __debugbreak();
                        count = 0;
                    }
                    nglMesh* mesh = parts->mMeshPtrs.mList[count];
                    if (mesh != nullptr)
                    {
                        math::Mat43 localToWorld;
                        localToWorld.x.v = matrix.x.v;
                        localToWorld.y.v = matrix.y.v;
                        localToWorld.z.v = matrix.z.v;
                        localToWorld.w.v = matrix.w.v;
                        meshParams.Flags = (unsigned int)v75;
                        int nBones = auxGetNBones(mesh);
                        meshParams.NBones = nBones;
                        trRefEntityLocal* renderEntity =
                            (trRefEntityLocal*)entity->mRenderEntity;
                        if (nBones != 0)
                        {
                            ValidatePakId(pakId);
                            int v42 = 0;
                            if (xmodel->lod[0] == nullptr)
                            {
                                do
                                {
                                    ++v42;
                                } while (xmodel->lod[v42] == nullptr);
                            }
                            int numBones2 = 0;
                            if (((XModelLodLocal*)xmodel->lod[v42])
                                    ->xmodelParts
                                != nullptr)
                            {
                                int v44 = 0;
                                if (xmodel->lod[0] == nullptr)
                                {
                                    do
                                    {
                                        ++v44;
                                    } while (xmodel->lod[v44] == nullptr);
                                }
                                numBones2 = (int)(
                                    (XModelLodLocal*)xmodel->lod[v44])
                                               ->xmodelParts->mHierarchy.mSize;
                            }
                            if (nBones != numBones2)
                            {
                                AeAssert::gCurrentAuthor = AeAssert::JRS;
                                AeAssert::gCurrentFile =
                                    "c:\\cod\\code\\game\\tr_xmodel.cpp";
                                AeAssert::gCurrentLine = 1150;
                                AeAssert::gCurrentExpr =
                                    "NBones == xmod->GetNumBones()";
                                if (!AeAssert::IsIgnored()
                                    && AeAssert::Assert(
                                        "Number of bones mismatch in skinned character."))
                                    __debugbreak();
                            }
                            meshParams.Flags |= 8u;
                            unsigned char iflIndex = renderEntity->iflIndex;
                            if (iflIndex != 255)
                            {
                                unsigned int rid = nglTextureFrameParamID;
                                unsigned long long bit = 1ULL << rid;
                                *(unsigned int*)shaderParams |=
                                    (unsigned int)bit;
                                *((unsigned int*)shaderParams + 1) |=
                                    (unsigned int)(bit >> 32);
                                ((unsigned int*)shaderParams)[2 + rid] =
                                    iflIndex;
                            }
                        }
                        else
                        {
                            localToWorld =
                                VehicleWorldMatrix(&matrixArray[mi], matrix);
                            float mScale = renderEntity->mScale;
                            if (mScale == 1.0f)
                                mScale = entity->modelscale;
                            if (mScale != 1.0f)
                                auxSetScale(&meshParams, mScale, mScale,
                                            mScale);
                            meshParams.Flags |= 0x80u;
                            meshParams.LOD = (unsigned int)GetScaledMeshLOD(
                                mesh, &localToWorld, &meshParams, gZoomRatio);
                        }
                        unsigned int rid = isRotatingTextureParamID;
                        unsigned long long bit = 1ULL << rid;
                        *(unsigned int*)shaderParams |= (unsigned int)bit;
                        *((unsigned int*)shaderParams + 1) |=
                            (unsigned int)(bit >> 32);
                        ((unsigned int*)shaderParams)[2 + rid] = 0;
                        nglMeshNode* node = _codListAddMesh(
                            mesh, localToWorld, &meshParams, shaderParams,
                            nullptr);
                        obj->mFlags &=
                            ~(unsigned int)g_DOBJF_NOT_RENDERED_LAST_FRAME;
                        if (render_shadow)
                            do_shadow(obj, modelIndex, mi, mesh, matrix,
                                      localToWorld, meshParams,
                                      *shaderParams, node);
                    }
                    ++mi;
                } while (mi < meshCount);
            }
            ++modelIndex;
            if (modelIndex >= obj->numModels)
                break;
        }
    }
    return (obj->mFlags & (unsigned int)g_DOBJF_NOT_RENDERED_LAST_FRAME) == 0;
}

// ============================================================================
// R_AddXModelSurfaces_DistanceHack - ea: 0x006CF200
// ============================================================================
int R_AddXModelSurfaces_DistanceHack(DObj* obj, Entity* entity,
                                     const math::Mat43& matrix)
{
    nglShaderParamSet* shaderParams =
        (nglShaderParamSet*)nglListAlloc(4 * nglShaderParamSet::NumParams + 8, 8);
    *(unsigned int*)shaderParams = 0;
    *((unsigned int*)shaderParams + 1) = 0;
    nglLightContext* lightContext = nglCreateLightContext();
    ModelLightingHack();
    unsigned int ctxRid = nglLightContextParamID;
    unsigned long long ctxBit = 1ULL << ctxRid;
    *(unsigned int*)shaderParams |= (unsigned int)ctxBit;
    *((unsigned int*)shaderParams + 1) |= (unsigned int)(ctxBit >> 32);
    ((unsigned int*)shaderParams)[2 + ctxRid] =
        (unsigned int)lightContext;

    int v87 = ((entity->mFlags & 2) != 0) << 6;
    nglMeshParams meshParams = {};
    int modelIndex = 0;
    if (obj->numModels != 0)
    {
        while (1)
        {
            DObjSkelMatLocal* matrixArray =
                (DObjSkelMatLocal*)DObjGetMatrixArray(obj, modelIndex);
            int pakId = obj->models[modelIndex].mPakId;
            XModel* xmodel = (XModel*)obj->models[modelIndex].mValue;
            int mLOD;
            if (modelIndex != 0)
            {
                mLOD = -1;
            }
            else if (obj->mLODOverride < 0)
            {
                if (obj->mLODAnim < 0)
                    mLOD = obj->mLOD;
                else
                    mLOD = obj->mLODAnim;
            }
            else
            {
                mLOD = obj->mLODOverride;
            }
            ValidatePakId(pakId);
            int v14 = mLOD;
            if (mLOD < 0)
            {
                v14 = 0;
                while (xmodel->lod[v14] == nullptr)
                    ++v14;
            }
            XModelParts* parts =
                (XModelParts*)((XModelLodLocal*)xmodel->lod[v14])->xmodelParts;

            for (int t = 0;; ++t)
            {
                ValidatePakId(pakId);
                int v18 = mLOD;
                if (mLOD < 0)
                {
                    v18 = 0;
                    while (xmodel->lod[v18] == nullptr)
                        ++v18;
                }
                unsigned int meshCount;
                if (((XModelLodLocal*)xmodel->lod[v18])->xmodelParts
                    != nullptr)
                {
                    int v20 = mLOD;
                    if (mLOD < 0)
                    {
                        v20 = 0;
                        while (xmodel->lod[v20] == nullptr)
                            ++v20;
                    }
                    meshCount =
                        ((XModelLodLocal*)xmodel->lod[v20])
                            ->xmodelParts->mHierarchy.mSize;
                }
                else
                {
                    meshCount = 0;
                }
                if (t >= (int)meshCount)
                    break;
                int count = t;
                if (t >= (int)parts->mMeshPtrs.mSize)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "../ae\\inplace/InplaceVector.h";
                    AeAssert::gCurrentLine = 81;
                    AeAssert::gCurrentExpr = "index < mSize";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Bounds check"))
                        __debugbreak();
                    count = 0;
                }
                nglMesh* mesh = parts->mMeshPtrs.mList[count];
                if (mesh != nullptr)
                {
                    math::Mat43 localToWorld =
                        VehicleWorldMatrix(&matrixArray[t], matrix);
                    meshParams.Flags = (unsigned int)v87;
                    math::Position3 sphereCenter;
                    auxGetSphereCenter(&sphereCenter, mesh);
                    __m128 sphereWorld = _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(sphereCenter.v,
                                                      sphereCenter.v, 0),
                                       localToWorld.x.v),
                            _mm_mul_ps(_mm_shuffle_ps(sphereCenter.v,
                                                      sphereCenter.v, 0x55),
                                       localToWorld.y.v)),
                        _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(sphereCenter.v,
                                                      sphereCenter.v, 0xAA),
                                       localToWorld.z.v),
                            localToWorld.w.v));
                    const math::Mat44& w2v =
                        ((nglSceneLocal*)nglBuildScene)->WorldToView;
                    __m128 viewPos = _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(sphereWorld, sphereWorld,
                                                      0),
                                       w2v.x.v),
                            _mm_mul_ps(_mm_shuffle_ps(sphereWorld, sphereWorld,
                                                      0x55),
                                       w2v.y.v)),
                        _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(sphereWorld, sphereWorld,
                                                      0xAA),
                                       w2v.z.v),
                            w2v.w.v));
                    float viewZ =
                        _mm_shuffle_ps(viewPos, viewPos, 0xAA).m128_f32[0];
                    float radius = auxGetSphereRadius(mesh);
                    if (radius + viewZ > tr.viewParms.zFar)
                    {
                        float newZ = tr.viewParms.zFar - radius;
                        float scale = newZ / viewZ;
                        __m128 axisX = _mm_setr_ps(
                            w2v.x.v.m128_f32[0], w2v.y.v.m128_f32[0],
                            w2v.z.v.m128_f32[0], 0.0f);
                        __m128 axisY = _mm_setr_ps(
                            w2v.x.v.m128_f32[1], w2v.y.v.m128_f32[1],
                            w2v.z.v.m128_f32[1], 0.0f);
                        __m128 axisZ = _mm_setr_ps(
                            w2v.x.v.m128_f32[2], w2v.y.v.m128_f32[2],
                            w2v.z.v.m128_f32[2], 0.0f);
                        __m128 camPos = _mm_xor_ps(
                            _mm_castsi128_ps(_mm_set1_epi32(0x80000000)),
                            _mm_add_ps(
                                _mm_add_ps(
                                    _mm_mul_ps(
                                        _mm_set1_ps(w2v.w.v.m128_f32[0]),
                                        axisX),
                                    _mm_mul_ps(
                                        _mm_set1_ps(w2v.w.v.m128_f32[1]),
                                        axisY)),
                                _mm_mul_ps(
                                    _mm_set1_ps(w2v.w.v.m128_f32[2]),
                                    axisZ)));
                        __m128 scaledRay =
                            _mm_mul_ps(viewPos, _mm_set1_ps(scale));
                        __m128 newW = _mm_sub_ps(
                            _mm_add_ps(
                                _mm_add_ps(
                                    _mm_add_ps(
                                        _mm_mul_ps(_mm_shuffle_ps(scaledRay,
                                                                  scaledRay, 0),
                                                   axisX),
                                        _mm_mul_ps(_mm_shuffle_ps(scaledRay,
                                                                  scaledRay,
                                                                  0x55),
                                                   axisY)),
                                    _mm_mul_ps(_mm_shuffle_ps(scaledRay,
                                                              scaledRay, 0xAA),
                                               axisZ)),
                                camPos),
                            _mm_mul_ps(sphereWorld, _mm_set1_ps(scale)));
                        localToWorld.w.v = newW;
                        auxSetScale(&meshParams, scale, scale, scale);
                    }
                    meshParams.Flags |= 0x80u;
                    meshParams.LOD = (unsigned int)GetScaledMeshLOD(
                        mesh, &localToWorld, &meshParams, gZoomRatio);
                    _codListAddMesh(mesh, localToWorld, &meshParams,
                                    shaderParams, nullptr);
                    obj->mFlags &=
                        ~(unsigned int)g_DOBJF_NOT_RENDERED_LAST_FRAME;
                }
            }
            ++modelIndex;
            if (modelIndex >= obj->numModels)
                break;
        }
    }
    return (obj->mFlags & (unsigned int)g_DOBJF_NOT_RENDERED_LAST_FRAME) == 0;
}

// ============================================================================
// R_AddViewModelSurfaces - ea: 0x006D1250
// ============================================================================
int R_AddViewModelSurfaces(int client_index, DObj* obj,
                           const math::Mat43& matrix)
{
    nglShaderParamSet* shaderParams =
        (nglShaderParamSet*)nglListAlloc(4 * nglShaderParamSet::NumParams + 8, 8);
    *(unsigned int*)shaderParams = 0;
    *((unsigned int*)shaderParams + 1) = 0;
    calc_lighting(nullptr, matrix, 0.0f, *shaderParams);

    nglMeshParams meshParams = {};
    int modelIndex = 0;
    if (obj->numModels != 0)
    {
        while (1)
        {
            DObjSkelMatLocal* matrixArray =
                (DObjSkelMatLocal*)DObjGetMatrixArray(obj, modelIndex);
            int pakId = obj->models[modelIndex].mPakId;
            XModel* xmodel = (XModel*)obj->models[modelIndex].mValue;
            int mLOD;
            if (modelIndex != 0)
            {
                mLOD = -1;
            }
            else if (obj->mLODOverride < 0)
            {
                if (obj->mLODAnim < 0)
                    mLOD = obj->mLOD;
                else
                    mLOD = obj->mLODAnim;
            }
            else
            {
                mLOD = obj->mLODOverride;
            }
            ValidatePakId(pakId);
            int v12 = mLOD;
            if (mLOD < 0)
            {
                v12 = 0;
                while (xmodel->lod[v12] == nullptr)
                    ++v12;
            }
            XModelParts* parts =
                (XModelParts*)((XModelLodLocal*)xmodel->lod[v12])->xmodelParts;

            int mi = 0;
            for (;;)
            {
                ValidatePakId(pakId);
                int v16 = mLOD;
                if (mLOD < 0)
                {
                    v16 = 0;
                    while (xmodel->lod[v16] == nullptr)
                        ++v16;
                }
                unsigned int meshCount;
                if (((XModelLodLocal*)xmodel->lod[v16])->xmodelParts
                    != nullptr)
                {
                    int v18 = mLOD;
                    if (mLOD < 0)
                    {
                        v18 = 0;
                        while (xmodel->lod[v18] == nullptr)
                            ++v18;
                    }
                    meshCount =
                        ((XModelLodLocal*)xmodel->lod[v18])
                            ->xmodelParts->mHierarchy.mSize;
                }
                else
                {
                    meshCount = 0;
                }
                int v22 = mi;
                if (mi >= (int)meshCount)
                    break;
                if (mi >= (int)parts->mMeshPtrs.mSize)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile =
                        "../ae\\inplace/InplaceVector.h";
                    AeAssert::gCurrentLine = 81;
                    AeAssert::gCurrentExpr = "index < mSize";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Bounds check"))
                        __debugbreak();
                    v22 = 0;
                }
                nglMesh* mesh = parts->mMeshPtrs.mList[v22];
                *((void**)&meshParams.Scale[3]) = mesh;
                if (mesh != nullptr)
                {
                    math::Mat43 worldTransform = matrix;
                    meshParams.Flags = 0;
                    int nBones = auxGetNBones(mesh);
                    meshParams.NBones = nBones;
                    if (nBones != 0)
                    {
                        ValidatePakId(pakId);
                        int v30 = 0;
                        if (xmodel->lod[0] == nullptr)
                        {
                            do
                            {
                                ++v30;
                            } while (xmodel->lod[v30] == nullptr);
                        }
                        int v34 = 0;
                        if (((XModelLodLocal*)xmodel->lod[v30])->xmodelParts
                            != nullptr)
                        {
                            int v32 = 0;
                            if (xmodel->lod[0] == nullptr)
                            {
                                do
                                {
                                    ++v32;
                                } while (xmodel->lod[v32] == nullptr);
                            }
                            v34 = (int)(
                                (XModelLodLocal*)xmodel->lod[v32])
                                      ->xmodelParts->mHierarchy.mSize;
                        }
                        if (nBones != v34)
                        {
                            AeAssert::gCurrentAuthor = AeAssert::JRS;
                            AeAssert::gCurrentFile =
                                "c:\\cod\\code\\game\\tr_xmodel.cpp";
                            AeAssert::gCurrentLine = 1360;
                            AeAssert::gCurrentExpr =
                                "NBones == xmod->GetNumBones()";
                            if (!AeAssert::IsIgnored()
                                && AeAssert::Assert(
                                    "Number of bones mismatch in skinned character."))
                                __debugbreak();
                        }
                        meshParams.Bones = matrixArray;
                        meshParams.Flags |= 8u;
                        render_view_model_arms(client_index, meshParams,
                                               matrix, worldTransform);
                    }
                    else
                    {
                        worldTransform =
                            VehicleWorldMatrix(&matrixArray[mi], matrix);
                        render_view_model_weapon(client_index, meshParams,
                                                 worldTransform);
                    }
                    _codListAddMesh(mesh, worldTransform, &meshParams,
                                    shaderParams, nullptr);
                    obj->mFlags &=
                        ~(unsigned int)g_DOBJF_NOT_RENDERED_LAST_FRAME;
                }
                ++mi;
            }
            ++modelIndex;
            if (modelIndex >= obj->numModels)
                break;
        }
    }
    return (obj->mFlags & (unsigned int)g_DOBJF_NOT_RENDERED_LAST_FRAME) == 0;
}

// ============================================================================
// R_AddMenuModelSurfaces - ea: 0x006CF9E0
// ============================================================================
int R_AddMenuModelSurfaces(DObj* obj, Entity* entity, const math::Mat43& matrix,
                           float alpha, nglShaderParamSet& shaderParams,
                           nglLightContext* ctx, bool render_shadow,
                           bool maxLod)
{
    (void)alpha;
    (void)ctx;
    if (entity == nullptr || entity->scr_vehicle != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JSV;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_xmodel.cpp";
        AeAssert::gCurrentLine = 904;
        AeAssert::gCurrentExpr = "entity && !entity->scr_vehicle";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("this function is for non-vehicle entities"))
            __debugbreak();
    }
    bool poseLodSet = (entity->flags & EF_NON_EXISTING) == 0
                      && obj->mPose[0] != nullptr;
    int lodOverride = obj->mLODOverride;
    if (lodOverride < 0)
        lodOverride = obj->mLOD;
    if (poseLodSet && lodOverride < obj->mPose[0]->LOD)
        obj->SetLODAnim(obj->mPose[0]->LOD);
    else
        obj->mLODAnim = -1;

    int v89 = ((entity->mFlags & 2) != 0) << 6;
    if (maxLod)
    {
        obj->SetLOD(0);
    }
    else
    {
        computeLOD(obj, *((const math::Position3*)&matrix.w.v));
    }

    nglMeshParams meshParams = {};
    int modelIndex = 0;
    if (obj->numModels != 0)
    {
        while (1)
        {
            DObjSkelMatLocal* matrixArray =
                (DObjSkelMatLocal*)DObjGetMatrixArray(obj, modelIndex);
            int pakId = obj->models[modelIndex].mPakId;
            XModel* xmodel = (XModel*)obj->models[modelIndex].mValue;
            int lod = obj->mLODOverride;
            if (lod >= 0 || (lod = obj->mLODAnim) >= 0
                || (lod = obj->mLOD) >= 0)
            {
                XModelLod** p = &xmodel->lod[lod];
                do
                {
                    ValidatePakId(pakId);
                    if (lod < 5 && *p != nullptr)
                        break;
                    --lod;
                    --p;
                } while (lod >= 0);
            }
            ValidatePakId(pakId);
            int v22 = lod;
            if (lod < 0)
            {
                v22 = 0;
                while (xmodel->lod[v22] == nullptr)
                    ++v22;
            }
            XModelParts* parts =
                (XModelParts*)((XModelLodLocal*)xmodel->lod[v22])->xmodelParts;
            ValidatePakId(pakId);
            int v25 = lod;
            if (lod < 0)
            {
                v25 = 0;
                while (xmodel->lod[v25] == nullptr)
                    ++v25;
            }
            int meshCount;
            if (((XModelLodLocal*)xmodel->lod[v25])->xmodelParts != nullptr)
                meshCount = (int)xmodel->GetXModelParts(lod)->mHierarchy.mSize;
            else
                meshCount = 0;

            int t = 0;
            if (meshCount > 0)
            {
                do
                {
                    int count = t;
                    if (t >= (int)parts->mMeshPtrs.mSize)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "../ae\\inplace/InplaceVector.h";
                        AeAssert::gCurrentLine = 81;
                        AeAssert::gCurrentExpr = "index < mSize";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Bounds check"))
                            __debugbreak();
                        count = 0;
                    }
                    nglMesh* mesh = parts->mMeshPtrs.mList[count];
                    if (mesh != nullptr)
                    {
                        math::Mat43 localToWorld;
                        localToWorld.x.v = matrix.x.v;
                        localToWorld.y.v = matrix.y.v;
                        localToWorld.z.v = matrix.z.v;
                        localToWorld.w.v = matrix.w.v;
                        meshParams.Flags = (unsigned int)v89;
                        int nBones = auxGetNBones(mesh);
                        meshParams.NBones = nBones;
                        trRefEntityLocal* renderEntity =
                            (trRefEntityLocal*)entity->mRenderEntity;
                        if (nBones != 0)
                        {
                            ValidatePakId(pakId);
                            int v44 = 0;
                            if (xmodel->lod[0] == nullptr)
                            {
                                do
                                {
                                    ++v44;
                                } while (xmodel->lod[v44] == nullptr);
                            }
                            int numBones2 = 0;
                            if (((XModelLodLocal*)xmodel->lod[v44])
                                    ->xmodelParts
                                != nullptr)
                            {
                                int v46 = 0;
                                if (xmodel->lod[0] == nullptr)
                                {
                                    do
                                    {
                                        ++v46;
                                    } while (xmodel->lod[v46] == nullptr);
                                }
                                numBones2 = (int)(
                                    (XModelLodLocal*)xmodel->lod[v46])
                                               ->xmodelParts->mHierarchy.mSize;
                            }
                            if (nBones != numBones2)
                            {
                                AeAssert::gCurrentAuthor = AeAssert::JRS;
                                AeAssert::gCurrentFile =
                                    "c:\\cod\\code\\game\\tr_xmodel.cpp";
                                AeAssert::gCurrentLine = 965;
                                AeAssert::gCurrentExpr =
                                    "NBones == xmod->GetNumBones()";
                                if (!AeAssert::IsIgnored()
                                    && AeAssert::Assert(
                                        "Number of bones mismatch in skinned character."))
                                    __debugbreak();
                            }
                            meshParams.Flags |= 8u;
                            unsigned char iflIndex = renderEntity->iflIndex;
                            if (iflIndex != 255)
                            {
                                unsigned int rid = nglTextureFrameParamID;
                                unsigned long long bit = 1ULL << rid;
                                unsigned int* arr =
                                    (unsigned int*)&shaderParams;
                                arr[0] |= (unsigned int)bit;
                                arr[1] |= (unsigned int)(bit >> 32);
                                arr[2 + rid] = iflIndex;
                            }
                        }
                        else
                        {
                            localToWorld =
                                VehicleWorldMatrix(&matrixArray[t], matrix);
                            float mScale = renderEntity->mScale;
                            if (mScale == 1.0f)
                                mScale = entity->modelscale;
                            if (mScale != 1.0f)
                                auxSetScale(&meshParams, mScale, mScale,
                                            mScale);
                            meshParams.Flags |= 0x80u;
                            meshParams.LOD = (unsigned int)GetScaledMeshLOD(
                                mesh, &localToWorld, &meshParams, gZoomRatio);
                        }
                        nglMeshNode* node = _codListAddMesh(
                            mesh, localToWorld, &meshParams, &shaderParams,
                            nullptr);
                        obj->mFlags &=
                            ~(unsigned int)g_DOBJF_NOT_RENDERED_LAST_FRAME;
                        if (render_shadow)
                            do_shadow(obj, modelIndex, t, mesh, matrix,
                                      localToWorld, meshParams, shaderParams,
                                      node);
                    }
                    ++t;
                } while (t < meshCount);
            }
            ++modelIndex;
            if (modelIndex >= obj->numModels)
                break;
        }
    }
    return (obj->mFlags & (unsigned int)g_DOBJF_NOT_RENDERED_LAST_FRAME) == 0;
}
