// ============================================================================
// ngl_dx_state.cpp - cached D3D render/texture state management (6 funcs).
// Source: src/dx/ngl_dx_state.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_dx_state.o).
// The dword_403xx cells are ngl's pushbuffer method-encode table (.textbss,
// filled at D3D init); the dword_BC2xxx cells are the XDK D3D8 library's
// internal state cache (shim externs - never implemented here).
// ============================================================================

#include "ngl/ngl_dx_state.h"

// ============================================================================
// Globals (data)
// ============================================================================
nglDxRenderState nglDxState;

// ngl pushbuffer method-encode table (.textbss, filled by ngl_dx_core init).
extern unsigned int dword_40300;
extern unsigned int dword_40304;
extern unsigned int dword_4033C;
extern unsigned int dword_40340;
extern unsigned int dword_40344;
extern unsigned int dword_40348;
extern unsigned int dword_4034C;
extern unsigned int dword_40350;

// XDK D3D8 internal state cache (shim).
extern unsigned int D3D__DirtyFlags;
extern unsigned int D3D__TextureState[4][32];
extern unsigned int dword_BC2CF8;   // ALPHAFUNC cache
extern unsigned int dword_BC2CFC;   // ALPHABLENDENABLE cache
extern unsigned int dword_BC2D00;   // ALPHATESTENABLE cache
extern unsigned int dword_BC2D04;   // ALPHAREF cache
extern unsigned int dword_BC2D08;   // SRCBLEND cache
extern unsigned int dword_BC2D0C;   // DESTBLEND cache
extern unsigned int dword_BC2D38;   // BLENDOP cache
extern unsigned int dword_BC2D3C;   // BLENDCOLOR cache
extern unsigned int dword_BC2DAC;   // SPECULARENABLE cache

// ============================================================================
// nglDxRenderState::Reset - ea: 0x84FC90
// ============================================================================
void nglDxRenderState::Reset() {
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SPECULARENABLE, 1u) == 0) {
        D3D__DirtyFlags |= 0x3000u;
        dword_BC2DAC = 1;
    }
    PrevBM = -1;
}

// ============================================================================
// nglDxRenderState::SetBlendMode - ea: 0x84FCC0
// BM packs blend op/src/dst/alpha-test in a Treyarch bitfield (kept verbatim).
// ============================================================================
void nglDxRenderState::SetBlendMode(unsigned int BM) {
    if (BM == PrevBM)
        return;
    PrevBM = BM;

    if ((BM & 0x20000) != 0) {
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 1u) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40304, 1u);
            dword_BC2CFC = 1;
        }
        unsigned int v3 = (BM & 0x8000FFFF | ((BM & 0x60000000 | (BM >> 4) & 0x1E00000) >> 5)) >> 16;
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_BLENDOP, BM & 0xF000 | (BM >> 8) & 0xF) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40350, BM & 0xF000 | (BM >> 8) & 0xF);
            dword_BC2D38 = BM & 0xF000 | (BM >> 8) & 0xF;
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SRCBLEND, v3) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40344, v3);
            dword_BC2D08 = (BM & 0x8000FFFF | ((BM & 0x60000000 | (BM >> 4) & 0x1E00000) >> 5)) >> 16;
        }
        unsigned int dst = (0x1000000 & BM | (((0xBFFFFC + 4) & BM | 0x3C000 & (BM >> 4)) >> 5)) >> 9;
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_DESTBLEND, dst) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40348, dst);
            dword_BC2D0C = (0x1000000 & BM | (((0xBFFFFC + 4) & BM | 0x3C000 & (BM >> 4)) >> 5)) >> 9;
        }
        if ((v3 == 32771 || v3 == 32772 || dst == 32771 || dst == 32772) &&
            D3DDevice_SetRenderState_ParameterCheck(D3DRS_BLENDCOLOR, BM << 24) == 0) {
            D3DDevice_SetRenderState_Simple(dword_4034C, BM << 24);
            dword_BC2D3C = BM << 24;
        }
    } else if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHABLENDENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40304, 0);
        dword_BC2CFC = 0;
    }

    if ((BM & 0x10000) != 0) {
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 1u) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40300, 1u);
            dword_BC2D00 = 1;
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAFUNC, 0x204u) == 0) {
            D3DDevice_SetRenderState_Simple(dword_4033C, 0x204u);
            dword_BC2CF8 = 516;
        }
        if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHAREF, BM) == 0) {
            D3DDevice_SetRenderState_Simple(dword_40340, BM);
            dword_BC2D04 = BM;
        }
    } else if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_ALPHATESTENABLE, 0) == 0) {
        D3DDevice_SetRenderState_Simple(dword_40300, 0);
        dword_BC2D00 = 0;
    }
}

