// ============================================================================
// xsurface.h - XSurface / XModelParts types (render.o xmodel.cpp)
// Layouts from IDA local types (codmp_xboxr.xbe.h).
// ============================================================================

#pragma once

#include <stdint.h>

struct XBlendInfo {
    float offset[3];   // +0x00
    int   boneIndex;   // +0x0C
    float boneWeight;  // +0x10
};
static_assert(sizeof(XBlendInfo) == 0x14, "XBlendInfo size mismatch");

struct XRigidVertexInfo_s {
    float normal[3];   // +0x00
    float offset[3];   // +0x0C
};
static_assert(sizeof(XRigidVertexInfo_s) == 0x18,
              "XRigidVertexInfo_s size mismatch");

struct XSimpleBlendInfo_s {
    float offset[3];   // +0x00
    int   boneIndex;   // +0x0C
};
static_assert(sizeof(XSimpleBlendInfo_s) == 0x10,
              "XSimpleBlendInfo_s size mismatch");

struct XSimpleVertexInfo_s {
    float normal[3];          // +0x00
    int   numWeights;         // +0x0C
    XSimpleBlendInfo_s blend; // +0x10
};
static_assert(sizeof(XSimpleVertexInfo_s) == 0x20,
              "XSimpleVertexInfo_s size mismatch");

struct XVertexInfo_s {
    float normal[3];   // +0x00
    int   numWeights;  // +0x0C
    XBlendInfo blend;  // +0x10
};
static_assert(sizeof(XVertexInfo_s) == 0x24, "XVertexInfo_s size mismatch");

union XVariantVertexInfo_u {
    XRigidVertexInfo_s* rigid;   // +0x00
    XSimpleVertexInfo_s* simple; // +0x00
    XVertexInfo_s* info;         // +0x00
};

class XSurface {
public:
    uint8_t tileMode;            // +0x00
    int16_t numVerts;            // +0x02
    int16_t numTris;             // +0x04
    int16_t boneIndex;           // +0x06
    int32_t partBits[4];         // +0x08
    XBlendInfo* blends;          // +0x18
    uint16_t* triIndexes;        // +0x1C
    XVariantVertexInfo_u verts;  // +0x20
    float (*texCoords)[2];       // +0x24
    void* surfRigidARB;          // +0x28
    void* surfRigidATI;          // +0x2C
    void* surfRigidNV;           // +0x30
};
static_assert(sizeof(XSurface) == 0x34, "XSurface size mismatch");

enum XSurfTile_e : int32_t {
    XSURF_TILE_NONE = 0x0,
    XSURF_TILE_ACCIDENTAL = 0x1,
    XSURF_TILE_INTENTIONAL = 0x2,
};

struct DSurface_s {
    int16_t modelIndex;   // +0x00
    int16_t subMatIndex;  // +0x02
};
static_assert(sizeof(DSurface_s) == 4, "DSurface_s size mismatch");

struct DObjSkelMat;  // core_types.h (0x40 bytes)
