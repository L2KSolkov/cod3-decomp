// ============================================================================
// ngl_xb_internal.h — Xbox-internal NGL helpers (vertex declarations, push quad).
// Source: source/xbox/ngl_xb_internal.cpp
//
// Owned by ngl_xboxr:ngl_xb_internal.o:
//   nglXbPushQuad           @0x14D5FA8 (data, nglXbPushQuadClass)
//   nglXbPushQuadClass::ctor (inline COMDAT)
// ============================================================================
#ifndef COD3_NGL_NGL_XB_INTERNAL_H
#define COD3_NGL_NGL_XB_INTERNAL_H

#include "d3d8.h"

// ============================================================================
// nglXbPushQuadClass — push-buffer quad writer (4 bytes, verified against IDA)
// ============================================================================
class nglXbPushQuadClass {
public:
    unsigned int* PB;  // +0x00

    nglXbPushQuadClass() : PB(0) {}
};
static_assert(sizeof(nglXbPushQuadClass) == 4, "nglXbPushQuadClass size mismatch");

// ngl_xb_internal.o (data, defined in ngl_xb_internal.cpp)
extern nglXbPushQuadClass nglXbPushQuad;

// ============================================================================
// nglDirect3DDevice — Xbox D3D8 device wrapper (static helpers)
// ============================================================================
class nglDirect3DDevice {
public:
    // Build a D3D vertex attribute format table from a vertex shader input list.
    // ea: 0x8540B0
    static void CreateVertexDeclaration(const _D3DVERTEXSHADERINPUT* pVertexElements,
                                        _D3DVERTEXATTRIBUTEFORMAT** ppDecl);
};

#endif // COD3_NGL_NGL_XB_INTERNAL_H
