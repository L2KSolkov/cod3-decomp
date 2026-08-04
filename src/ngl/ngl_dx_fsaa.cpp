// ============================================================================
// ngl_dx_fsaa.cpp — full-screen anti-aliasing mode selection (1 func + 4 data).
// Source: source/ngl_dx_fsaa.cpp
// Verified against IDA (ngl_xboxr:ngl_dx_fsaa.o):
//   nglDxSetFSAA   @0x850240 (?nglDxSetFSAA@@YA_NW4nglFSAAMode@@@Z)
//   nglFSAAScaleX  @0x1242050 (data, float)
//   nglFSAAScaleY  @0x1242054 (data, float)
//   nglFSAA        @0x1242058 (data, nglFSAAMode)
//   nglFSAAParams  @0x14D5E80 (data, math::Vector4)
// ============================================================================
#include "ngl_dx_fsaa.h"

#include <xmmintrin.h>

// ngl_internal.o (data, not yet ported -> /FORCE:UNRESOLVED)
extern struct nglDisplayModeType {
    int          Width;           // +0x00
    int          Height;          // +0x04
    bool         Progressive;     // +0x08
    bool         PAL;             // +0x09
    bool         Widescreen;      // +0x0A
    unsigned int ImpersonateMode; // +0x0C
} nglDisplayMode;

// ngl_dx_core.o (data, not yet ported -> /FORCE:UNRESOLVED)
extern _D3DPRESENT_PARAMETERS_ nglPresentParams;

// ngl_dx_core.o (func, not yet ported -> /FORCE:UNRESOLVED)
extern void nglDxResetDevice(void);

// tl_system.o (tl_xboxr, ported)
extern void tlWarning(const char* fmt, ...);

// ============================================================================
// Data globals owned by ngl_dx_fsaa.o
// ============================================================================
float         nglFSAAScaleX = 1.0f;
float         nglFSAAScaleY = 1.0f;
nglFSAAMode   nglFSAA = NGLFSAA_NONE;
math::Vector4 nglFSAAParams;

// ============================================================================
// nglDxSetFSAA — pick an FSAA mode, configure the scale/params and D3D state.
// ea: 0x850240
// ============================================================================
bool nglDxSetFSAA(nglFSAAMode mode) {
    if (nglDisplayMode.Width == 1920 && mode != NGLFSAA_NONE) {
        tlWarning("None of the multi/super sampling modes can be used with 1920x1080i !\n");
        return false;
    }

    bool result = true;
    unsigned int stateValue = 1;

    if (mode == NGLFSAA_NONE) {
        nglFSAAParams.v = _mm_setr_ps(1.0f, 1.0f, 0.53125f, 0.0f);
        nglFSAAScaleX = 1.0f;
        nglFSAAScaleY = 1.0f;
        stateValue = 0;
    } else if (mode == NGLFSAA_2X_MULTI_LINEAR || mode == NGLFSAA_2X_MULTI_QUINCUNX) {
        nglFSAAParams.v = _mm_setr_ps(1.0f, 1.0f, 0.03125f, 0.0f);
        nglFSAAScaleX = 2.0f;
        nglFSAAScaleY = 1.0f;
    } else if (mode == NGLFSAA_4X_MULTI_LINEAR || mode == NGLFSAA_4X_MULTI_GAUSSIAN) {
        nglFSAAParams.v = _mm_setr_ps(1.0f, 1.0f, 0.03125f, 0.0f);
        nglFSAAScaleX = 2.0f;
        nglFSAAScaleY = 2.0f;
    } else {
        // invalid mode -> fall back to NONE
        nglFSAAParams.v = _mm_setr_ps(1.0f, 1.0f, 0.53125f, 0.0f);
        nglFSAAScaleX = 1.0f;
        nglFSAAScaleY = 1.0f;
        mode = NGLFSAA_NONE;
        result = false;
        stateValue = 0;
        tlWarning("Invalid FSAA mode passed to nglDxSetFSAA, setting it to NGLFSAA_NONE !\n");
    }

    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_MULTISAMPLEANTIALIAS, stateValue) == 0)
        D3DDevice_SetRenderState_MultiSampleAntiAlias(stateValue);

    nglFSAA = mode;
    nglPresentParams.MultiSampleType = (unsigned int)mode;
    nglDxResetDevice();
    return result;
}
