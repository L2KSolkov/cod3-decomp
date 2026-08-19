// ============================================================================
// cdSimpleAlphaShader.h — simple alpha shader (6 non-inline funcs).
// Source: source/cdSimpleAlphaShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleAlphaShader.o):
//   cdSimpleAlphaShaderMat::ctor @0x7C7E10
//   InitCDSimpleAlphaShader  @0x7C7EA0
//   ToggleCDSimpleAlphaShader @0x7C7EF0
//   cdSimpleAlphaShader::Register @0x7C7F10
//   cdSimpleAlphaShader::AddNode @0x7C7F30
//   cdSimpleAlphaShaderNode::Render @0x7C8220
// ============================================================================
#ifndef COD3_RENDER_CDSIMPLEALPHASHADER_H
#define COD3_RENDER_CDSIMPLEALPHASHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdSimpleAlphaShaderMat — simple alpha shader material (72 bytes)
// ============================================================================
struct cdSimpleAlphaShaderMat : nglMaterial {
    nglTexture* mTexture;      // +0x10
    int         mCullMode;     // +0x14
    int         mBlendMode;    // +0x18
    int         mFlags;        // +0x1C
    float       mTint[3];      // +0x20
    float       mAlpha;        // +0x2C
    float       mAlphaCutoff;  // +0x30
    float       mPulseRate;    // +0x34
    float       mMinTint;      // +0x38
    float       mMaxTint;      // +0x3C
    float       mMinAlpha;     // +0x40
    float       mMaxAlpha;     // +0x44

    cdSimpleAlphaShaderMat(nglTexture* iTexture);  // @0x7C7E10
};
static_assert(sizeof(cdSimpleAlphaShaderMat) == 0x48, "cdSimpleAlphaShaderMat size mismatch");

// ============================================================================
// cdSimpleAlphaShaderNode — simple alpha shader render node (24 bytes)
// ============================================================================
struct cdSimpleAlphaShaderNode : nglShaderNode {
    cdSimpleAlphaShaderMat* mMaterial;  // +0x14
};
static_assert(sizeof(cdSimpleAlphaShaderNode) == 0x18, "cdSimpleAlphaShaderNode size mismatch");

// ============================================================================
// cdSimpleAlphaShader — simple alpha shader (16 bytes)
// ============================================================================
class cdSimpleAlphaShader : public nglShader {
public:
    virtual tlFixedString GetName(); // @0x7C8D60
    virtual void Register();  // @0x7C7F10
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7C7F30
};
static_assert(sizeof(cdSimpleAlphaShader) == 0x10, "cdSimpleAlphaShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdSimpleAlphaShaderVertex.o)
// ============================================================================
namespace cdSimpleAlphaRender {
    extern unsigned long VS[2];               // ?VS@cdSimpleAlphaRender@@3PAKA
    extern unsigned int const* VShaderTable[2];  // ?VShaderTable@cdSimpleAlphaRender@@3PAPBIA
}
namespace cdSimpleAlphaPixel {
    extern unsigned long* PS[2];               // ?PS@cdSimpleAlphaPixel@@3PAPAKA
    extern unsigned int const* PShaderTable[2]; // ?PShaderTable@cdSimpleAlphaPixel@@3PAPBIA
}
namespace cdSimpleAlphaPixel_Fullbright {
    extern unsigned long* PS[2];               // ?PS@cdSimpleAlphaPixel_Fullbright@@3PAPAKA
    extern unsigned int const* PShaderTable[2]; // ?PShaderTable@cdSimpleAlphaPixel_Fullbright@@3PAPBIA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdSimpleAlphaShader* gCDSimpleAlphaShader;  // @0x10DE058

void InitCDSimpleAlphaShader();  // @0x7C7EA0
void ToggleCDSimpleAlphaShader();                // @0x7C7EF0

#endif // COD3_RENDER_CDSIMPLEALPHASHADER_H
