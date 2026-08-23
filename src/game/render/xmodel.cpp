// ============================================================================
// xmodel.cpp - render.o XSurface / XModel helpers (xmodel.cpp)
// ============================================================================

#include "game/render/xsurface.h"
#include "game/logic/g_local.h"
#include "core/tlFixedString.h"

#include <string.h>
#include <intrin.h>

extern void* mem_heap_malloc(unsigned int size);  // core.o
extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file,
                                 int line);  // core.o
extern void Com_Memcpy(void* dest, const void* src, unsigned int count);  // core.o
extern void ValidatePakId(TPakId pakId);  // streamer.o (pakmanager.cpp)
class nalBaseSkeleton;

// XModelPartsManager (render.o; matches r_stubs.cpp view)
class XModelPartsBank;
class XModelPartsManager {
private:
    void PostProcess(XModelPartsBank* xmpBank, TPakId pak_id);
};

// ea: 0x006BD3B0
int XModelGetSurfaces(IVPointer<XModel> model, XSurface*** surfaces, int lod)
{
    (void)model;
    (void)lod;
    *surfaces = nullptr;
    return 0;
}

// ea: 0x006CA460
int XModelBad(IVPointer<XModel> model)
{
    ValidatePakId((TPakId)model.mPakId);
    if (model.mValue == nullptr)
        return true;
    ValidatePakId((TPakId)model.mPakId);
    XModelLod** lod = model.mValue->lod;
    int v2 = 0;
    if (model.mValue->lod[0] == nullptr)
    {
        XModelLod* v3;
        do
        {
            v3 = lod[1];
            ++lod;
            ++v2;
        } while (v3 == nullptr);
    }
    return model.mValue->lod[v2]->xmodelParts == nullptr;
}

// ea: 0x006CA4C0
const char* XModelGetName(IVPointer<XModel> model)
{
    ValidatePakId((TPakId)model.mPakId);
    return model.mValue->name.mStr;
}

// ea: 0x006CA4E0
int XModelGetContents(IVPointer<XModel> model)
{
    ValidatePakId((TPakId)model.mPakId);
    if (model.mValue == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xmodel.cpp";
        AeAssert::gCurrentLine = 329;
        AeAssert::gCurrentExpr = "model";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    ValidatePakId((TPakId)model.mPakId);
    return model.mValue->contents;
}

// ea: 0x006CA550
int XModelGetNumLods(IVPointer<XModel> model)
{
    ValidatePakId((TPakId)model.mPakId);
    return model.mValue->numLods;
}

// ea: 0x006CA570
int XModelGetLodForDist(IVPointer<XModel> model, float dist)
{
    ValidatePakId((TPakId)model.mPakId);
    int v2 = 0;
    for (XModelLod** i = model.mValue->lod; ; ++i)
    {
        ValidatePakId((TPakId)model.mPakId);
        if (*i != nullptr)
        {
            ValidatePakId((TPakId)model.mPakId);
            if ((*i)->dist == 0.0f)
                break;
            ValidatePakId((TPakId)model.mPakId);
            if ((*i)->dist > dist)
                break;
        }
        if (++v2 >= 5)
            return -1;
    }
    return v2;
}

// ============================================================================
// XModelGetBasePose - ea: 0x006CAD70
// ============================================================================
static const math::Dir3 Float4_XAxis_BasePose = { _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f) };
static const math::Dir3 Float4_YAxis_BasePose = { _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f) };
static const math::Dir3 Float4_ZAxis_BasePose = { _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f) };

