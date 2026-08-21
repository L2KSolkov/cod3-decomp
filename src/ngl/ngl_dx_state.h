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
    void SetSrcBlend(unsigned int v);   // ea: 0x7C8C90
    void SetDestBlend(unsigned int v);  // ea: 0x7C8CC0
    void SetBlendOp(unsigned int v);    // ea: 0x7C8CF0
    void FSAAFixup();                   // ea: 0x84FED0
    void Init();                        // ea: 0x84FEE0
    void SetBlendColor(unsigned int v); // ea: 0x850000; map/stack ABI is unsigned 32-bit
    // ea: 0x8504F0 (inline COMDAT)
    void SetMultiSampleAntiAlias(bool v) {
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_MULTISAMPLEANTIALIAS,
                                                     static_cast<unsigned int>(v)) == 0)
            D3DDevice_SetRenderState_MultiSampleAntiAlias(static_cast<unsigned int>(v));
    }
    int SetMaxAnisotropy(int stage, int v);  // ea: 0x841E70
};
static_assert(sizeof(nglDxRenderState) == 4, "nglDxRenderState size mismatch");

// ngl_dx_state.o (data, defined in ngl_dx_state.cpp)
extern nglDxRenderState nglDxState;

#endif // COD3_NGL_NGL_DX_STATE_H
