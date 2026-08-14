// ============================================================================
// dobj.cpp - render.o DObj helpers (DObj.cpp)
// ============================================================================

#include "game/render/xsurface.h"
#include "game/logic/g_local.h"

#include <intrin.h>

extern void ValidatePakId(TPakId pakId);  // streamer.o (pakmanager.cpp)
extern int XModelGetLodForDist(IVPointer<XModel> model, float dist);  // xmodel.cpp

// ea: 0x006BBA40
const char* XModelParts::GetBoneName(unsigned int i) const
{
    if (i >= mHierarchy.mSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\XModelParts.h";
        AeAssert::gCurrentLine = 215;
        AeAssert::gCurrentExpr = "i >= 0 && i < mHierarchy.size()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bad Bone Index"))
            __debugbreak();
    }
    return mHierarchy.mList[i].mName.mStr;
}

// DSkel - DObj skeleton part bits + matrices (IDA-verified)
struct DSkel {
    int animPartBits[4];      // +0x00
    int controlPartBits[4];   // +0x10
    int skelPartBits[4];      // +0x20
    DObjSkelMat mat[1];       // +0x30
};

// ============================================================================
// DObj model/bone accessors (DObj.cpp)
// ============================================================================

// ea: 0x006CC8F0
void DObjGetBoneInfo(DObj* obj, XBoneInfo** boneInfo)
{
    int j = 0;
    if (obj->numModels == 0)
        return;
    IVPointer<XModel>* models = obj->models;
    do
    {
        ValidatePakId((TPakId)models->mPakId);
        XModel* mValue = models->mValue;
        int v6 = 0;
        if (mValue->lod[0] == nullptr)
        {
            do
            {
                ++v6;
            } while (mValue->lod[v6] == nullptr);
        }
        int v8 = 0;
        int size = 0;
        if (mValue->lod[v6]->xmodelParts != nullptr)
        {
            if (mValue->lod[0] == nullptr)
            {
                do
                {
                    ++v8;
                } while (mValue->lod[v8] == nullptr);
            }
            size = mValue->lod[v8]->xmodelParts->mHierarchy.mSize;
        }
        for (unsigned int i = 0; i < (unsigned int)size; ++i)
        {
            ValidatePakId((TPakId)models->mPakId);
            XModel* v11 = models->mValue;
            int v13 = 0;
            if (v11->lod[0] == nullptr)
            {
                do
                {
                    ++v13;
                } while (v11->lod[v13] == nullptr);
            }
            XModelParts* xmodelParts = v11->lod[v13]->xmodelParts;
            unsigned int mSize = xmodelParts->mBoneInfos.mSize;
            unsigned int v18 = i;
            if (i >= mSize)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
                AeAssert::gCurrentLine = 81;
                AeAssert::gCurrentExpr = "index < mSize";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
                    __debugbreak();
                if (i >= mSize)
                    v18 = 0;
            }
            *boneInfo++ = &xmodelParts->mBoneInfos.mList[v18];
        }
        ++models;
        ++j;
    } while (j < obj->numModels);
}