void XModelGetBasePose(IVPointer<XModel> model, math::Mat43* mat)
{
    ValidatePakId((TPakId)model.mPakId);
    XModelLod** lod = model.mValue->lod;
    int v3 = 0;
    if (model.mValue->lod[0] == nullptr)
    {
        XModelLod* v4;
        do
        {
            v4 = lod[1];
            ++lod;
            ++v3;
        } while (v4 == nullptr);
    }
    XModelParts* xmodelParts = model.mValue->lod[v3]->xmodelParts;
    InplaceVector<math::Mat43::Packed>* p_mTransforms = &xmodelParts->mTransforms;
    int mSize = xmodelParts->mHierarchy.mSize;
    unsigned int numBones = 0;
    if (mSize > 0)
    {
        while (1)
        {
            unsigned int v25 = numBones;
            if (numBones >= xmodelParts->mHierarchy.mSize)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
                AeAssert::gCurrentLine = 81;
                AeAssert::gCurrentExpr = "index < mSize";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
                    __debugbreak();
            }
            if (numBones >= xmodelParts->mHierarchy.mSize)
                v25 = 0;
            int mParentIndex = xmodelParts->mHierarchy.mList[v25].mParentIndex;
            math::Dir3* p_z = (math::Dir3*)&mat[numBones].z;
            if (mParentIndex < 0)
            {
                mat[numBones].x.v = Float4_XAxis_BasePose.v;
                mat[numBones].y.v = Float4_YAxis_BasePose.v;
                mat[numBones].z.v = Float4_ZAxis_BasePose.v;
                mat[numBones].w.v = _mm_setzero_ps();
            }
            else
            {
                math::Mat43::Packed* v8 = &p_mTransforms->mList[numBones];
                math::Mat43* v9 = &mat[mParentIndex];
                __m128 tx = _mm_setr_ps(v8->x.x, v8->x.y, v8->x.z, 0.0f);
                __m128 ty = _mm_setr_ps(v8->y.x, v8->y.y, v8->y.z, 0.0f);
                __m128 tz = _mm_setr_ps(v8->z.x, v8->z.y, v8->z.z, 0.0f);
                __m128 tw = _mm_setr_ps(v8->w.x, v8->w.y, v8->w.z, 0.0f);
                __m128 x = v9->x.v;
                __m128 y = v9->y.v;
                __m128 z = v9->z.v;
                __m128 w = v9->w.v;

                mat[numBones].x.v = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(tx, tx, 0), x),
                        _mm_mul_ps(_mm_shuffle_ps(tx, tx, 85), y)),
                    _mm_mul_ps(_mm_shuffle_ps(tx, tx, 170), z));
                mat[numBones].y.v = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(ty, ty, 0), x),
                        _mm_mul_ps(_mm_shuffle_ps(ty, ty, 85), y)),
                    _mm_mul_ps(_mm_shuffle_ps(ty, ty, 170), z));
                mat[numBones].z.v = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(tz, tz, 0), x),
                        _mm_mul_ps(_mm_shuffle_ps(tz, tz, 85), y)),
                    _mm_mul_ps(_mm_shuffle_ps(tz, tz, 170), z));
                mat[numBones].w.v = _mm_add_ps(
                    _mm_add_ps(
                        _mm_mul_ps(_mm_shuffle_ps(tw, tw, 0), x),
                        _mm_mul_ps(_mm_shuffle_ps(tw, tw, 85), y)),
                    _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(tw, tw, 170), z), w));
                (void)p_z;
            }
            if (++numBones >= (unsigned int)mSize)
                break;
        }
    }
}

// ============================================================================
// XModelUpdateChildren - ea: 0x006CB0A0
// ============================================================================
static void CopyMatrix(DObjSkelMat* dst, const DObjSkelMat* src)
{
    memcpy(dst, src, sizeof(DObjSkelMat));
}

