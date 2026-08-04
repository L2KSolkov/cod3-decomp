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
#include "ngl/ngl_dx_gpu.h"

// Forward declarations for tlInitList friends (defined in aeps headers)
class apsSimpleMeshShader;
class apsSimpleMeshRenderer;
class nglShader;
class cdSimpleColorShader;
cdSimpleColorShader* InitCDSimpleColorShader();
class cdWorldColorShader;
cdWorldColorShader* InitCDWorldColorShader();

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
    friend class apsSimpleMeshShader;
    friend class apsSimpleMeshRenderer;
    friend class nglShader;
    friend class cdSimpleColorShader;
    friend cdSimpleColorShader* InitCDSimpleColorShader();
    friend class cdWorldColorShader;
    friend cdWorldColorShader* InitCDWorldColorShader();
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
