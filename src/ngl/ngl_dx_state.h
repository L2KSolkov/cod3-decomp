// ============================================================================
// nglDxRenderState - cached D3D blend/alpha render state (4 bytes).
// Source: c:\cod\code\tl\ngl\include\dx\ngl_dx_state.h
// Layout from IDA: { unsigned int PrevBM; } (4 bytes).
// ============================================================================

#ifndef COD3_NGL_NGL_DX_STATE_H
#define COD3_NGL_NGL_DX_STATE_H

#include "d3d8.h"

struct nglDxRenderState {
    unsigned int PrevBM;  // +0x00

    void Reset();                       // ea: 0x84FC90
    void SetBlendMode(unsigned int BM); // ea: 0x84FCC0
    void FSAAFixup();                   // ea: 0x84FED0
    void Init();                        // ea: 0x84FEE0
    void SetBlendColor(int v);          // ea: 0x850000
    int SetMaxAnisotropy(unsigned int stage, unsigned int v);  // ea: 0x841E70
};
static_assert(sizeof(nglDxRenderState) == 4, "nglDxRenderState size mismatch");

// ngl_dx_state.o (data, defined in ngl_dx_state.cpp)
extern nglDxRenderState nglDxState;

#endif // COD3_NGL_NGL_DX_STATE_H