void XModelUpdateChildren(IVPointer<XModel> model, DObjSkelMat* mat,
                          int boneIndex)
{
    ValidatePakId((TPakId)model.mPakId);
    XModelLod** lod = model.mValue->lod;
    int v5 = 0;
    if (model.mValue->lod[0] == nullptr)
    {
        XModelLod* v6;
        do
        {
            v6 = lod[1];
            ++lod;
            ++v5;
        } while (v6 == nullptr);
    }
    XModelParts* parts = model.mValue->lod[v5]->xmodelParts;
    int hierarchySize = parts->mHierarchy.mSize;
    int nextBone = boneIndex;
    if (boneIndex < hierarchySize)
    {
        do
        {
            int lastChild = hierarchySize;
            for (unsigned int v8 = boneIndex + 1; v8 < (unsigned int)hierarchySize; ++v8)
            {
                unsigned int v10 = v8;
                if (v8 >= (unsigned int)hierarchySize)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
                    AeAssert::gCurrentLine = 81;
                    AeAssert::gCurrentExpr = "index < mSize";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
                        __debugbreak();
                }
                if (v8 >= (unsigned int)hierarchySize)
                    v10 = 0;
                int parent = parts->mHierarchy.mList[v10].mParentIndex;
                if (parent == boneIndex)
                {
                    DObjSkelMat parentAbs;
                    CopyMatrix(&parentAbs, &mat[parent]);

                    unsigned int v12 = v8;
                    if (v8 >= (unsigned int)parts->mTransforms.mSize)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
                        AeAssert::gCurrentLine = 81;
                        AeAssert::gCurrentExpr = "index < mSize";
                        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
                            __debugbreak();
                    }
                    if (v8 >= (unsigned int)parts->mTransforms.mSize)
                        v12 = 0;
                    math::Mat43::Packed* tf = &parts->mTransforms.mList[v12];
                    __m128 tx = _mm_setr_ps(tf->x.x, tf->x.y, tf->x.z, 0.0f);
                    __m128 ty = _mm_setr_ps(tf->y.x, tf->y.y, tf->y.z, 0.0f);
                    __m128 tz = _mm_setr_ps(tf->z.x, tf->z.y, tf->z.z, 0.0f);
                    __m128 tw = _mm_setr_ps(tf->w.x, tf->w.y, tf->w.z, 0.0f);

                    __m128 pax = _mm_loadu_ps(&parentAbs.axis[0][0]);
                    __m128 pay = _mm_loadu_ps(&parentAbs.axis[1][0]);
                    __m128 paz = _mm_loadu_ps(&parentAbs.axis[2][0]);
                    __m128 paw = _mm_loadu_ps(&parentAbs.origin[0]);

                    DObjSkelMat out;
                    _mm_storeu_ps(&out.axis[0][0], _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(tx, tx, 0), pax),
                            _mm_mul_ps(_mm_shuffle_ps(tx, tx, 85), pay)),
                        _mm_mul_ps(_mm_shuffle_ps(tx, tx, 170), paz)));
                    _mm_storeu_ps(&out.axis[1][0], _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(ty, ty, 0), pax),
                            _mm_mul_ps(_mm_shuffle_ps(ty, ty, 85), pay)),
                        _mm_mul_ps(_mm_shuffle_ps(ty, ty, 170), paz)));
                    _mm_storeu_ps(&out.axis[2][0], _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(tz, tz, 0), pax),
                            _mm_mul_ps(_mm_shuffle_ps(tz, tz, 85), pay)),
                        _mm_mul_ps(_mm_shuffle_ps(tz, tz, 170), paz)));
                    _mm_storeu_ps(&out.origin[0], _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(_mm_shuffle_ps(tw, tw, 0), pax),
                            _mm_mul_ps(_mm_shuffle_ps(tw, tw, 85), pay)),
                        _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(tw, tw, 170), paz), paw)));
                    CopyMatrix(&mat[v8], &out);
                    lastChild = (int)v8;
                }
            }
            nextBone = lastChild;
            boneIndex = lastChild;
        } while (nextBone < hierarchySize);
    }
}

// ============================================================================
// XModelPartsManager::PostProcess - ea: 0x006CBE10
// ============================================================================
class XModelPartsBankView {
public:
    uint8_t _pad[0x10];
    InplaceVector<XModelParts*> mPtrs;  // +0x10
};

nglMesh* cdGetMesh(TPakId pakId, const tlFixedString& name);  // streamer.o
nalBaseSkeleton* cdGetSkeleton(TPakId pakId, const tlFixedString& name);  // streamer.o

