// ============================================================================
// nglDebug - NGL debug flags + host scene-dump I/O API.
// Source: c:\cod\code\tl\ngl\include\nglDebug.h
// nglDebugStruct (17 bytes) verified against IDA local type (ngl_debug.o).
// ============================================================================

#ifndef COD3_NGL_NGL_DEBUG_H
#define COD3_NGL_NGL_DEBUG_H

struct ngliDebugStruct {};   // empty base (size 1)

struct nglDebugStruct : ngliDebugStruct {
    unsigned char ShowPerfInfo;                    // +0x00
    unsigned char ShowPerfBar;                     // +0x01
    unsigned char ProfileShaders;                  // +0x02
    unsigned char ScreenShot;                      // +0x03
    unsigned char DisableQuads;                    // +0x04
    unsigned char DisableFonts;                    // +0x05
    unsigned char DisableMeshes;                   // +0x06
    unsigned char DisableVSync;                    // +0x07
    unsigned char DisableScratch;                  // +0x08
    unsigned char DebugPrints;                     // +0x09
    unsigned char DumpFrameLog;                    // +0x0A
    unsigned char DumpSceneFile;                   // +0x0B
    unsigned char DumpTextures;                    // +0x0C
    unsigned char DrawLightSpheres;                // +0x0D
    unsigned char DrawMeshSpheres;                 // +0x0E
    unsigned char DisableDuplicateMaterialWarning; // +0x0F
    unsigned char DisableMissingTextureWarning;    // +0x10
};
static_assert(sizeof(nglDebugStruct) == 0x11, "nglDebugStruct size mismatch");

// ============================================================================
// Perf info structs (ngl_debug.o data; layout from IDA, 128 bytes total).
// ============================================================================
struct ngliPerfInfoStruct {
    unsigned int ListWorkUsage;     // +0x00
    unsigned int ScratchWorkUsage;  // +0x04
    unsigned int PhysListWorkUsage; // +0x08
    float        QuadMS;            // +0x0C
    float        FontMS;            // +0x10
    unsigned char _pad14[4];        // +0x14
    unsigned __int64 CPUStart;              // +0x18
    volatile unsigned __int64 RenderStart;  // +0x20
    volatile unsigned __int64 RenderFinish; // +0x28
    unsigned __int64 ListSubmitCycles;      // +0x30
    unsigned __int64 ListSendCycles;        // +0x38
    unsigned __int64 QuadCycles;            // +0x40
    unsigned __int64 FontCycles;            // +0x48
};

struct nglPerfInfoStruct : ngliPerfInfoStruct {
    float        FPS;           // +0x50
    float        TotalMS;       // +0x54
    float        TotalSeconds;  // +0x58
    float        RenderMS;      // +0x5C
    float        CPUMS;         // +0x60
    float        FrameMS;       // +0x64
    float        ListSendMS;    // +0x68
    float        ListSubmitMS;  // +0x6C
    unsigned int TotalPolys;    // +0x70
    unsigned int TotalVerts;    // +0x74
    unsigned int NodeCount;     // +0x78
};
static_assert(sizeof(nglPerfInfoStruct) == 0x80, "nglPerfInfoStruct size mismatch");

extern nglPerfInfoStruct nglPerfInfo;      // ngl_debug.o
extern nglPerfInfoStruct nglSyncPerfInfo;  // ngl_debug.o

// ngl_debug.o (data, not yet ported - extern until ngl_debug ports)
extern nglDebugStruct nglSyncDebug;

// ngl_dx_scenedump.o (host file I/O, ported in ngl_dx_scenedump.cpp)
void nglHostPrintf(void* File, const char* Format, ...);
void* nglHostOpen(const char* FileName);
void nglHostClose(void* File);

#endif // COD3_NGL_NGL_DEBUG_H
