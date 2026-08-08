// ============================================================================
// ngl_dx_buf.h - NGL work/push buffer management (ngl_dx_buf.o).
// Source: src/dx/ngl_dx_buf.cpp (ngl_xboxr)
// ============================================================================
#ifndef COD3_NGL_NGL_DX_BUF_H
#define COD3_NGL_NGL_DX_BUF_H

enum nglBufferType {
    NGLBUF_LIST_WORK = 0,
    NGLBUF_SCRATCH_INDEX = 1,
    NGLBUF_SCRATCH_VERTEX = 2,
};

// ngl_dx_buf.o (data, defined in ngl_dx_buf.cpp)
extern unsigned int nglPushBufferSize;
extern unsigned int nglPushBufferKickoffSize;
extern unsigned char* nglWorkBuffers;
extern unsigned char* nglWorkBufferA;
extern unsigned char* nglWorkBufferB;
extern unsigned int nglWorkBuffersSize;
extern unsigned int nglWorkBuffersUsed;

// ngl_dx_buf.o (functions, defined in ngl_dx_buf.cpp)
void nglGetUnusedWorkBuffer(void** Ptr, unsigned int* Size);
void nglXbSetPushBufferSize(unsigned int PushBufferSize, unsigned int PushBufferKickoffSize);
void nglSetBufferSize(nglBufferType BufID, unsigned int Size, bool AllowResize);
void ngliInitWorkBuffers();

#endif // COD3_NGL_NGL_DX_BUF_H
