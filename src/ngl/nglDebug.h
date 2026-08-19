// ============================================================================
// nglDebug - NGL debug flags + host scene-dump I/O API.
// Source: c:\cod\code\tl\ngl\include\nglDebug.h
// nglDebugStruct (17 bytes) verified against IDA local type (ngl_debug.o).
// ============================================================================

#ifndef COD3_NGL_NGL_DEBUG_H
#define COD3_NGL_NGL_DEBUG_H

#include "core/math_types.h"

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
extern int nglPerfBarNumVB;

// ============================================================================
// Debug-flag + profiling API (ngl_debug.o)
// ============================================================================
unsigned char* nglGetDebugFlagPtr(const char* Flag);
void nglSetDebugFlag(const char* Flag, unsigned char Set);
void nglSetDebugFlag(const char* Flag, int Set);
int nglGetDebugFlag(const char* Flag);
void nglDebugInit();
void nglAveragePerfInfo(unsigned int Frames);
void nglProfileShaders();
void nglDestroyProfiler();
class nglShader;
bool nglProfileEvalShader(nglShader* Shader);
void nglRenderPerfBar();
void nglFlushLinesBatch();
void nglInitShaderProfiling();

// Debug shape drawing (ngl_debug.o).
void nglDebugAddLine(const math::Position3* pt1, const math::Position3* pt2,
                     unsigned int color);
void nglDebugAddLine(const math::Position3& pt1, const math::Position3& pt2,
                     unsigned int color);
void nglDebugAddRay(const math::Position3* pt1, const math::Position3* pt2,
                    unsigned int Color);
void nglDebugAddRay(const math::Position3& pt1, const math::Position3& pt2,
                    unsigned int Color);
void nglDebugAddAxes(const math::Mat43* LToW, unsigned int Length,
                     unsigned int ColX, unsigned int ColY, unsigned int ColZ);
void nglDebugAddAxes(const math::Mat43& LToW, float Length,
                     unsigned int ColX, unsigned int ColY, unsigned int ColZ);
void nglDebugAddBox(const math::Mat43* mat, const math::DiagMat33* size,
                    unsigned int color);
void nglDebugAddBox(const math::Mat43& mat, const math::DiagMat33& size,
                    unsigned int color);
void nglDebugAddBoxSolid(const math::Mat43* LToW, const math::DiagMat33* size,
                         unsigned int Color);
void nglDebugAddBoxSolid(const math::Mat43& LToW, const math::DiagMat33& size,
                         unsigned int Color);
void nglDebugAddGrid(const math::Mat43* mat, int w, int h, float xstep,
                     float zstep, unsigned int color);
void nglDebugAddGrid(const math::Mat43& mat, int w, int h, float xstep,
                     float zstep, unsigned int color);
void nglDebugAddEllipse(const math::Mat43* LToW, float RadiusX, float RadiusZ,
                        unsigned int Color);
void nglDebugAddEllipse(const math::Mat43& LToW, float RadiusX, float RadiusZ,
                        unsigned int Color);
void nglDebugAddCircle(const math::Mat43* LToW, float Radius, unsigned int Color);
void nglDebugAddCircle(const math::Mat43& LToW, float Radius, unsigned int Color);
void nglDebugAddCylinder(const math::Mat43* LToW, float Radius, float Height,
                         unsigned int Color);
void nglDebugAddCylinder(const math::Mat43& LToW, float Radius, float Height,
                         unsigned int Color);
void nglDebugAddCylinderSolid(const math::Mat43* LToW, float Radius, float Height,
                              unsigned int Color);
void nglDebugAddCylinderSolid(const math::Mat43& LToW, float Radius, float Height,
                              unsigned int Color);
void nglDebugAddEllipsoid(const math::Mat43* LToW, const math::Dir3* Radius,
                          unsigned int Color);
void nglDebugAddEllipsoid(const math::Mat43& LToW, const math::Dir3& Radius,
                          unsigned int Color);
void nglDebugAddEllipsoidSolid(const math::Mat43* LToW, const math::Dir3* Radius,
                               unsigned int Color);
void nglDebugAddEllipsoidSolid(const math::Mat43& LToW, const math::Dir3& Radius,
                               unsigned int Color);
void nglDebugAddSphere(const math::Position3* Pos, float Radius,
                       unsigned int Color);
void nglDebugAddSphere(const math::Position3& Pos, float Radius,
                       unsigned int Color);
void nglDebugAddSphereSolid(const math::Position3* Pos, float Radius,
                            unsigned int Color);
void nglDebugAddSphereSolid(const math::Position3& Pos, float Radius,
                            unsigned int Color);
void nglDebugAddPyramid(const math::Mat43* LToW, const math::Dir3* Size,
                        unsigned int Color);
void nglDebugAddPyramid(const math::Mat43& LToW, const math::Dir3& Size,
                        unsigned int Color);
void nglDebugAddPyramidSolid(const math::Mat43* LToW, const math::Dir3* Size,
                             unsigned int Color);
void nglDebugAddPyramidSolid(const math::Mat43& LToW, const math::Dir3& Size,
                             unsigned int Color);
void nglDebugAddLabel(const math::Position3& Pos, unsigned int Color,
                      const char* Label, ...);
void nglDebugAddTriData(unsigned int NTris, const math::Position3* Points,
                        const unsigned char* Indices, unsigned int Color,
                        unsigned int Skip, unsigned int Mapping0,
                        unsigned int Mapping1, unsigned int Mapping2);
void nglDebugAddTriList(unsigned int NTris, const math::Position3* Points,
                        const unsigned char* Indices, unsigned int Color);
void nglDebugAddQuadList(unsigned int NQuads, const math::Position3* Points,
                         const unsigned char* Indices, unsigned int Color);
void nglDebugAddTriStrip(unsigned int NTris, const math::Position3* Points,
                         const unsigned char* Indices, unsigned int Color);
void nglDebugAddTri(const math::Position3* pt1, const math::Position3* pt2,
                    const math::Position3* pt3, unsigned int Color);
void nglDebugAddTri(const math::Position3& pt1, const math::Position3& pt2,
                    const math::Position3& pt3, unsigned int Color);
void nglDebugAddQuad(const math::Position3* pt1, const math::Position3* pt2,
                     const math::Position3* pt3, const math::Position3* pt4,
                     unsigned int Color);
void nglDebugAddQuad(const math::Position3& pt1, const math::Position3& pt2,
                     const math::Position3& pt3, const math::Position3& pt4,
                     unsigned int Color);

// ngl_dx_scenedump.o (host file I/O, ported in ngl_dx_scenedump.cpp)
void nglHostPrintf(void* File, const char* Format, ...);
void* nglHostOpen(const char* FileName);
void nglHostClose(void* File);

#endif // COD3_NGL_NGL_DEBUG_H
