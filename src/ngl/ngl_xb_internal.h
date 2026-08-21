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
extern unsigned int dword_417FC;
extern unsigned int gpuHashVertexBuffer;
extern unsigned int gpuHashVertexFormat;
extern unsigned int gpuHashIndexBuffer;

class nglXbPushQuadClass {
public:
    unsigned int* PB;  // +0x00

    nglXbPushQuadClass() : PB(0) {}

    void End() {
        *PB = dword_417FC;
        PB[1] = 0;
        PB += 2;
        D3DDevice_EndPush(PB);
    }

    void Pos(float x, float y, float z) {
        *PB++ = *(unsigned int*)&x;
        *PB++ = *(unsigned int*)&y;
        *PB++ = *(unsigned int*)&z;
    }

    void Color(unsigned int ColorValue) {
        *PB++ = ColorValue;
    }

    void UV(float u, float v) {
        *PB++ = *(unsigned int*)&u;
        *PB++ = *(unsigned int*)&v;
    }

    unsigned int* Begin(unsigned int VertexSize) {
        gpuHashVertexBuffer = 0;
        gpuHashVertexFormat = 0;
        D3DDevice_SetVertexShaderInputDirect(NULL, 0, NULL);
        gpuHashIndexBuffer = 0;
        D3DDevice_SetIndices(NULL, 0);
        unsigned int v3 = (4 * VertexSize) >> 2;
        PB = D3DDevice_BeginPush(v3 + 5);
        *PB = dword_417FC;
        PB[1] = 8;
        unsigned int* result = PB;
        PB[2] = (v3 << 18) + 1073747992;
        PB += 3;
        return result;
    }
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