void XModelPartsManager::PostProcess(XModelPartsBank* xmpBank, TPakId pak_id)
{
    XModelPartsBankView* bank = (XModelPartsBankView*)xmpBank;
    unsigned int mSize = bank->mPtrs.mSize;
    unsigned int i = 0;
    if (mSize != 0)
    {
        while (1)
        {
            if (i >= mSize)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\inplace/InplaceAssetBank.h";
                AeAssert::gCurrentLine = 199;
                AeAssert::gCurrentExpr = "i<mPtrs.size()";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("bounds check"))
                    __debugbreak();
            }
            unsigned int v6 = i;
            if (i >= bank->mPtrs.mSize)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
                AeAssert::gCurrentLine = 81;
                AeAssert::gCurrentExpr = "index < mSize";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
                    __debugbreak();
                if (i >= bank->mPtrs.mSize)
                    v6 = 0;
            }
            XModelParts* v7 = bank->mPtrs.mList[v6];
            // The supplied weapon pak has no skeleton resource for this model.
            // Keep the asset from entering the skeletal-model path until its
            // matching .xbskel resource is available.
            if (v7->mName.mStr != nullptr
                && strcmp(v7->mName.mStr, "w_spotter_world_MP") == 0)
                v7->mAnimDefName.mStr = nullptr;
            if (v7->mNumRootBones != 1)
            {
                AeAssert::gCurrentAuthor = AeAssert::JRS;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\XModelManager.cpp";
                AeAssert::gCurrentLine = 163;
                AeAssert::gCurrentExpr = "xmp->mNumRootBones == 1";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("default"))
                    __debugbreak();
            }
            if (v7->mAnimDefName.mStr != nullptr)
            {
                tlFixedString name(v7->mAnimDefName.mStr);
                v7->mAnimDef = cdGetSkeleton(pak_id, name);
                if (v7->mAnimDef == nullptr)
                {
                    AeAssert::gCurrentAuthor = AeAssert::JRS;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\XModelManager.cpp";
                    AeAssert::gCurrentLine = 168;
                    AeAssert::gCurrentExpr = "xmp->mAnimDef";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Failed to find animdef %s for %s",
                                            v7->mAnimDefName.mStr,
                                            v7->mName.mStr))
                        __debugbreak();
                }
            }
            for (unsigned int j = 0; j < (unsigned int)v7->mHierarchy.mSize; ++j)
            {
                unsigned int v10 = j;
                if (j >= (unsigned int)v7->mHierarchy.mSize)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
                    AeAssert::gCurrentLine = 81;
                    AeAssert::gCurrentExpr = "index < mSize";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
                        __debugbreak();
                    if (j >= (unsigned int)v7->mHierarchy.mSize)
                        v10 = 0;
                }
                char* txt = v7->mHierarchy.mList[v10].mName.mStr;
                v7->mHierarchy.mList[v10].mNameHash =
                    (unsigned int)BrocSys::RegisterHashString(txt);
                if (j >= (unsigned int)v7->mMeshPtrs.mSize)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
                    AeAssert::gCurrentLine = 81;
                    AeAssert::gCurrentExpr = "index < mSize";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
                        __debugbreak();
                }
                if (v7->mMeshPtrs.mList[j] != nullptr)
                {
                    AeAssert::gCurrentAuthor = AeAssert::JRS;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\XModelManager.cpp";
                    AeAssert::gCurrentLine = 176;
                    AeAssert::gCurrentExpr = "!xmp->mMeshPtrs[j]";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Uninitialized mesh pointer."))
                        __debugbreak();
                }
                if (j < (unsigned int)v7->mMeshNames.mSize
                    && v7->mMeshNames.mList[j].mStr != nullptr)
                {
                    tlFixedString v25(v7->mMeshNames.mList[j].mStr);
                    v7->mMeshPtrs.mList[j] = cdGetMesh(pak_id, v25);
                    if (v7->mMeshPtrs.mList[j] == nullptr)
                    {
                        tlFixedString v24(v7->mMeshNames.mList[j].mStr);
                        v7->mMeshPtrs.mList[j] = cdGetMesh(pak_id, v24);
                    }
                }
            }
            mSize = bank->mPtrs.mSize;
            ++i;
            if (i >= mSize)
                break;
        }
    }
}

// ============================================================================
// XModelGetStaticBounds - ea: 0x006CBBA0
// ============================================================================
struct XModelCollSurf {
    math::Position3 mins;   // +0x00
    math::Position3 maxs;   // +0x10
    int boneIdx;            // +0x20
};

struct XModelCollisionView {
    uint8_t pad[0x38];
    InplaceVector<XModelCollSurf const*> collSurfs;
};
static_assert(offsetof(XModelCollisionView, collSurfs) == 0x38,
              "XModel collision layout mismatch");

