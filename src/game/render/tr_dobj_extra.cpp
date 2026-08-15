// ============================================================================
// tr_dobj_extra.cpp - render.o DObjTraceline (0x6CCD90, DObj.cpp)
// Isolated TU: local IDA-verified views only (math_types.h + locals) so the
// DObj/DObjTrace_s tags match the binary map mangles.
// ============================================================================

#include "core/math_types.h"

#include <intrin.h>
#include <math.h>
#include <stdint.h>

namespace AeAssert {
enum ECoderId { COD3 = 0, JRS = 3 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmtstring, ...);
}

extern "C" int __fpclass(float);

static bool IS_NAN(float x)
{
    return (__fpclass(x) & 0x297) != 0;
}

// ============================================================================
// IDA-verified views (codmp_xboxr.xbe.h local types)
// ============================================================================
enum TPakId { kPakTypeNone = -1 };

template <class T>
struct IVec {
    unsigned int mSize;  // +0x00
    T* mList;            // +0x04
};

struct InplaceStringLocal {
    const char* mStr;  // +0x00
};

class XBoneInfo {
public:
    math::Position3::Packed mBounds[2];  // +0x00
    math::Dir3::Packed mOffset;          // +0x18
    float mRadiusSquared;                // +0x24
};
static_assert(sizeof(XBoneInfo) == 0x28, "XBoneInfo size mismatch");

struct XBoneHierarchy {
    InplaceStringLocal mName;    // +0x00
    unsigned int mNameHash;      // +0x04
    int mParentIndex;            // +0x08
};

class XModelParts {
public:
    IVec<math::Mat43::Packed> mTransforms;         // +0x00
    IVec<XBoneInfo> mBoneInfos;                    // +0x08
    IVec<XBoneHierarchy> mHierarchy;               // +0x10
    IVec<unsigned char> mPartClassifications;      // +0x18
    IVec<InplaceStringLocal> mMeshNames;           // +0x20
    IVec<void*> mMeshPtrs;                         // +0x28
    int mNumRootBones;                             // +0x30
    InplaceStringLocal mAnimDefName;               // +0x34
    void* mAnimDef;                                // +0x38
    InplaceStringLocal mName;                      // +0x3C
};
static_assert(sizeof(XModelParts) == 0x40, "XModelParts size mismatch");

struct XModelLod {
    float dist;                    // +0x00
    InplaceStringLocal filename;   // +0x04
    XModelParts* xmodelParts;      // +0x08
};

class XModel {
public:
    math::Position3 mins;   // +0x00
    math::Position3 maxs;   // +0x10
    XModelParts* parts;     // +0x20
    XModelLod* lod[5];      // +0x24
};

struct IVPointerXModel {
    XModel* mValue;  // +0x00
    int mPakId;      // +0x04
};

class DObj {
public:
    uint8_t _pad0[0x60];               // +0x00
    unsigned char modelParents[8];     // +0x60
    uint8_t _pad68[0x70 - 0x68];
    void* skel;                        // +0x70
    uint8_t _pad74[0x7C - 0x74];
    int ignoreCollision;               // +0x7C
    IVPointerXModel models[8];         // +0x80
    uint8_t _padC0[0xCE - 0xC0];
    unsigned char numModels;           // +0xCE
    unsigned char numBones;            // +0xCF
};

struct HashString {
    unsigned int mHash;  // +0x00
};

struct DObjTrace_s {
    float fraction;             // +0x00
    int surfaceflags;           // +0x04
    float normal[3];            // +0x08
    HashString partName;        // +0x14
    unsigned int partGroup;     // +0x18
    unsigned char startsolid;   // +0x1C
    unsigned char allsolid;     // +0x1D
};

struct DObjSkelMat {
    float axis[3][4];  // +0x00
    float origin[4];   // +0x30
};

extern DObjSkelMat* DObjGetMatrixArray(const DObj* obj, int modelIndex);
extern void ValidatePakId(TPakId pakId);

