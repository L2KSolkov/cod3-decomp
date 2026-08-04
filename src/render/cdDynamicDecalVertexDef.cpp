// ============================================================================
// cdDynamicDecalVertexDef.cpp — dynamic decal vertex format builder
// (2 non-inline funcs).
// Source: source/cdDynamicDecalVertexDef.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdDynamicDecalVertexDef.o):
//   ProcessCDDynamicDecalVertexDef     @0x7CD440 (?ProcessCDDynamicDecalVertexDef@@YAXPAX@Z)
//   InitCDDynamicDecalVertexDefBuilder @0x7CD460 (?InitCDDynamicDecalVertexDefBuilder@@YAXXZ)
//   cdDynamicDecalVertexFormat         @0x14CD9AC (data, gpuVertexFormat)
// ============================================================================
#include "cdDebugVertexDef.h"

#include <intrin.h>

// tl_xboxr (ported)
extern bool  _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// Data globals owned by cdDynamicDecalVertexDef.o
// ============================================================================
gpuVertexFormat cdDynamicDecalVertexFormat;

// File-local static vertex element list (cdDynamicDecalVertexElements @0xE3C8C0).
// Initialized data:
//   [0] = { 0, 0x00, Format=0x32 }   position
//   [1] = { 0, 0x0C, Format=0x22 }   offset 12
//   [2] = { 0, 0x14, Format=0x40 }   offset 20
//   [3] = { 0, 0, Format=D3DVSDT_END(2), 0, 0 }
static _D3DVERTEXSHADERINPUT cdDynamicDecalVertexElements[] = {
    { 0, 0, 0x32, 0, 0 },  // [0]
    { 0, 12, 0x22, 0, 0 }, // [1]
    { 0, 20, 0x40, 0, 0 }, // [2]
    { 0, 0, 2, 0, 0 },     // [3] — END
};

// ============================================================================
// ProcessCDDynamicDecalVertexDef — JIV stub (assert only).
// ea: 0x7CD440
// ============================================================================
void ProcessCDDynamicDecalVertexDef(void* p) {
    if (_tlAssert("cdDynamicDecalVertexDef.cpp", 102, "false", "JIV STUB"))
        __debugbreak();
}

// ============================================================================
// InitCDDynamicDecalVertexDefBuilder — build the dynamic decal vertex format.
// ea: 0x7CD460
// ============================================================================
_D3DVERTEXATTRIBUTEFORMAT* InitCDDynamicDecalVertexDefBuilder() {
    gpuVertexFormat v2;
    gpuVertexFormat* v0 = gpuCreateVertexFormat(&v2, 0x18, cdDynamicDecalVertexElements);
    cdDynamicDecalVertexFormat.VertexSize = v0->VertexSize;
    cdDynamicDecalVertexFormat.Elements = v0->Elements;
    cdDynamicDecalVertexFormat.VertexDeclaration = v0->VertexDeclaration;
    return v0->VertexDeclaration;
}
