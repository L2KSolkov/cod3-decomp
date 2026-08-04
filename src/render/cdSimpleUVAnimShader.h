// ============================================================================
// cdSimpleUVAnimShader.h — simple UV anim shader (7 non-inline funcs).
// Source: source/cdSimpleUVAnimShader.cpp (render_xboxr)
// Verified against IDA (render_xboxr:cdSimpleUVAnimShader.o):
//   cdSimpleUVAnimShaderMat::ctor @0x7C6D70
//   InitCDSimpleUVAnimShader  @0x7C6E00
//   ToggleCDSimpleUVAnimShader @0x7C6E50
//   cdSimpleUVAnimShader::Register @0x7C6E70
//   cdSimpleUVAnimShader::AddNode @0x7C6ED0
//   cdSimpleUVAnimShaderNode::SetTextureMatrix @0x7C6F40
//   cdSimpleUVAnimShaderNode::Render @0x7C70E0
// ============================================================================
#ifndef COD3_RENDER_CDSIMPLEUVANIMSHADER_H
#define COD3_RENDER_CDSIMPLEUVANIMSHADER_H

#include "cdSimpleColorShader.h"

// ============================================================================
// cdSimpleUVAnimShaderMat — simple UV anim shader material (96 bytes)
// ============================================================================
struct cdSimpleUVAnimShaderMat : nglMaterial {
    nglTexture* mTexture;   // +0x10
    int         mCullMode;  // +0x14
    // +0x18: padding to 0x20
    math::Mat44 mMatrix;    // +0x20

    cdSimpleUVAnimShaderMat(nglTexture* iTexture);  // @0x7C6D70
};
static_assert(sizeof(cdSimpleUVAnimShaderMat) == 0x60, "cdSimpleUVAnimShaderMat size mismatch");

// ============================================================================
// cdSimpleUVAnimShaderNode — simple UV anim shader render node (96 bytes)
// ============================================================================
struct cdSimpleUVAnimShaderNode : nglShaderNode {
    cdSimpleUVAnimShaderMat* mMaterial;     // +0x14
    // +0x18: padding to 0x20
    math::Mat44 mTextureMatrix;             // +0x20
};
static_assert(sizeof(cdSimpleUVAnimShaderNode) == 0x60, "cdSimpleUVAnimShaderNode size mismatch");

// ============================================================================
// cdSimpleUVAnimShader — simple UV anim shader (16 bytes)
// ============================================================================
class cdSimpleUVAnimShader : public nglShader {
public:
    virtual void Register();  // @0x7C6E70
    virtual void AddNode(nglMeshNode* iMeshNode, nglMeshSection* iSection, nglMaterial* iMat);  // @0x7C6ED0
    virtual void GetName(tlFixedString& name);
    virtual void AddNodeFlags(unsigned int flags);
};
static_assert(sizeof(cdSimpleUVAnimShader) == 0x10, "cdSimpleUVAnimShader size mismatch");

// ============================================================================
// Shader data externs (defined in render_xboxr:cdSimpleUVAnimShaderVertex.o)
// ============================================================================
namespace cdSimpleUVAnimRender {
    extern unsigned long* VS;                // ?VS@cdSimpleUVAnimRender@@3PAKA
    extern unsigned int const** VShaderTable; // ?VShaderTable@cdSimpleUVAnimRender@@3PAPBIA
    extern unsigned long Shader;             // ?Shader@cdSimpleUVAnimRender@@3KA
}
namespace cdSimpleUVAnimPixel {
    extern unsigned long** PS;               // ?PS@cdSimpleUVAnimPixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdSimpleUVAnimPixel@@3PAPBIA
    extern unsigned long* Shader;            // ?Shader@cdSimpleUVAnimPixel@@3PAKA
}
namespace cdSimpleUVAnimFullbrightPixel {
    extern unsigned long** PS;               // ?PS@cdSimpleUVAnimFullbrightPixel@@3PAPAKA
    extern unsigned int const** PShaderTable; // ?PShaderTable@cdSimpleUVAnimFullbrightPixel@@3PAPBIA
    extern unsigned long* Shader;            // ?Shader@cdSimpleUVAnimFullbrightPixel@@3PAKA
}

// ============================================================================
// Externs
// ============================================================================
extern void nglDxRegisterVShader(unsigned long* VS, const unsigned int* Microcode);  // ngl_dx_shader.o
extern void nglDxRegisterPShader(unsigned long** PS, const unsigned int* Microcode);  // ngl_dx_shader.o

extern cdSimpleUVAnimShader* gCDSimpleUVAnimShader;  // @0x10DE044

cdSimpleUVAnimShader* InitCDSimpleUVAnimShader();  // @0x7C6E00
char ToggleCDSimpleUVAnimShader();                 // @0x7C6E50

#endif // COD3_RENDER_CDSIMPLEUVANIMSHADER_H
