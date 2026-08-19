// ============================================================================
// cdBackgroundShader.h — background shader (6 non-inline funcs).
// Source: source/cdBackgroundShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdBackgroundShader.o):
//   cdBackgroundShaderMat::ctor @0x7E0520
//   InitCDBackgroundShader  @0x7E05B0
//   ToggleCDBackgroundShader @0x7E0600
//   cdBackgroundShader::Register @0x7E0620
//   cdBackgroundShader::AddNode @0x7E0680
//   cdBackgroundShaderNode::Render @0x7E06F0
// ============================================================================
#ifndef COD3_RENDER_CDBACKGROUNDSHADER_H
#define COD3_RENDER_CDBACKGROUNDSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdBackgroundShaderMat — background shader material (20 bytes)
// ============================================================================
struct cdBackgroundShaderMat : nglMaterial {
    nglTexture* mTexture;  // +0x10

    cdBackgroundShaderMat(nglTexture* iTexture);  // @0x7E0520
};
static_assert(sizeof(cdBackgroundShaderMat) == 0x14, "cdBackgroundShaderMat size mismatch");

// ============================================================================
// cdBackgroundShaderNode — background shader render node (24 bytes)
// ============================================================================
struct cdBackgroundShaderNode : nglShaderNode {
    cdBackgroundShaderMat* mMaterial;  // +0x14

    void Render() override;              // @0x7E06F0
};
static_assert(sizeof(cdBackgroundShaderNode) == 0x18, "cdBackgroundShaderNode size mismatch");

// ============================================================================
// cdBackgroundShader — background shader (16 bytes)
// ============================================================================
class cdBackgroundShader : public nglShader {
public:
    virtual void Register();  // @0x7E0620
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7E0680
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdBackgroundShader) == 0x10, "cdBackgroundShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdBackgroundShaderVertex.o)
// ============================================================================
namespace cdBackgroundRender {
    extern unsigned long* VS;                // ?VS@cdBackgroundRender@@3PAKA
    extern unsigned int const** VShaderTable; // ?VShaderTable@cdBackgroundRender@@3PAPBIA
    extern unsigned long Shader;             // ?Shader@cdBackgroundRender@@3KA
}
namespace cdBackgroundPixel {
    extern unsigned long** PS;               // ?PS@cdBackgroundPixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdBackgroundPixel@@3PAPBIA
    extern unsigned long* Shader;            // ?Shader@cdBackgroundPixel@@3PAKA
}
namespace cdBackgroundFullbrightPixel {
    extern unsigned long** PS;               // ?PS@cdBackgroundFullbrightPixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdBackgroundFullbrightPixel@@3PAPBIA
    extern unsigned long* Shader;            // ?Shader@cdBackgroundFullbrightPixel@@3PAKA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdBackgroundShader* gCDBackgroundShader;    // @0x10DE5F4
extern cdBackgroundShader* g_cdBackgroundShader;   // @0x10DE5F0

void InitCDBackgroundShader();  // @0x7E05B0
void ToggleCDBackgroundShader();               // @0x7E0600

#endif // COD3_RENDER_CDBACKGROUNDSHADER_H