// ea: 0x006CCA50
const char* DObjGetBoneName(DObj* obj, int boneIndex)
{
    if (obj == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
        AeAssert::gCurrentLine = 1419;
        AeAssert::gCurrentExpr = "obj";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int numModels = obj->numModels;
    int baseBoneIndex = 0;
    int j = 0;
    if (obj->numModels == 0)
        return nullptr;
    IVPointer<XModel>* models = obj->models;
    XModel* model = nullptr;
    TPakId pakId = kPakTypeNone;
    int v14 = 0;
    for (IVPointer<XModel>* i = models; ; models = i)
    {
        XModel* mValue = models->mValue;
        model = models->mValue;
        pakId = (TPakId)models->mPakId;
        ValidatePakId(pakId);
        int v7 = 0;
        if (mValue->lod[0] == nullptr)
        {
            do
            {
                ++v7;
            } while (mValue->lod[v7] == nullptr);
        }
        int mSize;
        if (mValue->lod[v7]->xmodelParts != nullptr)
        {
            int v10 = 0;
            if (mValue->lod[0] == nullptr)
            {
                do
                {
                    ++v10;
                } while (mValue->lod[v10] == nullptr);
            }
            mSize = mValue->lod[v10]->xmodelParts->mHierarchy.mSize;
        }
        else
        {
            mSize = 0;
        }
        v14 = boneIndex - baseBoneIndex;
        if (v14 < 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
            AeAssert::gCurrentLine = 1429;
            AeAssert::gCurrentExpr = "index >= 0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        if (v14 < mSize)
            break;
        ValidatePakId(pakId);
        int v15 = 0;
        if (mValue->lod[0] == nullptr)
        {
            do
            {
                ++v15;
            } while (mValue->lod[v15] == nullptr);
        }
        if (model->lod[v15]->xmodelParts != nullptr)
        {
            int v18 = 0;
            if (mValue->lod[0] == nullptr)
            {
                do
                {
                    ++v18;
                } while (mValue->lod[v18] == nullptr);
            }
            if ((model->lod[v18]->xmodelParts->mHierarchy.mSize & 0x80000000) != 0)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
                AeAssert::gCurrentLine = 1432;
                AeAssert::gCurrentExpr = "model->GetNumBones() >= 0";
                if (!AeAssert::IsIgnored())
                {
                    ValidatePakId(pakId);
                    int v21 = 0;
                    if (mValue->lod[0] == nullptr)
                    {
                        do
                        {
                            ++v21;
                        } while (mValue->lod[v21] == nullptr);
                    }
                    unsigned int v26;
                    if (model->lod[v21]->xmodelParts != nullptr)
                    {
                        int v23 = 0;
                        if (mValue->lod[0] == nullptr)
                        {
                            do
                            {
                                ++v23;
                            } while (mValue->lod[v23] == nullptr);
                        }
                        v26 = model->lod[v23]->xmodelParts->mHierarchy.mSize;
                    }
                    else
                    {
                        v26 = 0;
                    }
                    if (AeAssert::Assert("%i", v26))
                        __debugbreak();
                }
            }
        }
        ValidatePakId(pakId);
        int v27 = 0;
        if (mValue->lod[0] == nullptr)
        {
            do
            {
                ++v27;
            } while (mValue->lod[v27] == nullptr);
        }
        int v33;
        if (model->lod[v27]->xmodelParts != nullptr)
        {
            int v30 = 0;
            if (mValue->lod[0] == nullptr)
            {
                do
                {
                    ++v30;
                } while (mValue->lod[v30] == nullptr);
            }
            v33 = model->lod[v30]->xmodelParts->mHierarchy.mSize;
        }
        else
        {
            v33 = 0;
        }
        bool v34 = j + 1 < numModels;
        baseBoneIndex += v33;
        ++j;
        ++i;
        if (!v34)
            return nullptr;
    }
    ValidatePakId(pakId);
    int v37 = 0;
    if (model->lod[0] == nullptr)
    {
        do
        {
            ++v37;
        } while (model->lod[v37] == nullptr);
    }
    return model->lod[v37]->xmodelParts->GetBoneName(v14);
}

// ea: 0x006CCD10
int DObjBad(DObj* obj)
{
    int j = obj->numModels - 1;
    if (obj->numModels == 0)
        return 0;
    for (IVPointer<XModel>* i = &obj->models[j]; ; --i)
    {
        TPakId mPakId = (TPakId)i->mPakId;
        XModel* mValue = i->mValue;
        ValidatePakId(mPakId);
        if (mValue == nullptr)
            break;
        ValidatePakId(mPakId);
        int v8 = 0;
        if (mValue->lod[0] == nullptr)
        {
            do
            {
                ++v8;
            } while (mValue->lod[v8] == nullptr);
        }
        if (mValue->lod[v8]->xmodelParts == nullptr)
            break;
        if (--j < 0)
            return 0;
    }
    return 1;
}

// ea: 0x006CDEF0
int DObjGetLodForDist(DObj* obj, int modelIndex, float dist)
{
    return XModelGetLodForDist(obj->models[modelIndex], dist);
}

// ea: 0x006BDC10
void DObjInit()
{
}

// ea: 0x006BDC20
void DObjShutdown()
{
}

// ea: 0x006BDC30
int DObjIgnoreCollision(DObj* obj, int modelIndex)
{
    return ((1 << modelIndex) & obj->ignoreCollision) != 0;
}

// ea: 0x006BDDF0
void DObjCompleteHierarchyBits(DObj* obj, int* const partBits)
{
    (void)partBits;
    if (obj == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
        AeAssert::gCurrentLine = 623;
        AeAssert::gCurrentExpr = "obj";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (obj->numBones == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
        AeAssert::gCurrentLine = 624;
        AeAssert::gCurrentExpr = "obj->numBones > 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (obj->numModels == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
        AeAssert::gCurrentLine = 629;
        AeAssert::gCurrentExpr = "numModels > 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
}

// ea: 0x006BDEE0
void DObjCalcSkel(DObj* obj, int* const partBits)
{
    (void)obj;
    (void)partBits;
}

// ea: 0x006BDF70
int DObjGetAllocSkelSize(DObj* obj)
{
    return (obj->numBones << 6) + 48;
}

// ea: 0x006BDFD0
int DObjGetNumModels(DObj* obj)
{
    return obj->numModels;
}

// ea: 0x006BDFE0
int DObjSetRotTransIndex(DObj* obj, int* partBits, int boneIndex)
{
    (void)obj;
    (void)partBits;
    (void)boneIndex;
    return 1;
}

// ea: 0x006BDFF0
int DObjSetControlRotTransIndex(DObj* obj, int* const partBits,
                                int boneIndex)
{
    (void)partBits;
    if (obj == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
        AeAssert::gCurrentLine = 1220;
        AeAssert::gCurrentExpr = "obj";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (obj->skel == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
        AeAssert::gCurrentLine = 1221;
        AeAssert::gCurrentExpr = "obj->skel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (boneIndex < 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
        AeAssert::gCurrentLine = 1222;
        AeAssert::gCurrentExpr = "boneIndex >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (boneIndex >= obj->numBones)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
        AeAssert::gCurrentLine = 1223;
        AeAssert::gCurrentExpr = "boneIndex < obj->numBones";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (boneIndex < 0)
        return 0;
    DSkel* skel = (DSkel*)obj->skel;
    int v3 = boneIndex >> 3;
    int v4 = 1 << (boneIndex & 7);
    if ((skel->skelPartBits[v3] & v4) != 0)
        return 0;
    skel->controlPartBits[v3] |= v4;
    skel->animPartBits[v3] |= v4;
    return 1;
}

// ea: 0x006BE150
int DObjGetNumSurfaces(DObj* obj, int* lods)
{
    (void)obj;
    (void)lods;
    return 0;
}

// ea: 0x006BE160
XSurface* DObjGetSurface(DObj* obj, int modelIndex, int subMatIndex,
                         int* lods)
{
    (void)obj;
    (void)modelIndex;
    (void)subMatIndex;
    (void)lods;
    return nullptr;
}

// ea: 0x006BE170
const char* DObjGetSurfaceName(DObj* obj, int modelIndex, int subMatIndex,
                               int* lods)
{
    (void)obj;
    (void)modelIndex;
    (void)subMatIndex;
    (void)lods;
    return "NoMoreSurfaces";
}

// ea: 0x006BE180
void DObjGetSurfaces(DObj* obj, DSurface_s* surfaces,
                     int* const partBits, int* lods)
{
    (void)obj;
    (void)surfaces;
    (void)partBits;
    (void)lods;
}
