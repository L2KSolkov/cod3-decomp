// ============================================================================
// cdDebugVertexDef.cpp — debug vertex format builder (2 non-inline funcs).
// Source: source/cdDebugVertexDef.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdDebugVertexDef.o):
//   ProcessCDDebugVertexDef       @0x7C4B80 (?ProcessCDDebugVertexDef@@YAXPAX@Z)
//   InitCDDebugVertexDefBuilder   @0x7C4BA0 (?InitCDDebugVertexDefBuilder@@YAXXZ)
//   cddebug_vertex_format         @0x14CD4EC (data, gpuVertexFormat)
//   tlInitListFunction            (inline COMDAT, defined in header)
// ============================================================================
#include "cdDebugVertexDef.h"

#include <intrin.h>

// tl_xboxr (ported)
extern bool  _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// Data globals owned by cdDebugVertexDef.o
// ============================================================================
gpuVertexFormat cddebug_vertex_format;

// File-local static vertex declaration (elem @0xE3C198). Initialized data:
//   Input[0]  = { StreamIndex=0, Offset=0, Format=0x32, TessType=0, TessSource=0 }
//   Input[1+] = { 0, 0, Format=D3DVSDT_END(2), 0, 0 }
static _D3DVERTEXATTRIBUTEFORMAT elem = {
    {
        { 0, 0, 0x32, 0, 0 },  // Input[0] — position (VertexSize=12)
        { 0, 0, 2, 0, 0 },     // Input[1] — END
        { 0, 0, 2, 0, 0 },     // Input[2] — END
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
// tlInitListFunction — init-list entry (inline COMDAT in this object).
// Register() invokes the registered function.
// ============================================================================
tlInitListFunction::tlInitListFunction(void (__cdecl* fn)()) {
    next = tlInitList::head;
    tlInitList::head = this;
    initfn = fn;
}

tlInitListFunction::~tlInitListFunction() {
}

void tlInitListFunction::Register() {
    initfn();
}

// ============================================================================
// ProcessCDDebugVertexDef — JIV stub (assert only).
// ea: 0x7C4B80
// ============================================================================
void ProcessCDDebugVertexDef(void* p) {
    if (_tlAssert("cdDebugVertexDef.cpp", 88, "false", "JIV STUB"))
        __debugbreak();
}

// ============================================================================
// InitCDDebugVertexDefBuilder — set up the debug vertex format.
// ea: 0x7C4BA0
// ============================================================================
void InitCDDebugVertexDefBuilder() {
    cddebug_vertex_format.VertexSize = 12;
    cddebug_vertex_format.VertexDeclaration = &elem;
}
