// ============================================================================
// xmodel.cpp - render.o XSurface / XModel helpers (xmodel.cpp)
// ============================================================================

#include "game/render/xsurface.h"
#include "game/logic/g_local.h"

#include <string.h>
#include <intrin.h>

extern void* mem_heap_malloc(unsigned int size);  // core.o
extern void* mem_heap_malloc_ctx(int alignment, unsigned int size,
                                 const char* ctx, const char* file,
                                 int line);  // core.o
extern void Com_Memcpy(void* dest, const void* src, unsigned int count);  // core.o

// ea: 0x006BD3B0
int XModelGetSurfaces(IVPointer<XModel> model, XSurface*** surfaces, int lod)
{
    (void)model;
    (void)lod;
    *surfaces = nullptr;
    return 0;
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
