// ============================================================================
// ngl_dx_core.h - D3D device core types + ngl_dx_core.o data/functions.
// Source: src/dx/ngl_dx_core.cpp (ngl_xboxr)
// nglFrameLockType values verified against the nglDxSetFrameLockParams switch
// (IDA disasm 0x8404F0); nglDisplayModeType lives in ngl_dx_quad.h.
// ============================================================================
#ifndef COD3_NGL_NGL_DX_CORE_H
#define COD3_NGL_NGL_DX_CORE_H

#include "d3d8.h"

struct nglDisplayModeType;
class gpuD3DDevice;
struct jqBatch;
namespace apk {
class apkFile;
struct apkFileSection;
}

enum nglFrameLockType {
    NGLFL_NONE = 0,
    NGLFL_ONE = 1,
    NGLFL_TWO = 2,
    NGLFL_ONE_OR_IMMEDIATE = 3,
    NGLFL_TWO_OR_IMMEDIATE = 4,
};

// ngl_dx_core.o (data, defined in ngl_dx_core.cpp)
extern unsigned char nglGammaRamp[0x300];
extern int nglFenceEndOfRendering;
extern nglFrameLockType nglFrameLock;
extern nglFrameLockType nglPrevFrameLock;
extern unsigned int nglXbDisplayModeFlag[7];
extern _D3DPRESENT_PARAMETERS_ nglPresentParams;
extern _D3DCAPS8 nglDxCaps;
extern Direct3D* nglD3D;
extern gpuD3DDevice* nglDev;
extern unsigned int nglFrameLockImmediate;
extern unsigned int nglFlipCycle;
extern unsigned int nglLastFlipCycle;

// ngl_dx_core.o (functions, defined in ngl_dx_core.cpp)
long nglDxCheckErrorD3D(long Status, const char* FileName, unsigned int Line);
void nglFlip();
void nglSetDisplayMode(unsigned int* Modes, unsigned int ModeCount);
void nglXbSetGammaRamp(unsigned char* Ramp);
void nglDxInitOcclusionQuery();
void nglXbInitPushBufferSize();
void nglDxInitDisplayMode();
void nglDxInitPresentParams();
void nglRenderStartCallback(unsigned long Param);
void nglRenderFinishCallback(unsigned long Param);
void nglVBlankCallback(_D3DVBLANKDATA* VBlankData);
void ngliLoadPhysicalSection(apk::apkFile* File, apk::apkFileSection* Section, void* UserData);
void ngliUnloadPhysicalSection(apk::apkFile* File, apk::apkFileSection* Section, void* UserData);
void ngliPreInit();
void ngliPostInit();
void ngliExit();
void nglDxSetFrameLockParams(nglFrameLockType flock);
nglFrameLockType nglSetFrameLock(nglFrameLockType flock);
void nglWaitForRendering();
void ngliListInit();
void nglRenderPerfInfo();
void nglRenderDebug();
void nglListSendBatch(jqBatch* Batch);
void ngliWaitForResource();
void nglDxResetDevice();

#endif // COD3_NGL_NGL_DX_CORE_H
