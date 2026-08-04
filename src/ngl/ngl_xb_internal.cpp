// ============================================================================
// ngl_xb_internal.cpp — Xbox-internal NGL helpers (1 non-inline func + 1 data).
// Source: source/xbox/ngl_xb_internal.cpp
// Verified against IDA (ngl_xboxr:ngl_xb_internal.o):
//   CreateVertexDeclaration @0x8540B0 (?CreateVertexDeclaration@nglDirect3DDevice@@SAXPBU_D3DVERTEXSHADERINPUT@@PAPAU_D3DVERTEXATTRIBUTEFORMAT@@@Z)
//   nglXbPushQuad            @0x14D5FA8 (data, nglXbPushQuadClass)
//   nglXbPushQuadClass::ctor @0x8540A0 (inline COMDAT)
// ============================================================================
#include "ngl_xb_internal.h"

#include <intrin.h>

// tl_system.o (tl_xboxr, ported)
extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern bool  _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// Data globals owned by ngl_xb_internal.o
// ============================================================================
nglXbPushQuadClass nglXbPushQuad;

// D3DVSDT_END terminator value (Format == 2 marks the end of the element list)
static const unsigned int D3DVSDT_END = 2;

// ============================================================================
// nglDirect3DDevice::CreateVertexDeclaration — convert a D3D8 vertex shader
// input list into a vertex attribute format table. The input list is
// terminated by an element with Format == D3DVSDT_END (2); remaining table
// entries are filled with the terminator.
// ea: 0x8540B0
// ============================================================================
void nglDirect3DDevice::CreateVertexDeclaration(const _D3DVERTEXSHADERINPUT* pVertexElements,
                                                _D3DVERTEXATTRIBUTEFORMAT** ppDecl) {
    _D3DVERTEXATTRIBUTEFORMAT* decl = (_D3DVERTEXATTRIBUTEFORMAT*)tlMemAlloc(0x100, 8, 0x1000000);

    unsigned int count = 0;
    if (pVertexElements->Format != D3DVSDT_END) {
        do {
            ++count;
        } while (pVertexElements[count].Format != D3DVSDT_END);

        if (count > 0x10 && _tlAssert("src/xbox/ngl_xb_internal.cpp", 21,
                                      "NElements <= NMaxElements", "Too many vertex elements.")) {
            __debugbreak();
        }
    }

    if (count != 0) {
        for (unsigned int i = 0; i < count; ++i)
            decl->Input[i] = pVertexElements[i];
    }

    if (count < 0x10) {
        for (unsigned int i = count; i < 0x10; ++i) {
            decl->Input[i].StreamIndex = 0;
            decl->Input[i].Offset = 0;
            decl->Input[i].Format = D3DVSDT_END;
            decl->Input[i].TessType = 0;
            decl->Input[i].TessSource = 0;
        }
    }

    *ppDecl = decl;
}
