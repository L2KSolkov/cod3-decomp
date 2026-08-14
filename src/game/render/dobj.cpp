// ============================================================================
// dobj.cpp - render.o DObj helpers (DObj.cpp)
// ============================================================================

#include "game/render/xsurface.h"
#include "game/logic/g_local.h"

#include <intrin.h>

// DSkel - DObj skeleton part bits + matrices (IDA-verified)
struct DSkel {
    int animPartBits[4];      // +0x00
    int controlPartBits[4];   // +0x10
    int skelPartBits[4];      // +0x20
    DObjSkelMat mat[1];       // +0x30
};

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
