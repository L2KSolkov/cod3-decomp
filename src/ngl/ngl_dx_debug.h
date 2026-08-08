// ============================================================================
// ngliShaderProfile - per-shader render profiling record (52 bytes, IDA type).
// Source: c:\cod\code\tl\ngl\include\dx\ngl_dx_debug.h
// nglPerfInfoStruct / ngliPerfInfoStruct are owned by ngl_debug.o; only the
// fields used by ngl_dx_debug are declared here.
// ============================================================================

#ifndef COD3_NGL_NGL_DX_DEBUG_H
#define COD3_NGL_NGL_DX_DEBUG_H

#include "core/tlFixedString.h"

// ngl_debug.o (data, not yet ported)
struct ngliPerfInfoStruct {
    unsigned int ListWorkUsage;     // +0x00
    unsigned int ScratchWorkUsage;  // +0x04
    unsigned int PhysListWorkUsage; // +0x08
    float        QuadMS;            // +0x0C
    float        FontMS;            // +0x10
};

struct nglPerfInfoStruct : ngliPerfInfoStruct {
    float        FPS;               // +0x14
    float        TotalMS;           // +0x18
    float        TotalSeconds;      // +0x1C
    float        RenderMS;          // +0x20
    float        CPUMS;             // +0x24
    float        FrameMS;           // +0x28
    float        ListSendMS;        // +0x2C
    float        ListSubmitMS;      // +0x30
    unsigned int TotalPolys;        // +0x34
    unsigned int TotalVerts;        // +0x38
    unsigned int NodeCount;         // +0x3C
};

extern nglPerfInfoStruct nglPerfInfo;      // ngl_debug.o
extern nglPerfInfoStruct nglSyncPerfInfo;  // ngl_debug.o

// ============================================================================
// ngliShaderProfile - 52 bytes
// ============================================================================
struct ngliShaderProfile {
    tlFixedString Name;         // +0x00
    float         RenderMS;     // +0x20
    float         CPUMS;        // +0x24
    int           ListBuffer;   // +0x28
    int           ScratchBuffer;// +0x2C
    int           NodeCount;    // +0x30

    static void PrintHeader();                 // ea: 0x851130
    void Print();                              // ea: 0x851140
    void Record();                             // ea: 0x851170
    void Compare(ngliShaderProfile* Profile);  // ea: 0x8511B0
};
static_assert(sizeof(ngliShaderProfile) == 0x34, "ngliShaderProfile size mismatch");

// ============================================================================
// ngliGetDebugFlagPtr - ea: 0x851120 (compiled out in release)
// ============================================================================
unsigned char* ngliGetDebugFlagPtr(const char* Flag);

#endif // COD3_NGL_NGL_DX_DEBUG_H
