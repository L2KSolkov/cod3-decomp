// ============================================================================
// cdScratchVertexDef.cpp — scratch vertex format builder (2 non-inline funcs).
// Source: source/cdScratchVertexDef.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdScratchVertexDef.o):
//   ProcessCDScratchVertexDef     @0x7C5350 (?ProcessCDScratchVertexDef@@YAXPAX@Z)
//   InitCDScratchVertexDefBuilder @0x7C5370 (?InitCDScratchVertexDefBuilder@@YAXXZ)
//   cdscratch_vertex_format       @0x14CD510 (data, gpuVertexFormat)
// ============================================================================
#include "cdDebugVertexDef.h"

#include <intrin.h>

// tl_xboxr (ported)
extern bool  _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// Data globals owned by cdScratchVertexDef.o
// ============================================================================
gpuVertexFormat cdscratch_vertex_format;

// File-local static vertex declaration (elem_0 @0xE3C2F0). Initialized data:
//   Input[0] = { 0, 0x00, Format=0x32 }   position
//   Input[1] = { 0, 0x0C, Format=0x22 }   offset 12
//   Input[2] = { 0, 0x14, Format=0x40 }   offset 20
//   Input[3+] = { 0, 0, Format=D3DVSDT_END(2), 0, 0 }
static _D3DVERTEXATTRIBUTEFORMAT elem_0 = {
    {
        { 0, 0, 0x32, 0, 0 },  // Input[0]
        { 0, 12, 0x22, 0, 0 }, // Input[1]
        { 0, 20, 0x40, 0, 0 }, // Input[2]
        { 0, 0, 2, 0, 0 },     // Input[3] — END
        { 0, 0, 2, 0, 0 },     // Input[4] — END
        { 0, 0, 2, 0, 0 },     // Input[5] — END
        { 0, 0, 2, 0, 0 },     // Input[6] — END
        { 0, 0, 2, 0, 0 },     // Input[7] — END
        { 0, 0, 2, 0, 0 },     // Input[8] — END
        { 0, 0, 2, 0, 0 },     // Input[9] — END
        { 0, 0, 2, 0, 0 },     // Input[10] — END
        { 0, 0, 2, 0, 0 },     // Input[11] — END
        { 0, 0, 2, 0, 0 },     // Input[12] — END
        { 0, 0, 2, 0, 0 },     // Input[13] — END
        { 0, 0, 2, 0, 0 },     // Input[14] — END
        { 0, 0, 2, 0, 0 },     // Input[15] — END
    }
};

// ============================================================================
// ProcessCDScratchVertexDef — JIV stub (assert only).
// ea: 0x7C5350
// ============================================================================
void ProcessCDScratchVertexDef(void* p) {
    if (_tlAssert("cdScratchVertexDef.cpp", 117, "false", "JIV STUB"))
        __debugbreak();
}

// ============================================================================
// InitCDScratchVertexDefBuilder — set up the scratch vertex format.
// ea: 0x7C5370
// ============================================================================
void InitCDScratchVertexDefBuilder() {
    cdscratch_vertex_format.VertexSize = 24;
    cdscratch_vertex_format.VertexDeclaration = &elem_0;
}
