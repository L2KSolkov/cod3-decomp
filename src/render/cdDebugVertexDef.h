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
void InitCDSimpleColorShader();
class cdWorldColorShader;
void InitCDWorldColorShader();
class cdSimpleShader;
void InitCDSimpleShader();
class cdDecalShader;
void InitCDDecalShader();
class cdGunShader;
void InitCDGunShader();
class cdCharShader;
void InitCDCharShader();
class cdPrelitShader;
void InitCDPrelitShader();
class cdWorldShader;
void InitCDWorldShader();
class cdSimpleAlphaShader;
void InitCDSimpleAlphaShader();
class cdSimplePrelitShader;
void InitCDSimplePrelitShader();
class cdSimpleSpecularShader;
void InitCDSimpleSpecularShader();
class cdSimpleUVAnimShader;
void InitCDSimpleUVAnimShader();
class cdBackgroundShader;
void InitCDBackgroundShader();
class cdDebugShader;
void InitCDDebugShader();
class cdWheelMarkShader;
void InitCDWheelMarkShader();
class cdFlagShader;
void InitCDFlagShader();
class cdDynamicDecalShader;
void InitCDDynamicDecalShader();
class cdPropellerShader;
void InitCDPropellerShader();
class cdGunSightShader;
void InitCDGunSightShader();
class cdGunSightSpecularShader;
void InitCDGunSightSpecularShader();
class cdCharSpecularShader;
void InitCDCharSpecularShader();
class cdAirplaneMetalShader;
void InitCDAirplaneMetalShader();
class cdScratchShader;
void InitCDScratchShader();
class cdWorldVertexLitShader;
void InitCDWorldVertexLitShader();
class cdWorldPointLitShader;
void InitCDWorldPointLitShader();

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
    friend void InitCDSimpleColorShader();
    friend class cdWorldColorShader;
    friend void InitCDWorldColorShader();
    friend class cdSimpleShader;
    friend void InitCDSimpleShader();
    friend class cdDecalShader;
    friend void InitCDDecalShader();
    friend class cdGunShader;
    friend void InitCDGunShader();
    friend class cdCharShader;
    friend void InitCDCharShader();
    friend class cdPrelitShader;
    friend void InitCDPrelitShader();
    friend class cdWorldShader;
    friend void InitCDWorldShader();
    friend class cdSimpleAlphaShader;
    friend void InitCDSimpleAlphaShader();
    friend class cdSimplePrelitShader;
    friend void InitCDSimplePrelitShader();
    friend class cdSimpleSpecularShader;
    friend void InitCDSimpleSpecularShader();
    friend class cdSimpleUVAnimShader;
    friend void InitCDSimpleUVAnimShader();
    friend class cdBackgroundShader;
    friend void InitCDBackgroundShader();
    friend class cdDebugShader;
    friend void InitCDDebugShader();
    friend class cdWheelMarkShader;
    friend void InitCDWheelMarkShader();
    friend class cdFlagShader;
    friend void InitCDFlagShader();
    friend class cdDynamicDecalShader;
    friend void InitCDDynamicDecalShader();
    friend class cdPropellerShader;
    friend void InitCDPropellerShader();
    friend class cdGunSightShader;
    friend void InitCDGunSightShader();
    friend class cdGunSightSpecularShader;
    friend void InitCDGunSightSpecularShader();
    friend class cdCharSpecularShader;
    friend void InitCDCharSpecularShader();
    friend class cdAirplaneMetalShader;
    friend void InitCDAirplaneMetalShader();
    friend class cdScratchShader;
    friend void InitCDScratchShader();
    friend class cdWorldVertexLitShader;
    friend void InitCDWorldVertexLitShader();
    friend class cdWorldPointLitShader;
    friend void InitCDWorldPointLitShader();
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
