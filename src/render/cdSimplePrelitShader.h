// ============================================================================
// cdSimplePrelitShader.h — simple prelit shader (6 non-inline funcs).
// Source: source/cdSimplePrelitShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimplePrelitShader.o):
//   cdSimplePrelitShaderMat::ctor @0x7D58E0
//   InitCDSimplePrelitShader  @0x7D5970
//   ToggleCDSimplePrelitShader @0x7D59C0
//   cdSimplePrelitShader::Register @0x7D59E0
//   cdSimplePrelitShader::AddNode @0x7D5A10
//   cdSimplePrelitShaderNode::Render @0x7D5A80
// ============================================================================
#ifndef COD3_RENDER_CDSIMPLEPRELITSHADER_H
#define COD3_RENDER_CDSIMPLEPRELITSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdSimplePrelitShaderMat — simple prelit shader material (24 bytes)
// ============================================================================
struct cdSimplePrelitShaderMat : nglMaterial {
    nglTexture* mTexture;   // +0x10
    int         mCullMode;  // +0x14

    cdSimplePrelitShaderMat(nglTexture* iTexture);  // @0x7D58E0
};
static_assert(sizeof(cdSimplePrelitShaderMat) == 0x18, "cdSimplePrelitShaderMat size mismatch");

// ============================================================================
// cdSimplePrelitShaderNode — simple prelit shader render node (24 bytes)
// ============================================================================
struct cdSimplePrelitShaderNode : nglShaderNode {
    cdSimplePrelitShaderMat* mMaterial;  // +0x14

    void Render() override;  // @0x7D5A80
};
static_assert(sizeof(cdSimplePrelitShaderNode) == 0x18, "cdSimplePrelitShaderNode size mismatch");

// ============================================================================
// cdSimplePrelitShader — simple prelit shader (16 bytes)
// ============================================================================
class cdSimplePrelitShader : public nglShader {
public:
    virtual tlFixedString GetName(); // @0x7D5D90
    virtual void Register();  // @0x7D59E0
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7D5A10
};
static_assert(sizeof(cdSimplePrelitShader) == 0x10, "cdSimplePrelitShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdSimplePrelitShaderVertex.o)
// ============================================================================
namespace cdSimplePrelitRender {
    extern unsigned long* VS;                  // ?VS@cdSimplePrelitRender@@3PAKA
    extern unsigned int const** VShaderTable;   // ?VShaderTable@cdSimplePrelitRender@@3PAPBIA
}
namespace cdSimplePrelitPixel {
    extern unsigned long** PS;                 // ?PS@cdSimplePrelitPixel@@3PAPAKA
    extern unsigned int const** PShaderTable;   // ?PShaderTable@cdSimplePrelitPixel@@3PAPBIA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdSimplePrelitShader* gCDSimplePrelitShader;  // @0x10DE57C

void InitCDSimplePrelitShader();  // @0x7D5970
void ToggleCDSimplePrelitShader();                 // @0x7D59C0

#endif // COD3_RENDER_CDSIMPLEPRELITSHADER_H
