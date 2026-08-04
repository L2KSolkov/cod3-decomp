// ============================================================================
// ngl_dx_fsaa.h — FSAA (full-screen anti-aliasing) mode definitions.
// Source: c:\cod\code\tl\ngl_xboxr\include\ngl_dx_fsaa.h
//
// Owned by ngl_xboxr:ngl_dx_fsaa.o. Data globals:
//   nglFSAAScaleX/Y    @0x1242050 / 0x1242054 (map)
//   nglFSAA            @0x1242058 (map)
//   nglFSAAParams      @0x14D5E80 (map)
// ============================================================================
#ifndef COD3_NGL_NGL_DX_FSAA_H
#define COD3_NGL_NGL_DX_FSAA_H

#include "core/math_types.h"
#include "d3d8.h"

// ============================================================================
// nglFSAAMode — full-screen anti-aliasing mode (all values from IDA).
// Values are the Xbox D3D8 multisample-type encodings passed straight into
// nglPresentParams.MultiSampleType.
// ============================================================================
enum nglFSAAMode {
    NGLFSAA_NONE                = 17,       // 0x11
    NGLFSAA_2X_MULTI_LINEAR     = 4129,     // 0x1021
    NGLFSAA_4X_MULTI_LINEAR     = 4130,     // 0x1022
    NGLFSAA_2X_MULTI_QUINCUNX   = 4385,     // 0x1121
    NGLFSAA_4X_MULTI_GAUSSIAN   = 4642,     // 0x1222
};

// ngl_dx_fsaa.o (data, defined in ngl_dx_fsaa.cpp)
extern float        nglFSAAScaleX;
extern float        nglFSAAScaleY;
extern nglFSAAMode  nglFSAA;
extern math::Vector4 nglFSAAParams;

// ============================================================================
// nglDxSetFSAA — select an FSAA mode; returns false if invalid.
// ea: 0x850240
// ============================================================================
bool nglDxSetFSAA(nglFSAAMode mode);

#endif // COD3_NGL_NGL_DX_FSAA_H