// ============================================================================
// DObjTraceline - ray vs per-bone AABB sweep (DObj.cpp)
// ============================================================================
void DObjTraceline(const DObj* obj, const math::Position3& start,
                   const math::Position3& end, unsigned char* priorityMap,
                   DObjTrace_s* trace, float extraDistanceCheck)
{
    float v106 = end.v.m128_f32[2] - start.v.m128_f32[2];  // dz
    float v105 = end.v.m128_f32[1] - start.v.m128_f32[1];  // dy
    float delta = end.v.m128_f32[0] - start.v.m128_f32[0]; // dx
    float invL2 = 1.0f / (((v106 * v106) + (v105 * v105)) + (delta * delta));

    float* MatrixArray = (float*)DObjGetMatrixArray(obj, 0);
    trace->surfaceflags = 0;
    trace->startsolid = 0;
    trace->allsolid = 0;
    trace->partName.mHash = 0;
    trace->partGroup = 0;
    trace->normal[1] = 0.0f;
    trace->normal[2] = 0.0f;
    trace->normal[0] = 0.0f;

    if (obj->skel == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
        AeAssert::gCurrentLine = 1573;
        AeAssert::gCurrentExpr = "skel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    unsigned int lowestPriority = 2;
    if (IS_NAN(start.v.m128_f32[0]) || IS_NAN(start.v.m128_f32[1])
        || IS_NAN(start.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
        AeAssert::gCurrentLine = 1575;
        AeAssert::gCurrentExpr =
            "!IS_NAN((start)[0]) && !IS_NAN((start)[1]) && !IS_NAN((start)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(end.v.m128_f32[0]) || IS_NAN(end.v.m128_f32[1])
        || IS_NAN(end.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
        AeAssert::gCurrentLine = 1576;
        AeAssert::gCurrentExpr =
            "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }

    int globalBoneIndex = 0;
    int hitT = -1;
    float hitSign = 0.0f;
    unsigned __int16 classificationArray[132];

    if (priorityMap != nullptr)
    {
        int j = 0;
        if (obj->numModels != 0)
        {
            const XModelParts* model;
            int size;
            int ignoreCollision;
            unsigned int v26;
            int localBoneIndex;
            while (j < obj->numModels)
            {
                ValidatePakId((TPakId)obj->models[j].mPakId);
                XModel* mValue = obj->models[j].mValue;
                int v15 = 0;
                if (mValue->lod[0] == nullptr)
                {
                    do
                    {
                        ++v15;
                    } while (mValue->lod[v15] == nullptr);
                }
                TPakId mPakId = (TPakId)obj->models[j].mPakId;
                model = mValue->lod[v15]->xmodelParts;
                ValidatePakId(mPakId);
                XModel* v18 = obj->models[j].mValue;
                XModelLod* v19 = v18->lod[0];
                int v20 = 0;
                if (v19 == nullptr)
                {
                    XModelLod** v21 = v18->lod;
                    do
                    {
                        ++v21;
                        ++v20;
                    } while (*v21 == nullptr);
                }
                if (v18->lod[v20]->xmodelParts != nullptr)
                {
                    int v22 = 0;
                    if (v19 == nullptr)
                    {
                        XModelLod** v23 = v18->lod;
                        XModelLod* v24;
                        do
                        {
                            v24 = v23[1];
                            ++v23;
                            ++v22;
                        } while (v24 == nullptr);
                    }
                    size = v18->lod[v22]->xmodelParts->mHierarchy.mSize;
                }
                else
                {
                    size = 0;
                }
                ignoreCollision = obj->ignoreCollision & (1 << j);
                v26 = 0;
                localBoneIndex = 0;
                if (size <= 0)
                    goto nextModel;

                while (1)
                {
                    const XModelParts* v27 = model;
                    if (v26 >= v27->mPartClassifications.mSize)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
                        AeAssert::gCurrentLine = 91;
                        AeAssert::gCurrentExpr = "index < mSize";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Bounds check"))
                            __debugbreak();
                    }
                    if (localBoneIndex >= v27->mPartClassifications.mSize)
                        v26 = 0;
                    int mList = v27->mPartClassifications.mList[v26];
                    unsigned int v110 = (unsigned int)mList;
                    unsigned int currentPriority = priorityMap[mList];
                    int classification = mList;
                    if (currentPriority == 1)
                    {
                        int mNumRootBones = v27->mNumRootBones;
                        if (localBoneIndex >= mNumRootBones)
                        {
                            // XModelParts::GetBoneParent(v27, localBoneIndex -
                            // mNumRootBones) inlined; mHierarchy at +0x10 per IDA.
                            int parent = localBoneIndex - mNumRootBones;
                            if ((unsigned int)parent >= v27->mHierarchy.mSize)
                            {
                                AeAssert::gCurrentAuthor = AeAssert::JRS;
                                AeAssert::gCurrentFile =
                                    "c:\\cod\\code\\game\\XModelParts.h";
                                AeAssert::gCurrentLine = 217;
                                AeAssert::gCurrentExpr =
                                    "i >= 0 && i < mHierarchy.size()";
                                if (!AeAssert::IsIgnored()
                                    && AeAssert::Assert("Bad Bone Index"))
                                    __debugbreak();
                            }
                            unsigned int vParent = (unsigned int)parent;
                            if ((unsigned int)parent >= v27->mHierarchy.mSize)
                                vParent = 0;
                            mList =
                                classificationArray[globalBoneIndex
                                                    - v27->mHierarchy.mList[vParent]
                                                          .mParentIndex];
                            classification = mList;
                        }
                        else
                        {
                            unsigned char v31 = obj->modelParents[j];
                            if (v31 == 0xFF)
                            {
                                mList = 0;
                                classification = 0;
                            }
                            else
                            {
                                mList = classificationArray[v31];
                                classification = mList;
                            }
                        }
                        v110 = (unsigned int)mList;
                        currentPriority = priorityMap[mList];
                    }
                    classificationArray[globalBoneIndex] =
                        (unsigned __int16)mList;
                    if (ignoreCollision != 0)
                        goto miss;

                    int v33 = localBoneIndex;
                    if (localBoneIndex >= model->mBoneInfos.mSize)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
                        AeAssert::gCurrentLine = 91;
                        AeAssert::gCurrentExpr = "index < mSize";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Bounds check"))
                            __debugbreak();
                    }
                    if (localBoneIndex >= model->mBoneInfos.mSize)
                        v33 = 0;
                    XBoneInfo* v34 = &model->mBoneInfos.mList[v33];
                    float mRadiusSquared = v34->mRadiusSquared;
                    float* v36 = (float*)v34;
                    if (mRadiusSquared == 0.0f
                        || lowestPriority > currentPriority)
                        goto miss;

                    if (IS_NAN(MatrixArray[0]) || IS_NAN(MatrixArray[1])
                        || IS_NAN(MatrixArray[2]))
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
                        AeAssert::gCurrentLine = 1623;
                        AeAssert::gCurrentExpr =
                            "!IS_NAN((boneMatrix->axis[0])[0]) && !IS_NAN((boneMatrix->axis[0])[1]) && !IS_NAN((boneMatrix->axis[0])[2])";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Invalid vector"))
                            __debugbreak();
                    }
                    if (IS_NAN(MatrixArray[4]) || IS_NAN(MatrixArray[5])
                        || IS_NAN(MatrixArray[6]))
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
                        AeAssert::gCurrentLine = 1624;
                        AeAssert::gCurrentExpr =
                            "!IS_NAN((boneMatrix->axis[1])[0]) && !IS_NAN((boneMatrix->axis[1])[1]) && !IS_NAN((boneMatrix->axis[1])[2])";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Invalid vector"))
                            __debugbreak();
                    }
                    if (IS_NAN(MatrixArray[8]) || IS_NAN(MatrixArray[9])
                        || IS_NAN(MatrixArray[10]))
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
                        AeAssert::gCurrentLine = 1625;
                        AeAssert::gCurrentExpr =
                            "!IS_NAN((boneMatrix->axis[2])[0]) && !IS_NAN((boneMatrix->axis[2])[1]) && !IS_NAN((boneMatrix->axis[2])[2])";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Invalid vector"))
                            __debugbreak();
                    }
                    if (IS_NAN(MatrixArray[12]) || IS_NAN(MatrixArray[13])
                        || IS_NAN(MatrixArray[14]))
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
                        AeAssert::gCurrentLine = 1626;
                        AeAssert::gCurrentExpr =
                            "!IS_NAN((boneMatrix->origin)[0]) && !IS_NAN((boneMatrix->origin)[1]) && !IS_NAN((boneMatrix->origin)[2])";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Invalid vector"))
                            __debugbreak();
                    }

                    // Sphere reject against the transformed bone offset.
                    float v37 = v36[6];
                    float v38 = v36[8];
                    float v39 = v36[7];
                    float v40 = (((MatrixArray[8] * v38) + (MatrixArray[0] * v37))
                                 + (MatrixArray[4] * v39))
                                + MatrixArray[12];
                    float v41 = (((MatrixArray[5] * v39) + (MatrixArray[1] * v37))
                                 + (MatrixArray[9] * v38))
                                + MatrixArray[13];
                    float v42 = (MatrixArray[10] * v38)
                                + (MatrixArray[6] * v39);
                    float v43 = start.v.m128_f32[1];
                    float v44 = (v42 + (MatrixArray[2] * v37)) + MatrixArray[14];
                    float v45 = start.v.m128_f32[0];
                    float v46 = (start.v.m128_f32[2] - v44) * v106;
                    float v96 = start.v.m128_f32[2] - v44;
                    float v47 = v43 - v41;
                    float v48 = v45 - v40;
                    float v49 = 0.0f
                                - (((v46 + (v47 * v105)) + (v48 * delta))
                                   * invL2);
                    float sphereFraction = v49;
                    float v50;
                    if (v49 >= 1.0f)
                    {
                        v48 = end.v.m128_f32[0] - v40;
                        float v51 =
                            ((end.v.m128_f32[2] - v44)
                             * (end.v.m128_f32[2] - v44))
                            + ((end.v.m128_f32[1] - v41)
                               * (end.v.m128_f32[1] - v41));
                        v50 = v51 + (v48 * v48);
                    }
                    else if (v49 > 0.0f)
                    {
                        v50 = ((((v49 * v106) + v96) * ((v49 * v106) + v96))
                               + (((v49 * v105) + v47) * ((v49 * v105) + v47)))
                              + (((v49 * delta) + v48) * ((v49 * delta) + v48));
                    }
                    else
                    {
                        float v51 = (v96 * v96) + (v47 * v47);
                        v50 = v51 + (v48 * v48);
                    }
                    float diff2 =
                        ((extraDistanceCheck * extraDistanceCheck) + v36[9])
                        - v50;
                    if (diff2 <= 0.0f
                        || (lowestPriority == currentPriority
                            && sphereFraction - sqrtf(diff2 * invL2)
                                   >= trace->fraction))
                        goto miss;

                    // Transform ray endpoints into bone space.
                    float v52 = start.v.m128_f32[2] - MatrixArray[14];
                    float v53 = start.v.m128_f32[1] - MatrixArray[13];
                    float v54 = start.v.m128_f32[0] - MatrixArray[12];
                    float localStart[3];
                    float localEnd[3];
                    localStart[0] =
                        ((v52 * MatrixArray[2]) + (MatrixArray[1] * v53))
                        + (MatrixArray[0] * v54);
                    float v55 = v52 * MatrixArray[6];
                    float v56 = (v52 * MatrixArray[10])
                                + (v54 * MatrixArray[8]);
                    float v57 = (v55 + (v54 * MatrixArray[4]))
                                + (MatrixArray[5] * v53);
                    float v58 = MatrixArray[1];
                    float v59 = MatrixArray[9] * v53;
                    float v60 = end.v.m128_f32[1] - MatrixArray[13];
                    float v61 = v56 + v59;
                    float v62 = end.v.m128_f32[0] - MatrixArray[12];
                    localStart[1] = v57;
                    localStart[2] = v61;
                    float v63 = end.v.m128_f32[2] - MatrixArray[14];
                    localEnd[0] = ((v63 * MatrixArray[2])
                                   + (v58 * v60))
                                  + (MatrixArray[0] * v62);
                    float v64 = (v63 * MatrixArray[6])
                                + (v62 * MatrixArray[4]);
                    float v65 = ((v63 * MatrixArray[10])
                                 + (v62 * MatrixArray[8]))
                                + (MatrixArray[9] * v60);
                    localEnd[1] = v64 + (MatrixArray[5] * v60);
                    localEnd[2] = v65;

                    if (IS_NAN(localStart[0]) || IS_NAN(localStart[1])
                        || IS_NAN(localStart[2]))
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
                        AeAssert::gCurrentLine = 1667;
                        AeAssert::gCurrentExpr =
                            "!IS_NAN((localStart)[0]) && !IS_NAN((localStart)[1]) && !IS_NAN((localStart)[2])";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Invalid vector"))
                            __debugbreak();
                    }
                    if (IS_NAN(localEnd[0]) || IS_NAN(localEnd[1])
                        || IS_NAN(localEnd[2]))
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
                        AeAssert::gCurrentLine = 1668;
                        AeAssert::gCurrentExpr =
                            "!IS_NAN((localEnd)[0]) && !IS_NAN((localEnd)[1]) && !IS_NAN((localEnd)[2])";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Invalid vector"))
                            __debugbreak();
                    }

                    // Slab test over min then max corner, each expanded by
                    // extraDistanceCheck.
                    float enterFrac = 0.0f;
                    float leaveFrac = trace->fraction;
                    float sign = -1.0f;
                    bool bStartSolid = true;
                    bool bEndSolid = true;
                    float* bounds = v36;
                    for (;;)
                    {
                        if (IS_NAN(bounds[0]) || IS_NAN(bounds[1])
                            || IS_NAN(bounds[2]))
                        {
                            AeAssert::gCurrentAuthor = AeAssert::COD3;
                            AeAssert::gCurrentFile =
                                "c:\\cod\\code\\game\\DObj.cpp";
                            AeAssert::gCurrentLine = 1681;
                            AeAssert::gCurrentExpr =
                                "!IS_NAN((bounds)[0]) && !IS_NAN((bounds)[1]) && !IS_NAN((bounds)[2])";
                            if (!AeAssert::IsIgnored()
                                && AeAssert::Assert("Invalid vector"))
                                __debugbreak();
                        }
                        float v69 = sign * extraDistanceCheck;
                        for (int axis = 0;; ++axis)
                        {
                            float v71 = localEnd[axis];
                            float v73 = bounds[axis] + v69;
                            float v74 = (localStart[axis] - v73) * sign;
                            float v75 = (v71 - v73) * sign;
                            if (v74 <= 0.0f)
                            {
                                if (v75 <= 0.0f)
                                    goto nextAxis;
                                float v77 = v74 - v75;
                                bEndSolid = false;
                                if (v77 >= 0.0f)
                                {
                                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                                    AeAssert::gCurrentFile =
                                        "c:\\cod\\code\\game\\DObj.cpp";
                                    AeAssert::gCurrentLine = 1710;
                                    AeAssert::gCurrentExpr = "dist < 0";
                                    if (!AeAssert::IsIgnored()
                                        && AeAssert::Assert("old cod assert"))
                                        __debugbreak();
                                }
                                if (v74 > (v77 * leaveFrac))
                                {
                                    leaveFrac = v74 / v77;
                                    if (enterFrac >= (v74 / v77))
                                        goto miss;
                                }
                                goto nextAxis;
                            }
                            if (v75 > 0.0f)
                                goto miss;
                            float v76 = v74 - v75;
                            bStartSolid = false;
                            if (v76 <= 0.0f)
                            {
                                AeAssert::gCurrentAuthor = AeAssert::COD3;
                                AeAssert::gCurrentFile =
                                    "c:\\cod\\code\\game\\DObj.cpp";
                                AeAssert::gCurrentLine = 1696;
                                AeAssert::gCurrentExpr = "dist > 0";
                                if (!AeAssert::IsIgnored()
                                    && AeAssert::Assert("old cod assert"))
                                    __debugbreak();
                            }
                            if (v74 <= (v76 * enterFrac))
                                goto nextAxis;
                            enterFrac = v74 / v76;
                            if ((v74 / v76) >= leaveFrac)
                                goto miss;
                            hitSign = sign;
                            hitT = axis;
                        nextAxis:
                            if (axis + 1 >= 3)
                                break;
                        }
                        if (sign == 1.0f)
                            break;
                        sign = 1.0f;
                        bounds += 3;
                    }

                    if (!bStartSolid)
                    {
                        if (lowestPriority == currentPriority)
                        {
                            if (enterFrac >= trace->fraction)
                                goto miss;
                        }
                        else
                        {
                            lowestPriority = currentPriority;
                        }
                        trace->fraction = enterFrac;
                        if (localBoneIndex >= model->mHierarchy.mSize)
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
                        unsigned int v85 = (unsigned int)localBoneIndex;
                        if (localBoneIndex >= model->mHierarchy.mSize)
                        {
                            AeAssert::gCurrentAuthor = AeAssert::COD3;
                            AeAssert::gCurrentFile =
                                "../ae\\inplace/InplaceVector.h";
                            AeAssert::gCurrentLine = 91;
                            AeAssert::gCurrentExpr = "index < mSize";
                            if (!AeAssert::IsIgnored()
                                && AeAssert::Assert("Bounds check"))
                                __debugbreak();
                            if (localBoneIndex >= model->mHierarchy.mSize)
                                v85 = 0;
                        }
                        trace->partName.mHash =
                            model->mHierarchy.mList[v85].mNameHash;
                        trace->partGroup = v110;
                        if (hitT < 0)
                        {
                            AeAssert::gCurrentAuthor = AeAssert::COD3;
                            AeAssert::gCurrentFile =
                                "c:\\cod\\code\\game\\DObj.cpp";
                            AeAssert::gCurrentLine = 1758;
                            AeAssert::gCurrentExpr = "hitT >= 0";
                            if (!AeAssert::IsIgnored()
                                && AeAssert::Assert("old cod assert"))
                                __debugbreak();
                        }
                        if (hitT >= 3)
                        {
                            AeAssert::gCurrentAuthor = AeAssert::COD3;
                            AeAssert::gCurrentFile =
                                "c:\\cod\\code\\game\\DObj.cpp";
                            AeAssert::gCurrentLine = 1759;
                            AeAssert::gCurrentExpr = "hitT < 3";
                            if (!AeAssert::IsIgnored()
                                && AeAssert::Assert("old cod assert"))
                                __debugbreak();
                        }
                        trace->normal[0] = MatrixArray[4 * hitT] * hitSign;
                        trace->normal[1] =
                            MatrixArray[4 * hitT + 1] * hitSign;
                        trace->normal[2] =
                            MatrixArray[4 * hitT + 2] * hitSign;
                        goto miss;
                    }
                    trace->startsolid = 1;
                    if (!bEndSolid)
                        goto miss;
                    trace->allsolid = 1;
                    trace->fraction = 0.0f;
                    if (localBoneIndex >= model->mHierarchy.mSize)
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
                    unsigned int v81 = (unsigned int)localBoneIndex;
                    if (localBoneIndex >= model->mHierarchy.mSize)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::COD3;
                        AeAssert::gCurrentFile =
                            "../ae\\inplace/InplaceVector.h";
                        AeAssert::gCurrentLine = 91;
                        AeAssert::gCurrentExpr = "index < mSize";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Bounds check"))
                            __debugbreak();
                        if (localBoneIndex >= model->mHierarchy.mSize)
                            v81 = 0;
                    }
                    trace->partName.mHash =
                        model->mHierarchy.mList[v81].mNameHash;
                    trace->partGroup = (unsigned int)classification;
                    trace->normal[1] = 0.0f;
                    trace->normal[2] = 0.0f;
                    trace->normal[0] = 0.0f;
                    return;

                miss:
                    MatrixArray += 16;
                    ++localBoneIndex;
                    ++globalBoneIndex;
                    if (localBoneIndex >= size)
                        break;
                    v26 = (unsigned int)localBoneIndex;
                }

            nextModel:
                ++j;
            }
        }
    }
}
