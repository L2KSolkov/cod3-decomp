// ============================================================================
// cdDebugVertexDef.h — debug vertex format builder + tl init-list function.
// Source: source/cdDebugVertexDef.cpp (render_xboxr)
//
// Owned by render_xboxr:cdDebugVertexDef.o:
//   cddebug_vertex_format    @0x14CD4EC (data, gpuVertexFormat)
//   tlInitListFunction       (inline COMDAT class)
// ============================================================================
#ifndef COD3_RENDER_CDDEBUGVERTEXDEF_H
#define COD3_RENDER_CDDEBUGVERTEXDEF_H

#include <intrin.h>

#include "d3d8.h"

// ============================================================================
// gpuVertexFormat — GPU vertex format descriptor (12 bytes, verified against IDA)
// ============================================================================
struct gpuVertexFormat {
    int                          VertexSize;         // +0x00
    const _D3DVERTEXSHADERINPUT* Elements;           // +0x04
    _D3DVERTEXATTRIBUTEFORMAT*   VertexDeclaration;  // +0x08
};
static_assert(sizeof(gpuVertexFormat) == 0x0C, "gpuVertexFormat size mismatch");

// ============================================================================
// gpuCreateVertexFormat — build a gpuVertexFormat from a D3D8 vertex element
// list (terminated by Format == D3DVSDT_END). Inline COMDAT in cdGlowShader.o.
// ea: 0x7C2670
// ============================================================================
inline gpuVertexFormat* gpuCreateVertexFormat(gpuVertexFormat* result, unsigned int size,
                                              const _D3DVERTEXSHADERINPUT* elements) {
    extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
    extern bool  _tlAssert(const char* file, int line, const char* expr, const char* desc);

    _D3DVERTEXATTRIBUTEFORMAT* decl = (_D3DVERTEXATTRIBUTEFORMAT*)tlMemAlloc(0x100, 8, 0);
    const _D3DVERTEXSHADERINPUT* src = elements;

    unsigned int n = 0;
    if (elements->Format != 2) {
        do {
            if (n >= 0x10 && _tlAssert("c:\\cod\\code\\tl\\ngl\\include\\dx/ngl_dx_gpu.h", 211,
                                       "n < 16", "Too many vertex elements."))
                __debugbreak();
            decl->Input[n].StreamIndex = src->StreamIndex;
            decl->Input[n].Offset = src->Offset;
            decl->Input[n].Format = src->Format;
            decl->Input[n].TessType = src->TessType;
            ++src;
            ++n;
        } while (src->Format != 2);
    }

    if (n < 0x10) {
        for (unsigned int i = n; i < 0x10; ++i) {
            decl->Input[i].StreamIndex = 0;
            decl->Input[i].Offset = 0;
            decl->Input[i].Format = 2;
            decl->Input[i].TessType = 0;
            decl->Input[i].TessSource = 0;
        }
    }

    result->VertexSize = size;
    result->Elements = elements;
    result->VertexDeclaration = decl;
    return result;
}

// ============================================================================
// tlInitList — base of the init-list chain (8 bytes, verified against IDA).
// class keyword => 'V' mangling; head is private static (mangled @@0PAV1@A),
// owned by tl_xboxr:tl_initlist.o. Virtual dtor => real vtable.
// ============================================================================
class tlInitList {
public:
    tlInitList* next;  // +0x04 (vtable at +0x00)

    tlInitList() : next(0) {}
    virtual ~tlInitList() {}

private:
    static tlInitList* head;  // ?head@tlInitList@@0PAV1@A (tl_initlist.o)

    friend class tlInitListFunction;
};
static_assert(sizeof(tlInitList) == 8, "tlInitList size mismatch");

// ============================================================================
// tlInitListFunction — init-list entry that invokes a function on register
// (12 bytes, verified against IDA). Inline COMDAT in cdDebugVertexDef.o.
// ============================================================================
class tlInitListFunction : public tlInitList {
public:
    void (__cdecl* initfn)();  // +0x08

    tlInitListFunction(void (__cdecl* fn)());  // ea: 0x7C4B00
    virtual ~tlInitListFunction();             // ea: 0x7C4B30
    virtual void Register();                    // ea: 0x7C4B40
};
static_assert(sizeof(tlInitListFunction) == 0x0C, "tlInitListFunction size mismatch");

#endif // COD3_RENDER_CDDEBUGVERTEXDEF_H
