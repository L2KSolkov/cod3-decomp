// ============================================================================
// cdGlowShader.h — glow shader (7 non-inline funcs).
// Source: source/cdGlowShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdGlowShader.o):
//   ToggleCDGlowShader @0x7C1280
//   InitCDGlowShader  @0x7C12A0
//   SetupCDGlowShader @0x7C1370
//   GlowSetTaps @0x7C13E0
//   LerpTaps @0x7C1430
//   GlowFilter @0x7C1540
//   GlowCallback @0x7C1900
// ============================================================================
#ifndef COD3_RENDER_CDGLOWSHADER_H
#define COD3_RENDER_CDGLOWSHADER_H

#include "cdSimpleColorShader.h"
#include "ngl/ngl_dx_gpu.h"  // gpuVertexFormat, gpuCreateVertexFormat

// ============================================================================
// FilterTaps — glow filter tap weights (32 bytes)
// ============================================================================
struct FilterTaps {
    float Taps[8];  // +0x00
};
static_assert(sizeof(FilterTaps) == 0x20, "FilterTaps size mismatch");

// ============================================================================
// cdGlowShader — glow shader (16 bytes)
// ============================================================================
class cdGlowShader : public nglShader {
public:
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdGlowShader) == 0x10, "cdGlowShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdGlowShaderVertex.o)
// ============================================================================
namespace cdGlowRender1 {
    extern unsigned long* VS;                // ?VS@cdGlowRender1@@3PAKA
    extern unsigned int const** VShaderTable; // ?VShaderTable@cdGlowRender1@@3PAPBIA
    extern unsigned long Shader;             // ?Shader@cdGlowRender1@@3KA
}
namespace cdGlowRender4 {
    extern unsigned long* VS;                // ?VS@cdGlowRender4@@3PAKA
    extern unsigned int const** VShaderTable; // ?VShaderTable@cdGlowRender4@@3PAPBIA
    extern unsigned long Shader;             // ?Shader@cdGlowRender4@@3KA
}
namespace cdGlowShrink {
    extern unsigned long** PS;               // ?PS@cdGlowShrink@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdGlowShrink@@3PAPBIA
    extern unsigned long* Shader;            // ?Shader@cdGlowShrink@@3PAKA
}
namespace cdGlowBlur {
    extern unsigned long** PS;               // ?PS@cdGlowBlur@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdGlowBlur@@3PAPBIA
    extern unsigned long* Shader;            // ?Shader@cdGlowBlur@@3PAKA
}
namespace cdGlowApply {
    extern unsigned long** PS;               // ?PS@cdGlowApply@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdGlowApply@@3PAPBIA
    extern unsigned long* Shader;            // ?Shader@cdGlowApply@@3PAKA
}

// ============================================================================
// Globals (owned by this object / ShaderCommon)
// ============================================================================
namespace ShaderCommon {
    extern int gGlowEnable;  // ?gGlowEnable@ShaderCommon@@3HA
}
extern gpuVertexFormat gGlowVertexFormat;  // @0x10DDE90
extern FilterTaps GlowFilters[4];          // @0x10DDEA0 (GlowFilters + 3 more FilterTaps)
extern nglTexture* GlowTargets[2];         // ?GlowTargets@@3PAPAUnglTexture@@A
extern unsigned int GlowTargetPrev;        // ?GlowTargetPrev@@3IA
extern const _D3DVERTEXSHADERINPUT gGlowVertexElements[];  // @0xE3BB78

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned long** PS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern nglTexture* nglCreateTexture(unsigned int Flags, unsigned int Format, int Width, int Height,
                                    int Depth, int Levels);  // ngl_gpu_texture.o

extern cdGlowShader* gCDGlowShader;  // @0x10DDF2C

void ToggleCDGlowShader();   // @0x7C1280
void InitCDGlowShader();     // @0x7C12A0
void SetupCDGlowShader();    // @0x7C1370
void GlowSetTaps(const FilterTaps* t0, const FilterTaps* t1, const FilterTaps* t2, const FilterTaps* t3);  // @0x7C13E0
FilterTaps* LerpTaps(FilterTaps* result, const FilterTaps* Taps0, const FilterTaps* Taps1, float t);  // @0x7C1430

#endif // COD3_RENDER_CDGLOWSHADER_H
