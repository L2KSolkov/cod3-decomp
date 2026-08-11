// ============================================================================
// ngl_dx_buf.cpp - work/push buffer allocation (4 funcs, verified vs IDA).
// Source: src/dx/ngl_dx_buf.cpp (ngl_xboxr)
// Data: nglPushBufferSize/nglPushBufferKickoffSize/nglWorkBuffers/
// nglWorkBufferA/nglWorkBufferB/nglWorkBuffersSize/nglWorkBuffersUsed.
// ============================================================================

#include "ngl/ngl_dx_buf.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_gpu.h"
#include "d3d8.h"

// ============================================================================
// Cross-object externs
// ============================================================================
extern class gpuD3DDevice* nglDev;                    // ngl_dx_core.o
extern void ngliWaitForResource(void);                // ngl_dx_core.o
extern unsigned char* nglListWork;                    // ngl_scene.o
extern unsigned char* nglListWorkPos;                 // ngl_scene.o
extern int nglListWorkSize;                           // ngl_scene.o
extern int nglScratchIndexBufferSize;                 // ngl_gpu_meshedit.o
extern int nglScratchVertexBufferSize;                // ngl_gpu_meshedit.o
extern D3DIndexBuffer* nglScratchIndexBufferA;        // ngl_gpu_meshedit.o
D3DIndexBuffer* nglScratchIndexBufferB = nullptr;     // ?nglScratchIndexBufferB (ngl_gpu_meshedit.o)
extern D3DVertexBuffer* nglScratchVertexBufferA;      // ngl_gpu_meshedit.o
D3DVertexBuffer* nglScratchVertexBufferB = nullptr;   // ?nglScratchVertexBufferB (ngl_gpu_meshedit.o)
extern void* tlMemAlloc(unsigned int Size, unsigned int Align, unsigned int Flags);
extern void tlMemFree(void* Ptr);
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// Data (ngl_dx_buf.o)
// ============================================================================
unsigned int nglPushBufferSize = 0x100000;
unsigned int nglPushBufferKickoffSize = 0x10000;
unsigned char* nglWorkBuffers = NULL;
unsigned char* nglWorkBufferA = NULL;
unsigned char* nglWorkBufferB = NULL;
unsigned int nglWorkBuffersSize = 0;
unsigned int nglWorkBuffersUsed = 0;

// ============================================================================
// nglGetUnusedWorkBuffer - ea: 0x850520
// ============================================================================
void nglGetUnusedWorkBuffer(void** Ptr, unsigned int* Size) {
    *Ptr = &nglWorkBuffers[nglWorkBuffersUsed];
    *Size = nglWorkBuffersSize - nglWorkBuffersUsed;
}

// ============================================================================
// nglXbSetPushBufferSize - ea: 0x850550
// ============================================================================
void nglXbSetPushBufferSize(unsigned int PushBufferSize, unsigned int PushBufferKickoffSize) {
    nglPushBufferSize = PushBufferSize;
    nglPushBufferKickoffSize = PushBufferKickoffSize;
}

// ============================================================================
// nglSetBufferSize - ea: 0x850570
// ============================================================================
void nglSetBufferSize(nglBufferType BufID, unsigned int Size, bool AllowResize) {
    unsigned int v3 = (Size + 3) & 0xFFFFFFFC;
    if (nglDev != NULL)
        ngliWaitForResource();
    if (BufID != NGLBUF_LIST_WORK) {
        if (BufID == NGLBUF_SCRATCH_INDEX) {
            if (nglScratchIndexBufferB != NULL) {
                D3DResource_Release((D3DResource*)nglScratchIndexBufferB);
                nglScratchIndexBufferB = NULL;
            }
            if (nglScratchIndexBufferA != NULL) {
                D3DResource_Release((D3DResource*)nglScratchIndexBufferA);
                nglScratchIndexBufferA = NULL;
            }
            if (v3 != 0)
                nglScratchIndexBufferA = D3DDevice_CreateIndexBuffer2(v3);
            nglScratchIndexBufferSize = (Size + 3) & 0xFFFFFFFC;
        } else if (BufID == NGLBUF_SCRATCH_VERTEX) {
            if (nglScratchVertexBufferB != NULL) {
                D3DResource_Release((D3DResource*)nglScratchVertexBufferB);
                nglScratchVertexBufferB = NULL;
            }
            if (nglScratchVertexBufferA != NULL) {
                D3DResource_Release((D3DResource*)nglScratchVertexBufferA);
                nglScratchVertexBufferA = NULL;
            }
            if (v3 != 0)
                nglScratchVertexBufferA = D3DDevice_CreateVertexBuffer2(v3);
            nglScratchVertexBufferSize = (Size + 3) & 0xFFFFFFFC;
        } else if (_tlAssert("src/dx/ngl_dx_buf.cpp", 136, "false",
                             "Invalid BufID passed to nglSetBufferSize !")) {
            __debugbreak();
        }
    } else {
        nglListWorkSize = (Size + 3) & 0xFFFFFFFC;
    }
    unsigned int v4 = nglListWorkSize;
    nglWorkBuffersUsed = nglListWorkSize;
    if (nglWorkBuffers == NULL || (nglListWorkSize != (int)nglWorkBuffersSize && AllowResize)) {
        if (nglWorkBufferA != NULL) {
            tlMemFree(nglWorkBufferA);
            v4 = nglWorkBuffersUsed;
            nglWorkBufferA = NULL;
        }
        if (nglWorkBufferB != NULL) {
            tlMemFree(nglWorkBufferB);
            v4 = nglWorkBuffersUsed;
            nglWorkBufferB = NULL;
        }
        if (v4 != 0 && v3 != 0) {
            nglWorkBuffersSize = v4;
            nglWorkBufferA = (unsigned char*)tlMemAlloc(v4, 8, 0x1000000);
            nglListWorkPos = nglWorkBufferA;
            nglListWork = nglWorkBufferA;
            v4 = nglWorkBuffersUsed;
        }
    }
    if (v4 > nglWorkBuffersSize
        && _tlAssert("src/dx/ngl_dx_buf.cpp", 165, "nglWorkBuffersUsed <= nglWorkBuffersSize",
                     "Not enough work buffer space available !"))
        __debugbreak();
}

// ============================================================================
// ngliInitWorkBuffers - ea: 0x850710
// ============================================================================
void ngliInitWorkBuffers() {
    if (nglListWork == NULL)
        nglSetBufferSize(NGLBUF_LIST_WORK, 0x40000, true);
    if (nglScratchIndexBufferSize == 0)
        nglSetBufferSize(NGLBUF_SCRATCH_INDEX, 0x100000, true);
    if (nglScratchVertexBufferSize == 0)
        nglSetBufferSize(NGLBUF_SCRATCH_VERTEX, 0x800000, true);
}
