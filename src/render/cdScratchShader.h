// ============================================================================
// cdScratchShader.h — scratch shader (6 non-inline funcs).
// Source: source/cdScratchShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdScratchShader.o):
//   cdScratchMaterial::ctor @0x7C5660
//   ToggleCDScratchShader @0x7C5690
//   cdScratchShader::Register @0x7C56B0
//   InitCDScratchShader  @0x7C56E0
//   cdScratchShader::AddNode @0x7C5720
//   cdScratchShaderNode::Render @0x7C57F0
// ============================================================================
#ifndef COD3_RENDER_CDSCRATCHSHADER_H
#define COD3_RENDER_CDSCRATCHSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdScratchMaterial — scratch material (32 bytes)
// ============================================================================
struct cdScratchMaterial : nglMaterial {
    nglTexture*   Texture;   // +0x10
    unsigned int  BlendMode; // +0x14
    int           MapFlags;  // +0x18
    bool          HeatHaze;  // +0x1C

    cdScratchMaterial(nglTexture* tex, unsigned int BlendMode, int mapflags, bool HeatHaze);  // @0x7C5660
};
static_assert(sizeof(cdScratchMaterial) == 0x20, "cdScratchMaterial size mismatch");

// ============================================================================
// cdScratchShaderNode — scratch shader render node (24 bytes)
// ============================================================================
struct cdScratchShaderNode : nglShaderNode {
    cdScratchMaterial* Material;  // +0x14

    void Render() override;        // @0x7C57F0
};
static_assert(sizeof(cdScratchShaderNode) == 0x18, "cdScratchShaderNode size mismatch");

// ============================================================================
// cdScratchShader — scratch shader (16 bytes)
// ============================================================================
class cdScratchShader : public nglShader {
public:
    virtual tlFixedString GetName(); // @0x7C5E40
    virtual void Register();  // @0x7C56B0
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7C5720
};
static_assert(sizeof(cdScratchShader) == 0x10, "cdScratchShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdScratchShaderVertex.o)
// ============================================================================
namespace cdScratchShaderVertex {
    extern unsigned long* VS;                // ?VS@cdScratchShaderVertex@@3PAKA
    extern unsigned int const** VShaderTable; // ?VShaderTable@cdScratchShaderVertex@@3PAPBIA
}
namespace cdScratchShaderPixel {
    extern unsigned long** PS;               // ?PS@cdScratchShaderPixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdScratchShaderPixel@@3PAPBIA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdScratchShader* gCDScratchShader;  // @0x10DE02C

void InitCDScratchShader();   // @0x7C56E0
void ToggleCDScratchShader(); // @0x7C5690

#endif // COD3_RENDER_CDSCRATCHSHADER_H