int XModelGetStaticBounds(IVPointer<XModel> model, float (*const axis)[3],
                          math::Position3& mins, math::Position3& maxs,
                          const math::Mat43* bones, int nbones)
{
    ValidatePakId((TPakId)model.mPakId);
    const XModelCollisionView* view =
        reinterpret_cast<const XModelCollisionView*>(model.mValue);
    unsigned int mSize = view->collSurfs.mSize;
    if (mSize == 0)
        return 0;
    mins.v = _mm_set1_ps(3.4028235e38f);
    maxs.v = _mm_set1_ps(-3.4028235e38f);

    const InplaceVector<XModelCollSurf const*>& collSurfs = view->collSurfs;
    if (collSurfs.mList == nullptr)
        return 0;
    for (unsigned int i = 0; i < collSurfs.mSize; ++i)
    {
        ValidatePakId((TPakId)model.mPakId);
        const XModelCollSurf* surf = collSurfs.mList[i];
        int boneIdx = surf->boneIdx;
        if (boneIdx < 0 || boneIdx >= nbones)
        {
            AeAssert::gCurrentAuthor = AeAssert::JSV;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xmodel.cpp";
            AeAssert::gCurrentLine = 809;
            AeAssert::gCurrentExpr = "bone_index >= 0 && bone_index < nbones";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("bad bone index"))
                __debugbreak();
        }
        const math::Mat43& bone = bones[boneIdx];
        for (int corner = 0; corner < 8; ++corner)
        {
            __m128 p = _mm_setr_ps(
                (corner & 1) != 0 ? surf->mins.v.m128_f32[0]
                                  : surf->maxs.v.m128_f32[0],
                (corner & 2) != 0 ? surf->mins.v.m128_f32[1]
                                  : surf->maxs.v.m128_f32[1],
                (corner & 4) != 0 ? surf->mins.v.m128_f32[2]
                                  : surf->maxs.v.m128_f32[2],
                0.0f);
            __m128 world = _mm_add_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_shuffle_ps(p, p, 0), bone.x.v),
                    _mm_mul_ps(_mm_shuffle_ps(p, p, 85), bone.y.v)),
                _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(p, p, 170), bone.z.v),
                           bone.w.v));
            float v24[3];
            MatrixTransformVector(&world.m128_f32[0], axis, v24);
            for (int j = 0; j < 3; ++j)
            {
                if (mins.v.m128_f32[j] > v24[j])
                    mins.v.m128_f32[j] = v24[j];
                if (v24[j] > maxs.v.m128_f32[j])
                    maxs.v.m128_f32[j] = v24[j];
            }
        }
    }
    return 1;
}


// ea: 0x006BD3C0
const char* XModelGetSurfaceName(IVPointer<XModel> model, int subMatIndex,
                                 int lod)
{
    (void)model;
    (void)subMatIndex;
    (void)lod;
    return "NoSurfaces";
}

// ea: 0x006BD3D0
XSurface* XSurfaceCloneSurface(XSurface* surface)
{
    XSurface* v1 = (XSurface*)mem_heap_malloc(0x34u);
    memcpy(v1, surface, 0x34u);
    v1->surfRigidATI = nullptr;
    v1->surfRigidNV = nullptr;
    float (*tex)[2] = (float(*)[2])mem_heap_malloc_ctx(
        16, 8 * v1->numVerts, "hunk",
        "c:\\cod\\code\\game\\xmodel.cpp", 213);
    v1->texCoords = tex;
    Com_Memcpy(tex, surface->texCoords, 8 * v1->numVerts);
    return v1;
}

// ea: 0x006BD440
int XSurfaceGetNumVerts(XSurface* surface)
{
    return surface->numVerts;
}

// ea: 0x006BD450
int XSurfaceGetNumTris(XSurface* surface)
{
    return surface->numTris;
}

// ea: 0x006BD460
XSurfTile_e XSurfaceTileMode(XSurface* surface)
{
    return (XSurfTile_e)surface->tileMode;
}

