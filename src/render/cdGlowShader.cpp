// ============================================================================
// cdGlowShader.cpp — glow shader (7 non-inline funcs).
// Source: source/cdGlowShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdGlowShader.o):
//   ToggleCDGlowShader @0x7C1280
//   InitCDGlowShader  @0x7C12A0
//   SetupCDGlowShader @0x7C1370
//   GlowSetTaps @0x7C13E0
//   LerpTaps @0x7C1430
// ============================================================================
#include "cdGlowShader.h"

#include <intrin.h>

// ============================================================================
// Globals
// ============================================================================
gpuVertexFormat gGlowVertexFormat;     // @0x10DDE90
FilterTaps GlowFilters[4];             // @0x10DDEA0
nglTexture* GlowTargets[2];            // ?GlowTargets@@3PAPAUnglTexture@@A
unsigned int GlowTargetPrev;           // ?GlowTargetPrev@@3IA

// ============================================================================
// ToggleCDGlowShader — flip the glow enable flag.
// ea: 0x7C1280
// ============================================================================
void ToggleCDGlowShader() {
    ShaderCommon::gGlowEnable = (ShaderCommon::gGlowEnable == 0);
}

// ============================================================================
// InitCDGlowShader — allocate the shader, register the glow shaders.
// ea: 0x7C12A0
// ============================================================================
void InitCDGlowShader() {
    cdGlowShader* v0 = (cdGlowShader*)mem_heap_malloc(0x10);
    if (v0 != NULL) {
        v0->next = tlInitList::head;
        tlInitList::head = v0;
        v0->Disabled = false;
        // vftable = cdGlowShader
        ShaderCommon::ShaderSwitching.__s0[0] &= ~1;
    } else {
        v0 = NULL;
    }
    gCDGlowShader = v0;
    nglDxRegisterVShader((unsigned int*)cdGlowRender1::VS, cdGlowRender1::VShaderTable[0]);
    cdGlowRender1::Shader = cdGlowRender1::VS[0];
    nglDxRegisterVShader((unsigned int*)cdGlowRender4::VS, cdGlowRender4::VShaderTable[0]);
    cdGlowRender4::Shader = cdGlowRender4::VS[0];
    nglDxRegisterPShader((unsigned int**)cdGlowShrink::PS, cdGlowShrink::PShaderTable[0]);
    cdGlowShrink::Shader = cdGlowShrink::PS[0];
    nglDxRegisterPShader((unsigned int**)cdGlowBlur::PS, cdGlowBlur::PShaderTable[0]);
    cdGlowBlur::Shader = cdGlowBlur::PS[0];
    nglDxRegisterPShader((unsigned int**)cdGlowApply::PS, cdGlowApply::PShaderTable[0]);
    cdGlowApply::Shader = cdGlowApply::PS[0];
}

// ============================================================================
// SetupCDGlowShader — build the glow vertex format + render targets.
// ea: 0x7C1370
// ============================================================================
void SetupCDGlowShader() {
    gpuVertexFormat v1;
    gGlowVertexFormat = *gpuCreateVertexFormat(&v1, 0x2C, gGlowVertexElements);
    GlowTargets[0] = nglCreateTexture(0x11, 6, 256, 256, 1, 1);
    GlowTargets[1] = nglCreateTexture(0x11, 6, 256, 256, 1, 1);
}

// ============================================================================
// GlowSetTaps — store the four glow filter tap sets.
// ea: 0x7C13E0
// ============================================================================
void GlowSetTaps(const FilterTaps* t0, const FilterTaps* t1, const FilterTaps* t2,
                 const FilterTaps* t3) {
    GlowFilters[0] = *t0;
    GlowFilters[1] = *t1;
    GlowFilters[2] = *t2;
    GlowFilters[3] = *t3;
}

// ============================================================================
// LerpTaps — lerp two tap sets by t.
// ea: 0x7C1430
// ============================================================================
FilterTaps* LerpTaps(FilterTaps* result, const FilterTaps* Taps0, const FilterTaps* Taps1,
                     float t) {
    FilterTaps out;
    for (int i = 0; i < 8; ++i) {
        out.Taps[i] = (Taps0->Taps[i] * (1.0f - t)) + (Taps1->Taps[i] * t);
    }
    *result = out;
    return result;
}