// ============================================================================
// nglDxRenderState::FSAAFixup - ea: 0x84FED0
// ============================================================================
void nglDxRenderState::FSAAFixup() {
    PrevBM = -1;
}

// ============================================================================
// nglDxRenderState::Init - ea: 0x84FEE0
// ============================================================================
void nglDxRenderState::Init() {
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SPECULARENABLE, 1u) == 0) {
        D3D__DirtyFlags |= 0x3000u;
        dword_BC2DAC = 1;
    }
    PrevBM = -1;
    for (unsigned int v2 = 0; v2 < 4; ++v2) {
        if (D3DDevice_SetTextureState_ParameterCheck(v2, D3DTSS_DEFERRED_TEXTURE_STATE_MAX, 1u) == 0) {
            D3D__DirtyFlags |= 0x800u;
            D3D__TextureState[v2][D3DTSS_DEFERRED_TEXTURE_STATE_MAX] = 1;
        }
        if (D3DDevice_SetTextureState_ParameterCheck(v2, D3DTSS_ALPHAOP, 1u) == 0) {
            D3D__DirtyFlags |= 0x800u;
            D3D__TextureState[v2][D3DTSS_ALPHAOP] = 1;
        }
        if (D3DDevice_SetTextureState_ParameterCheck(v2, D3DTSS_TEXTURETRANSFORMFLAGS, 0) == 0) {
            D3D__DirtyFlags |= 0x400u;
            D3D__TextureState[v2][D3DTSS_TEXTURETRANSFORMFLAGS] = 0;
        }
        if (D3DDevice_SetTextureState_ParameterCheck(v2, D3DTSS_MINFILTER, 2u) == 0) {
            D3D__TextureState[v2][D3DTSS_MINFILTER] = 2;
            D3D__DirtyFlags |= 1 << v2;
        }
        if (D3DDevice_SetTextureState_ParameterCheck(v2, D3DTSS_MAGFILTER, 2u) == 0) {
            D3D__TextureState[v2][D3DTSS_MAGFILTER] = 2;
            D3D__DirtyFlags |= 1 << v2;
        }
        if (D3DDevice_SetTextureState_ParameterCheck(v2, D3DTSS_MIPFILTER, 2u) == 0) {
            D3D__TextureState[v2][D3DTSS_MIPFILTER] = 2;
            D3D__DirtyFlags |= 1 << v2;
        }
    }
}

// ============================================================================
// nglDxRenderState::SetBlendColor - ea: 0x850000
// ============================================================================
void nglDxRenderState::SetBlendColor(int v) {
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_BLENDCOLOR, v) == 0) {
        D3DDevice_SetRenderState_Simple(dword_4034C, v);
        dword_BC2D3C = v;
    }
}

// ============================================================================
// nglDxRenderState::SetMaxAnisotropy - ea: 0x841E70
// ============================================================================
int nglDxRenderState::SetMaxAnisotropy(unsigned int stage, unsigned int v) {
    int result = D3DDevice_SetTextureState_ParameterCheck(stage, D3DTSS_MAXANISOTROPY, v);
    if (result == 0) {
        D3D__DirtyFlags |= 1 << stage;
        D3D__TextureState[stage][D3DTSS_MAXANISOTROPY] = v;
        return 1 << stage;
    }
    return result;
}