// ea: 0x006BD470
void XSurfaceGetTris(XSurface* surface, uint16_t* pIndexes,
                     uint16_t offset)
{
    if (((uintptr_t)surface->triIndexes & 3) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xmodel.cpp";
        AeAssert::gCurrentLine = 237;
        AeAssert::gCurrentExpr = "(((int)surface->triIndexes) & 3) == 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (((uintptr_t)pIndexes & 3) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xmodel.cpp";
        AeAssert::gCurrentLine = 238;
        AeAssert::gCurrentExpr = "(((int)pIndexes) & 3) == 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if ((surface->numTris & 1) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xmodel.cpp";
        AeAssert::gCurrentLine = 239;
        AeAssert::gCurrentExpr = "(surface->numTris & 1) == 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (offset != 0)
    {
        uint16_t* triIndexes = surface->triIndexes;
        int v4 = offset | (offset << 16);
        uint16_t* v5 = pIndexes;
        int v6 = surface->numTris >> 1;
        do
        {
            *v5 = (uint16_t)(v4 + *triIndexes);
            int v7 = *(triIndexes + 1);
            uint16_t* v8 = triIndexes + 2;
            uint32_t* v9 = (uint32_t*)(v5 + 2);
            *v9++ = (uint32_t)(v4 + v7);
            *v9 = (uint32_t)(v4 + *(v8 + 1));
            v5 = (uint16_t*)(v9 + 1);
            triIndexes = v8 + 4;
            --v6;
        } while (v6 != 0);
    }
    else
    {
        memcpy(pIndexes, surface->triIndexes, 6 * surface->numTris);
    }
}

// ea: 0x006BD5C0
XVariantVertexInfo_u XSurfaceGetVertexInfoArray(XSurface* surf)
{
    return surf->verts;
}

// ea: 0x006BD5D0
float (*XSurfaceGetTexCoordArray(XSurface* surf))[2]
{
    return surf->texCoords;
}

// ea: 0x006BD5E0
XBlendInfo* XSurfaceGetBlendInfoArray(XSurface* surf)
{
    return surf->blends;
}

// ea: 0x006BD5F0
int XSurfaceGetBoneIndex(XSurface* surf)
{
    return surf->boneIndex;
}

// ea: 0x006BD600
void XSurfaceRemapTextureCoordinates(XSurface* surf, float* const scale,
                                     float* const offset, int x, int y)
{
    if (surf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xmodel.cpp";
        AeAssert::gCurrentLine = 293;
        AeAssert::gCurrentExpr = "surf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (offset == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xmodel.cpp";
        AeAssert::gCurrentLine = 294;
        AeAssert::gCurrentExpr = "offset";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (scale == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xmodel.cpp";
        AeAssert::gCurrentLine = 295;
        AeAssert::gCurrentExpr = "scale";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (x >= 2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xmodel.cpp";
        AeAssert::gCurrentLine = 296;
        AeAssert::gCurrentExpr = "x == 0 || x == 1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", x))
            __debugbreak();
    }
    if (y >= 2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xmodel.cpp";
        AeAssert::gCurrentLine = 297;
        AeAssert::gCurrentExpr = "y == 0 || y == 1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", y))
            __debugbreak();
    }
    if (x == y)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xmodel.cpp";
        AeAssert::gCurrentLine = 298;
        AeAssert::gCurrentExpr = "x != y";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float* texCoords = (float*)surf->texCoords;
    for (int i = surf->numVerts; i != 0; --i)
    {
        float v7 = texCoords[y];
        texCoords[0] = (texCoords[x] * scale[0]) + offset[0];
        texCoords[1] = (v7 * scale[1]) + offset[1];
        texCoords += 2;
    }
}

