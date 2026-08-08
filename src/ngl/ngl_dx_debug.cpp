// ============================================================================
// ngl_dx_debug.cpp - shader profiling + debug flag lookup (5 funcs).
// Source: src/xbox/ngl_dx_debug.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_dx_debug.o).
// ============================================================================

#include "ngl/ngl_dx_debug.h"

// tl_xboxr (ported)
extern void tlPrint(const char* text);
extern void tlPrintf(const char* fmt, ...);

// ============================================================================
// ngliGetDebugFlagPtr - ea: 0x851120
// ============================================================================
unsigned char* ngliGetDebugFlagPtr(const char* Flag) {
    (void)Flag;
    return NULL;   // flag lookup compiled out in release
}

// ============================================================================
// ngliShaderProfile::PrintHeader - ea: 0x851130
// ============================================================================
void ngliShaderProfile::PrintHeader() {
    tlPrint("  render     cpu     list  scratch    nodes  name\n");
}

// ============================================================================
// ngliShaderProfile::Print - ea: 0x851140
// ============================================================================
void ngliShaderProfile::Print() {
    tlPrintf("%7.2f  %7.2f  %7d  %7d  %7d  %s\n",
             RenderMS, CPUMS, ListBuffer, ScratchBuffer, NodeCount, Name.str);
}

// ============================================================================
// ngliShaderProfile::Record - ea: 0x851170
// ============================================================================
void ngliShaderProfile::Record() {
    RenderMS = nglPerfInfo.RenderMS;
    CPUMS = nglPerfInfo.ListSendMS;
    ListBuffer = (int)nglPerfInfo.ListWorkUsage;
    ScratchBuffer = (int)nglPerfInfo.PhysListWorkUsage;
    NodeCount = (int)nglSyncPerfInfo.NodeCount;
}

// ============================================================================
// ngliShaderProfile::Compare - ea: 0x8511B0
// ============================================================================
void ngliShaderProfile::Compare(ngliShaderProfile* Profile) {
    float v2 = RenderMS - Profile->RenderMS;
    if (v2 < 0.0f)
        v2 = 0.0f;
    RenderMS = v2;
    float v3 = CPUMS - Profile->CPUMS;
    if (v3 < 0.0f)
        v3 = 0.0f;
    CPUMS = v3;
    ListBuffer = ListBuffer - Profile->ListBuffer < 0 ? 0 : ListBuffer - Profile->ListBuffer;
    ScratchBuffer = ScratchBuffer - Profile->ScratchBuffer < 0
                  ? 0 : ScratchBuffer - Profile->ScratchBuffer;
    NodeCount = NodeCount - Profile->NodeCount < 0 ? 0 : NodeCount - Profile->NodeCount;
}