// ea: 0x006BD800
void XSurfaceGetVerts(XSurface* surf, DObjSkelMat* boneMatrix,
                      float* pVert, float* pTexCoord, float* pNormal)
{
    XVariantVertexInfo_u v6;
    v6.rigid = surf->verts.rigid;
    int boneIndex = surf->boneIndex;
    if (pTexCoord != nullptr)
    {
        unsigned int v9 = 8 * surf->numVerts;
        memcpy(pTexCoord, surf->texCoords, v9);
    }
    if (boneIndex == -1)
    {
        XBlendInfo* blends = surf->blends;
        int numVerts = surf->numVerts;
        if (numVerts != 0)
        {
            for (int n = 0; n < numVerts; ++n)
            {
                XRigidVertexInfo_s* rigid = &v6.rigid[n];
                float* dst = pVert + n * 3;
                if (pNormal != nullptr)
                {
                    float* pn = pNormal + n * 3;
                    pn[0] = (boneMatrix->axis[2][0] * rigid->normal[2])
                            + (boneMatrix->axis[1][0] * rigid->normal[1])
                            + (boneMatrix->axis[0][0] * rigid->normal[0]);
                    pn[1] = (boneMatrix->axis[2][1] * rigid->normal[2])
                            + (boneMatrix->axis[1][1] * rigid->normal[1])
                            + (boneMatrix->axis[0][1] * rigid->normal[0]);
                    pn[2] = (boneMatrix->axis[2][2] * rigid->normal[2])
                            + (boneMatrix->axis[1][2] * rigid->normal[1])
                            + (boneMatrix->axis[0][2] * rigid->normal[0]);
                }
                dst[0] = (boneMatrix->axis[2][0] * rigid->offset[2])
                         + (boneMatrix->axis[1][0] * rigid->offset[1])
                         + (boneMatrix->axis[0][0] * rigid->offset[0])
                         + boneMatrix->origin[0];
                dst[1] = (boneMatrix->axis[2][1] * rigid->offset[2])
                         + (boneMatrix->axis[1][1] * rigid->offset[1])
                         + (boneMatrix->axis[0][1] * rigid->offset[0])
                         + boneMatrix->origin[1];
                dst[2] = (boneMatrix->axis[2][2] * rigid->offset[2])
                         + (boneMatrix->axis[1][2] * rigid->offset[1])
                         + (boneMatrix->axis[0][2] * rigid->offset[0])
                         + boneMatrix->origin[2];
                float v27 = rigid->offset[0];
                if (v27 == 0.0f)
                {
                    // rigid vertex: no additional blends
                }
                else
                {
                    dst[0] *= rigid->normal[2];
                    dst[1] *= rigid->normal[2];
                    dst[2] *= rigid->normal[2];
                    for (int b = 0; b < (int)v27; ++b)
                    {
                        XBlendInfo* blend = &blends[b];
                        float boneWeight = blend->boneWeight;
                        dst[0] += ((blend->offset[0] * boneMatrix[blend->boneIndex].axis[0][0]
                                    + blend->offset[1] * boneMatrix[blend->boneIndex].axis[1][0]
                                    + blend->offset[2] * boneMatrix[blend->boneIndex].axis[2][0]
                                    + boneMatrix[blend->boneIndex].origin[0]) * boneWeight);
                        dst[1] += ((blend->offset[0] * boneMatrix[blend->boneIndex].axis[0][1]
                                    + blend->offset[1] * boneMatrix[blend->boneIndex].axis[1][1]
                                    + blend->offset[2] * boneMatrix[blend->boneIndex].axis[2][1]
                                    + boneMatrix[blend->boneIndex].origin[1]) * boneWeight);
                        dst[2] += ((blend->offset[0] * boneMatrix[blend->boneIndex].axis[0][2]
                                    + blend->offset[1] * boneMatrix[blend->boneIndex].axis[1][2]
                                    + blend->offset[2] * boneMatrix[blend->boneIndex].axis[2][2]
                                    + boneMatrix[blend->boneIndex].origin[2]) * boneWeight);
                    }
                }
            }
        }
    }
    else if (surf->numVerts != 0)
    {
        for (int n = 0; n < surf->numVerts; ++n)
        {
            XRigidVertexInfo_s* rigid = &v6.rigid[n];
            float* dst = pVert + n * 3;
            DObjSkelMat& m = boneMatrix[boneIndex];
            if (pNormal != nullptr)
            {
                float* pn = pNormal + n * 3;
                pn[0] = (m.axis[2][0] * rigid->normal[2])
                        + (m.axis[1][0] * rigid->normal[1])
                        + (m.axis[0][0] * rigid->normal[0]);
                pn[1] = (m.axis[2][1] * rigid->normal[2])
                        + (m.axis[1][1] * rigid->normal[1])
                        + (m.axis[0][1] * rigid->normal[0]);
                pn[2] = (m.axis[2][2] * rigid->normal[2])
                        + (m.axis[1][2] * rigid->normal[1])
                        + (m.axis[0][2] * rigid->normal[0]);
            }
            dst[0] = (m.axis[2][0] * rigid->offset[2])
                     + (m.axis[1][0] * rigid->offset[1])
                     + (m.axis[0][0] * rigid->offset[0]) + m.origin[0];
            dst[1] = (m.axis[2][1] * rigid->offset[2])
                     + (m.axis[1][1] * rigid->offset[1])
                     + (m.axis[0][1] * rigid->offset[0]) + m.origin[1];
            dst[2] = (m.axis[2][2] * rigid->offset[2])
                     + (m.axis[1][2] * rigid->offset[1])
                     + (m.axis[0][2] * rigid->offset[0]) + m.origin[2];
        }
    }
}
